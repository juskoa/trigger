#include "TTCITBOARD.h"
#include <bitset>
//---------------------------------------------------------------------
TTCITBOARD::TTCITBOARD(string const name,w32 const boardbase,int vsp)
:
BOARD(name,boardbase,vsp,1),
ssm(0),
VERSION(0x4),
CONTROL(0x8),
STATUS(0xc),
READ_SSM_ADDRESS(0x14),
READ_SSM_WORD(0x18),
RESET(0x28),
RESET_SNAPSHOT_N(0x8c),
TIME_L0_L1(0x24), 
RESET_COUNTERS(0x40), // first counter-4
COUNT_ERR_BCNT(0x88),  // last counter
f_lastbcid(0xffffffff),f_lastorbit(0xffffffff)
{
}
int TTCITBOARD::ReadAllCounters(w32 l0l1time)
{
 const char* names[18]={"L0","L1","L1m","L2a","L2r","ERR_PP","ERR_L0S","ERR_L1S","ERR_L1MM","ERR_L1MS","ERR_L1MI","ERR_L1MD","ERR_L2MM","ERR_L2MS","ERR_L2MI","ERR_L2MD","ERR_CAL","ERR_BCNT"};
 w32 counts[18];
 vmew(TIME_L0_L1,l0l1time);
 vmew(RESET_COUNTERS,0x0);
 usleep(100000);
 int i=0;
 for(w32 add=RESET_COUNTERS+4;add<= COUNT_ERR_BCNT; add+=4){
   counts[i]=vmer(add); i++;
 }
 printf("======================L0L1time: %i \n",l0l1time);
 i=0;
 for(w32 add=RESET_COUNTERS+4;add<= COUNT_ERR_BCNT; add+=4){
   printf("%i %s: %u \n",i,names[i],counts[i]); 
   i++;
 }
 return 0;
}
void TTCITBOARD::ClearQueues()
{
 for(w32 i=0;i<qttcab.size();i++) delete qttcab[i];
 qttcab.clear();
}
void TTCITBOARD::Print()
{
 //printf("%s:",ttcitname.c_str());
 //printboardinfo("");
}
/*
 * Starts ssm and exits. ssm is waiting
 */
int TTCITBOARD::startSSM()
{
 ssm=GetSSM();
 // reset address
 resetSSMAddress();
 for(w32 i=0;i<Mega;i++){
  ssm[i]=0;
  vmew(READ_SSM_WORD,0);
 }
 usleep(10000);
 //printf("address after reset= 0x%x\n",vmer(READ_SSM_ADDRESS));
 //
 resetSSMAddress();
 return 0;
} 
/*
 * reads SSM
 */ 
int TTCITBOARD::readSSM()
{
 int ic=0;
 while((ic<10) && (vmer(READ_SSM_ADDRESS)==0)){
   usleep(100000);
   ic++; 
 }
 if(ic==10){
  printf("TTCBOARD:readSSM: ssm not read, trigger not received after 1sec \n");
  return 1;
 }
 ssm=GetSSM();
 usleep(50000);
 //printf("after reset status: 0x%x\n",getStatus());
 usleep(100000);
 //printf("after usleep and control 2 status: 0x%x\n",getStatus());
 //printf("# word= 0x%x\n",vmer(READ_SSM_ADDRESS));
 //w32 stat=getStatus();
 getStatus();
 //printf("after usleep and control 3 status: 0x%x\n",stat);

 for(int i=0;i<Mega;i++){
  ssm[i]=vmer(READ_SSM_WORD);
 }

 return 0;
}
/*
 * Original routine written during debuging.
 * If triggers are not coming waiting for them.
 */ 
void TTCITBOARD::start_stopSSM()
{
 ssm=GetSSM();
 //vmew(RESET,0);
 // reset address
 resetSSMAddress();
 for(w32 i=0;i<Mega;i++){
  ssm[i]=0;
  vmew(READ_SSM_WORD,0);
 }
 usleep(10000);
 //
 resetSSMAddress();
 // tu caka
 while(vmer(READ_SSM_ADDRESS)==0)continue; 
 usleep(50000);
 usleep(100000);
 //printf("# word= 0x%x\n",vmer(READ_SSM_ADDRESS));
 //w32 stat=getStatus();
 //printf("after usleep and control 3 status: 0x%x\n",stat);

 for(int i=0;i<Mega;i++){
  ssm[i]=vmer(READ_SSM_WORD);
  //usleep(100000);
 }
}
/*
 * Modified routine : CTP (BUSY board) start/stop
 */ 
