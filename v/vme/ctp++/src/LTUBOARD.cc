#include "LTUBOARD.h"
//---------------------------------------------------------------------
LTUBOARD::LTUBOARD(string const name,w32 const boardbase,int vsp)
:
BOARD(name,boardbase,vsp,1),
ltuname(""),
NL1dat(108),NL2dat(149),
STANDALONE_MODE(0x534)
{
 ltuname=d_name+int2char((boardbase/0x1000) % 0x810);
 this->AddSSMmode("inmon",0);
 //cout << ltuname << endl;
}
void LTUBOARD::Print()
{
 printf("%s:",ltuname.c_str());
 printboardinfo("");
}
void LTUBOARD::ClearQueues()
{
 for(w32 i=0;i<qorbit.size();i++) delete qorbit[i];
 qorbit.clear();
 for(w32 i=0;i<ql1data.size();i++) delete ql1data[i];
 ql1data.clear();
 for(w32 i=0;i<ql2data.size();i++) delete ql2data[i];
 ql2data.clear();
 for(w32 i=0;i<qttcb.size();i++) delete qttcb[i];
 qttcb.clear();
 for(w32 i=0;i<qttcL1.size();i++) delete qttcL1[i];
 qttcL1.clear();
 for(w32 i=0;i<qttcL2.size();i++) delete qttcL2[i];
 qttcL2.clear();
 ql0strobe.clear();
 ql1strobe.clear();
 ql2strobe.clear();
 qorbit0.clear();
}
int LTUBOARD::longsignal(w32 &lsigflag,w32 bit,w32 issm,w32 &icount)
/*
 * Long signal in ssm canal
 */
{
 // char text[256];
 if(lsigflag){
    if(bit) icount=icount+1;
    else{
         //printf("Orbit %i \n",issm);
	 ssmrecord *orb = new ssmrecord(issm-icount,icount);
	 if(icount != 39){
          printf("Warning: Longsignal: ORBIT length != 39 instead %i at bc= %i\n",icount,issm);
         }
         qorbit.push_back(orb);
	 icount=0;
	 lsigflag=0;
	 //COUNT[canal]++;      
    }		     
 }else if(bit) lsigflag=1;  
 return 0;
}
int LTUBOARD::shortsignal(w32 level,w32 bit,w32 issm)
{
 if(bit){
   //printf("issm= %i \n",issm);
   ql0strobe.push_back(issm);
 }
 return 0;	
}
int LTUBOARD::activesignal(w32 level,w32 &asigflag,w32 bit,w32 issm,w32 &icount)
{
 //printf("activesignale: %i \n",issm);
 if(bit){
  if(!asigflag){
     if(level==1) ql1strobe.push_back(issm);
     else ql2strobe.push_back(issm);
     asigflag=1;
     }else{
      printf("Error: L%1i arrives while data active. BC=%i \n",level,issm);
      return 1;
  }		
 }
 return 0;
}
int LTUBOARD::lxdata(w32 NLxdat,w32 &l2sflag,w32 bit,w32 issm,w32 &icount,w32* data)
{
  if(l2sflag){
            data[icount]=bit;
            icount++;
            if(icount == NLxdat){
             //lxprint(i,0,NLxdat,LxDATA,name);
             w16* lxdd=new w16[NLxdat];
             for(w32 i=0;i<NLxdat;i++)lxdd[i]=data[i];
             ssmrecord *lxd=new ssmrecord(issm-NLxdat+1,lxdd,NLxdat);
	     if(NLxdat==NL2dat)ql2data.push_back(lxd);
             else ql1data.push_back(lxd);
	     icount=0;
             l2sflag=0;
	    }	    
           }else{
	    if(bit){
             // error can happen at the beginning  of ssm
             char level='1';
             if(NLxdat==NL2dat){
               level='2';
               if(issm<=NL2dat) return 0;
             }
             if(issm<=NL1dat) return 0;
             printf("Error l%c data: outside strobe i.e.: shifted or longer, BC= %i\n",level,issm);		                 
	     return 1;	     
	    }	    
           }		
 return 0;
}
void LTUBOARD::txprint(int i,w32 *TXS)
{
 int k;
 //char *ttcadl[]={"ZERO","L1h","L1d","L2h","L2d","L2r ","RoIh","RoId"};
 int ttcadd,e,code,data,chck;
 ttcadd=0;
 for(k=0;k<14;k++)ttcadd=ttcadd+(1<<k)*TXS[14-k];
 e=TXS[15];
 code=0;
 for(k=0;k<4;k++)code=code+(1<<k)*TXS[20-k];
 data=0;
 for(k=0;k<12;k++)data=data+(1<<k)*TXS[32-k];
 chck=0;
 for(k=0;k<8;k++)chck=chck+(1<<k)*TXS[39-k];
 if(code > 7){
  printf("Error: txprint: unexpected code= 0x%x , data=0x%x bc= %i\n",code,data,i);
  //exit(2);
  return;
 }
 ssmrecord *ss = new ssmrecord(i-39,code,e,ttcadd,data,chck);
 qttcb.push_back(ss);
}
int LTUBOARD::AnalSSM()
/*
 Compares L1 and L2data serial data with TTC messages
*/
{
 CreateRecordSSM();
 CheckLx(1);
 CheckLx(2);
 return 0;
}
w32 LTUBOARD::GetSSMBC(w32 issm)
{
 w32 j=0;
 while(j<qorbit.size() && (issm-qorbit[j]->issm)>=3564)j++;
 if(j==qorbit.size()){
   printf("Error: orbit not found for bc=%i \n",issm);
   return 0;
 }else{
   return issm - qorbit[j]->issm+1;
 }
}
int LTUBOARD::CheckOrbits()
{
 for(w32 i=1;i<qorbit.size();i++){
  if((qorbit[i]->issm - qorbit[i-1]->issm) != 3564u){
    printf("Wrong distance between orbits %i \n",qorbit[i]->issm - qorbit[i-1]->issm );
  }
 }
 return 0;
}
int LTUBOARD::AnalTotalSSM2()
{
 CreateRecordSSM();
 CheckOrbits();
 CreateTTCL12();
 if(ql0strobe.size()==0){
   printf("No L0 in SSM \n");
   return 0;
 }
 // Constants
 w32 L0L1=260;
 w32 L0L2=4150;  // LTU
 //w32 L0L2=4260;   // L2_DELAY=4318 gives L0L2 4260
 //int L2dSSMOffset=92; // Offset between bc from SSM and from L2data
 //int L2dSSMOffset=44; // LTU
 int L2dSSMOffset;
 //if(GetStatus())L2dSSMOffset=92;
 if(GetStatus())L2dSSMOffset=3561;
 else L2dSSMOffset=44; 
 // Ques sizes
 w32 l0size=ql0strobe.size();
 w32 l1size=ql1strobe.size();
 w32 l1dize=ql1data.size();
 w32 l2size=ql2strobe.size();
 w32 l2dize=ql2data.size();
 w32 l2ttcsize=qttcL2.size();
 // First L0 after orbit, use 2nd orbit since first can be incomplete
 w32 l0first=0;
 while(l0first < l0size && (ql0strobe[l0first] < qorbit[1]->issm))l0first++;
 if(l0first==l0size){
  printf("L0 strobe not found \n");
  ierror++;
  return 1;
 }else{
  printf("First orbit at BC %i, first L0 after orbit BC= %i index= %i \n",qorbit[0]->issm,ql0strobe[l0first],l0first);
 }
 // First L1 after first L0
 w32 l1first=0;
 while(l1first<l1size && (ql1strobe[l1first] < ql0strobe[l0first]))l1first++;
 if(l1first==l1size){
  printf("L1 strobe not found \n");
  ierror++;
  return 1;
 }else{
   printf("First L1 after first L0 at BC= %i index= %i \n",ql1strobe[l1first],l1first);
 }
 // First L2 after first L0
 w32 l2first=0;
 while(l2first < l2size && (ql2strobe[l2first] - ql0strobe[l0first]) != L0L2){
     //printf("%i \n",ql2strobe[l2first] - ql0strobe[l0first]);
     l2first++;
 }
 if(l2first==l2size){
  printf("Error: First L2 strobe not found. \n");
  ierror++;
  return 1;
 }else{
   printf("First L2 after first L0 at BC= %i index= %i \n",ql2strobe[l2first],l2first);
 }
 // L1data corresponding to first L1 strobe
 w32 l1dfirst=0;
 while(l1dfirst<l1dize && (ql1data[l1dfirst]->issm != ql1strobe[l1first]))l1dfirst++;
 if(l1dfirst==l1dize){
  printf("Error: First L1 data not found. \n");
  ierror++;
  return 1;
 }else{
  printf("First L1 data at index %i \n",l1dfirst);
 }
 // L2data corresponding to first L2 strobe
 w16 bc1,orbit11,orbit21;
 w32 l2dfirst=0;
 while(l2dfirst<l2dize && (ql2data[l2dfirst]->issm != ql2strobe[l2first]))l2dfirst++;
 if(l2dfirst==l2dize){
  printf("Error: First L2 data not found. \n");
  ierror++;
  return 1;
 }else{
  printf("First L2 data at index %i \n",l2dfirst);
  w16 dwords2[NL2words];
  L2Serial2Words(l2dfirst,dwords2);
  bc1 = dwords2[0];
  orbit11 = dwords2[1];
  orbit21 = dwords2[2];
 }
 // Find first L2 ttc message
 w32 l2ttcfirst=l2ttcsize;
 for(w32 i=0;i < l2ttcsize; i++){
  w16* data=qttcL2[i]->sdata;
  if(bc1==data[0] && orbit11==data[1] && orbit21==data[2]){
    l2ttcfirst=i;
    break;
  }
 }
 if(l2ttcfirst==l2ttcsize){
   printf("Error: L2 TTC message not found 0x%x 0x%x%x\n",bc1,orbit11,orbit21);
   ierror++;
   return 1;
 }else{
   printf("L2 TTC message found at index %i \n",l2ttcfirst);
 }
 //
 w32 il0=l0first;
 w32 il1=l1first;
 w32 il2=l2first;
 w32 il1d=l1dfirst;
 w32 il2d=l2dfirst;
 w32 il2t=l2ttcfirst;
 while(il0<l0size && il1<l1size && il2<l2size && il1d<l1dize && il2d<l2dize && il2t < l2ttcsize){
  w32 l0bc = ql0strobe[il0];
  w32 l1bc = ql1strobe[il1];
  w32 l2bc = ql2strobe[il2];
  if((l1bc-l0bc) != L0L1){
    printf("ErrorL L0L1 time violation at L0 BC= %i \n",l0bc);
    ierror++;
    return 1;
  }
  if((l2bc-l0bc) != L0L2){
    printf("ErrorL L0L2 time violation at L0 BC= %i \n",l0bc);
    ierror++;
    return 1;
  }
  w32 bcissm = GetSSMBC(l0bc);
  // L1data
  // ...
  // L2data
  w16 dwords2[NL2words];
  L2Serial2Words(il2d,dwords2);
  w32 bcl2da = dwords2[0];
  int offset = bcl2da-bcissm;
  //printf("bcissm= %i bcl2da= %i offset= %i \n",bcissm,bcl2da,offset);
  if(offset<0) offset = offset+3564;
  if(offset != L2dSSMOffset){
    printf("bcissm= %i bcl2da= %i offset= %i \n",bcissm,bcl2da,offset);
    printf("Error for L2data L2 issm=%i (l0 issm=%i), offset %i instead of %i \n",l2bc,l0bc,offset,L2dSSMOffset);
    ierror++;
  }
  //w32 orbitl2=(dwords2[1]<<12)+dwords2[2];
  //CheckL2TTC(bcl2da,dwords2[1],dwords2[2],l2bc);
  //CheckL2TTC(dwords2,l2bc);
  CheckL2TTC(il2t,dwords2,l2bc);
  il0++;il1++,il2++;il1d++;il2d++;il2t++;
 }
 printf("# of L0 strobe: %i \n",l0size);
 printf("# of L1 strobe: %i \n",l1size);
 printf("# of L2 strobe: %i \n",l2size);
 printf("# of L1 data  : %i \n",l1dize);
 printf("# of L2 data  : %i \n",l2dize);
 printf("# of L2a TTC messages:  : %i \n",l2ttcsize);
 return 0;
}
int LTUBOARD::CreateTTCL12()
{
 bool l1active=0;
 bool l2active=0;
 w32 issm=0;
 w32 il1=0;
 w32 il2=0;
 w16* datal1=0; 
 w16* datal2=0; 
 //bool l1active=0;
 //
 // First L1 header
 w32 i=0;
 while(i<qttcb.size() && qttcb[i]->ttcode != 1)i++;
 w32 l1first=i;
 // First L2 header
 i=0;
 while(i<qttcb.size() && qttcb[i]->ttcode != 3)i++;
 w32 l2first=i;
 // L1 and L2
 i=0;
 while(i<qttcb.size()){
   w32 ttccode=qttcb[i]->ttcode;
   w32 ttcdata=qttcb[i]->tdata;
   //printf("issm=%i i=%i ttcode=%i L2active= %i\n",qttcb[i]->issm,i,ttccode,l2active);
   if(ttccode==1){
    if(i<l1first){ i++; continue;}
    if(l1active){
      printf("Error: TTC L1 header arrived too soon issm= %i\n",qttcb[i]->issm);
      ierror++;
    }else{
      l1active=1;
      issm=qttcb[i]->issm;
      datal1=new w16[NL1words];
      datal1[il1]=ttcdata;
      il1++;
    }
   }
   else if( ttccode == 3 ){
    if(i<l2first){ i++; continue;}
    if(l2active){
      printf("Error: TTC L2 header arrived too soon issm= %i\n",qttcb[i]->issm);
      ierror++;
    }else{
      l2active=1;
      issm=qttcb[i]->issm;
      datal2=new w16[NL2words];
      datal2[il2]=ttcdata;
      il2++;
    }
   }
   else if(ttccode == 2){
    if(i<l1first){i++; continue;}
    if(!l1active){
      printf("Error: TTC L1 word arrived without header issm=%i \n",qttcb[i]->issm);
    }else{
      if(il1<NL1words){
        datal1[il1]=ttcdata;
        il1++;
        if(il1==NL1words){
          l1active=0;
          il1=0;
          ssmrecord *l1d=new ssmrecord(issm,datal1,NL1words);
          qttcL1.push_back(l1d); 
        }
      }else{
        printf("Error: too many TTC L1 words issm=%i\n",qttcb[i]->issm);
      }
    }
   }
   else if( ttccode == 4 ){
    if(i<l2first){i++; continue;}
    if(!l2active){
      printf("Error: TTC L2 word arrived without header issm=%i \n",qttcb[i]->issm);
    }else{
      if(il2<NL2words){
        datal2[il2]=ttcdata;
        il2++;
        if(il2==NL2words){
          l2active=0;
          il2=0;
          ssmrecord *l2d=new ssmrecord(issm,datal2,NL2words);
          qttcL2.push_back(l2d); 
        }
      }else{
        printf("Error: too many TTC L2 words issm=%i \n",qttcb[i]->issm);
      }
    }
   }else {
     printf("Warning: unknown ttc code = %i i=%i l2first=%i %i\n",ttccode,i,l2first,l2active);
   }
   i++;
 }
 return 0;
}
// Compares bcid and orbit
// Searched whole que
int LTUBOARD::CheckL2TTC(w32 bcid,w32 orbit1,w32 orbit2,w32 issm)
{
 //printf("CheckTTC L2 BC= 0x%x ORB= 0x%x%x \n",bcid,orbit1,orbit2);
 w32 i=0;
 while(i<qttcL2.size()){
   w16* data=qttcL2[i]->sdata;
   if(data[0] == bcid && data[1]==orbit1 && data[2]==orbit2){
    //for(w32 k=0;k<NL2words;k++)printf("%x ",data[k]);
    //printf("\n");
    break;
   }
   i++;
 }
 if(i==qttcL2.size()){
   printf("Error: TTC L2a message not found  at issm=%i  \n",issm);
   printf("CheckTTC L2 BC= 0x%x ORB= 0x%x%x \n",bcid,orbit1,orbit2);
   ierror++;
 }
 return 0;
}
// May compare all data
// searched all que
int LTUBOARD::CheckL2TTC(w16* dataser,w32 issm)
{
 //printf("CheckTTC L2 BC= 0x%x ORB= 0x%x%x \n",bcid,orbit1,orbit2);
 w32 i=0;
 while(i<qttcL2.size()){
   w16* data=qttcL2[i]->sdata;
   if(data[0] == dataser[0] && data[1]==dataser[1] && data[2]==dataser[2]){
    //for(w32 k=0;k<NL2words;k++)printf("%x ",data[k]);
    //printf("\n");
    break;
   }
   i++;
 }
 if(i==qttcL2.size()){
   printf("Error: TTC L2a message not found  at issm=%i  \n",issm);
   printf("CheckTTC L2 BC= 0x%x ORB= 0x%x%x \n",dataser[0],dataser[1],dataser[2]);
   ierror++;
 }
 return 0;
}
// Compares only one message, so missing or extra is detected
int LTUBOARD::CheckL2TTC(w32 ittc,w16* dataser,w32 issm)
{
 //printf("CheckTTC L2 BC= 0x%x ORB= 0x%x%x \n",bcid,orbit1,orbit2);
 w16* data=qttcL2[ittc]->sdata;
 bool err=0;
 for(w32 i=0;i<NL2words;i++){
   err |= (dataser[i] != data[i]);
   if(err){
    printf("Error: TTC and serial data diff: i=%i ",i);
    for(w32 k=0;k<NL2words;k++)printf("%x %x :",dataser[k],data[k]);
    printf("\n");
    //printf("CheckTTC L2 BC= 0x%x ORB= 0x%x%x \n",dataser[0],dataser[1],dataser[2]);
    ierror++;
    return 1;
   }
 }
 return 0;
}
int LTUBOARD::AnalTotalSSM()
/*
 Full analysis - seraching in ques.
*/ 
{
 CreateRecordSSM();
 if(ql0strobe.size()==0){
   printf("No L0 in SSM \n");
   return 0;
 }
 printf("# of L0 strobe: %i \n",ql0strobe.size());
 printf("# of L1 strobe: %i \n",ql1strobe.size());
 printf("# of L2 strobe: %i \n",ql2strobe.size());
 printf("# of L1 data  : %i \n",ql1data.size());
 printf("# of L2 data  : %i \n",ql2data.size());
 // Counters
 w32 l0c=0,l1c=0,l2c=0,l1cd=0,l2cd=0;
 // indexs in the ques
 w32 jorbit=0;
 w32 jl1=0;
 w32 jl1d=0;
 w32 jl2=0;
 w32 jl2d=0;
 // First L0 after orbit
 w32 il0afterorbit=0;
 while(ql0strobe[il0afterorbit] < qorbit[0]->issm)il0afterorbit++;
 printf("First orbit at BC %i corresponding to L0 %i in que \n",qorbit[0]->issm,il0afterorbit);
 //
 for(w32 i=il0afterorbit;i<ql0strobe.size();i++){
  w32 il0=ql0strobe[i];
  l0c++;
  w32 il1=0;
  w32 il2=0;
  w32 bcidssm=0;
  printf("L0: %i \n", il0);
  // find ssm BC
  bool orbitfound=0;
  for(w32 j=jorbit;j<qorbit.size();j++){
    ssmrecord* orbit=qorbit[j];
    if((il0-orbit->issm)<3564){
      printf("Orbit:  l0bcid= 0x%x length=%i \n",il0-orbit->issm,orbit->data);
      jorbit=j;
      orbitfound=1;
      bcidssm=il0-orbit->issm;
      break;
    } 
  }
  if(!orbitfound){
    printf("Orbit not found for L0 at %i \n",il0);
    ierror++;
    return 1;
  }
  // Find L1
  w32 j=jl1;
  while((j<ql1strobe.size()) && (ql1strobe[j] < il0)){
    printf("Warning: L1 without L0 at index=%i issm=%i \n",j,ql1strobe[j]);
    j++;
  }
  if(j>=ql1strobe.size()){
    printf("L1 not found for L0 at issm=%i \n",il0);
    return 2;
  }else{
    printf("L1: %i  j=%i i=%i \n",ql1strobe[j]-il0, j,i);
    if((ql1strobe[j]-il0) != 260){
     printf("Error: L0-L1 time violation issm=%i \n",il0);
     return 3;
    }
    l1c++;
    il1=ql1strobe[j];
    jl1=++j;
  }
  // Find L1 data
  w16 dwords1[NL1words];
  j=jl1d;
  w32 l1dsize=ql1data.size();
  while((j<l1dsize) && (jl1<<l1dsize) && (ql1data[j]->issm < ql1strobe[jl1])){
    L1Serial2Words(j,dwords1);
    //for(w32 k=0;k<NL1words;k++)printf("%03x.",dwords1[k]); printf("j= %i\n",j);
    j++;
  }
  if(j>=l1dsize || jl1 >= l1dsize){
    //printf("l1c======%i \n",l1c);
    if(l1c != ql1strobe.size())
    {
      printf("Error: L1 data not found \n");
      printf("l0=%i l1=%i l2=%i l1d=%i l2d=%i \n",l0c,l1c,l2c,l1cd,l2cd);
      ierror++;
      return 3;
    }
  }else{
    l1cd++;
    jl1d=j;
    printf("First L1 data after L1 strobe: ");
    for(w32 k=0;k<NL1words;k++)printf("%03x ",dwords1[k]); printf("\n");
  }
  // Find L2 strobe
  /* This way it does not work because for T>3999 they can interleave
  j=jl2;
  while((j<ql2strobe.size()) &&  (ql2strobe[j] < il1)){
    printf("Warning: L2 without L1 at %i %i \n",j,ql2strobe[j]);
    j++;
  }
  */
  j=jl2;
  //while((j<ql2strobe.size()) &&  ((ql2strobe[j] - il0) != 4260)) j++;
  while((j<ql2strobe.size()) &&  ((ql2strobe[j] - il0) != 4150)) j++;
  if(j>=ql2strobe.size()){
    printf("Error: L2 strobe not found for L0 at %i , L1 at %i \n",il0,il1);
    ierror++;
    return 2;
  }else{
    l2c++;
    printf("L2 strobe: %i \n",ql2strobe[j]-il0);
    il2=ql1strobe[j];
    jl2=++j;
  }
  // Find L2 data
  w16 dwords2[NL2words];
  j=jl2d;
  w32 l2dsize=ql2data.size();
  //while( (j<l2dsize) && (jl2<l2dsize) && (ql2data[j]->issm < ql2strobe[jl2]))
  while(j<l2dsize){
    L2Serial2Words(j,dwords2);
    //for(w32 k=0;k<NL2words;k++)printf("%03x ",dwords2[k]); printf(" j=%i \n",j);
    if((dwords2[0]-bcidssm)== 45) break;
    j++;
  }
  //printf(" j= %i jl2=%i  l2datasize= %i\n",j,jl2,ql2data.size());
  if((j >= l2dsize) || (jl2 >= l2dsize)){
   if(l2c != l2dsize)
   {
    printf("Error: L2 data not found for L0 at %i , L1 at %i \n",il0,il2);
    printf("l0=%i l1=%i l2=%i l1d=%i l2d=%i \n",l0c,l1c,l2c,l1cd,l2cd);
    ierror++;
    return 2;
   }
  }else{
   l2cd++;
   printf("L2Data: ");
   for(w32 k=0;k<NL2words;k++)printf("%03x ",dwords2[k]); 
   printf(" Comp: 0x%x %i \n",bcidssm,dwords2[0] - bcidssm);
   jl2d=j;
  }
 }
 printf("l0=%i l1=%i l2=%i l1d=%i l2d=%i \n",l0c,l1c,l2c,l1cd,l2cd);
 return 0;
}
void LTUBOARD::L1Serial2Words(w32 i,w16* dwords)
{
 w32 NLxdat=NL1dat;
 w32 datstart=0;
 w16* sdata=ql1data[i]->sdata;
 // Serial data bits to words
 for(w32 j=0;j<NL1words;j++)dwords[j]=0;
 for(w32 j=datstart;j<NLxdat;j++){
   w32 j12=(j-datstart)/12;
   w32 jre=(j-datstart) % 12;
   dwords[j12]+=sdata[j]*(1<<(11-jre));
 }
}
void LTUBOARD::L2Serial2Words(w32 i,w16* dwords)
{
 w32 NLxdat=NL2dat;
 w32 datstart=1;
 w16* sdata=ql2data[i]->sdata;
 // Serial data bits to words
 for(w32 j=0;j<NL2words;j++)dwords[j]=0;
 for(w32 j=datstart;j<NLxdat;j++){
   w32 j12=(j-datstart)/12;
   w32 jre=(j-datstart) % 12;
   dwords[j12]+=sdata[j]*(1<<(11-jre));
 }
}
int LTUBOARD::CheckLx(int level){
// Analyses Lx data. No L1/L2 interleaving assumed.
// Requires to start from first L0
 w32 NLxwords;
 w8 ttchead;
 deque<ssmrecord *> qdat;
 if(level==2){
  NLxwords=NL2words;
  ttchead=3;
  qdat=ql2data;
 }else{
  NLxwords=NL1words;
  ttchead=1;
  qdat=ql1data;
 }
 ///printf("l2strobe: %i %i\n",ql2strobe.size(),ql2data.size());
 ///printf("l1strobe: %i %i\n",ql1strobe.size(),ql1data.size());
 ///printf("ttcb : %i\n",qttcb.size());
 w16 dwords[NLxwords];
 int count=0;
 for(w32 i=0;i<qdat.size();i++){
  // Serial data bits to words
  if(level==1) L1Serial2Words(i,dwords);
  else L2Serial2Words(i,dwords);
  //
  w32 issm = qdat[i]->issm;
  //printf("L2data: %i %i  0x%x 0x%llx\n",ql2strobe[i],issm,bcid,data2);
  //printf("%i \n",ql2strobe[i]-(ql2data[i])->issm);
  w32 j=0;
  while(j<qttcb.size()){
    //printf("test ttcb: %i 0x%x 0x%x \n",qttcb[j]->issm,qttcb[j]->ttcode);
    if((qttcb[j]->ttcode == ttchead) && (qttcb[j]->issm > issm)){
       //printf("ttcb: %i 0x%x \n",qttcb[j]->issm,qttcb[j]->tdata);
       if(qttcb[j]->tdata != dwords[0]){
        ierror++;
        printf("Check L%1i Error: serial data different from ttc: 0x%x 0x%x  BC=%i\n",level, dwords[0],qttcb[j]->tdata,issm);
       }
       w32 iword=1;
       while(j<qttcb.size() && iword<NLxwords){
         //printf("ttcode= %i \n",qttcb[j]->ttcode);
    	 if(qttcb[j]->ttcode == (ttchead+1)){
           //printf("ttcb: %i %i 0x%x 0x%x \n",iword,qttcb[j]->issm,qttcb[j]->tdata,dwords[iword]);
           if(qttcb[j]->tdata != dwords[iword]){
             ierror++;
             printf("Check L%1i Error: serial data different from ttc: 0x%x 0x%x  BC=%i \n",level,dwords[iword],qttcb[j]->tdata,issm);
	   }
	   iword++;
         }
	j++;
       }       
       break;
    }
    j++;
  }
  count++;  
 }
 printf("Number of checked L%1i: %i \n",level,count);
 return 0;
}
void LTUBOARD::FindOrbits()
{
 w32 *ss=GetSSM();
 w32 i=0;
 while(i<Mega){
   if(ss[i] & 1){
    qorbit0.push_back(i);
    i+=39;
   }
   i++;
 }
}
/*------------------------------------------------------------------------------
 * analyse only channel B because you dont know where to start
 * you have 42 options
 */
