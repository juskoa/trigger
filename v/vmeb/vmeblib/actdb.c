/* actdb.c
stdout: has to start with 'INFO ' or 'ERROR ' -routines here
are called from vme/pydim/server.c which is popened from
pydimserver.py
Used from:
pydim/act_main.c -test
pydim/server.c
ctp_proxy/main_act.c -load ctp config files or .partition from ACT
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vmewrap.h"
#include "vmeblib.h"
//#define ACT_DB
#ifdef CPLUSPLUS
extern "C" {
#ifdef ACT_DB
#include "act.h"
#endif
}
#endif
#define MAXlast_Value 1000
#ifdef ACT_DB
ACT_handle handle;
//ACT_handle *acthandle=NULL;
ACT_instance *instance;
char last_Value[MAXlast_Value+1];
#endif
#define ACT_MAXNAME 100
#define ACT_MAXVER 100

char *vmecfdir; 
/*------------------------------------------------------------ open */
int actdb_open() {
char *actdb;
int rco;
vmecfdir=getenv("VMECFDIR");
if( (actdb=getenv("ACT_DB")) == NULL) {
  printf("ERROR actdb_open() failed: ACT_DB env. variable xxx:xxx@hostname/ACT not set\n");
  rco=1;
} else {
#ifdef ACT_DB
  printf("INFO Opening ACT:%s\n", actdb);
  rco= ACT_open("",&handle);
  if(handle==NULL) { //if(rco!=0) {
    printf("ERROR ACT_open failed. rc:%d\n", rco);
    rco=2;
  }else{
    printf("INFO ACT opened succesfuly.\n"); rco=0;
    //acthandle= &handle;
  }
#else
  printf("INFO actdb_open: no attemt for ACT:  ACT_DB undefined\n");
  rco=0;
#endif
};
return(rco);
}
/*------------------------------------------------------------ close */
int actdb_close() {
int rc;
#ifdef ACT_DB
rc= ACT_close(handle); //acthandle= NULL;
#else
rc=0;
#endif
return(rc);
}
/*------------------------------------------------------------ cpNameVer */
#ifdef ACT_DB
void cpNameVer(ACT_instance *instance, char *actname, char *actversion) {
strcpy(actname, instance[0].instance);
strcpy(actversion, instance[0].version);
}
#endif
/*--------------------------------------------------------- actdb_getdbfile()
rc:0: $VMECFDIR/CFG/ctp/DB/... created
      actname/actversion filled
*/
int actdb_getdbfile(char *finame, char *actname, char *actversion, char *ctplite) {
int rc=0;
#ifdef ACT_DB
int err; char CtpConfigItem[64];
FILE *f;
char fname[184];
actname[0]='\0'; actversion[0]='\0';
sprintf(CtpConfigItem,"/%s/%s", ctplite, finame);
err=ACT_getActiveItem(handle,CtpConfigItem, &instance);
if( err != 0 ) {
  printf("ERROR Cannot get %s item. Error code:%d\n",CtpConfigItem, err);
  rc=1;
} else {
  //snprintf(fname,sizeof(fname),"%s/CFG/ctp/pardefs/%s.partition",vmecfdir,name);
  sprintf(fname,"%s/CFG/ctp/DB/%s",vmecfdir,finame);
  if(instance != NULL) {
    printf("INFO Writing /%s to %s\n",ctplite, fname);
    f=fopen(fname,"w"); last_Value[0]='\0';
    if(f != NULL) {
      int i,lng;
      if(instance[0].size< MAXlast_Value) {
        lng= instance[0].size;
      } else {
        lng=MAXlast_Value;
      };
      strncpy(last_Value, (char *)instance[0].value, lng);last_Value[lng]='\0';
      for(i=0;i<instance[0].size;i++) {
        if(fputc(((char *)instance[0].value)[i],f) == EOF) {
          printf("ERROR Cannot write to %s",fname); rc=-1;
          break;
        };
      };
      fclose(f); 
      cpNameVer(instance, actname, actversion);
      //strcpy(actname, instance[0].instance); strcpy(actversion, instance[0].version);
    } else {
      printf("ERROR Cannot open %s\n",fname); rc=-3;
    };
  } else {
    printf("INFO %s not defined in ACT\n",fname); rc=-2;
  };
  ACT_destroyInstance(instance);
};
#else
printf("INFO ACT not compiled in\n"); rc=-2;
#endif
return(rc);
}
/*---------------------------------------------- actdb_getdbfile_openclose(f)
*/
int actdb_getdbfile_openclose(char *fn, char *ctplite) {
int rc=0;
#ifdef ACT_DB
int err;
char actname[ACT_MAXNAME], actversion[ACT_MAXVER];
if((err=actdb_open())!=0) {
  printf("ERROR actdb_open. RC:%d\n",err);
  return(-4);
};
if((err= actdb_getdbfile(fn, actname, actversion, ctplite)) !=0) {rc= err; goto STP;};
STP:
ACT_close(handle);
#else
  printf("INFO ACT not read (not compiled/linked)\n");
  rc=0;
#endif
return(rc);
}
/*------------------------------ actdb_getdbstring(fn,openclose,sval,maxl, ctplite)
I: 
fn: 
1. "CTP instance name"   -i.e. not starting with "/", get "/CTP/"+fn or /CTPlite/...
                          according to ctplite
2. starting with "/":    -complete item name, i.e.:
  - "/part PHYSICS_2/Source of CTP config"
  - "/part PHYSICS_2/CTP config"  -partition definition (.partition)
    -not used yet for .partition -> string (as of 6.5.2011)
openclose:
1: open/close act
0: do not open/close act (already opened)

rc:0: ok value filled
1 cannot get item
2 too long (only part of the value is returned)
-4 open err
*/
int actdb_getdbstring(char *fn, int openclose, char *value, int maxl, char *ctplite) {
int rc=0;
#ifdef ACT_DB
int err; char CtpConfigItem[164];
ACT_instance *instance;
if(openclose==1) {
  if((err=actdb_open())!=0) {
    printf("ERROR actdb_open. RC:%d\n",err);
    return(-4);
  };
};
if(fn[0]=='/') {   // complete item name (i.e. /CTP/... or /CTPlite/...)
  strcpy(CtpConfigItem, fn);
} else {           // CTP config item (i.e. "/CTP[lite]/"+fn
  sprintf(CtpConfigItem,"/%s/%s", ctplite, fn);
};
err=ACT_getActiveItem(handle,CtpConfigItem, &instance);
if( err != 0 ) {
  printf("ERROR Cannot get %s item. Error code:%d\n",CtpConfigItem, err);
  rc=1;
} else {
  if(instance != NULL) {
    int lng=maxl-1;
    if(instance[0].size < (maxl-1)) {
      lng= instance[0].size;
    } else {
      rc=2;
    };
    //printf("INFO Copying %d bytes\n",lng);
    strncpy(value, (char *)instance[0].value, lng);
    value[lng]='\0';
  } else {
    printf("INFO %s not defined in ACT\n",CtpConfigItem); rc=-2;
  };
  ACT_destroyInstance(instance);
};
//STP:
if(openclose==1) {
  ACT_close(handle);
};
#else
  printf("INFO ACT not read (not compiled/linked)\n");
  rc=0;
#endif
return(rc);
}
/*--------------------------------------------------------- actdb_getdbfiles()
Op: download files + create $dbctp/actCfgInstances.list
rc:0 ok
*/
int actdb_getdbfiles() {
int rc=0;
#ifdef ACT_DB
int err;
char actname[ACT_MAXNAME], actversion[ACT_MAXVER];
char cfglist[20*200]="";
if((err=actdb_open())!=0) {
  printf("ERROR actdb_open. RC:%d\n",err);
  return(-4);
};
if((err= actdb_getdbfile("ctp.cfg",actname,actversion,"CTP")) !=0) {rc= err; goto STP;};
sprintf(cfglist, "%s ctp.cfg %s %s\n", cfglist, actname, actversion);
if((err= actdb_getdbfile("aliases.txt",actname,actversion,"CTP")) !=0) {rc= err; goto STP;};
sprintf(cfglist, "%s aliases.txt %s %s\n", cfglist, actname, actversion);
if((err= actdb_getdbfile("ctpinputs.cfg",actname,actversion,"CTP")) !=0) {rc= err; goto STP;};
sprintf(cfglist, "%s ctpinputs.cfg %s %s\n", cfglist, actname, actversion);
/*if((err= actdb_getdbfile("L0.INPUTS",actname,actversion)) !=0) {rc= err; goto STP;};
sprintf(cfglist, "%s L0.INPUTS %s %s\n", cfglist, actname, actversion);
if((err= actdb_getdbfile("CTP.SWITCH",actname,actversion)) !=0) {rc= err; goto STP;};
sprintf(cfglist, "%s CTP.SWITCH %s %s\n", cfglist, actname, actversion);
if((err= actdb_getdbfile("VALID.CTPINPUTS",actname,actversion)) !=0) {rc= err; goto STP;};
sprintf(cfglist, "%s VALID.CTPINPUTS %s %s\n", cfglist, actname, actversion); */
if((err= actdb_getdbfile("VALID.DESCRIPTORS",actname,actversion,"CTPlite")) !=0) {rc= err; goto STP;};
sprintf(cfglist, "%s VALID.DESCRIPTORS %s %s\n", cfglist, actname, actversion);
if((err= actdb_getdbfile("FillingScheme",actname,actversion,"CTP")) !=0) {rc= err; goto STP;} else {
  sprintf(cfglist, "%s FillingScheme %s %s\n", cfglist, actname, actversion);
  if(strncmp(last_Value,"bcmasks",7)==0) {
    if((err= actdb_getdbfile("VALID.BCMASKS",actname,actversion,"CTP")) !=0) {rc= err; goto STP;};
  } else {
    printf("INFO VALID.BCMASKS not read (%10.10s not BCMASKS option in FillingScheme)\n",last_Value);
    actname[0]='\0'; actversion[0]='\0';
  };
  sprintf(cfglist, "%s VALID.BCMASKS %s %s\n", cfglist, actname, actversion);
};
if((err= actdb_getdbfile("TRIGGER.PFS", actname, actversion,"CTP")) !=0) {rc= err; goto STP;};
sprintf(cfglist, "%s TRIGGER.PFS %s %s\n", cfglist, actname, actversion);
//if((err= actdb_getdbfile("VALID.LTUS", actname, actversion,"CTP")) !=0) {rc= err; goto STP;};
//if((err= actdb_getdbfile("ttcparts.cfg", actname, actversion,"CTP")) !=0) {rc= err; goto STP;};
STP:
ACT_close(handle);
#else
  printf("INFO ACT not read (not compiled/linked)\n");
  rc=0;
#endif
return(rc);
}

/*----------------------------------------------------- actdb_getInstances()
*/
int actdb_getInstances(char *itemname) {
int instancesNumber=0;
#ifdef ACT_DB
int err,ix;
ACT_instance *instancesArray;

if((err=actdb_open())!=0) {
  printf("ERROR actdb_open RC:%d\n",err);
  return(-4);
};
if((err=ACT_getItem(handle, itemname, 
      &instancesArray, &instancesNumber))!=0) {
  printf("ERROR ACT_getItem RC:%d\n",err);
  return(-4);
};
printf("INFO   ix  size A st itemname:inst_name   instances:%d\n", instancesNumber);
for(ix=0; ix<instancesNumber; ix++) {
  printf("INFO %3d: %5d %d %2d %s:%s.%s\n", ix,
    instancesArray[ix].size,
    instancesArray[ix].isActive,
    instancesArray[ix].status,
    instancesArray[ix].item,
    instancesArray[ix].instance,
    instancesArray[ix].version);
};
if((err=ACT_destroyInstanceArray(instancesArray, 
      instancesNumber))!=0) {
  printf("ERROR ACT_destroyInstanceArray RC:%d\n",err);
  return(-4);
};
ACT_close(handle);
#endif
return(instancesNumber);
}
