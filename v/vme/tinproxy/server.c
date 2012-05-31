#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#ifdef CPLUSPLUS
#include <dis.hxx>
#else
#include <dis.h>
#endif

#define MAXCMDL 200
#define MAXCTPINPUTS 5
#define MYDETNAME "SPD"
int QUIT=0;
char state[32]="notfilledyet";   // SMI state

/* StatusString[n] holds information about the status of
   physical signal:
N -normal operation
T -sending toggle
S -sending signature
R -the random generator is active instead of normal CTP input signal
   (was I -signal is disabled, which is obsolete)
E -the signal is in 'error' state
*/
char StatusString[MAXCTPINPUTS+1];   // +1:for \0 (end of string)

/*----------------------------------- start of DETECTOR specific code */
static int Delay=0xdeadbeaf;
static int dethw[MAXCTPINPUTS];
void TRGDET_cntl(int ctpin, int oc) {
dethw[ctpin-1]= oc;
/*--------------------------------------------------
 Here set option code oc for ctp input ctpin (1,...) in your FE... 
----------------------------------------------------*/
}
int TRGDET_status(int ctpin) {
/* ctpin: 1,...
   rc: 0,1,2,3 or -1 in case of error
*/
  /*--------------------------------------------------
   Here include the program code reading 2 bits of Option code (for 
   trigger ctpin) from your front end trigger logic. 
  */
return dethw[ctpin-1];
}
void TRGDET_setdelay(int delay) {
  /*--------------------------------------------------
   Here include the program code setting delay
  */
Delay= delay;
}
int TRGDET_getdelay() {
  /*--------------------------------------------------
   Here include the program code reading delay
  */
return(Delay);
}
/*----------------------------------- end of DETECTOR specific code */

/* check the input parameters: 
RC: 0 input parameters ok
    1 error on stdout */
int checkn(char *cmdname, int *n2, int tag, int size) {
char cmderr[MAXCMDL];
int rc;
printf("opion code:%d input:%d\n", n2[0], n2[1]);
if( (n2[1]>MAXCTPINPUTS) || (n2[1]<=0) ) {
  sprintf(cmderr, "Bad triggering signal number %d. Expected:1 - %d.",
     n2[1], MAXCTPINPUTS);
  rc=1;
} else {
  strcpy(cmderr,cmdname);
  rc=0;
};
printf("%s: optioncode:%d inp:%d  tag:%d size:%d\n", 
  cmderr, n2[0], n2[1], tag, size);
return(rc);
}

/* int -> char conversion.
Input: Option code (0,1,2,3)
Output: NTSR
*/
char oc2a(int opcode) {
char occ;
switch (opcode) {
case 0: occ='N'; break;   // Normal
case 1: occ='T'; break;   // Toggle
case 2: occ='S'; break;   // Signature
case 3: occ='R'; break;   // Random
};
return(occ);
}

void set_oc(void *tagv, void *n2v, int *size)  {  
/* Input parameters:
n2[0] -required option code (0,1,2 or 3)
n2[1] -CTP input (1,2,...MAXCTPINPUTS)
*/
long *tag= (long *)tagv;
int *n2= (int *)n2v;
if( checkn("SET_OPTIONCODE", n2, *tag, *size) ) return;

TRGDET_cntl(n2[1], n2[0]);
StatusString[n2[1]-1]=oc2a(n2[0]);
}  
void set_delay(void *tag, void *delay, int *size)  {  
/* Input parameters:
delay: delay to be set
*/
TRGDET_setdelay(*(int *)delay);
}  

void get_oc(void *tag,  void **msgp, int *size, int *blabla) {
int i, opcode;
char localStatusString[MAXCTPINPUTS+1];   // +1:for \0 (end of string)

for(i=0; i<MAXCTPINPUTS; i++) {
  opcode= TRGDET_status(i+1);
  localStatusString[i]= oc2a(opcode);
  if(localStatusString[i]!=StatusString[i]) {
     printf("Internal error. Expected status %c for input %d\n\
but got %c\n", StatusString[i], i+1, localStatusString[i]);
       StatusString[i]= localStatusString[i];
  };
};
*msgp= (void *)StatusString;
*size= strlen(StatusString)+1;   // "" -empty string would be 1 byte message
//printf("get_oc size:%d msg:%s\n", *size, *(char *)msgp);
printf("get_oc size:%d msg:%s\n", *size, (char *)*msgp);
}
void get_delay(void *tag,  void **delay, int *size, int *blabla) {
*size=4;
Delay=TRGDET_getdelay();
*delay= &Delay;
}

void gotsignal(int signum) {
signal(SIGINT, gotsignal); siginterrupt(SIGINT, 0);
printf("got SIGINT signal, quitting...:%d\n", signum);
QUIT=signum;
}
main()  {
int i;
int statusid, delayid;
char command[MAXCMDL];
signal(SIGINT, gotsignal); siginterrupt(SIGINT, 0);

for(i=0; i<MAXCTPINPUTS; i++) {   
  TRGDET_cntl(i+1, 0);
  StatusString[i]='N';   /* normal */
};
printf("DETECTOR_NAME:%s\n",MYDETNAME);
printf("Commands/services:\n");
strcpy(command, MYDETNAME); strcat(command, "/SET_OPTIONCODE");
dis_add_cmnd(command,"L:2", set_oc, 18);  
printf("%s\n", command);
strcpy(command, MYDETNAME); strcat(command, "/SET_DELAY");
dis_add_cmnd(command,"L:1", set_delay, 18);  
printf("%s\n", command);

strcpy(command, MYDETNAME); strcat(command, "/STATUS_OPTIONCODE");
statusid= dis_add_service(command,"C", StatusString, MAXCTPINPUTS+1, get_oc, 4567);  
printf("%s\n", command);
strcpy(command, MYDETNAME); strcat(command, "/STATUS_DELAY");
delayid= dis_add_service(command,"L", (void *)&Delay, MAXCTPINPUTS+1, get_delay, 4567);  
printf("%s\n", command);

dis_start_serving(MYDETNAME);  
printf("serving...\n");
printf("Status of CTP inputs:%s\n",StatusString);

while(1) {
  if(QUIT!=0) break;
  sleep(2);  
};  
printf("stop serving...\n");
dis_remove_service(statusid);
dis_remove_service(delayid);
dis_stop_serving();
}   

