/* fanio DIM server
Commands -common for all detectors (detector specified in the message):
        IO    message
ENABLE  IN    detname,mask
SAVE    IN    detname,mask
REFRESH IN    detname       added 20.10.2014

Services (one per detector):
                 IO   response
detname/STATUS   IN   enadis,current
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#ifdef CPLUSPLUS
#include <dis.hxx>
#else
#include <dis.h>
#endif
#include "../fanio.h"
#include "vmewrap.h"

#define MAXCMDL 200
#define MAXINPUTS 5

typedef struct {  // ENABLE and SAVE message
  char name[16];  // detname
  unsigned int mask;
} Tenablemsg;
typedef struct {  // detname/STATUS response
  unsigned int ed;
  unsigned int busy;
} Tstatusmsg;
typedef struct {
  w32 xCONTROL_REG;
  w32 xBUSY_MASK;
  w32 xREAD_INPUTS;
} Tsim;
Tsim sim;

Tstatusmsg statusmsg={0xabcd, 0x1234};

typedef struct {
  char name[16];  // detname
  char base[16];  // VME base address 
  w32 defenable;  // default for BUSY_MASK regiser
  unsigned int dimserid;
  char io;        // 'i' or 'o'
} Tfanin;
#define Nfanios 7   // all fan in/outs
#define Nfanis 4    // only fanins
Tfanin fanis[Nfanios]= {
  {"ssd",  "0xe00000", 0x6cf00, 0, 'i'},
  {"daq",  "0x980000", 0x1, 0, 'i'},
  //{"fmd",  "0xa80000", 0, 0, 'i'},
  {"hmpid","0x200000", 0x03fff, 0, 'i'},
  {"ssd",  "0xf00000", 0, 0, 'o'},
  {"fmd",  "0x680000", 0, 0, 'o'},
  {"hmpid","0x100000", 0, 0, 'o'}
};

/*----------------------------------- start of DETECTOR specific code */
/*----------------------*/ void HW_init_common(char *base) {
int rc;
#ifdef SIMHW
rc=0; sim.xCONTROL_REG=0;
#else
rc= vmeopen(base, "0x200");
//if(rc!=0) { printf("vmeopen rc:%d base:%s\n", rc, base); return; };
vmew32(CONTROL_REG, 0);
printf("HW_init_common:%s\n", base);
rc= vmeclose();
#endif
if(rc!=0) { printf("vmeclose rc:%d base:%s\n", rc, base); };
fflush(stdout);
}
/*----------------------*/ void HW_init_in(char *base, w32 enabled) {
int rc;
#ifdef SIMHW
rc=0; sim.xBUSY_MASK=enabled;
#else
rc= vmeopen(base, "0x200");
//if(rc!=0) { printf("vmeopen rc:%d base:%s\n", rc, base); return; };
vmew32(BUSY_MASK, enabled);
printf("HW_init_in: %s enabled:0x%x\n", base, enabled);
rc= vmeclose();
#endif
if(rc!=0) { printf("vmeclose rc:%d base:%s\n", rc, base); };
fflush(stdout);
}
/*----------------------*/ void HW_enable(char *base, w32 mask) {
int rc;
#ifdef SIMHW
rc=0; sim.xBUSY_MASK=mask;
#else
rc= vmeopen(base, "0x200");
if(rc!=0) { printf("vmeopen rc:%d base:%s\n", rc, base); return; };
vmew32(BUSY_MASK, mask);
printf("HW_enable: %s mask:0x%x\n", base, mask);
rc= vmeclose();
#endif
if(rc!=0) { printf("ERROR vmeclose rc:%d base:%s\n", rc, base); };
fflush(stdout);
}
/*----------------------*/ void HW_getmask(char *base, Tstatusmsg *msg) {
w32 ed=0xdeadbeaf;
w32 busy=0xbeaf;
int rc;
#ifdef SIMHW
rc=0; 
ed= sim.xBUSY_MASK;
busy= ~sim.xBUSY_MASK; //busy= sim.READ_INPUTS;
printf("HW_getmask: %s ed:0x%x busy:0x%x\n", base, ed, busy);
msg->ed=ed; msg->busy=busy;
#else
rc= vmeopen(base, "0x200");
if(rc!=0) { printf("vmeopen rc:%d base:%s\n", rc, base); return; };
ed= vmer32(BUSY_MASK); busy= vmer32(READ_INPUTS);
//printf("HW_getmask: %s ed:0x%x busy:0x%x\n", base, ed, busy);
msg->ed=ed; msg->busy=busy;
rc= vmeclose();
#endif
if(rc!=0) { printf("vmeclose rc:%d base:%s\n", rc, base); };
}
/*----------------------------------- end of DETECTOR specific code */

