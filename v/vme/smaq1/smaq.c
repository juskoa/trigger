#include <stdio.h>
#include <unistd.h>    /* usleep */
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>

#include "vmewrap.h"
#include "../ctp/ctplib/ctplib.h"
#include "../ctp/ctp.h"
#define DBMAIN
#include "../ctp_proxy/Tpartition.h"
#include "ssmctp.h"

char dirname[40]="last";
int siteflag=0;
void getdatetime(char *);
int dumpssm(int board,char *filename);
int syncSSM(int n, int *boards);
w32 orbitstatus;
/////////////////////////////////////////////////////////////////////////////////////
#define MAXCOUNTERS 160
#define NINP 24
#define L0OFFSET 65 
#define L0TIMEOFFSET 13
#define L1TIMEOFFSET 5
#define L1OFFSET 5
int inpLX[NINP];
double getTime(w32 oldval,w32 newval){
 w32 diff;
 double milisecs;
 diff=dodif32(oldval,newval);
 milisecs=diff*0.4/1000.;
 return milisecs;
}
//---------------------------------------------------------------------------------
char inpLXNames[NINP][32];
void initNames(){
 //  Choose inputs HERE !
 //            1  2  3  4  5  6  7
 int inpCH[]={ 1,2,3,4,10,21,0};
 int i,j;
 readTables();
 printf("L1 inputs: ");
 j=0;
 for(i=0;i<NCTPINPUTS;i++){
    if(validCTPINPUTs[i].level==1){
      strcpy(inpLXNames[j],validCTPINPUTs[i].name);
      printf("%s ",inpLXNames[j]);
      j++;
    }
 }
 printf("\n");
 for(i=0;i<NINP;i++)inpLX[i]=0;
 for(i=0;i<NINP;i++){
    if(inpCH[i] == 0) break;
    if(inpCH[i])inpLX[inpCH[i]-1]=1;
 }
}
//------------------------------------------------------------------
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
//--------------------------------------------------------------------------------
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
//----------------------------------------------------------------------------------------
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
  if(inpLX[i] == 0) continue;
  fprintf(f,"Corel %s x %s:",inpLXNames[inp-1],inpLXNames[i]);
  for(j=0;j<NCOREL;j++)fprintf(f, "%i ",corel[j][i]);
  fprintf(f, "\n");
 }
 prt=0;
 for(i=0;i<3564;i++) { if(prt>100) break; ;  // print only first 100
   if(bcl0[i]) { prt++; fprintf(f,"BC at L0:%4i %i\n",i,bcl0[i]); };
 };
 return 0;
}
//-----------------------------------------------------------------------------------------`
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
  if(inpLX[i]){
    if(chans[i])sprintf(channels,"%s L0[%i]:%i",channels,i+1,chans[i]);
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
//---------------------------------------------------------------------------------------------------
/*
   get ssms of l1 board and board board2
*/
int getSSMs(int inpnum,int board,FILE *f){
 char dt[32];
 char filename[256];
int rcscp; char cmd[100];
 getdatetime(dt);
 dt[10]='_';
 //dt[13]='_';
 //dt[16]='_';
 usleep(4000); // fine even for triggering with BOBR signal
 stopSSM(board);   // which is coming 19 orbits before interaction
 strcpy(filename,"");
 sprintf(filename,"l1_%i_%s.dmp",inpnum,dt);
 //dumpSSM(1,filename);
 readSSM(board);
 dumpssm(board,filename);
  /* scp: not very nice (.dmp file is copied over network 3 times:
     1. when created (WORK directory is NFS monted)
     2. when scp reads is over NFS back
     3. when scp send it to alidcscom027)
  Better idea: use pydimserver on alidcscom026: .dmp file can
  be accessed directly on 026 machine and copied to 027 machine
  */
 if(siteflag){
   sprintf(cmd, "scp -Bq2 WORK/%s trigger@alidcscom027:SMAQ/%s/", 
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
 //checkInputs2(1,f,inpnum);
 setomSSM(board,0xb);
 startSSM1(board);
 fflush(f);
 usleep(300000);
 return 0;
}
//--------------------------------------------------------------------------------------
 w32 prevRead[NINP];
 w32 firstRead[NINP];
 w32 prevtime;
int printCounters(w32 *l0, int trig, FILE *f){
 int i; w32 curtime, deltime;
 char dt[20];
 char msg[200];
 getdatetime(dt);
 //printf("%s\n",dt);
 getCountersBoard(2,L1OFFSET+NINP,l0);
 curtime= l0[L1TIMEOFFSET]; deltime=dodif32(prevtime, curtime); 
 if(trig>24) {
   sprintf(msg, "===========Triggering on %i (27:INT1 28:INT2)\n",trig);
 } else {
   sprintf(msg, "===========Triggering on %s[%i]\n",inpLXNames[trig-1],trig);
 };
 /* printf("%s", msg); */fprintf(f, "%s", msg);
 printf("counter              abs        rel       rate  \n");
 // fprintf(f,"counter              abs        rel       rate  \n");
 for(i=0;i<NINP;i++){
   if(inpLX[i]){
     int ic; w32 count, relcount, abscount;
     float rate;
     ic=L1OFFSET+i+1;
     count= l0[ic];
     relcount=dodif32(prevRead[i], count);
     abscount=dodif32(firstRead[i], count);
     prevRead[i]= count; 
     //if(i==2 ) {
     if(strncmp(inpLXNames[i], "0S", 2)==0) {
       relcount=relcount/4; abscount= abscount/4;};    // SPD 
     if(strncmp(inpLXNames[i], "BOBR", 4)==0) {
       relcount=relcount/3564; abscount= abscount/3564;};    // BOBR is 1 orbit wide
     rate= relcount*1.0E6/(deltime*0.4);
     sprintf(msg,"%s[%2i]: %10u %10u %10.1f\n",
       inpLXNames[i],i+1, abscount, relcount, rate);
     printf("%s", msg); //fprintf(f, "%s", msg);
     fflush(f);
   }
 }
 printf("\n");  //fprintf("\n");  
 prevtime= curtime;
 return 0;
}
//-------------------------------------------------------------------------------------------------
/*FGROUP SMAQ - Snapshot Memory Aquisition
  board -      1 = L0 board, 2 = L1 board, ...
  inpnum - inpnumber on l1 level to trigger on: 1..24
  Output:
         - log file - contains also BC of interaction records
         - l0 ssm dump
*/
int inputsSMAQ(int board, int inpnum){
 w32 last[MAXCOUNTERS];
 w32 l0first[MAXCOUNTERS];
 int counteroffset,countermax;
 int i,ico;
 int trigold,trig;
 int trigcond;
 FILE *f;
 char *environ;
 char fnpath[1024],logname[1024];
 char dt[32];
 initNames();
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
 counteroffset=L1OFFSET;
 countermax=counteroffset+NINP+1;  // to be fast
 //countermax=MAXCOUNTERS;
 getCountersBoard(2,L1OFFSET + NINP,l0first);
 for(i=0;i<NINP;i++){
   int ic;
   ic= L1OFFSET+i+1;
   firstRead[i]=l0first[ic]; prevRead[i]=l0first[ic]; 
 }
 // 1st readings
 getCountersBoard(board,countermax,last);
 trigold=last[counteroffset+inpnum];  //counting from 1
 //startSSM
 setomSSM(board,0xb);startSSM1(board);   // IN, continuous
 usleep(100000);
 ico=1;
 while(1){
    // we trigger on CTP inputs counter change
    getCountersBoard(board,countermax,last);
    //printf("new \n");
    //for (int i=0;i<countermax;i++)printf("%u ",last[i]);
    //printf("\n"); 
    trig=last[counteroffset+inpnum];
    trigcond= (trig != trigold);
    //printf("trig: %i old %u new %u \n",inpnum,trigold,trig);
    if(trigcond){
      getSSMs(inpnum,board,f);   // to be checked: what happens if inpnum=0?
      printf("Writing ssm %i\n",ico++);
      printf("trig: %i old %u new %u \n",inpnum,trigold,trig);
      getCountersBoard(board,countermax,last);
      trigold=last[counteroffset+inpnum];
    };
    //usleep(1000); 
 }
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
//ctpshmbase= (Tctpshm *)mallocShared(CTPSHMKEY, sizeof(Tctpshm), &ctpsegid);
ctpshmbase= (Tctpshm *)malloc( sizeof(Tctpshm));
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
 if((inpnum<1 )|| (inpnum>100)){
  printf("Expected: 0 < input number <25 \n");
  return 2;
 }
 initSMAQ();
 datadir= getenv("SMAQDATA");
 if(datadir !=NULL) {
   strcpy(dirname, datadir);
 };
 inputsSMAQ(2,inpnum);
 vmeclose();
 return 0;
}

