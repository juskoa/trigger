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
rc= ACT_close(handle);
#else
rc=0;
#endif
return(rc);
}
/*------------------------------------------------------------ cpNameVer */
void cpNameVer(ACT_instance *instance, char *actname, char *actversion) {
strcpy(actname, instance[0].instance);
strcpy(actversion, instance[0].version);
}
/*--------------------------------------------------------- actdb_getdbfile()
rc:0: $VMECFDIR/CFG/ctp/DB/... created
      actname/actversion filled
*/
int actdb_getdbfile(char *finame, char *actname, char *actversion) {
int rc=0;
#ifdef ACT_DB
int err; char CtpConfigItem[64];
FILE *f;
char fname[184];
actname[0]='\0'; actversion[0]='\0';
sprintf(CtpConfigItem,"/CTP/%s", finame);
err=ACT_getActiveItem(handle,CtpConfigItem, &instance);
if( err != 0 ) {
  printf("ERROR Cannot get %s item. Error code:%d\n",CtpConfigItem, err);
  rc=1;
} else {
  //snprintf(fname,sizeof(fname),"%s/CFG/ctp/pardefs/%s.partition",vmecfdir,name);
  sprintf(fname,"%s/CFG/ctp/DB/%s",vmecfdir,finame);
  if(instance != NULL) {
    printf("INFO Opening %s\n",fname);
    f=fopen(fname,"w"); last_Value[0]='\0';
    if(f != NULL) {
      int i,lng;
      if(instance[0].size< MAXlast_Value) {
        lng= instance[0].size;
      } else {
        lng=MAXlast_Value;
      };
      strncpy(last_Value, (char *)instance[0].value, lng);
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
int actdb_getdbfile_openclose(char *fn) {
int rc=0;
#ifdef ACT_DB
int err;
char actname[ACT_MAXNAME], actversion[ACT_MAXVER];
if((err=actdb_open())!=0) {
  printf("ERROR actdb_open. RC:%d\n",err);
  return(-4);
};
if((err= actdb_getdbfile(fn, actname, actversion)) !=0) {rc= err; goto STP;};
STP:
ACT_close(handle);
#else
  printf("INFO ACT not read (not compiled/linked)\n");
  rc=0;
#endif
return(rc);
}
/*---------------------------------------------- actdb_getdbstring(f,s,l)
I: 
fn: 
1. "CTP instance name"   -i.e. not starting with "/", get "/CTP/"+fn
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
int actdb_getdbstring(char *fn, int openclose, char *value, int maxl) {
int rc=0;
#ifdef ACT_DB
int err; char CtpConfigItem[164];
if(openclose==1) {
  if((err=actdb_open())!=0) {
    printf("ERROR actdb_open. RC:%d\n",err);
    return(-4);
  };
};
if(fn[0]=='/') {   // complete item name
  strcpy(CtpConfigItem, fn);
} else {           // CTP config item (i.e. "/CTP/"+fn
  sprintf(CtpConfigItem,"/CTP/%s", fn);
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
if((err= actdb_getdbfile("ctp.cfg",actname,actversion)) !=0) {rc= err; goto STP;};
sprintf(cfglist, "%s ctp.cfg %s %s\n", cfglist, actname, actversion);
if((err= actdb_getdbfile("L0.INPUTS",actname,actversion)) !=0) {rc= err; goto STP;};
sprintf(cfglist, "%s L0.INPUTS %s %s\n", cfglist, actname, actversion);
if((err= actdb_getdbfile("CTP.SWITCH",actname,actversion)) !=0) {rc= err; goto STP;};
sprintf(cfglist, "%s CTP.SWITCH %s %s\n", cfglist, actname, actversion);
if((err= actdb_getdbfile("VALID.CTPINPUTS",actname,actversion)) !=0) {rc= err; goto STP;};
sprintf(cfglist, "%s VALID.CTPINPUTS %s %s\n", cfglist, actname, actversion);
if((err= actdb_getdbfile("VALID.DESCRIPTORS",actname,actversion)) !=0) {rc= err; goto STP;};
sprintf(cfglist, "%s VALID.DESCRIPTORS %s %s\n", cfglist, actname, actversion);
if((err= actdb_getdbfile("FillingScheme",actname,actversion)) !=0) {rc= err; goto STP;} else {
  sprintf(cfglist, "%s FillingScheme %s %s\n", cfglist, actname, actversion);
  if(strncmp(last_Value,"bcmasks",7)==0) {
    if((err= actdb_getdbfile("VALID.BCMASKS",actname,actversion)) !=0) {rc= err; goto STP;};
  } else {
    printf("INFO VALID.BCMASKS not read (%10.10s not BCMASKS option in FillingScheme)\n",last_Value);
    actname[0]='\0'; actversion[0]='\0';
  };
  sprintf(cfglist, "%s VALID.BCMASKS %s %s\n", cfglist, actname, actversion);
};
//if((err= actdb_getdbfile("TRIGGER.PFS")) !=0) {rc= err; goto STP;};
//if((err= actdb_getdbfile("VALID.LTUS")) !=0) {rc= err; goto STP;};
//if((err= actdb_getdbfile("ttcparts.cfg")) !=0) {rc= err; goto STP;};
STP:
ACT_close(handle);
#else
  printf("INFO ACT not read (not compiled/linked)\n");
  rc=0;
#endif
return(rc);
}
/*--------------------------------------------------------- actdb_getPartition()
called from pydim/server.c 
I: part. name
Operation:
- check if partition active: "/part PHYSICS_2/Source of CTP config"
  (PHYSICS_2 replaced by name)
- download "/part PHYSICS_2/CTP config"
O: char filter[2000]: "" if filter not downloaded from ACT
           "filter value" downloaded from ACT
   char actname,actversion: "" or ACT instance name+ version
   rc:0 file name'.partition' created in $VMECFDIR/CFG/ctp/pardefs/
   rc:1 partition not defined in ACT or marked as 'Local File'
   rc:<0 another error
   -1: cannot write to output file
   -2: problem with ACT_getActiveItem (partition not defined in ACT)
   -3: cannot open output file
   -4: cannot open ACT
*/
int actdb_getPartition(char *name, char *filterpar, char *actname, char *actversion) {
int rc=0;
#ifdef ACT_DB
int err;
FILE *f;
char partitionCtpConfigItem[64];
char fname[184];
#define MAXFILTER 2000
char filter[MAXFILTER];
if((err=actdb_open())!=0) {
  printf("ERROR actdb_open. RC:%d\n",err);
  return(-4);
};
sprintf(partitionCtpConfigItem, "/part %s/Source of CTP config", name);
err= actdb_getdbstring(partitionCtpConfigItem, 0, filter, MAXFILTER);
if((err==0) && (strcmp(filter, "ACT database")==0)) {// or "Local File"
  if((err= actdb_getdbfile("filter",actname,actversion))!=0) {
    printf("INFO ACT filter not downloaded \n");
    filter[0]='\0';
  } else {
    int i;
    sprintf(fname,"%s/CFG/ctp/DB/filter",vmecfdir);
    i= readfile(fname, filter, 2000);
  /*  int i,maxi; 
    maxi= instance[0].size; if(maxi>(200-1)) maxi=200-1;
    for(i=0;i<maxi;i++) {
      filter[i]= ((char *)instance[0].value)[i];
    };
    filter[maxi]='\0';*/
    printf("INFO ACT filter downloaded::%d bytes\n", (int) strlen(filter));
  };
  strcpy(filterpar, filter);
  // '/part PHYSICS_1/CTP config'
  do_partitionCtpConfigItem(name, partitionCtpConfigItem);
  printf("INFO Act_getActiveItem(%s)\n", partitionCtpConfigItem);
  err=ACT_getActiveItem(handle,partitionCtpConfigItem, &instance);
  if( err != 0 ) {
    printf("ERROR Cannot get partition configuration item. Error code:%d\n",err);
    rc=1;
  } else {
    //snprintf(fname,sizeof(fname),"%s/CFG/ctp/pardefs/%s.partition",vmecfdir,name);
    sprintf(fname,"%s/CFG/ctp/pardefs/%s.partition",vmecfdir,name);
    if(instance != NULL) {
      printf("INFO Opening %s for instance %s ver.:%s.\n",
        fname,instance[0].instance, instance[0].version);  // OK
        //fname, "notyet","notyet");  // INVER
      f=fopen(fname,"w");
      if(f != NULL) {
        int i;
        for(i=0;i<instance[0].size;i++) {
          if(fputc(((char *)instance[0].value)[i],f) == EOF) {
            printf("ERROR Cannot write to %s",fname); rc=-1;
            break;
          };
        };
        fclose(f);
        cpNameVer(instance, actname, actversion);
        //strcpy(actname, instance[0].instance);  // INVER "notyet"); strcpy(actversion, instance[0].version);  // INVER "notyet");
      } else {
        printf("ERROR Cannot open %s\n",fname); rc=-3;
      };
    } else {
      printf("INFO partition %s not defined in ACT\n",fname); rc=-2;
    };
    ACT_destroyInstance(instance);
  };
  ACT_close(handle);
  if(rc==0) {
    char cmd[200];
    sprintf(cmd,"ls -l %s >/tmp/actexe.log", fname); system(cmd);
    sprintf(cmd,"dos2unx.bash %s 2>&1 >/tmp/actexe.log", fname); system(cmd);
    printf("INFO dos2unx:%s\n", fname);
  };
} else {
  rc=1;
  printf("INFO %s configured as '%s' in ACT\n",partitionCtpConfigItem, filter);
  actname[0]='\0'; actversion[0]='\0';
};
#else
  printf("INFO ACT not read (not compiled/linked)\n");
  rc=0;
#endif
return(rc);
}

/*----------------------------------------------------- actdb_getInstances()
*/
int actdb_getInstances(char *itemname) {
int err,ix;
int instancesNumber;
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
return(instancesNumber);
}