int findbase(char *name, char *base) {
/* find base of FANIN */
int ix;
strcpy(base, "error");
for(ix=0; ix<Nfanios; ix++) {
  if( (strcmp(fanis[ix].name,name)==0) && (fanis[ix].io=='i') ) {
    strcpy(base, fanis[ix].base);
    return(ix);
  };
};
return(-1);
}

/*-----------------*/ void set_save(void *tag, void *msgv, int *size)  {  
FILE *cfgfile;
Tenablemsg *msg= (Tenablemsg *)msgv;
cfgfile=fopen(msg->name, "w");
if(cfgfile==NULL) {
  printf("cfg file %s cannot be opened for write\n", msg->name);
  return;
};
fprintf(cfgfile, "0x%x\n", msg->mask);
printf("Default BUSY_MASK:0x%x saved for %s\n", msg->mask, msg->name);
fclose(cfgfile);
fflush(stdout);
}
/*------------------*/ void set_oc(void *tag, void *msgv, int *size)  {  
Tenablemsg *msg= (Tenablemsg *)msgv;
int nclients,ixt; char base[12];
ixt= findbase(msg->name, base);
HW_enable(base, msg->mask);
nclients= dis_update_service(fanis[ixt].dimserid);
printf("set_oc: name:%s mask:%x size:%d updated clients:%d\n", msg->name, msg->mask, *size, nclients);
fflush(stdout);
}  
/*------------------*/ void refresh(void *tag, void *msgv, int *size)  {  
Tenablemsg *msg= (Tenablemsg *)msgv;
int nclients,ixt; char base[12];
ixt= findbase(msg->name, base);
//HW_enable(base, msg->mask);
nclients= dis_update_service(fanis[ixt].dimserid);
printf("refresh: name:%s size:%d updated clients:%d\n", msg->name, *size, nclients);
fflush(stdout);
}  

/*----------*/ void get_oc(void *tag,  void **msgv, int *size, int *blabla) {
Tstatusmsg **msg= (Tstatusmsg **)msgv;
//printf("get_oc: tag:%d\n", *(int *)tag);
int itag; itag= *(int *)tag;
//printf("get_oc: tag:%d det:%s\n", itag, fanis[itag].name);
*msg= &statusmsg;
*size= sizeof(statusmsg);
HW_getmask(fanis[itag].base, *msg);
//printf("get_oc: ed:0x%x busy:0x%x size:%d\n", (*msg)->ed, (*msg)->busy, *size); fflush(stdout);
}

int main()  {
int ix, rc;
char command[MAXCMDL];
#ifdef SIMHW
printf("Commands:   SIMHW mode.\n");
#else
printf("Commands:\n");
#endif
strcpy(command, "ENABLE");
//dis_add_cmnd(command,NULL, set_oc, 18);  printf("%s\n", command);
dis_add_cmnd(command,"C:16;I:1", set_oc, 18);  printf("%s\n", command);
strcpy(command, "REFRESH");
dis_add_cmnd(command,"C:16", refresh, 20);  printf("%s\n", command);
strcpy(command, "SAVE");
//dis_add_cmnd(command,NULL set_save, 18);  printf("%s\n", command);
dis_add_cmnd(command,"C:16;I:1", set_save, 19);  printf("%s\n", command);
fflush(stdout);
printf("Services:\n");
for(ix=0; ix<Nfanios; ix++) {
  w32 defaultenable;
  HW_init_common(fanis[ix].base);
  if( fanis[ix].io=='o') continue;     // enough for FANOUT
  if(access(fanis[ix].name, W_OK)==0) {
    FILE *cfgfile;
    cfgfile=fopen(fanis[ix].name,"r");
    fscanf(cfgfile, "0x%x", &fanis[ix].defenable);
    fclose(cfgfile);
  };
  defaultenable= fanis[ix].defenable;
  HW_init_in(fanis[ix].base, defaultenable);
  strcpy(command, fanis[ix].name); strcat(command, "/STATUS");
  //dis_add_service(command,NULL, &statusmsg, sizeof(statusmsg), get_oc, ix);  
  fanis[ix].dimserid= dis_add_service(command,"I:1;I:1", &statusmsg, sizeof(statusmsg), get_oc, ix);  
  printf("%s\n", command);
}
rc= dis_start_serving("fanio");
if(rc==1) {
  printf("serving...\n");
} else {
  printf("cannot register on name server\n");
  exit(4);
};
while(1) {
  sleep(10);
}; return(0);
}   