void LTUBOARD::analTTCB(){
 int i,ioff,word;
 int bit,bitb,bit0,bitb0;
 w32 *ss=GetSSM();
 w32 data[64];
 int active=0,icount=0;
 int datab=3;
 // Find begining using also TTC busy
 bool start=0;
 ioff=1;
 word=ss[0];
 bit0  = (word & (1<<12)) == (1<<12);
 bitb0 = (word & (1<<13)) == (1<<13);
 while(ioff<Mega && !start){
   word=ss[ioff];  
   bit  = (word & (1<<12)) == (1<<12);
   bitb = (word & (1<<13)) == (1<<13);
   start = (bit0==1) && (bitb0==0) && (bit==1) && (bitb==1);
   bit0=bit;
   bitb0=bitb;
   ioff++;
 }
 printf("Start of TTC data: %i \n",ioff);
 //
 for(i=ioff;i<Mega;i++){
   word=ss[i];
   bit= (word & (1<<12)) == (1<<12);
   if(active){
    if(icount == 0){
      if(bit == 0) datab = 0;  //orbit and pp
      else datab = 1;          // data
      icount=icount+1;
      continue; 
    }
    //printf("datab=%i \n",datab);
    if(icount == 14 && datab == 0){
      //txprintOP(i,data,name);
      //printf("Orbit \n");
      active=0;
      continue ;
    }
    if(icount == 41 && datab == 1){
      //printf("Data \n");
      txprint(i,data);
      active=0;
      continue ;
    }
    //if(*icount>25  && *icount<(25+8)) data=data+ (bit<<(*icount-26));
    data[icount]=bit;
    icount=icount+1;
   }else{
    if(bit == 0){
     active=1 ;
     icount=0;
    } 
   }
 }
}
int LTUBOARD::CreateRecordSSM()
/* Not assuming anything.
 * Starting from  1st L0
 */
{
 ClearQueues();
 w32 orbitflag=0,iorbit=0;
 w32 ls2flag=0,ils2=0,il2data=0;
 w32 ls1flag=0,ils1=0,il1data=0;
 w32 l1DATA[NL1dat],l2DATA[NL2dat];
 w32 bit;
 int word;
 w32 *ss=GetSSM();
 analTTCB();
 printf("analTTCB finished \n");
 for(int i=1;i<Mega;i++){
    word=ss[i];
    for(w32 j=0;j<18;j++){
    	bit= ( (word & (1<<j)) == (1<<j));
        switch(j){
	     case 0:  // ORBIT
		longsignal(orbitflag,bit,i,iorbit);
		break;
             case 2:  // L0 strobe
                shortsignal(0,bit,i);
                break;
             case 3:  // L1 strobe
		activesignal(1,ls1flag,bit,i,ils1);
                break;
	     case 4:  // L1 data
		lxdata(NL1dat,ls1flag,bit,i,il1data,l1DATA);
		break;
	     case 5:  // L2 Strobe
		activesignal(2,ls2flag,bit,i,ils2);
                break;
             case 6:  // L2data
		lxdata(NL2dat,ls2flag,bit,i,il2data,l2DATA);
		break;
	     case 12:  // TTCB
		// This is done in analTTCB
		//channelB(ttcbflag,bit,i,ittcb,datab,ttcbdata);
		break;
        }
    }
 }
 return 0;
}
