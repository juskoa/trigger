#include "LTUBOARD.h"
class ssmrecord
{
  public:
    ssmrecord(w32 issm,w32 data);
    ssmrecord(w32 issm,w8* data,w32 Ndata);
    ssmrecord(w32 issm,w8 ttcode,w8 e,w16 address, w16 tdata, w8 chck);
    ssmrecord(const ssmrecord &obj);
    ssmrecord& operator=(const ssmrecord& rec);
    ~ssmrecord();
    w32 issm; // position in ssm
    w32 data; // orbit
    w8 *sdata;
    w8 ttcode,e;
    w16 address,tdata;
    w8 chck;
  //string name;
};
ssmrecord::ssmrecord(w32 issm,w32 data)
:issm(issm),data(data),sdata(0),
ttcode(0),e(0),address(0),tdata(0),chck(0){}
ssmrecord::ssmrecord(w32 issm,w8 ttcode,w8 e,w16 address, w16 tdata, w8 chck)
:issm(issm),data(0),sdata(0),
ttcode(ttcode),e(e),address(address),tdata(tdata),chck(chck){}
ssmrecord::ssmrecord(w32 issm,w8* data,w32 Ndata)
:issm(issm),data(Ndata),sdata(data),
ttcode(0),e(0),address(0),tdata(0),chck(0){}
ssmrecord::~ssmrecord()
{
 if(sdata) delete sdata;
}
ssmrecord::ssmrecord(const ssmrecord &obj)
{
 issm=obj.issm;
 data=obj.data;
 sdata=obj.sdata;
 ttcode=obj.ttcode;
 e=obj.e;
 address=obj.address;
 tdata=obj.tdata;
 chck=obj.chck;
}
ssmrecord& ssmrecord::operator=(const ssmrecord& rec)
{
 if(this != &rec){
  issm=rec.issm;
  data=rec.data;
  sdata=rec.sdata;
  ttcode=rec.ttcode;
  e=rec.e;
  address=rec.address;
  tdata=rec.tdata;
  chck=rec.chck;
 }
 return *this;
}
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
 ql0strobe.clear();
 ql1strobe.clear();
 ql2strobe.clear();
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
         //sprintf(text,"%s/%i",name,*icount+1);
	 //dump=addlist(dump,i-*icount-1,text);
	 ssmrecord *orb = new ssmrecord(issm-icount,icount);
	 //ql2strobe.push(issm);
	 if(icount != 39){
          printf("Warning: Longsignal: ORBIT length != 39 instead %i \n",icount);
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
             w8* lxdd=new w8[NLxdat];
             for(w32 i=0;i<NLxdat;i++)lxdd[i]=data[i];
             ssmrecord *lxd=new ssmrecord(issm-NLxdat+1,lxdd,NLxdat);
	     if(NLxdat==NL2dat)ql2data.push_back(lxd);
             else ql1data.push_back(lxd);
	     icount=0;
             l2sflag=0;
	    }	    
           }else{
	    if(bit){
             // error
             //lxprint(i,1,NLxdat,LxDATA,name);
             char level='1';
             if(NLxdat==NL2dat)level='2';
             printf("Error l%c data: outside strobe i.e.: shifted or longer, BC= %i\n",level,issm);		                 
	     return 1;	     
	    }	    
           }		
 return 0;
}
int LTUBOARD::channelB(w32 &flag,w32 bit,w32 issm,w32 &icount,w32& datab,w32* data)
{
 //printf("%i %i %i %i\n",bit,issm,datab,flag);
 if(flag){
    if(icount == 0){
      if(bit == 0) datab = 0;  //orbit and pp
      else datab = 1;          // data
      icount=icount+1;
      return 0; 
    }
    //printf("datab=%i \n",datab);
    if(icount == 14 && datab == 0){
      //txprintOP(i,data,name);
      //printf("Orbit \n");
      flag=0;
      return 0 ;
    }
    if(icount == 41 && datab == 1){
      //printf("Data \n");
      txprint(issm,data);
      flag=0;
      return 0 ;
    }
    //if(*icount>25  && *icount<(25+8)) data=data+ (bit<<(*icount-26));
    data[icount]=bit;
    icount=icount+1;
   }else{
    if(bit == 0){
     flag=1 ;
     icount=0;
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
  printf("txprint: unexpected code 0x%x \n",code);
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
int LTUBOARD::AnalTotalSSM()
/*
 Full analysis like in detector fron end.
*/ 
{
 CreateRecordSSM(0);
 w32 jorbit=0;
 w32 jl1=0;
 for(w32 i=0;i<ql0strobe.size();i++){
  w32 il0=ql0strobe[i];
  w32 bcidssm=0;
  printf("L0: %i \n", il0);
  // find ssm BC
  bool orbitfound=0;
  for(w32 j=jorbit;j<qorbit.size();j++){
    ssmrecord* orbit=qorbit[j];
    if((il0-orbit->issm)<3564){
      printf("Orbit: %i %i \n",il0-orbit->issm,orbit->data);
      jorbit=j;
      orbitfound=1;
      bcidssm=il0-orbit->issm;
      break;
    } 
  }
  if(!orbitfound){
    printf("Orbit not found for L0 at %i \n",il0);
    return 1;
  }
  // Find L1
  w32 j=jl1;
  while((ql1strobe[j] < il0) && (j<ql1strobe.size())){
    printf("Warning: L1 without L0 at %i %i \n",j,ql1strobe[j]);
    j++;
  }
  if(j>=ql1strobe.size()){
    printf("L1 not found for L0 at %i \n",il0);
    return 2;
  }else{
    printf("L1: %i \n",ql1strobe[j]-il0);
    if((ql1strobe[j]-il0) != 260){
     printf("Error: L0-L1 time violation \n");
     return 3;
    }
    jl1=++j;
  }
 }
 return 0;
}
int LTUBOARD::CheckLx(int level){
// Analyses Lx data. No L1/L2 interleaving assumed.
// Requires to start from first L0
 w32 NLxdat;
 w32 NLxwords;
 w8 ttchead;
 w32 datstart;
 deque<w32> qstr;
 deque<ssmrecord *> qdat;
 if(level==2){
  NLxdat=NL2dat;
  NLxwords=13;
  ttchead=3;
  datstart=1;
  qstr=ql2strobe;
  qdat=ql2data;
 }else{
  NLxdat=NL1dat;
  NLxwords=9;
  ttchead=1;
  datstart=0;
  qstr=ql1strobe;
  qdat=ql1data;
 }
 ///printf("l2strobe: %i %i\n",ql2strobe.size(),ql2data.size());
 ///printf("l1strobe: %i %i\n",ql1strobe.size(),ql1data.size());
 ///printf("ttcb : %i\n",qttcb.size());
 int count=0;
 for(w32 i=0;i<qstr.size();i++){
  if(i>=qdat.size()){
   printf("Warning: More data than strobes i=%i, qdat: %i qstr: %i \n",i,qdat.size(),qstr.size());
   break;
  }
  w8* sdata=qdat[i]->sdata;
  w16 dwords[NLxwords];
  // Serial data bits to words
  for(w32 j=0;j<NLxwords;j++)dwords[j]=0;
  for(w32 j=datstart;j<NLxdat;j++){
    w32 j12=(j-datstart)/12;
    w32 jre=(j-datstart) % 12;
    dwords[j12]+=sdata[j]*(1<<(11-jre));
  }
  //
  w32 issm = qdat[i]->issm;
  //printf("L2data: %i %i  0x%x 0x%llx\n",ql2strobe[i],issm,bcid,data2);
  //printf("%i \n",ql2strobe[i]-(ql2data[i])->issm);
  w32 j=0;
  while(j<qttcb.size()){
    //w32 data2=qttcb[j]->data2;
    //w32 data3=qttcb[j]->data3;
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
int LTUBOARD::CreateRecordSSM()
{
 return CreateRecordSSM(1);
}
int LTUBOARD::CreateRecordSSM(bool first)
/*
 * Assuming no fifo busy - low rate
 */
{
 ClearQueues();
 w32 orbitflag=0,iorbit=0;
 w32 ls2flag=0,ils2=0,il2data=0;
 w32 ls1flag=0,ils1=0,il1data=0;
 w32 ttcbflag=0,ittcb=0;
 w32 l1DATA[NL1dat],l2DATA[NL2dat],ttcbdata[64];
 w32 bit,datab=3;
 int word;
 w32 *ss=GetSSM();
 // search first L0
 w32 ifirst=0;
 // to keep compatibility with old version
 if(first){
  for(int i=0;i<Mega;i++){
    word=ss[i];
    bit= ( (word & (1<<2)) == (1<<2));
    if(bit){
     ifirst=i;
     break;
    }
  }
 }
 //printf("ifirst= %i \n",ifirst);
 // Continue from first
 for(int i=ifirst;i<Mega;i++){
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
		channelB(ttcbflag,bit,i,ittcb,datab,ttcbdata);
		break;
        }
    }
 }
 return 0;
}