int TTCITBOARD::start_stopSSM(BUSYBOARD* bb)
{
 ssm=GetSSM();
 // reset address
 //resetSSMAddress();
 //for(w32 i=0;i<Mega;i++){
 // ssm[i]=0;
 // vmew(READ_SSM_WORD,0);
 //}
 // switch off triggers
 bb->SetDAQBUSY(0xff);
 // wait to clean buffers
 usleep(100000);
 // reset again - this makes ttc wait for input
 resetSSMAddress();
 //w32 add=vmer(READ_SSM_ADDRESS);
 //printf("0 address : 0x%x\n",add);
 //printf("control: 0x%x\n",vmer(CONTROL));
 // start triggers
 bb->SetDAQBUSY(0x0);
 usleep(20000);
 // stop triggers
 bb->SetDAQBUSY(0xff);
 usleep(20000);
 //add=vmer(READ_SSM_ADDRESS);;
 //while((add=vmer(READ_SSM_ADDRESS))==0)continue; 
 //printf("1 address : 0x%x\n",add);
 //usleep(100000);
 //printf("# word= 0x%x\n",vmer(READ_SSM_ADDRESS));
 //w32 stat=getStatus();
 //printf("after usleep and control 3 status: 0x%x\n",stat);

 //resetSSMAddress(); 
 //add=vmer(READ_SSM_ADDRESS);
 //printf("20 address : 0x%x\n",add);
 for(int i=0;i<Mega-3;i++){
  ssm[i]=vmer(READ_SSM_WORD);
  //usleep(100000);
 }
 //add=vmer(READ_SSM_ADDRESS);
 //printf("2 address : 0x%x\n",add);
 //bb->SetDAQBUSY(0x0);
 return 0;
}
/*
 * Modified routine : LTU start/stop
 */ 
