#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef CPLUSPLUS
#include <dic.hxx>
#else
#include <dic.h>
#endif
 
#define OCCMD "/STATUS_OPTIONCODE"
#define STATLEN strlen(OCCMD)
#define DELAYCMD "/STATUS_DELAY"
#define DELAYLEN strlen(DELAYCMD)
#define MAXCTPINPUTS 10

char cmd[80];
char ctpinput[8];
char ocode[8];

/* T:toggling, S:sending signature, N:normal, X: failed */
char StatusString[MAXCTPINPUTS];   
char StatusFailed[MAXCTPINPUTS];   /* /STATUS service failed */
int Delay=-3;
int StatusDelayFailed=-2;

void callback(void *tag, int *rc) {
printf("callback tag:%d rc:%d\n", *(int *)tag, *rc);
if(*rc == 1) {
  printf("%s OK, ocode:%s ctpinput:%s\n",cmd,ocode, ctpinput);
} else {
  printf("%s %s %s not executed by server\n",cmd,ocode, ctpinput);
};
}
void callbackDelay(void *tag, void *buf, int *size) {
printf("callbackDelay tag:%d size:%d\n", *(int *)tag, *size);
if(*size == sizeof(int)) {
  printf("%s OK, *buf:%d Delay:%d\n", cmd, *(int *)buf, Delay);
} else {
  printf("%s not executed by server, size:%d\n", cmd, *size);
};
}

int main(int argc, char **argv) {
int i,rc,ixslash;
if((argc<2) || (argc>4)) {
  printf("Start client by one of the following commands (TheDETECTOR_NAME for test is SPD):\n\
\n\
1.\n\
client DETECTOR_NAME/SET_OPTIONCODE CODE N\n\
where: DETECTOR_NAME is SPD,MUON_TRG,EMC... (i.e. capitals)\n\
       CODE          is required option code:\n\
          0   Normal\n\
          1   Toggling\n\
          2   Signature\n\
          3   Random \n\
       N             is DIM channel number (1,2,...) -default is 1\n\
\n\
2.\n\
client DETECTOR_NAME/STATUS_OPTIONCODE\n\
Status will be returned as string, 1 letter for each provided channel:\n\
N: Normal   T: Toggling   S: Signature   R: Random\n\
\n\
3. (optional)\n\
client DETECTOR_NAME/SET_DELAY delay\n\
where: delay is non negative integer number \n\
4.(optional)\n\
client DETECTOR_NAME/STATUS_DELAY\n\
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
  rc= dic_info_service(cmd, ONCE_ONLY, 2, StatusString,MAXCTPINPUTS+1, 
      NULL, 3488, StatusFailed, MAXCTPINPUTS+1);
  usleep(1000000);
  printf("%s %s length:%d\n",cmd, StatusString, strlen(StatusString));
} else if( (strlen(cmd)>DELAYLEN) &&      // get Delay status
    (strcmp(&cmd[strlen(cmd)-DELAYLEN], DELAYCMD)==0) ) {
  /* service */
  rc= dic_info_service(cmd, ONCE_ONLY, 2, &Delay ,sizeof(int), 
      callbackDelay, 3488, &StatusDelayFailed, sizeof(int));
  usleep(1000000);
  printf("cmd:%s Delay:%d\n",cmd, Delay);
} else {                           // send command
  /* command */
  /* rc= dic_cmnd_callback(cmd, ctpinput, strlen(ctpinput)+1, callback, 33);*/
  int ocinp[2];
  ocinp[0]=atoi(ocode); ocinp[1]=atoi(ctpinput);
  rc= dic_cmnd_callback(cmd, &ocinp[0], 8, callback, 33);
};
/*printf("rc:%d\n",rc); */
sleep(2);
} 
