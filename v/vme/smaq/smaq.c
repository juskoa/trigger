/*BOARD SYNCctp 0x820000 0xd000 A24
*/
/*
24.8.2008:
SMAQDATA -environment variable. Name of directory in alidcscom707:SMAQ/datadir
          default: "last"
          see bin/smaq script
28.8.2008
two new features:
- option with interface board is printing BC number as found on interface board
- checkInputs2: calcultes correlation between inpnum (= trigger) and other selec  ted inputs to get alignment even faster.
11.9.
send 'bobr' DIM command to ACR07/BOBR server running on aldaqacr07 machine
22.9. shmaccess (or malloc) added
14.11. compile/link ok (mainly symbolic links created)
*/
#include <stdio.h>
#include <unistd.h>    /* usleep */
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#ifdef CPLUSPLUS
#include <dic.hxx>
#else
#include <dic.h>
#endif

#include "vmewrap.h"
#include "../ctp/ctplib/ctplib.h"
#include "../ctp/ctp.h"
#define DBMAIN
#include "../ctp_proxy/Tpartition.h"
#include "shmaccess.h"
#include "ssmctp.h"
#include "intint.h"

int quit=0;
char dirname[40]="last";
int siteflag=0;
void getdatetime(char *);
int dumpssm(int board,char *filename);
//double rnlx();
//void setseeds(long,int);
int syncSSM(int n, int *boards);
w32 orbitstatus;
// DIM 
char DETSET_COM[256]="NONE";
char DETSTAT_COM[256]="NONE";
#define MAXCTPINPUTS 24 
char StatusString[MAXCTPINPUTS+1];

/*---------------------------------------------*/ void gotsignal(int signum) {
char msg[100];
// SIGUSR1:  // kill -s USR1 pid
signal(signum, gotsignal); siginterrupt(signum, 0);
sprintf(msg, "got signal:%d", signum); prtLog(msg);
if((signum==SIGUSR1) || (signum==SIGKILL) ) {
  quit=1;
};quit=signum;  // for ANY signal
}

void callback(void *tag, int *retcode){
 char command[100];
 printf("callback: %li %i ",*(long *)tag,*retcode);
 if(*retcode)printf("succesful.");
 else{ 
   printf(" failed: Wrong detector name or detector server not running.\n");
   return;
 }
 switch(*(long *)tag){
   case(333):
        strcpy(command,"STARTRUNCOUNT");
        break;
   case(18):
        printf(" Command %s executed.\n",DETSET_COM);
        break;
   case(33):
        printf(" Command bobr executed.\n");
        break;
   case(4567):
        printf("Service %s,\n CTP inputs status:<%s>\n",DETSTAT_COM,StatusString);
        break;
   default:
        printf("Unknown tag %li \n",*(long *)tag);
        return;
 }
}
void beepni() {
int rc;
char msg[20]="bobr";
// see trg@aldaqacr07:aj/pydim
rc= dic_cmnd_callback("ACR07/BOBR", msg, strlen(msg)+1, &callback, 33);
}
/////////////////////////////////////////////////////////////////////////////////////
#define MAXCOUNTERS 160
#define NINP 24
#define L0OFFSET 65 
#define L0TIMEOFFSET 13
#define L1OFFSET 5
#define BEEPPOS 24 
int inpL0[NINP];
double getTime(w32 oldval,w32 newval){
 w32 diff;
 double milisecs;
 diff=dodif32(oldval,newval);
 milisecs=diff*0.4/1000.;
 return milisecs;
}
//char inpL0Names[NINP][32];
char inpL0Names[NINP][32];
void initNames(int board){
 //  Choose inputs HERE !
 //            1  2  3  4  5  6  7
 int inpCH[]={ 1,2,3,4,10,21,0};
 int i,ix;
 //readTables();  no need -is in initSMAQ()
/*for(i=0;i<NINP;i++){
  inpL0[i]=0;
  strcpy(inpL0Names[i],validCTPINPUTs[i].name);
}*/
for(i=0;i<NINP;i++){
  inpL0[i]=0;
  strcpy(inpL0Names[i],"unknown");
};
i=0;
for(ix=0;ix<NCTPINPUTS;ix++){
  if(validCTPINPUTs[ix].level!=(board-1)) continue;
  i= validCTPINPUTs[ix].inputnum-1;
  strcpy(inpL0Names[i],validCTPINPUTs[ix].name);
};
for(i=0;i<NINP;i++){
  if(inpCH[i] == 0) break;
  if(inpCH[i] && (strcmp(inpL0Names[i],"unknown")!=0)) {
    inpL0[inpCH[i]-1]=1;
    printf("initNames:%s: %d(1..24)\n", inpL0Names[i], i+1);
  };
};
}
//------------------------------------------------------------------
//  INT on the int board
#define NCOREL 16
w32 corel[NCOREL][NINP];
w32 bcl0[3564];
void initCorrel(){
 int i,j;
 for(i=0;i<NINP;i++){
    for(j=0;j<NCOREL;j++)corel[j][i]=0;
 }
 for(i=0;i<3564;i++)bcl0[i]=0;
}
/*
- check content of channesl i
- calculate distance between signals in inpnum
*/
int checkInputs3(int board,FILE *f, int inp){
 w32 *sm;
 w32 mask;
 int i,j;
 if(inp>24) return 0;
 sm=sms[board].sm;
 mask=(1<<(inp+7));
 j=0;
 for(i=0;i<Mega;i++){
  if(sm[i] & mask){
    if(j==0) j=i;
    else{
     fprintf(f,"Dist: %i \n",i-j);
     j=i;
    }
  }
 }
 return 0;
}