int TTCITBOARD::start_stopSSM(LTUBOARD* ltu)
{
 ssm=GetSSM();
 // reset address
 resetSSMAddress();
 for(w32 i=0;i<Mega;i++){
  ssm[i]=0;
  vmew(READ_SSM_WORD,0);
 }
 usleep(10000);
 printf("address after reset= 0x%x\n",vmer(READ_SSM_ADDRESS));
 //
 // reset again - this makes ttc wait for input
 resetSSMAddress();
 if(ltu->SLMstart()) return 1;
 usleep(24000);
 ltu->SLMquit();
 usleep(3000);
 while(vmer(READ_SSM_ADDRESS)==0)continue; 
 usleep(50000);
 printf("after reset status: 0x%x\n",getStatus());
 usleep(100000);
 printf("after usleep and control 2 status: 0x%x\n",getStatus());
 printf("# word= 0x%x\n",vmer(READ_SSM_ADDRESS));
 w32 stat=getStatus();
 printf("after usleep and control 3 status: 0x%x\n",stat);

 //resetSSMAddress(); 
 for(int i=0;i<Mega;i++){
  ssm[i]=vmer(READ_SSM_WORD);
  //usleep(100000);
 }
 return 0;
}
/*
 * Analyse que dump.
 * Assume that TTC sequence start from beginning
 *
*/
int TTCITBOARD::AnalyseSSM()
{
 int ret=0;
 if(qttcab.size() ==  0){
   printf("Empty ssm memory \n");
   return 0;
 }
 w32 l0,i0;
 w32 issm0=qttcab[0]->issm;
 //if(issm0 == 262){
 if(issm0 == 282){
   l0=2;
   i0=0;
 }else if(issm0 == 279){
   l0=-1;
   i0=0;
 }else if(issm0 == 3){
   l0=0;
   i0=1;
   printf("L1=32, skippinng \n");
   return 0;
 }else{
 //if((issm0 != 262) && (issm0 != 266) && (issm0 != 265)){
   // for l1reject some new beginning of ssm shows up
   l0=0;
   i0=0;
   //printf("Error: first L1 expected at 262,266  but found at %i \n",qttcab[0]->issm);
   printf("Warning: first L1 expected at 282  or at 3 but found at  ------------------ %i\n",qttcab[0]->issm);
   //return 2;
 }
 // Is L0 last ?
 ssmrecord *ss=qttcab[qttcab.size()-1];
 printf("Last---------------%i %i\n",ss->ttcode,ss->data);
 //
 w32 cl0=1,cl1=0,cl1m=0,cl2a=0,cl2r=0;
 deque<w32> L1;
 deque<w32*> L1m;
 deque<w32*> L2m;
 //w32 L0L1time=260;
 w32 L0L1time=280;
 w32 l1mes[NL1words+1],l2mes[NL2words+1]; // one flag
 l1mes[0]=0;l2mes[0]=0;
 for(w32 i=i0;i<qttcab.size();i++){
   //printf("%i data=%i head= %i \n",qttcab[i]->issm,qttcab[i]->data,qttcab[i]->ttcode);
   w32 il1,il2;
   ssmrecord *ss=qttcab[i];
   w32 issm=ss->issm;
   if(ss->ttcode == 0){
     if((ss->data==1)){
      // L0 received 
      if((issm-l0)<L0L1time){
        printf("Error: L0L0 time violation - two L0 closer then %i:  %i at %i %i\n",L0L1time,issm-l0,issm,l0);	
        return 1;
      }
      l0=issm;
      cl0++;
     } else {
      // L1 received
      if((issm-l0) != L0L1time){
        printf("Error: L0L1 time violation - L1 arrived not in time: %i %i %i \n",issm-l0,issm,l0);
        //return 1;
        return 0;
      }
      L1.push_back(issm);
      cl1++;
     }
   } else if(ss->ttcode == 1){
     // L1h
     if(l1mes[0]){
       printf("Error: L1 shorter than 9 issm=%i \n",issm);
       return 1;
     }else{
       // new L1 message
       l1mes[0]=1;
       l1mes[1]=ss->tdata;
       il1=2;
       cl1m++;
     }
   } else if(ss->ttcode == 2){
     //L1 data
     if(l1mes[0]==0){
       printf("Error: L1 data without header issm=%i \n", issm);
       return 1;
     }
     l1mes[il1]=ss->tdata;
     //printf("il1= %i \n",il1);
     if(il1==NL1words){
       // last word of L1 mess
       il1=0;
       l1mes[0]=0;      
       //L1m.push_back(issm);
       w32* pp = new w32[NL1words+1];
       pp[0]=issm;
       for(int ii=1;ii<NL1words+1;ii++)pp[ii]=l1mes[ii];
       L1m.push_back(pp); // thos to be changed for pointers
     }
     il1++;
   } else if(ss->ttcode == 3){
       //L2h
       if(l2mes[0]){
         printf("Error: L2 shorter than 13 issm= %i \n",issm);
         return 1;
       }else{
         l2mes[0]=1;
         l2mes[1]=ss->tdata;
         il2=2;
	 cl2a++;
       }
   } else if(ss->ttcode == 4){
       // L2 data
       if(l2mes[0]==0){
         printf("Error: L2 data without header issm=%i\n",issm);
         return 1;
       }
       l2mes[il2]=ss->tdata;
       if(il2==NL2words){
         // last word
         il2=0;
         l2mes[0]=0;
         //L2m.push_back(issm);
         w32* pp = new w32[NL2words+1];
         pp[0]=issm;
         for(int ii=1;ii<NL2words+1;ii++)pp[ii]=l2mes[ii];
         L2m.push_back(pp);
       }
       il2++;
   }else if(ss->ttcode == 5){
         // L2r
         w32* pp = new w32[NL2words+1];
         pp[0]=issm;
         pp[1]=ss->tdata; //bcid
         // flog for L2r message
	 pp[2]=0xffffffff;
         for(int ii=3;ii<NL2words+1;ii++)pp[ii]=0;
         L2m.push_back(pp);
         cl2r++; 
   }else{ 
    printf("Error: unexpected code %i at %i \n",ss->ttcode,issm);
    return 1;
   }   
 }
 printf("# counts  : L0: %i L1: %i L1m: %i L2a: %i L2r: %i \n",cl0,cl1,cl1m,cl2a,cl2r);
 printf("# fifo sizes : L1 %i L1m %i L2m %i\n",L1.size(),L1m.size(),L2m.size());
 if(cl0 != cl1){
   //printf("Warning: different number of L0 and L1 detected. \n");
   //return 1;
 } 
 // Measurement of timeout due to the fifos
 //printf("###### of L1: %i , # if L1 mess: %i , # of L2 mess: %i \n",L1.size(),L1m.size(),L2m.size());
 if(L1.size() == 0){
   printf("Warning: No L1 \n");
   return 0;
 }
 if(L1.size() != L2m.size()){
   // check last l0 if not too close to the end of ssm
   if((Mega-L1[L1.size()-1])>4600){
    printf("Error: different # of L1 and L2m(L2a+L2r) : L1 %i L1m %i L2m %i\n",L1.size(),L1m.size(),L2m.size());
    return 1;
   }
 }
 // 1.) Looking for max delays for L1 and L2messages
 // 2.) comparing bcid from L2 message with 'local' bcid (from ssm) - variable delta
 // 3.) comparing bcid+orbit woth local bcid
 w32 delmaxL2=0;
 w32 delmaxL1=0;
 w32 bcl1=L1[0]%3564;  // BC from ssm position of first L1 
 int delta0 = L2m[0][1] - bcl1;
 w32 issml10 = L1[0];
 w32 orbit0    = (L2m[0][2]<<12)+L2m[0][3];
 w32 bc0 = L2m[0][1];
 // start orbit check only from the first l2a , skip l2r before
 w32 firstl2a=0;
 if(L2m[0][2] != 0xffffffff)firstl2a=1;
 //printf("%i %i %i \n", L1[0],bcl1,delta0);
 if(delta0<0) delta0=delta0+3564;   // L2m - L1 distance
 for(w32 i=0;i<L2m.size();i++){
    w32 issml1=L1[i];
    w32 issml2m=L2m[i][0];
    w32 bcl2m=L2m[i][1];
    // delay
    w32 delay1=L1m[i][0]-issml1;
    w32 delay2=issml2m-issml1;
    if(delmaxL2<delay2)delmaxL2=delay2;
    if(delmaxL1<delay1)delmaxL1=delay1;
    // bcoffset
    w32 bcl1=issml1%3564;
    int delta=bcl2m-bcl1;
    if(delta<0) delta = delta+3564;
    if(delta0 != delta){
      printf("Error: delta0= %i \n",delta0);
      printf("L1 issm: %7i  L1 bc: %4i L2m issm: %7i L2m bc: %4i delta: %i delay: %i\n",issml1,bcl1,issml2m,bcl2m,delta,delay2);
      return 1;
    }
    // bc and orbit offset
    w32 delissm = issml1-issml10;
    w32 newbc = ((delissm % 3564) + bc0);
    w32 neworb = orbit0 + delissm / 3564 + newbc / 3564;
    newbc = newbc % 3564;
    if(neworb >= 0xffffff) neworb=neworb-0xffffff;
    // BC check for both l2a and l2r
    if(newbc != bcl2m){
      printf("Error: bc offset from L2m and local wrong %x %x %x  \n",issml1,bcl2m,newbc);
      //return 1;
      ret=1;
    }
    // Orbit check only for L2a
    if(L2m[i][2] != 0xffffffff){
      if(firstl2a==0){  // skip first check and set first orbit for next check
        firstl2a=1;
        neworb=(L2m[i][2]<<12)+L2m[i][3];
      }else{
        w32 orbit=(L2m[i][2]<<12)+L2m[i][3];
        if((neworb != orbit) || (newbc != bcl2m)){
          printf("Error: bc and orbit offset from L2m and local wrong %x %x %x,%x %x \n",issml1,orbit,bcl2m,neworb,newbc);
          //return 1;
          ret=1;
        }
      }
    }
    issml10=issml1;
    //orbit0=orbit;
    //bc0=bcl2m;
    // this should work also for L2r
    orbit0=neworb;
    bc0=newbc;
    ret += CompareL1L2Data(L1m[i],L2m[i]);
 }
 for(w32 i=0;i<L1m.size();i++)delete L1m[i];
 for(w32 i=0;i<L2m.size();i++)delete L2m[i];
 printf("Max L2 delay: %i Max L1 delay: %i \n",delmaxL2,delmaxL1);
 if(ret==0) printf("NO ERROR detected. \n");
 return ret;
}
/*
 * Analyse que dump for Run3 ttc
 * Assume that TTC sequence start from beginning
 *
*/
int TTCITBOARD::AnalyseSSMRun3()
{
 // Parameters to check
 //
 // Distance between L0 triggers
 int L0DISTGEN=f_Nperiod;
 // L0-L1 distance
 uint L0L1DIST=300;
 //uint L0L1DIST=300;
 //
 int ret=0;
 if(qttcab.size() ==  0){
   printf("Empty ssm memory \n");
   return 0;
 }
 f_lastbcid=0xffffffff;
 f_lastorbit=0xffffffff;
 w32 l0,i0;
 w32 issm0=qttcab[0]->issm;
 int l0dist=0;
 if(issm0 == (L0L1DIST+2)){
   l0=2;
   l0dist=2;//282-280;
   i0=0;
 }else if(issm0 == 3){
   l0=0;
   i0=8;
   l0dist=3-(L0L1DIST+1);//281;
   printf("L1=3, skipping 1st L1 mes \n");
   //return 1;
 }else{
   printf("Error: first L1 expected at 3 or 282  but found at %i \n",qttcab[0]->issm);
   //printf("Warning: first L1 expected at 282  or at 3 but found at  ------------------ %i\n",qttcab[0]->issm);
   return 2;
 }
 // Is L0 last ?
 ssmrecord *ss=qttcab[qttcab.size()-1];
 //printf("Last---------------%i %i\n",ss->ttcode,ss->data);
 //////
 w32 cl0=1,cl1=0,cl1m=0;
 deque<w32> L1;
 deque<w32*> L1m;
 //w32 L0L1time=260;
 w32 L0L1time=L0L1DIST;
 w32 l1mes[NL1words3+1]; // one flag
 l1mes[0]=0;
 int l0dists[5];
 for(int i=0;i<5;i++)l0dists[i]=0;
 for(w32 i=i0;i<qttcab.size();i++){
   //printf("%i data=%i head= %i \n",qttcab[i]->issm,qttcab[i]->data,qttcab[i]->ttcode);
   w32 il1;
   ssmrecord *ss=qttcab[i];
   w32 issm=ss->issm;
   if(ss->ttcode == 0){
     if((ss->data==1)){
      // L0 received 
      if((issm-l0)<L0L1time){
        return 1;
      }
      // disatnce between l0
      int ddist=issm-l0dist;
      if(ddist == L0DISTGEN-1 ) l0dists[0]++;
      else if(ddist == L0DISTGEN)l0dists[1]++;
      else if(ddist == L0DISTGEN+1 )l0dists[2]++;
      else if(ddist == L0DISTGEN+2)l0dists[3]++;
      else l0dists[4]++;
      if(ddist != L0DISTGEN)
      {
	printf("dist != %i at %i : %i \n",L0DISTGEN, issm,ddist);
	ret++;
      }
      //
      l0dist=issm;	
      l0=issm;
      cl0++;
     } else {
      // L1 received
      if((issm-l0) != L0L1time){
        printf("Error: L0L1 time violation - L1 arrived not in time: %i %i %i \n",issm-l0,issm,l0);
        //return 1;
        return 0;
      }
      L1.push_back(issm);
      cl1++;
     }
   } else if(ss->ttcode == 1){
     // L1h
     if(l1mes[0]){
       printf("Error: L1 shorter than 9 issm=%i \n",issm);
       return 1;
     }else{
       // new L1 message
       l1mes[0]=1;
       l1mes[1]=ss->tdata;
       il1=2;
       cl1m++;
     }
   } else if(ss->ttcode == 2){
     //L1 data
     if(l1mes[0]==0){
       printf("Error: L1 data without header issm=%i \n", issm);
       return 1;
     }
     l1mes[il1]=ss->tdata;
     //printf("il1= %i \n",il1);
     if(il1==NL1words3){
       // last word of L1 mess
       il1=0;
       l1mes[0]=0;      
       //L1m.push_back(issm);
       w32* pp = new w32[NL1words3+1];
       pp[0]=issm;
       for(int ii=1;ii<NL1words3+1;ii++)pp[ii]=l1mes[ii];
       L1m.push_back(pp); // thos to be changed for pointers
       if(analyseL1mRun3(pp)) return 1;;	
     }
     il1++;
   }else{ 
    printf("Error: unexpected code %i at %i \n",ss->ttcode,issm);
    return 1;
   }   
 }
 printf("# counts  : L0: %i L1: %i L1m: %i \n",cl0,cl1,cl1m);
 printf("# fifo sizes : L1 %i L1m %i \n",L1.size(),L1m.size());
 printf("L0 distances %i,%i,%i,%i,other: %i %i %i %i %i\n",L0DISTGEN-1,L0DISTGEN,L0DISTGEN+1,L0DISTGEN+2,l0dists[0],l0dists[1],l0dists[2],l0dists[3],l0dists[4]);
 if(cl0 != cl1){
   //printf("Warning: different number of L0 and L1 detected. \n");
   //return 1;
 }
 return ret; 
}
int TTCITBOARD::analyseL1mRun3(w32* mes)
{
 if(f_lastbcid==0xffffffff)
 {
   f_lastbcid=mes[4];
   f_lastorbit=mes[7]+(mes[6] << 12);
   printf("1st BCID: 0x%x 1st ORBIT: 0x%x \n",f_lastbcid,f_lastorbit);
 }
 else
 {
   // check bcid
   w32 bcid=mes[4];
   w32 newbcid = (f_lastbcid+f_Nperiod) % 3564;
   if( bcid != newbcid)
   {
	printf("ssm pos: %i BCID error: bcid: 0x%x expected: 0x%x \n",mes[0],bcid,newbcid);
	return 1;
   }
   // check orbit
   w32 orbit=mes[7]+(mes[6] << 12);
   w32 neworbit = f_lastorbit + (f_lastbcid+f_Nperiod)/3564;
   //if(newbcid<bcid) neworbit+=1;
   neworbit = neworbit % 0xffffff;
   if( orbit != neworbit)
   {
	printf("ssm pos: %i ORBIT error: orbit: 0x%x expected: 0x%x \n",mes[0],orbit,neworbit);
	return 1;
   }
   f_lastbcid=bcid;
   f_lastorbit=orbit;
 }
 return 0;
}
int TTCITBOARD::CompareL1L2Data(w32* L1m,w32* L2m)
{
    if(L2m[2]==0xffffffff) return 0;   // L2r
    w32 l1[27],l2[27];
    for(int i=0;i<9;i++){
     l1[3*i]=L1m[NL1words-i] & 0xf;
     l1[3*i+1]=(L1m[NL1words-i] & 0xf0)>>4;
     l1[3*i+2]=(L1m[NL1words-i] & 0xf00)>>8;
     l2[3*i]=L2m[NL2words-i] & 0xf;
     l2[3*i+1]=(L2m[NL2words-i] & 0xf0)>>4;
     l2[3*i+2]=(L2m[NL2words-i] & 0xf00)>>8;
    }
    int flag=0;
    for(int i=0;i<25;i++){
       if(l1[i] != l2[i+2]){
         flag=1;
         printf("Warning: l1 classes different from l2 classes \n");
       }
       // Brutal check of trigger patern agains 0xf - used for tests ala didier
       //if(l2[i+2] != 0xf){
       //  flag=1;
       //  printf("Warning: class pattern != 0xf \n");
       //}
    }
    if(flag){
      for(int i=0;i<27;i++)printf("%1x",l1[i]);
      printf(" l1\n");
      for(int i=0;i<27;i++)printf("%1x",l2[i]);
      printf(" l2\n");
      return 1; // to be removed when L2r
    }
 
    /*
    for(int im=0;im<NL1words+1;im++){
     printf(" %03x ", L1m[im]);
    }
    printf("                 l1 mess=  \n");
    for(int im=0;im<NL2words+1;im++){
     printf(" %03x ", L2m[im]);
    }
    printf("                 l2 mess=  \n");
    */
 return 0;
}
/*
 * SSM dump as ssmrecord que used for AnalyseSSM
 * ttchead = 0  data=1  l0
 * ttchead = 0  data=2  l1
 * ttchead = 1  L1 head
 * ttchead = 2  L1 data
 * ttchead = 3  L2 head 
 * ttchead = 4  L2 data 
*/
void TTCITBOARD::Dump2quSSM()
{
 string ttcadl[]={"ZERO","L1h","L1d","L2h","L2d","L2r ","RoIh","RoId"};
 if(ssm==0){
   printf("Dump2quSSM: no ssm found , quiting. \n");
   return;
 }
 int notactive=Mega+10;
 int achan=notactive;
 for(int i =Mega-1;i>=0;i--){
   int j=Mega-1-i;
   if(ssm[i] & 0x20000){
     if(achan==notactive){
       // L0 or L1
       achan=i;
     }else if(achan==i+1){
      //L1 
      //printf("L1 at %i \n",j-1);
      achan=notactive;
      ssmrecord *l1 = new ssmrecord(j-1,2);
      qttcab.push_back(l1);
     }else{
      printf("Error \n");
     }
   }else{
      if(achan != notactive){
        //printf("L0 at %i \n",j-1);
        achan=notactive;
        ssmrecord *l0 = new ssmrecord(j-1,1);
        qttcab.push_back(l0);
      }
   }
   if(ssm[i] & 0x10000){
     w32 header=(ssm[i]&0xf000)>>12;
     w32 data=ssm[i]&0xfff;
     ////printf("%7i Data 0x%1x 0x%3x \n", j,ssm[i]&0xf000,data);
     string dd=ttcadl[header];
     //printf("%7i %s 0x%1x 0x%3x \n",j,dd.c_str(),header,data);
     ssmrecord *ss = new ssmrecord(j,header,0,0,data,0);
     qttcab.push_back(ss);
   }
 } 
}
void TTCITBOARD::Dump2quSSMRun3()
{
 string ttcadl[]={"ZERO","L1h","L1d","L2h","L2d","L2r ","RoIh","RoId"};
 if(ssm==0){
   printf("Dump2quSSM: no ssm found , quiting. \n");
   return;
 }
 int notactive=Mega+10;
 int achan=notactive;
 for(int i =Mega-1;i>=0;i--){
   int j=Mega-1-i;
   if(ssm[i] & 0x20000){
     if(achan==notactive){
       // L0 or L1
       achan=i;
     }else if(achan==i+1){
      //L1 
      //printf("L1 at %i \n",j-1);
      achan=notactive;
      ssmrecord *l1 = new ssmrecord(j-1,2);
      qttcab.push_back(l1);
     }else{
      printf("Error \n");
     }
   }else{
      if(achan != notactive){
        //printf("L0 at %i \n",j-1);
        achan=notactive;
        ssmrecord *l0 = new ssmrecord(j-1,1);
        qttcab.push_back(l0);
      }
   }
   if(ssm[i] & 0x10000){
     w32 header=(ssm[i]&0xf000)>>12;
     w32 data=ssm[i]&0xfff;
     ////printf("%7i Data 0x%1x 0x%3x \n", j,ssm[i]&0xf000,data);
     string dd=ttcadl[header];
     //printf("%7i %s 0x%1x 0x%3x \n",j,dd.c_str(),header,data);
     ssmrecord *ss = new ssmrecord(j,header,0,0,data,0);
     qttcab.push_back(ss);
   }
 } 
}

