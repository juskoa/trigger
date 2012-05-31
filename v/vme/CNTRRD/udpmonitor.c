/* udpmonitor.c -UDP server receiving UDP packets from any process

Has to be started in ~/CNTRRD directory. i.e.:
cd ~/CNTRRD ; $VMECFDIR/CNTRRD/linux/udpmonitor
 /home/trigger/CNTRRD > nohup $VMECFDIR/CNTRRD/linux/udpmonitor >logs/udpmonitor.log &

It is waiting for messages:
rrdname ds001:ds002:ds003 N:val1:U:val2
quit
N means: NOW   u means: UNDEFINED
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   // sleep
#include <signal.h>   // signal
#include <string.h> 
#include "udplib.h" 
//#include <time.h> 

//#include "common.h"
 
#define MAXLINELE 80
#define PORTr send2PORT
#define BUFLEN 512

FILE *rrdpipe;
int QUIT=0;
//FILE *dbgout=NULL;

void gotsignal(int signum) {
signal(SIGINT, gotsignal); siginterrupt(SIGINT, 0);
printf("got SIGINT signal:%d\n", signum);
QUIT=signum;
}

/*------------------------------*/ int main(int argc, char **argv) {
int rc, udpsockr;
int ixwait=0;
//setbuf(stdout, NULL);   nebavi
signal(SIGINT, gotsignal); siginterrupt(SIGINT, 0);

rrdpipe= popen("/usr/bin/rrdtool -", "w");
if(rrdpipe==NULL) {
  printf("Cannot open /usr/bin/rrdtool -\n");
  exit(8);
};
printf("rrdpipe opened, starting udp server...\n");
udpsockr= udpopenr(PORTr); if(rc==-1) {printf("udpopenr() error\n"); exit(8);};
//setlinebuf(htmlpipe);
while(1) {
  char bufrec[BUFLEN];
  char rrdname[40];
  int ix; int nsp=0, dsstart=-1;
  if(QUIT!=0) break;
  ixwait++;
  rc= udpwaitr(udpsockr, (unsigned char *)bufrec, BUFLEN); 
  if(rc==-1) {
    printf("udpwaitr() problem %d waiting on port:%d\n",ixwait,send2PORT); 
    fflush(stdout);sleep(1); 
    continue;
  };
  printf("got %d bytes:%s\n", rc, bufrec);
  fflush(stdout);
  //rrdname ds001:ds002:ds003 N:val1:U:val2
  if(strncmp((char *)bufrec, "quit ",5)==0) break;
  rrdname[0]='\0';
  for(ix=0; ix<rc; ix++) {   // check number of spaces
    if(bufrec[ix]==' ') {                // and find rrdname
      nsp++; 
      if(dsstart==-1) {
        dsstart=ix+1;
        strncpy(rrdname, (char *)bufrec, ix);
      };
    };
  };
  if(nsp==2) {
    fprintf(rrdpipe, "update rrd/%s.rrd -t %s\n", rrdname, &bufrec[dsstart]);
    fflush(rrdpipe);   // has to be here!
  } else {
    printf("Bad message:%s nsp:%d\n", bufrec, nsp);
  };
  //sleep(100);
};
printf("closing...\n");
pclose(rrdpipe); 
udpclose(udpsockr);
//fclose(dbgout);
return(0);
} 
