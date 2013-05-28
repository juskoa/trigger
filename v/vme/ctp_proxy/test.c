#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include "vmewrap.h"
#include "vmeblib.h"
#include "infolog.h"
#include "ctp.h"
#define DBMAIN
#include "Tpartition.h"
#include "ctp_proxy.h"

int quit=0;
void printStartedTp();

/*---------------------------------------------------- ctp_Disablernd1
Disable RND1 geerator for short time - >to enable BC triggers
*/
void ctp_Disablernd1(int usecs) {
//w32 l2anow,l2abefore,dif;
setALLDAQBusy();
//l2abefore= readCTPcnts();
vmew32(RANDOM_1, 0xffffffff);
unsetALLDAQBusy();
usleep(usecs);
setALLDAQBusy(); 
vmew32(RANDOM_1, 0);
//l2anow= readCTPcnts();
unsetALLDAQBusy();
}

/*---------------------------------------------*/ void gotsignal(int signum) {
char msg[100]; int rc;
// SIGUSR1:  // kill -s USR1 pid
signal(signum, gotsignal); siginterrupt(signum, 0);
sprintf(msg, "got signal:%d", signum);
prtLog(msg);
rc= ctp_Endproxy();
exit(8);
}

char line[100];
char mygetchar(){
char c; int i;
fgets(line, 99, stdin); c=line[0];
/*c=getchar();    -BUS ERROR */
//while((d=getchar()) != '\n');
//printf("myget %c %i \n",d,d);
for(i=0; i<=99; i++) {
  if(line[i]=='\n') line[i]='\0';
};
line[99]='\0';
return c;
}
void getmask(char *mask) {
fgets(mask, 64, stdin); mask[strlen(mask)-1]='\0';  // get rid of NL
}
void entername(char *prompt, char *pname){
  printf("%s", prompt);
  fgets(pname,80,stdin); pname[strlen(pname)-1]='\0';  // get rid of NL
  //strcpy(pname,"database/"); strcat(pname, name);
}
int run=1;
void printHelp() {
printf(
"next runN:%d\n\
Commands: l(init) s(start) f(stop) p(pause) r(resume) y(sync) N:runN++\n\
          M(init+start MUON_TRK) S(stop MUON_TRK) R(restart PHYSICS_1)\n\
          H(init+start HMPID) F(stop HMPID)\n\
          h(HW print) t(print partition)\n\
          A(print All partitions) T(print Started partitions) q(quit)\n\
:\n", run);
};
//-------------------------------------------
int main(int argc, char **argv){
 int rc;
 char c;
 char pname[64],mask[64]="0";
 Tpartition *part;
 char ACT_CONFIG[8]="YES"; // or "NO"
 //setlinebuf(stdout);   // see swtrcheck.py
signal(SIGUSR1, gotsignal); siginterrupt(SIGUSR1, 0);
signal(SIGQUIT, gotsignal); siginterrupt(SIGQUIT, 0);
signal(SIGINT, gotsignal); siginterrupt(SIGINT, 0);
signal(SIGTERM, gotsignal); siginterrupt(SIGTERM, 0);
signal(SIGBUS, gotsignal); siginterrupt(SIGBUS, 0);

printf("ctp_proxy TESTER, ACT_CONFIG:%s \n", ACT_CONFIG);
partmode[0]='\0';
infolog_SetFacility("CTP"); infolog_SetStream("",0);
cshmInit();
setglobalflags(argc, argv);
if((rc=ctp_Initproxy())!=0) exit(8);
 while(1) {
  printHelp();
  if((c = mygetchar()) == 'q') break;
  switch(c){
   case 'G':   // generate physics by disbaling rnd1
       ctp_Disablernd1(500000);
       break;
   case 'N': run=run+1; break;
   case 'M':
        strcpy(pname, "MUON_TRK");
        printf("Loading partition %s mask:\"\" run:9999\n",pname);
        prtProfTime(NULL);
        if(ctp_InitPartition(pname,"", run, ACT_CONFIG, errorReason) ){
         run++;
         printf("MUON_TRK load partition error:%s. \n", errorReason);
         break;
        };run++;
        printf("Starting partition %s\n",pname);
        ctp_StartPartition(pname, errorReason); 
        break;
   case 'S':
        strcpy(pname, "MUON_TRK");
        printf("Stoping partition %s\n",pname);
        ctp_StopPartition(pname);
        break;
   case 'R':
        strcpy(pname, "PHYSICS_1");
        printf("Stoping partition %s\n",pname);
        ctp_StopPartition(pname);
        printf("Loading partition %s mask:\"\" run:%d\n",pname,run);
        prtProfTime(NULL);
        if(ctp_InitPartition(pname,"", run, ACT_CONFIG, errorReason) ){
         run++;
         printf("%s load partition error:%s. \n", pname, errorReason);
         break;
        }; run++;
        printf("Starting partition %s\n",pname);
        ctp_StartPartition(pname, errorReason); 
        break;
   case 'H':
        strcpy(pname, "HMPID");
        printf("Loading partition %s mask:\"\" run:8888\n",pname);
        prtProfTime(NULL);
        ctp_InitPartition(pname,"",run,ACT_CONFIG, errorReason); run++;
        printf("Starting partition %s\n",pname);
        ctp_StartPartition(pname, errorReason);
        break;
   case 'l':
        entername("Partition name:", pname);
        entername("MODE:", partmode);
        printf("Loading partition %s  enter mask (integer or 0xHHH..):",pname);
        getmask(mask);
        prtProfTime(NULL);
        ctp_InitPartition(pname,mask,run,ACT_CONFIG, errorReason); run++ ;//return 1;
        break;
   case 's':
        entername("Partition name:", pname);
        printf("Starting partition %s...\n",pname);
        prtProfTime(NULL);
        ctp_StartPartition(pname, errorReason); //return 1;
        break;
   case 'F':
        strcpy(pname, "HMPID");
        printf("Stoping partition %s\n",pname);
        prtProfTime(NULL);
        ctp_StopPartition(pname);
        break;
   case 'f':
        entername("Partition name:", pname);
        printf("Stoping partition %s\n",pname);
        ctp_StopPartition(pname);
        break;
   case 'p':
   case 'y':
        entername("Partition name:", pname);
        if(c=='p') {
          ctp_PausePartition(pname);
          printf("Pausing partition %s\n",pname);
        } else {
          int i,reps=1;
          if(strlen(line)>=3) {
            reps= atoi(&line[2]);
          };
          printf("Going to send SYNC %d times\n", reps);
          for(i=0; i<reps; i++) {
            ctp_SyncPartition(pname, errorReason);
            usleep(1000000);
          };
          //printf("Sending sync for partition %s\n",pname);
        };
        break;
   case 'r':
        entername("Partition name:", pname);
        ctp_ResumePartition(pname);
        printf("Resuming partition %s\n",pname);
        break;             
   case 'h':
        printHardware(&HW,"test.c");
        break;
   case 't':             
        entername("Partition name:", pname);
        part= getPartitions(pname, AllPartitions);
        printTpartition("t command:",part);
        break;
   case 'A':
        printAllTp();
        break;
   case 'T':
        printStartedTp();
        break;
   default: 
         printf("Unknown command %c \n",c);
  }
 }
 ctp_Endproxy();
 return 0;
}