void TTCITBOARD::DumpqueSSM2file(const char *name)
{
 char filename[200];
 char *environ;
 char fnpath[1024];
 FILE *dump;
 sprintf(filename,"%s",name);
  // Open file
 environ= getenv("VMEWORKDIR"); strcpy(fnpath, environ);
 strcat(fnpath,"/"); strcat(fnpath,"WORK/"); 
 strcat(fnpath, filename); strcat(fnpath, ".dump");
 dump=fopen(fnpath,"w");
 if(dump == NULL){
  printf("Cannot open file: fnpath: %s\n", fnpath);
  exit(1);
 }
 printf("Dumping qttcab in %s \n",fnpath);
 for(w32 i=0;i<qttcab.size();i++){
   char line[128];
   qttcab[i]->Print2char(line);
   fprintf(dump,"%i %s \n",i,line);
 }
}
/*============================================================================
 * Text dump for visual debuging assumes alice run2 format
 */
void TTCITBOARD::DumptxtSSM()
{
 //char *ttcadl[]={"ZERO","L1h","L1d","L2h","L2d","L2r ","RoIh","RoId"};
 string ttcadl[]={"ZERO","L1h","L1d","L2h","L2d","L2r ","RoIh","RoId"};
 for(int i =Mega-1;i>=0;i--){
   int j=Mega-1-i;
   if(ssm[i] & 0x20000){
     printf("%7i A chanel  \n",j);
   }
   if(ssm[i] & 0x10000){
     w32 header=(ssm[i]&0xf000)>>12;
     w32 data=ssm[i]&0xfff;
     //printf("%7i Data 0x%1x 0x%3x \n", j,ssm[i]&0xf000,data);
     string dd=ttcadl[header];
     printf("%7i %s 0x%1x 0x%3x \n",j,dd.c_str(),header,data);
   }
 }
}
/*============================================================================
 * Text dump for visual debuging no assumtions
 */
