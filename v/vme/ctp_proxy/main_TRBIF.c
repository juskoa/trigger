#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vmeblib.h"
#include "infolog.h"
#include "vmewrap.h"
//#include "vmeblib.h"
#include "infolog.h"
#include "ctp.h"
#include "ctplib.h"
#define DBMAIN
#include "Tpartition.h"

int main(int argc, char **argv) {
TRBIF *grbif=NULL;
int rc=0;
FILE *cfgfile; char *env;
char fnpath[MAXNAMELENGTH+40];
char pname[MAXNAMELENGTH];
char errmsg[300]="";

if(argc<=1) {
  printf("./main_TRBIF part_name\n");
  return 1;
} else {
  strcpy(pname, argv[1]);
};
infolog_SetFacility("CTP"); infolog_SetStream("",0);
cshmInit();
//setglobalflags(argc, argv);
//if((rc=ctp_Initproxy())!=0) exit(8);
rc= vmeopen("0x820000", "0xd000");
if(rc!=0) {
  printf("vmeopen CTP vme:%d\n", rc); exit(8);
};
printf("main_TRBIF...\n");
checkCTP();   /* check which boards are in the crate - ctpboards */
readTables(); // enough only in ctp_proxy

env= getenv("dbctp"); 
/*strcpy(fnpath, env); strcat(fnpath, "../pardefs/");
strcat(fnpath, pname); strcat(fnpath, ".pcfg"); */
sprintf(fnpath, "%s/../pardefs/%s.pcfg", env, pname);
printf(":opening:%s:\n",fnpath);
cfgfile=fopen(fnpath,"r");
while(1) {
  TRBIF *rcgrbif=NULL;
  char *okstr; char line[MAXLINECFG];
  okstr= fgets(line, MAXLINECFG,cfgfile);
  if(okstr==NULL) break;
  if(strncmp("RBIF",line,4) == 0){
   grbif=RBIF2Partition(line,grbif);
   if(grbif == NULL) {
     sprintf(errmsg,"ParseFile: RBIF2Partition error. line:%s",line);
     rc= 1;
   };
  } else if(strncmp("L0F34",line, 5) == 0){
     //rcgrbif= L0342Partition(line, grbif);
     if(rcgrbif == NULL) {
       sprintf(errmsg,"ParseFile: L0342Partition error."); 
       rc= 1;
     };

  };
};
fclose(cfgfile); vmeclose();
if(errmsg[0]!='\0') {
  infolog_trg(LOG_ERROR, errmsg);
} else {
  //char m4[LEN_l0f34+1];
  //printTRBIF(grbif);
  //combine34(grbif->lut34, m4);
};
return(rc);
}
