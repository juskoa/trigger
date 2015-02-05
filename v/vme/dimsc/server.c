#include <stdio.h>
#include <unistd.h>
#include <string.h>
#ifdef CPLUSPLUS
//#include <smirtl.hxx>
#include <dic.hxx>
#else
//#include <smirtl.h>
#include <dic.h>
//
#endif
#define MAXCMDL 200
#define MAXCTPINPUTS 5
#define MYDETNAME "ABC"

char state[32]="notfilledyet";   // SMI state
char obj[128];

/* StatusString[n] holds information about the status of
   physical signal:
N -normal operation
T -sending toggle
S -sending signature
R -the random generator is active instead of normal CTP input signal
E -the signal is in 'error' state
*/
char StatusString[MAXCTPINPUTS+1];   // +1:for \0 (end of string)
int Delay=-1;

/*----------------------------------- start of DETECTOR specific code */
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
int TRGDET_getdelay() {
return(Delay);
}
void TRGDET_setdelay(int delay) {
Delay= delay;
}
/*----------------------------------- end of DETECTOR specific code */
/*von
void setsmi(char *newstate) {
// smi_set_state only in case of status or state change 
static char oldSTATUS[MAXCTPINPUTS+1]="";
if( (strcmp(newstate,state)!= 0) ||
     (strcmp(oldSTATUS,StatusString)!=0) ) {
  strcpy(state, newstate); 
  strcpy(oldSTATUS,StatusString);
  smi_set_par("CTP_INPUTS", StatusString, STRING);
  smi_set_state(state);
};
}
*/
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

void set_oc(void *tag, void *n2v, int *size)  {  
/* Input parameters:
n2[0] -required option code (0,1,2 or 3)
n2[1] -CTP input (1,2,...MAXCTPINPUTS)
*/
int *n2=(int *) n2v;
if( checkn((char *)"SET_OPTIONCODE", (int *)n2, *(int *)tag, *size) ) return;

TRGDET_cntl(n2[1], n2[0]);
StatusString[n2[1]-1]=oc2a(n2[0]);
}  
void set_delay(void *tag, void *delay, int *size)  {  
/* Input parameters:
delay: delay to be set
*/
TRGDET_setdelay(*(int *)delay);
}  

void get_oc(void *tag,  void **msgpv, int *size, int *blabla) {
int i, opcode;
char **msgp= (char **)msgpv;
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
*msgp= StatusString;
*size= strlen(StatusString)+1;   // "" -empty string would be 1 byte message
printf("get_oc size:%d msg:%s\n", *size, *msgp);
}
void get_delay(void *tag,  void **delayv, int *size, int *blabla) {
int rcdelay;
int **delay= (int **) delayv;
printf("get_delay tag: %d delayold:%d sizeold:%d\n", *(int *)tag, 
  *delay, *size);
rcdelay=TRGDET_getdelay();
*delay= &Delay;
*size=sizeof(int);
}

void SMI_handle_command() {
// no SMI commands foreseen for the time being
}

main()  {
int i;
int notReady, errorCondition;
char command[MAXCMDL];
for(i=0; i<MAXCTPINPUTS; i++) {   
  TRGDET_cntl(i+1, 0);
  StatusString[i]='N';   /* normal */
};
printf("DETECTOR_NAME:%s\n",MYDETNAME);
printf("Commands/services:\n");
strcpy(command, MYDETNAME); strcat(command, "/SET_OPTIONCODE");
dis_add_cmnd(command,(char *)"I:2", set_oc, 18);  
printf("%s\n", command);
strcpy(command, MYDETNAME); strcat(command, "/SET_DELAY");
dis_add_cmnd(command,(char *)"I:1", set_delay, 18);  
printf("%s\n", command);

strcpy(command, MYDETNAME); strcat(command, "/STATUS_OPTIONCODE");
dis_add_service(command,(char *)"C", StatusString, MAXCTPINPUTS+1, get_oc, 4567);  
printf("%s\n", command);
strcpy(command, MYDETNAME); strcat(command, "/STATUS_DELAY");
dis_add_service(command,(char *)"I:1", &Delay, sizeof(int), get_delay, 4568);  
printf("%s\n", command);
/*von
sprintf(obj,"TRIGGER::TIN-%s",MYDETNAME);
smi_attach(obj, SMI_handle_command);
setsmi("READY");
*/
dis_start_serving(MYDETNAME);  
printf("serving...\n");
printf("Status of CTP inputs:%s\n",StatusString);
/*
The following code sets the SMI status to ERROR, READY, or NOT_READY
It sets it to ERROR under the control of some external event: 
   in this example the ERROR state is triggered by the existence 
   of a "/tmp/goToError" file. The content of this file is the number
   of input, which becomes errorneous for the time, when this file
   exists. E.g.:
   echo 1 >/tmp/goToError
   echo 2 >/tmp/goToError
   rm /tmp/goToError

Otherwise it sets it to:
NOT_READY if at least 1 input is NOT_READY
READY     if ALL inputs are valid (Normal)
ERROR     ? (probably internal error)
          SHOULD NOT HAPPEN
*/
#define ERRFILE "/tmp/goToError"
#define NINPSTR 30
while(1) {
  int i, validinputs;
  if (errorCondition=access(ERRFILE,F_OK) == 0) {   // errorneous input
    FILE *errfile;
    int inp;
    char inpstr[NINPSTR];
    errfile= fopen(ERRFILE,"r");
    fgets(inpstr, NINPSTR, errfile);
    inp=atoi(inpstr);
    if((inp>0) && (inp<MAXCTPINPUTS)) {
      if(StatusString[inp-1]!='E') {
        printf("Input %d goes to ERROR state.\n",inp);
      };
      StatusString[inp-1]='E';
    } else {
      printf("/tmp/goToError contains incorrect number:%d\n", inp);
      //von setsmi("ERROR"); goto STATESET;
    };
    fclose(errfile);
  } else {            // ERRFILE doesn't exist i.e. all inputs are OK:
    for(i=0; i<MAXCTPINPUTS; i++) {   
      if(StatusString[i]=='E') {
        StatusString[i]='N';
        printf("Input %d: ERROR -> NORMAL\n",i+1);
      }; 
    };
  };
  /*von goto NOT_READY  if at least 1 input is in error state:
  validinputs=0;
  for(i=0; i<MAXCTPINPUTS; i++) {   
    if(StatusString[i]=='E') {
      continue;
    };
    if(StatusString[i]=='N') {
      validinputs++;
      continue;
    };
  };
  if(validinputs==MAXCTPINPUTS) {
    // goto READY     if ALL active inputs are valid (N) 
    setsmi("READY");
  } else {
    // goto NOT_READY if at least 1 active input is not valid 
    setsmi("NOT_READY");
  }; */
  STATESET:
  printf("Status:%s\n",StatusString);
  sleep(1);  
};  
}   

