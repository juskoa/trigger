#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>  // opendir,closedir
#include <dirent.h>
#include <libgen.h>   // dirname

#include "lexan.h"
#define MAXNAMELENGTH 80        // paths
#define ERRMSGL 300
#define MAXPARNAME 80

/*------------------------------------------------------------openFile()
*/
FILE *openFile(char *fname, char *rw) {
FILE *cfgfile;
char *environ;
char *dirn; 
char cmd[MAXNAMELENGTH+40+40];
//DIR *dirstream; int rc=8; char cfnpath[MAXNAMELENGTH+40];
char fnpath[MAXNAMELENGTH+40];
char cfnpath[MAXNAMELENGTH+40];
if(fname[0]=='/') { // absolute
  strcpy(fnpath,fname);
} else {  // relative path
  environ= getenv("VMECFDIR"); strcpy(fnpath, environ);
  strcat(fnpath,"/");
  strcat(fnpath,"CFG/ctp/DB/"); strcat(fnpath, fname);
};
/* invalidate NFS cache: not working (a nfs-mounted-file not refreshed on client)
strcpy(cfnpath, fnpath); dirn= dirname(cfnpath);
dirstream= opendir(dirn); 
if(dirstream !=NULL) {rc=closedir(dirstream);};
printf("INFO openFile:%s %s dir:%s closedir rc:%d\n", fnpath, rw, dirn, rc);
*/
/* 'ls' invalidated the nfs cache: */
strcpy(cfnpath, fnpath); dirn= dirname(cfnpath);
sprintf(cmd,"/bin/ls %s 2>&1 >/dev/null", dirn); system(cmd);
printf("INFO openFile:%s %s dir:%s\n", fnpath, rw, dirn);
cfgfile=fopen(fnpath,rw);
if(cfgfile == NULL){
  printf("ERROR fnpath:%s:\n", fnpath);
  //perror(strerror(errno));
  return(NULL);
};
return(cfgfile);
}
/*------------------------------------------------------------readdbfile()
*/
int readdbfile(char *fname, char *mem, int maxlen) {
FILE *cf; int sp;
cf= openFile(fname,"r");
if(cf != NULL) {
  sp=fread((void *)mem, 1, maxlen-1, cf); 
  mem[sp]='\0';
  fclose(cf); 
} else {
  sp=0;
};
return(sp);
}
/*------------------------------------------------------------writedbfile()
*/
int writedbfile(char *fname, char *mem) {
FILE *cf; int sp;
cf= openFile(fname,"w");
if(cf != NULL) {
  sp=fwrite((void *)mem, 1, strlen(mem), cf); 
  fclose(cf); 
} else {
  sp=0;
};
return(sp);
}
/*--------------------------------
 rc: 0: ok, 1: cannot access ctp.cfg 
   2: at least one of int1/2 is not defined */
int getINT12fromcfg(char *int1, char *int2, int max12) {
FILE *cfgfile;
int grc=0;
enum Ttokentype token;
char parname[MAXPARNAME];
char emsg[ERRMSGL];
char line[MAXLINELENGTH], value[MAXLINELENGTH];
cfgfile=openFile("ctp.cfg","r");
if(cfgfile == NULL) return(1); int1[0]='\0'; int2[0]='\0';
while(fgets(line, MAXLINELENGTH, cfgfile)){
  int ix;
  //printf("Decoding line:%s:\n",line);
  if(line[0]=='#') continue;
  if(line[0]=='\n') continue;
  ix=0; token= nxtoken(line, value, &ix);
  if(token!=tSYMNAME) {
    sprintf(emsg, "ERROR ctp.cfg line ignored:%s\n",line);
    printf("%s", emsg);
    continue;
  };
  value[MAXPARNAME-1]='\0'; strcpy(parname, value); 
  if((strcmp(parname,"L0_INTERACT1")==0)||(strcmp(parname,"L0_INTERACT2")==0)) {
    char *dest;
    while(1) {
      char c;
      c= line[ix];
      if((c==' ') || (c=='\t')) { ix++; continue;};
      break;
    };
    if(strcmp(parname,"L0_INTERACT1")==0) {
      dest= int1;
    } else {
      dest= int2;
    };
    copy2nl(dest, &line[ix], max12);
  };
};
if((int1[0]=='\0') ||(int2[0]=='\0')) {grc=2;};
fclose(cfgfile); return(grc);
}

