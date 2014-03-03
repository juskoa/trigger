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

#ifdef ACT_DB
extern ACT_handle handle;
#endif
extern char *vmecfdir; 
/* von
#define ACT_MAXNAME 100
#define ACT_MAXVER 100
*/
#include <sys/types.h>
#include <sys/shm.h>
#include "shmaccess.h"
//#define DBMAIN
#include "Tpartition.h"

EXTERN char *vmecfdir;
int actdb_open();
int actdb_close();
int actdb_getdbstring(char *fn, int openclose, char *value, int maxl);
void cpNameVer(ACT_instance *instance, char *actname, char *actversion);

/*--------------------------------------------------------- actdb_getff()
 * get filter to file
*/
void actdb_getff(char *filter) {
#ifdef ACT_DB
int err,ix; FILE *f; char fname[120];
ctpshmbase= (Tctpshm *)mallocShared(CTPSHMKEY, sizeof(Tctpshm), &ctpsegid);
if((err=actdb_open())!=0) {
  //printf("ERROR actdb_open. RC:%d\n",err);
  return;
}; filter[0]='\0';
for(ix=0; ix<NDETEC; ix++) {     // fo all /CTP/trgInput_* items
  int rc; char value[24]=""; char tiname[32]; char *ltuname; 
  ltuname= ctpshmbase->validLTUs[ix].name;
  if(ltuname[0]=='\0') continue;
  sprintf(tiname,"trgInput_%s", ltuname);
  rc= actdb_getdbstring(tiname, 0, value, 24);
  if(rc!=0) continue;
  if(strcmp(value,"ON")==0) {
    strcat(filter, ltuname); strcat(filter, " "); 
  };
  printf("INFO dbg %s value:%s rc:%d\n", tiname, value, rc);
};
err=actdb_close();
shmdt(ctpshmbase);
ix= strlen(filter); if(ix>0) filter[ix-1]='\0'; // remove trailing space
sprintf(fname,"%s/CFG/ctp/DB/filter",vmecfdir); f=fopen(fname,"w");
if(f == NULL) {
  printf("INFO actdb_getff: cannot write to %s\n", fname);
} else {
  fputs(filter, f);
  printf("INFO actdb_getff: %s written\n", fname);
  fclose(f);
};
#else
  printf("INFO actdb_getff: no attemt for ACT:  ACT_DB undefined\n");
#endif
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
ACT_instance *instance;
FILE *f;
char partitionCtpConfigItem[64];
char fname[184];
#define MAXFILTER 2000
char filter[MAXFILTER]="";
if((err=actdb_open())!=0) {
  printf("ERROR actdb_open. RC:%d\n",err);
  return(-4);
};
sprintf(partitionCtpConfigItem, "/part %s/Source of CTP config", name);
err= actdb_getdbstring(partitionCtpConfigItem, 0, filter, MAXFILTER);
if((err==0) && (strcmp(filter, "ACT database")==0)) {// or "Local File"
/*--------------- following
  if((err= actdb_getdbfile("filter",actname,actversion))!=0) {
    printf("INFO ACT filter not downloaded \n");
    filter[0]='\0';
  } else {
    int i;
    sprintf(fname,"%s/CFG/ctp/DB/filter",vmecfdir);
    i= readfile(fname, filter, 2000);
    //int i,maxi; 
    //maxi= instance[0].size; if(maxi>(200-1)) maxi=200-1;
    //for(i=0;i<maxi;i++) {
    //  filter[i]= ((char *)instance[0].value)[i];
    //};
    //filter[maxi]='\0';
    printf("INFO ACT filter downloaded::%d bytes\n", (int) strlen(filter));
  };
-----------------
replaced by:
*/
  actdb_getff(filter);   // readfilter to file and return in filter
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

