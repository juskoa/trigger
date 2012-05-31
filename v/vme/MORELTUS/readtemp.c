/* readtemp.exe
Read temperatures of all the boards according to 
/root/NOTES/boardsincrate file. 
If any temperature is out of 'logged' range, log it to stdout
If any temperature is out of 'mailed' range, send email
10.6. CRC monitoring added
20.6. borders,time logged at the start
*/

#include <stdio.h>
#include <stdlib.h>   // system
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "vmewrap.h"
#include "ltu.h"
//#include "../CTPcommon/vmefpga.h"

int quit=0;
int buserr=0;

void gotsignal(int signum) {
switch(signum) {
case SIGUSR1:
  signal(SIGUSR1, gotsignal); siginterrupt(SIGUSR1, 0);
  printf("got SIGUSR1 signal:%d\n", signum);
  quit=signum;
  break;
case SIGBUS:
/*  vmeish(); */
/*  printf("got SIGBUS signal:%d\n", signum); */
  buserr=1;
  break;
default:
  printf("got unknown signal:%d\n", signum);
  quit=signum;
};
fflush(stdout);
}

/*------------------------------------------------------------- GetDatTim() */
void GetDatTim(char *ddmmyy, char *hhmmss) {
/*char *ddmmyy;        string[9] dd.mm.yy */
/* char *hhmmss;         ---     hh:mm:ss */
int i;
time_t T;
 struct tm *LT;  /*LocalTime */
T=time(&T); LT = localtime(&T);
sprintf(ddmmyy,"%2.0d.%2.0d.%2.0d",LT->tm_mday,LT->tm_mon+1,LT->tm_year-100);
sprintf(hhmmss,"%2.0d:%2.0d:%2.0d",LT->tm_hour,LT->tm_min+1,LT->tm_sec);
for(i=0; i<9; i++) {
  if(ddmmyy[i] == ' ') ddmmyy[i] = '0';
  if(hhmmss[i] == ' ') hhmmss[i] = '0';
};
}

char *getdatetime() {
char date[9],time[9];
static char dt[20];
GetDatTim(date, time);
strcpy(dt, date); strcat(dt," "); strcat(dt, time);
return(dt);
}
/*------------------------------------------------------- sendmail() */
void sendmail(char *msg) {
FILE *f;
char cmd[500];
char to[]="-c rl@hep.ph.bham.ac.uk \
aj@hep.ph.bham.ac.uk";
f=fopen("/tmp/ctptemp.warning","w");
fprintf(f, "%s\n",msg);
fclose(f);
sprintf(cmd,"mail -s CTP-TEMPERATURE %s </tmp/ctptemp.warning", to);
system(cmd);
}
/*------------------------------------------------------- ReadTemperature() */
/* returns temperature on the board in centigrades */
int ReadTemperature() {  /* should work with A4 version too */
w32 temp2,status;
int i,rett;
vmew32(TEMP_START, DUMMYVAL);
for(i=0; i<3; i++) {
  usleep(300);
  status=vmer32(TEMP_STATUS);
  if( (status & 0x1) == 0) goto TEMPOK;
};
printf("ReadTemperature, TEMP_STATUS.BUSY timeout:\n");
return(-100);
TEMPOK:
temp2=vmer32(TEMP_READ)&0xff;
/*printf("ReadTemperature TEMP_READ:%x\n",temp2); */
/* do the conversion from binary 2's complement */
return(temp2);
}

typedef struct {
  int min;   /* min. temperature allowed */
  int max;   /* max. temperature allowed */
} Ttemprange;
Ttemprange logged={22,30};
Ttemprange mailed={20,33};
#define MAXBOARDS 16
#define BNL 20 
typedef struct {
  char name[BNL];
  char base[10];
  int  temp;
  int crcerror;   /* 1-> CRC error */
} TActBoard;
/*------------------------------------------------------------- main() */
int main(int argn, char **argv) {
int actboards,iab;
FILE *bic;
TActBoard bds[MAXBOARDS];
bic=fopen("/root/NOTES/boardsincrate","r");
if(bic==NULL) {
  exit(8);
};
signal(SIGBUS, gotsignal); siginterrupt(SIGBUS, 0);

/* first fill in bds array: */
actboards=0;
while(1) {
#define MAXLL 100
  int rco;
  char line[MAXLL];
  char *subs;
  char base[20];
  char board[BNL];
  subs=fgets(line,MAXLL,bic);
  if(subs==NULL) break;
  board[0]='\0';
  for(rco=0; rco<BNL; rco++) {
    if(line[rco]=='=') break;
    board[rco]=line[rco]; board[rco+1]='\0';
  };
  if(strcmp(board,"ttcvi")==0) continue;
  subs= strstr(line,"=0x");
  if(subs == NULL) continue;   /* bad line */
  strncpy(base, &subs[1], 8); base[8]='\0';
  strcpy(bds[actboards].name, board);
  strcpy(bds[actboards].base, base);
  bds[actboards].temp=0;
  bds[actboards].crcerror=0;
  actboards++;
};
printf("Start:%s Logmin:%d Logmax:%d Mailmin:%d Mailmax:%d\n",getdatetime(),
  logged.min, logged.max,mailed.min,mailed.max);
for(iab=0; iab<actboards; iab++) {
  printf("%s ", bds[iab].base);
}; printf("\n"); fflush(stdout);
fclose(bic);
while(1) {
 int logit;
 logit=0;   /* don't log temperatures */
 for(iab=0; iab<actboards; iab++) {
  int rcc, temp;
  char blength[]="0x300";
  rcc= vmeopen(bds[iab].base, blength);
  if(rcc>0) {
    printf("vmeopen error:base:%s rc:%d\n",bds[iab].base,rcc);
    goto STPCLOSE;
  };
  temp=ReadTemperature();
/*  printf("%8s %8s %d\n", bds[iab].name,bds[iab].base , temp); */
  bds[iab].temp= temp;
  if(temp<logged.min || temp>logged.max) logit=1;
  if(temp<mailed.min || temp>mailed.max) {
    char msg[200];
    sprintf(msg, "%8s %8s %d", bds[iab].name,bds[iab].base , temp);
    sendmail(msg);
  };
  /* check CRC: */
  if((strcmp(bds[iab].name,"ltu")!=0) &&
     (strcmp(bds[iab].name,"l0")!=0)
	  ) {    /* only busy, fo at 10.6.2005 */
    w32 crc;
    crc= vmer32(ConfigStatus);
    if((crc & 0x40) && (bds[iab].crcerror==0)) {
      char msg[200];
      sprintf(msg, "%s %8s %8s temp:%d -CRC ON", 
        getdatetime(),bds[iab].name,bds[iab].base , temp);
      sendmail(msg);
      printf("%s\n",msg);
      bds[iab].crcerror= 1;
    };
  };
  if(buserr!=0) {
    printf("Was buserr\n"); 
    buserr=0;
    goto STPCLOSE;
  };
  rcc= vmeclose(); 
  if(rcc) {
    printf("vmeclose error:%d\n",rcc);
    goto STP;
  };
  if(quit!=0) {
    printf("quit:%d\n",quit);
    goto STP;
  };
 };
 if(logit==1) {
   printf("%s ", getdatetime());
   for(iab=0; iab<actboards; iab++) {
     printf("%2d ", bds[iab].temp);
   }; 
/*   printf("%s",crctxt); */
   printf("\n"); fflush(stdout);
 };
 sleep(60);
};
STPCLOSE:vmeclose(); 
STP: ;
}