/*
- check content of channesl i
- calculate correlation of channel i with others
*/
int checkInputs2(int board,FILE *f, int inp){
 w32 *sm;
 w32 mask;
 int i,j,k,prt;
 if(inp>24) return 0;
 sm=sms[board].sm;
 mask=(1<<(inp+7));
 for(i=0;i<Mega;i++){
  if(sm[i] & mask){
    // find orbit and BC: k = distance between input signal and orbit 
    k=0;
    while(((i+k)>0) && ((sm[i+k] & 1) == 0))k--;
    //printf("i-k= %i k=%i \n",i-k,k);
    // BC
    if((i+k))bcl0[-k+40-7]++;    //was -5 in 2008 (today orbit is 38bcs long)
    // do correlation
    for(j=8;j<32;j++){
      k=0;
      while((i-k)>=0 && k<NCOREL){
        if(sm[i-k]&(1<<j))corel[k][j-8]++;
        k++;
      }
    }
  }
 }
 for(i=0;i<NINP;i++){
  if(inpL0[i] == 0) continue;
  fprintf(f,"Corel %s x %s:",inpL0Names[inp-1],inpL0Names[i]);
  for(j=0;j<NCOREL;j++)fprintf(f, "%i ",corel[j][i]);
  fprintf(f, "\n");
 }
 prt=0;
 for(i=0;i<3564;i++) { if(prt>100) break; ;  // print only first 100
   if(bcl0[i]) { prt++; fprintf(f,"BC at L%d:%4i %i\n",board-1,i,bcl0[i]); };
 };
 return 0;
}
//
/*
Checks content of ssm for non zero bits in channels.
*/
int checkInputs(int board,FILE *f){
 // Only for board L0 
 char channels[1024];
 int i,j;
 w32 *sm;
 w32 chans[NINP];
 //readSSM(board);   already done
 sm=sms[board].sm;
 for(i=0;i<NINP;i++)chans[i]=0;
 for(i=0;i<Mega;i++){
  for(j=8;j<32;j++){
    //printf("%i %i \n",j,i);
    if((!chans[j-8]) && ((1<<j) & sm[i])){
      chans[j-8]=i;
    }
  }
 }
 strcpy(channels,"");
 for(i=0;i<NINP;i++){
    //sprintf(channels,"%s L0[%i]:%i",channels,i+1,chans[i]);
   //printf("print %i %i\n",i,chans[i]);
  if(inpL0[i]){
    if(chans[i])sprintf(channels,"%s L%d[%i]:%i",channels,board-1,i+1,chans[i]);
    else {
      //printf("L0[%i]:N ",i+1);
   }
   }else{
   } 
 }
 //printf("\n");
 /*printf("SSM INPUTS (first hits):\n %s \n",channels);
 fprintf(f,"SSM INPUTS (first hits):\n %s \n",channels); */
 return 0;
}
/*
   get ssms of l0/1/2 board  (board123: 1,2 or 3) and INT board if inboard>0
*/
int getSSMs(int board123, int inpnum,int intboard,FILE *f){
 char dt[32];
 char filename[256];
int rcscp; char cmd[100];
 getdatetime(dt);
 dt[10]='_';
 //dt[13]='_';
 //dt[16]='_';
 usleep(4000); // fine even for triggering with BOBR signal
 stopSSM(board123);   // which is coming 19 orbits before interaction
 if(intboard)stopSSM(4);
 strcpy(filename,"");
 sprintf(filename,"l%d_%i_%s.dmp",board123-1,inpnum,dt);
 //dumpSSM(1,filename);
 readSSM(board123);
 dumpssm(board123,filename);
  /* scp: not very nice (.dmp file is copied over network 3 times:
     1. when created (WORK directory is NFS monted)
     2. when scp reads is over NFS back
     3. when scp send it to alidcscom707)
  Better idea: use pydimserver on alidcscom026: .dmp file can
  be accessed directly on 026 machine and copied to 707 machine
  */
 if(siteflag){
   sprintf(cmd, "scp -Bq2 WORK/%s trigger@alidcscom707:SMAQ/%s/", 
   filename, dirname);
   //printf("%s\n",cmd);
   rcscp= system(cmd);
   if(rcscp==0) {
     sprintf(cmd, "rm WORK/%s", filename);
     rcscp= system(cmd);
     if(rcscp!=0) {
       printf("rc:%d from remove file...\n", rcscp);
       fprintf(f, "rc:%d from remove file...\n", rcscp);
     };
   };
   //printf("%s ----------------------->dumped. rc:%d\n",filename, rcscp); 
   fprintf(f,"%s ----------------------->dumped. rc:%d\n",filename, rcscp);
 }else{
   printf("%s ----------------------->dumped.\n",filename); 
   fprintf(f,"%s ----------------------->dumped.\n",filename);
 } 
 //checkInputs(1,f);
 checkInputs2(board123,f,inpnum);
 setomSSM(board123,0xb);
 startSSM1(board123);
 if(intboard){  
   CTPRIRDList *INTlist=NULL; 
   //sprintf(filename,"b2_%s",dt);
   //dumpSSM(1,filename);
   //printf("%s ----------------------->dumped\n",filename);
   readSSM(4);
   INTlist=getCTPRIRDList(4,INTlist);
   //printlistN(INTlist,f);
   printBCs(INTlist,f);
   INTlist=freenumsN(INTlist);  
   setomSSM(4,0x3);
   startSSM1(4);
 }
 fflush(f);
 usleep(30000);    // was 300000= 300ms why? 30ms should be enough
 return 0;
}
//------------------------------------------------------------
 w32 prevRead[NINP];
 w32 firstRead[NINP];
 w32 prevtime;
