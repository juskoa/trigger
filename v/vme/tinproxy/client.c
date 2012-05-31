#include <stdio.h>   // printf
#include <stdlib.h>  //exit
#include <string.h>
#ifdef CPLUSPLUS
#include <dic.hxx>
#else
#include <dic.h>
#endif
 
#define OCCMD "/STATUS_OPTIONCODE"
#define SDCMD "/STATUS_DELAY"
#define STATLEN strlen(OCCMD)
#define STATDLEN strlen(SDCMD)
#define MAXCTPINPUTS 10

char cmd[80];
char ctpinput[8];
char ocode[8];

/* T:toggling, S:sending signature, N:normal, X: failed */
char StatusString[MAXCTPINPUTS];   
char StatusFailed[MAXCTPINPUTS];   /* /STATUS service failed */

int Delay;
int Delayfailed=1000;

void callback(void *tag, int *rc) {
printf("callback tag:%d rc:%d\n", *(int *)tag, *rc);
if(*rc == 1) {
  printf("%s %s %s OK\n",cmd,ocode, ctpinput);
} else {
  printf("%s %s %s not executed by server\n",cmd,ocode, ctpinput);
};
}

int main(int argc, char **argv) {
int i,rc,ixslash;
if((argc<2) || (argc>4)) {
  printf("Start client by one of the following commands (TheDETECTOR_NAME for test is SPD):\n\
\n\
1.\n\
client DETECTOR_NAME/SET_OPTIONCODE CODE N\n\
where: DETECTOR_NAME is SPD (with given example)\n\
       CODE          is required option code 0,1,2 or 3 -> N T S or R\n\
       N             is DIM channel number (1,2,...) -default is 1\n\
\n\
2.\n\
client DETECTOR_NAME/STATUS_OPTIONCODE\n\
Status will be returned as string, 1 letter for each detector CTP input:\n\
N: Normal   T: Toggling   S: Signature   R: Random\n\
\n\
\n");
  exit(4);
};
strncpy(cmd, argv[1], 80);
for(ixslash=0; ixslash<strlen(cmd); ixslash++) {
  if(cmd[ixslash]=='/') goto OKslash;
};
printf("bad input\n"); exit(4);
OKslash:
for(i=0; i<MAXCTPINPUTS; i++) { 
  StatusString[i]='?';            // not known
  StatusFailed[i]='X';            // server failed to respond
};
if(argc==4) {                    // signal# (1,2,...)
  strncpy(ctpinput, argv[3], 2);
} else {
  strcpy(ctpinput, "1");   // default is 1
};
if(argc>=3) {                    // option code (0-3)
  strncpy(ocode, argv[2], 2);
} else {
  strcpy(ocode, "0");   // default is normal
};
if( (strlen(cmd)>STATLEN) &&      // get status
    (strcmp(&cmd[strlen(cmd)-STATLEN], OCCMD)==0) ) {
  /* service */
  rc= dic_info_service(cmd, ONCE_ONLY, 1, StatusString,MAXCTPINPUTS+1, 
      NULL, 3488, StatusFailed, MAXCTPINPUTS+1);
  usleep(1000000);
  printf("%s %s\n",cmd, StatusString);
} else if( (strlen(cmd)>STATDLEN) &&      // get status
  (strcmp(&cmd[strlen(cmd)-STATDLEN], SDCMD)==0) ) {
  rc= dic_info_service(cmd, ONCE_ONLY, 1, &Delay , 4,
      NULL, 3488, &Delayfailed, 4);
  usleep(1000000);
  printf("%s %d\n",cmd, Delay);
} else {                           // send command
  /* command */
  /* rc= dic_cmnd_callback(cmd, ctpinput, strlen(ctpinput)+1, callback, 33);*/
  int ocinp[2];
  ocinp[0]=atoi(ocode); ocinp[1]=atoi(ctpinput);
  rc= dic_cmnd_callback(cmd, &ocinp[0], 8, callback, 33);
};
/*printf("rc:%d\n",rc); */
sleep(1);
} 
