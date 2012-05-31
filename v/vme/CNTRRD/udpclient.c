/* udpclient.c 
nohup linux/udpclient mon 60 &

*/
#include <stdio.h>
#include <stdlib.h>   // exit
#include <unistd.h>   // sleep
#include <string.h>
#include <math.h>     //sin
#include "udplib.h"

int main(int argn, char **argv) {
char *inl;
char *eof;
int sock; int ixmin=0; int seconds=60;
//FILE *ifile;
char line[100]; char rrdname[40];
if(argn<2) {
  printf("linux/udpclient rrdname [seconds]\n\
rrdname: rrddb name (mon)\n\
seconds: if present start endless loop updating ds001:ds002 with given interval\n\
");
  return(0);
};
strcpy(rrdname, argv[1]); printf("rrddb:%s\n",rrdname);
if(argn>2) {
  seconds= atoi(argv[2]);
  if(seconds<1) seconds=60;
  printf("Automatic update every %d secs\n", seconds);
};
//sock= udpopens("pcalicebhm05", send2PORT);
sock= udpopens("localhost", 9931);
if(sock==-1) {printf("udpopens error\n"); exit(8); };
while(1) {
  int rc;
  char buffer[500];
  ixmin++;
  if(argn>2) {
    sprintf(buffer, "%s ds001:ds002 N:%d:%f", rrdname, ixmin, 
      sin(ixmin/(360./3.1415)));
    sleep(seconds); goto SEND;
  };
  // interactive
  printf("ds001:ds002 N:val1:U   or ENTER:repeat last cmd   orquit\n");
  eof=fgets(line, 100, stdin);
  if(eof==NULL) break;
  inl= index(line, '\n');
  if( inl==NULL) {
    printf("bad mesage (NL missing)\n"); continue;
  };
  if(line[0]!='\n') {    // ENTER:repeat last cmd
    if(strncmp(line, "quit ",5)==0) break;
    *inl='\0';
    sprintf(buffer,"%s %s", rrdname, line);
  };
  SEND:fflush(stdout);
  rc= udpsend(sock, (unsigned char *)buffer, strlen(buffer)+1);
  //printf("rc:%d\n", rc);
};
return(0);
}

