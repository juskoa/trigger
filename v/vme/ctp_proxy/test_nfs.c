#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <semaphore.h>
#include "vmewrap.h"
#include "vmeblib.h"
#include "lexan.h"
#include "infolog.h"
#include "ctp.h"
#include "ctplib.h"
#include "Tpartition.h"

int quit=0;
/*---------------------------------------------*/ void gotsignal(int signum) {
char msg[100]; int rc;
// SIGUSR1:  // kill -s USR1 pid
signal(signum, gotsignal); siginterrupt(signum, 0);
sprintf(msg, "got signal:%d", signum);
prtLog(msg);
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
void printHelp() {
printf(
"o  -open/read/close $dbctp/filter file d\n\
:\n");
};
void orc() {
FILE *ffile;
char cline[MAXLINELENGTH];
ffile=openFile("filter","r");
if(ffile == NULL) {
  printf("filter not present...\n"); return;
};
printf("filter:\n");
while(fgets(cline, MAXLINELENGTH, ffile)){
  printf(cline);
}; fclose(ffile);
}
//-------------------------------------------
int main(int argc, char **argv){
//int rc;
char c; char pname[20];
//setlinebuf(stdout);   // see swtrcheck.py
signal(SIGUSR1, gotsignal); siginterrupt(SIGUSR1, 0);
signal(SIGQUIT, gotsignal); siginterrupt(SIGQUIT, 0);
signal(SIGINT, gotsignal); siginterrupt(SIGINT, 0);
signal(SIGTERM, gotsignal); siginterrupt(SIGTERM, 0);
signal(SIGBUS, gotsignal); siginterrupt(SIGBUS, 0);

printf("...\n");
//infolog_SetFacility("CTP"); infolog_SetStream("",0);
//setglobalflags(argc, argv);
while(1) {
  printHelp();
  if((c = mygetchar()) == 'q') break;
  switch(c){
   case 'o':
       orc();
       break;
   case 'l':
        entername("Partition name:", pname);
        printf("Loading partition %s  enter mask (integer or 0xHHH..):",pname);
        break;
   default: 
         printf("Unknown command %c \n",c);
  }
 }
 return 0;
}