void TTCITBOARD::DumphexbinSSM()
{
 for(int i =Mega-1;i>=0;i--){
   int j=Mega-1-i;
   if(ssm[i] & 0x20000){
     printf("%7i A chanel 0x%x  b",j, ssm[i]);
     std::cout << std::bitset<20>(ssm[i]) << std::endl;
   }

   //if(ssm[i] & 0x10000){
   //if(ssm[i] & 0x1ffff){
  // if(ssm[i] ){
   else if(ssm[i] & 0x10000){ 
     printf("%7i B chanel 0x%05x  b", j, ssm[i]);
     std::cout << std::bitset<20>(ssm[i]) << std::endl;
   }
 }
}
/*============================================================================
 * Text dump for visual debuging
 */
int TTCITBOARD::CheckClassPatternSSM()
{
 int ret=0;
 int istart;
 for(istart =Mega-1;istart>=0;istart--){
   if(ssm[istart] & 0x10000){
     w32 header=(ssm[istart]&0xf000)>>12;
     if(header==3) break;
   }
 }
 printf("First L2h at %i %i \n",Mega-istart,istart);
 if(istart<0) return 1;
 istart++;
 int il2=NL2words;
 for(int i =istart-1;i>=0;i--){
   int j=Mega-1-i;
   if(ssm[i] & 0x10000){
     w32 header=(ssm[i]&0xf000)>>12;
     w32 data=ssm[i]&0xfff;
     if(header==3){
       if(il2 != NL2words){
         printf("Warning: il2= %i at %i \n",il2,j);
         return 1;
       }else{
         il2=1;
       }     
     }else if(header==4){
        il2++;
        if(il2<5){
	  continue;
        }else if (il2<13){
          if(data != 0xfff){
            printf("Warning: data= 0x%x at %i \n",data,j);
            return 1;
          }
        }else if(il2==13){
          if(data != 0xf00){
            printf("Warning: data= 0x%x at %i \n",data,j);
            return 1;
          }
        } else {
          printf("Too many data: il2=%i at %i \n",il2,j);
	  return 1;
        }
     }else{
       continue;
     }
   }
 }
 return ret;
}