int smaqprintCounters(int trigboard, w32 *l0, int trig, FILE *f){
 int i; w32 curtime, deltime,timeadr,counteroffset;
 char dt[20];
 char msg[200];
 getdatetime(dt);
 //printf("%s\n",dt);
if(trigboard==3) {
  timeadr=5;
  counteroffset=5;   // L2OFFSET
} else if(trigboard==2) {
  timeadr=5;
  counteroffset=5;   // L1OFFSET
} else {
  timeadr=13;
  counteroffset=L0OFFSET;
};

 getCountersBoard(trigboard, counteroffset+NINP, l0, 3);
 curtime= l0[timeadr]; deltime=dodif32(prevtime, curtime); 
 if(trig>24) {
   sprintf(msg, "===========Triggering on %i (27:INT1 28:INT2)\n",trig);
 } else {
   sprintf(msg, "===========Triggering on %s[%i]\n",inpL0Names[trig-1],trig);
 };
 /* printf("%s", msg); */fprintf(f, "%s", msg);
 printf("counter              abs        rel       rate  \n");
 // fprintf(f,"counter              abs        rel       rate  \n");
 for(i=0;i<NINP;i++){
   if(inpL0[i]){
     int ic; w32 count, relcount, abscount;
     float rate;
     ic=L0OFFSET+i+1;
     count= l0[ic];
     relcount=dodif32(prevRead[i], count);
     abscount=dodif32(firstRead[i], count);
     prevRead[i]= count; 
     //if(i==2 ) {
     if(strncmp(inpL0Names[i], "0S", 2)==0) {
       relcount=relcount/4; abscount= abscount/4;};    // SPD 
     if(strncmp(inpL0Names[i], "BOBR", 4)==0) {
       relcount=relcount/3564; abscount= abscount/3564;};    // BOBR is 1 orbit wide
     rate= relcount*1.0E6/(deltime*0.4);
     sprintf(msg,"%s[%2i]: %10u %10u %10.1f\n",
       inpL0Names[i],i+1, abscount, relcount, rate);
     printf("%s", msg); //fprintf(f, "%s", msg);
     fflush(f);
   }
 }
 printf("\n");  //fprintf("\n");  
 prevtime= curtime;
 return 0;
}
/* max. time to wait for prepulse in seconds: */
#define PP_PERIOD 50
/*FGROUP SMAQ - Snapshot Memory Aquisition
  intboard -      if 0 : do not take int board
                     1 : take int board and trigger on l0 input board
                     2 : take int board and trigger on l0int1 counters 
                     3 : take int board and trigger on l0int2 counters
  inpnum - inpnumber on l0 level to trigger on: 1..24
                     on l1 level to trigger on:25..48
           0: trigger on LHCpp (BOBR card in the CTP crate)
  Output:
         - log file - contains also BC of interaction records
         - l0 ssm dump
*/
int inputsSMAQ(int intboard ,int inpnum012){
 //w32 L0counts[MAXCOUNTERS];
 w32 last[MAXCOUNTERS];
 w32 l0first[MAXCOUNTERS];
 int counteroffset,countermax,trigboard;
 w32 timeadr;
 int i,timeold,time,inpnum;
 double timediff;
 int trigold,trig;
 int vspbobr;
 int trigcond,beepcond;
 Tlhcpp lhcpp;
 FILE *f;
 char *environ;
 char fnpath[1024],logname[1024];
 char dt[32];
// Open the log file
 getdatetime(dt);
 dt[10]='_';
 environ= getenv("VMEWORKDIR"); 
 strcpy(fnpath, environ);
 strcat(fnpath,"/WORK/");  
 sprintf(logname,"%ssmaq_%s.log",fnpath,dt);
 f=fopen(logname,"w");
 if(f==NULL){
  printf("Cannot open file %s \n",logname);
  return 1;
 }
 printf("Log file %s opened.\n",logname);
// Check the site (ALICE or else)
 environ= getenv("VMESITE");  
 if(strcmp(environ, "ALICE")==0) siteflag=1;
// 
 initCorrel();
//
if(inpnum012>48) {
  inpnum= inpnum012-48;
  trigboard=3;
  timeadr=5;
  counteroffset=5;   // L2OFFSET
} else if(inpnum012>24) {
  inpnum= inpnum012-24;
  trigboard=2;
  timeadr=5;
  counteroffset=5;   // L1OFFSET
} else {
  inpnum= inpnum012;
  trigboard=1;
  timeadr=13;
  counteroffset=L0OFFSET;
};
initNames(trigboard);
if(intboard == 2){   // trigger on int1
  // set also lut table
  inpnum=27;
} else if(intboard == 3){  // trigger on int 2
  inpnum=28;
}

 //countermax=counteroffset+NINP+6;  // 6 for int counters
 countermax=counteroffset+NINP+30;  // 6 for int counters
 
 getCountersBoard(trigboard,counteroffset + NINP,l0first,3);
 for(i=0;i<NINP;i++){
   int ic;
   ic= counteroffset+i+1;
   firstRead[i]=l0first[ic]; prevRead[i]=l0first[ic]; 
   prevtime= l0first[L0TIMEOFFSET];
 }
 // open bobr vmespace
 if(inpnum==0) {
   if((vspbobr=bobrOpen()) == -1){
    printf("Cannot open bobr vme space\n");
    return 1;
   } else {
     int rc;
     rc= getlhcpp(vspbobr,1,0, &lhcpp);   // get next BST message
     if(rc!=0) {
       printf("rc:%d from getlhcpp()\n",rc);
       exit(8);
     };
   }; 
 };
 // 1st readings
 getCountersBoard(trigboard,countermax,last,3);
 timeold=last[timeadr];
 if(inpnum)trigold=last[counteroffset+inpnum];  //counting from 1
 else beepcond= (((lhcpp.Byte54)&0x1) != 0);
 //startSSM
 setomSSM(trigboard,0xb);startSSM1(1);   // IN, continuous
 if(intboard){
  setomSSM(4,0x3);startSSM1(4);  // OUT, continuous
  initprintBCs();
 }
 usleep(100000);
 /* todo:
  -we want counters content every 2 seconds
  -we want to trigger on lhcpp i.e. we should start to check
   for lhcpp when we are close to it (every 40 secs) 
  -we want to trigger on counter change (lhcpp or counter change)
 */
 while(1){
    int rc;
    if(inpnum==0) {  // we trigger on lhcpp
      rc= getlhcpp(vspbobr,1, PP_PERIOD, &lhcpp); 
      if(rc!=0) {
        printf("No LHCpp after %d secs. getlhcpp() rc:%d\n", PP_PERIOD, rc);
        trigcond=0;
      } else {
        trigcond=1;
      };
    } else {         // we trigger on CTP inputs counter change
      getCountersBoard(trigboard,countermax,last,3); 
      time=last[timeadr]; trig=last[counteroffset+inpnum];
      trigcond= (trig != trigold);
    };
    //printf("inpnum bobr: %i %x\n",inpnum,(lhcpp.Byte54));
    //beep=last[counteroffset+BEEPPOS];
    //printf("trig: %i old %u new %u \n",inpnum,trigold,trig);
    if(trigcond){
      //beepni(); cicolino not used recently
      getSSMs(trigboard, inpnum,intboard,f);   // to be checked: what happens if inpnum=0?
      getCountersBoard(trigboard,countermax,last,3); 
      time=last[timeadr]; trig=last[counteroffset+inpnum];
      trigold=trig;
      //countersRead();
      //break;
    };
    if(quit!=0) {
      // the request 'stop smaqing' registered (signal -s SIGUSR1 pid), let's stop
      fprintf(f, "quitting on signal:%d\n", quit);
      break;
    };
    usleep(1000); // was 200 at the start of Aug (can be much more for 1bobr/48 secs)
    timediff=getTime(timeold,time);
    //printf("time: old %u new %u diff %f\n",timeold,time,timediff); 
    if(timediff>1000.*2){   // per 1 secs
      //char dt[32];
      //smaqprintCounters(trigboard,L0counts, inpnum, f); 
      //getdatetime(dt);
      //dt[10]='_';
      //printf("time: diff %f %s\n",timediff,dt);
      timeold=time;
    }    
 }
 if(inpnum==0) bobrClose(vspbobr);
 fclose(f);
 return 0;
}
/*-----------------------------------------------------------
*/
void initSMAQ()
{
 int rc;
 //int vsp=-1;
 //rc= vmxopen(&vsp,"0x820000", "0xd000");
 rc= vmeopen("0x820000", "0xd000");
 if(rc!=0) {
  printf("vmeopen CTP vme:%d\n", rc); exit(8);
 };
ctpshmbase= (Tctpshm *)mallocShared(CTPSHMKEY, sizeof(Tctpshm), &ctpsegid);
//ctpshmbase= (Tctpshm *)malloc( sizeof(Tctpshm));
validCTPINPUTs= &ctpshmbase->validCTPINPUTs[0];
validLTUs= &ctpshmbase->validLTUs[0];

 checkCTP();
 initSSM();
} 
/*********************************************************
*/
int main(int argc, char **argv) {
 char *datadir;
 int inpnum;
 //setseeds(3,3); 
 if(argc != 2){
  printf("Expected: one argument - input number \n");
  return 1;
 }
 inpnum = atoi(argv[1]);
 if((inpnum<1 )|| (inpnum>48)){
  printf("Expected: 0 < input number <49 \n");
  return 2;
 }
signal(SIGUSR1, gotsignal); siginterrupt(SIGUSR1, 0);
 initSMAQ();
 datadir= getenv("SMAQDATA");
 if(datadir !=NULL) {
   strcpy(dirname, datadir);
 };
 inputsSMAQ(0,inpnum);
 vmeclose();
 return 0;
}

