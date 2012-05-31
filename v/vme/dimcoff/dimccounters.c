/* dimccounters.c -an example of dim client reading CTP counters */
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#ifdef CPLUSPLUS
#include <dic.hxx>
#else
#include <dic.h>
#endif
#include "ctpcounters.h"
#include "vmewrapdefs.h"
#include "vmeblib.h"
#include "infolog.h"

//CTP_COUNTERS
#define MAXLINE 256
#define NCLASS 50
#define NPARTIT 6
//#define RUNXCOUNTERSSTART 812
#define RUNXCOUNTERSSTART (CSTART_BUSY+NCOUNTERS_BUSY_RUNX1)
#define TAGstartcount 333
#define TAGstopcount 334
#define TAGprintruns 335
// Maximum time of orbit counter=(2**24-1)*3564*0.025
#define ORBMAX 1494849857
typedef struct{
 char partitionname[MAXLINE];
 int runnumber;
 char filename[MAXLINE];   // Counter file
 FILE *file;           // counter file
 char rcfgname[MAXLINE];
 FILE *rcfg;
 int nclasses;
 int *ActiveClasses;  // non zero value is the class number used in run
 w32 *ActiveCounts;   // counts of classes as in Active Classes
 w32 timetot;
 w32 period_counter;
 // last readings
 w32 timesec_last,timeusec_last,orbit_last;
 w32 *classcount_last;
}ActiveRun;
ActiveRun *activeruns[NPARTIT];

unsigned int cnts[NCOUNTERS]; 
int realcnts=NCOUNTERS;  
unsigned int cntsFailed=0xdeaddeed;

/*------------------------------------------------------------char2i()
 Purpose: Character to integer converter
 Parameters: input char a
             output: w32 *num
 Returns: error code: 0=ok
*/
int char2i(char a,w32 *num){
 *num=0;
 if(a >= 0x30 && a <= 0x39) *num = (a-0x30);
 else if (a >= 0x61 && a <= 0x66) *num = (a-0x57);
 else {
       char msg[200];
       sprintf(msg,"char2i: wrong pattern character:%c:\n",a);
       intError(msg);
       return 1;       
 }
 return 0; 
}
/*---------------------------------------------printActiveClasses
*/
void printActiveClasses(ActiveRun *ar){
 int i;
 printf("RUN %d:",ar->runnumber);
 for(i=0;i<ar->nclasses;i++)if(ar->ActiveClasses[i]){
  printf(" %i",ar->ActiveClasses[i]);
 }
 printf("\n");
}
/*-----------------------------------------------printActiveRun
*/
void printActiveRun(ActiveRun *ar){
 char msg[200];
 sprintf(msg,"RUN %d, partition:%s ",ar->runnumber,ar->partitionname);
 prtLog(msg);
 printf("%p %s\n",ar->file,ar->filename);
 printf("%p %s\n",ar->rcfg,ar->rcfgname);
 printActiveClasses(ar);
}
/*-----------------------------------------------printActiveRuns
*/
void printActiveRuns(){
 int i;
 for(i=0;i<NPARTIT;i++) if(activeruns[i])printActiveRun((activeruns[i]));
}
/*-------------------------------------------------------printL2ascalers()
*/
void printL2ascalers(ActiveRun *ar){
 int i;
 char msg[2500],l2a[50];
 printf("Run %i L2a scalers:\n",ar->runnumber);
 sprintf(msg,"Run:%i Partition:%s Duration(usecs) %i L2a counts [class,counts]: ",ar->runnumber,ar->partitionname,ar->timetot);
 for(i=0;i<ar->nclasses;i++){
   sprintf(l2a,"[%i,%i] ",ar->ActiveClasses[i],ar->ActiveCounts[i]);
   strcat(msg,l2a);
 }
  infolog_SetStream(ar->partitionname, ar->runnumber);
  infolog_trg(LOG_INFO, msg);
  infolog_SetStream("",0);
 printf("%s \n",msg);
}
/*---------------------------------------------------------------------
*/
ActiveRun *freeActiveRun(ActiveRun *ar){
 if(ar->ActiveClasses)free(ar->ActiveClasses);
 if(ar->ActiveCounts)free(ar->ActiveCounts);
 if(ar->classcount_last)free(ar->classcount_last);
 free(ar);
 return NULL;
}
/*-------------------------------------------------------getClassesDummy()
 Read database and find the classes
 Now: if 1st partition thenn clas=1
         2nd partition       clas ==2
*/
int getClassesDummy(int ip,int *classes,int *nclasses){
 int i,np=0;
 for(i=0;i<NPARTIT;i++){
    if(activeruns[i]->runnumber !=0)np++;
 }
 //for(i=0;i<NCLASS;i++)classes[i]=0;
 if(np==0){
  char msg[200];
  sprintf(msg, "xcounters :getClasses  np=0\n");
  intError(msg);
  return 1;
 }
 //printf("Run %i: active class %i \n",activeruns[ip].runnumber,np);
 classes[np-1]=1;
 //classes[27]=1;
 *nclasses=1;
 return 0;
}
/*-------------------------------------------------------getclas
Parse one class line in rcfg file
return 0 if class not found
*/
int getclas(char *line){
 int ix,classk=0;
 w32 dig1,dig2;
 ix=0;
 while( line[ix] == ' ')ix++;  //space before trigger descriptor
 while(line[ix] != ' ')ix++;   // trigger descriptor
 while( line[ix] == ' ')ix++;  //space after trigger descriptor
 if(char2i(line[ix],&dig1))return 0;
 ix++;
 if(line[ix] == ' '){
  classk = dig1;
  return classk;
 }else{
  if(char2i(line[ix],&dig2))return 0;
  if(line[ix+1] != ' ') return 0;
  classk = 10*dig1+dig2;
  if(classk<1 || classk >50) return 0;
  return classk;
 }
}
/*-------------------------------------------------------getName
*/
void getName(char *name,char *line){
 int ix=0;
 while((line[ix] != ':') && (line[ix] != '\n'))ix++;
 //printf("getName: %s",&line[ix]);
 if(line[ix] == '\n') return;
 ix++;
 while(line[ix]==' ')ix++;   //remove spaces
 strcpy(name,&line[ix]);
}
/*-------------------------------------------------------getClasses()
 Read database and find the classes
 Now: if 1st partition thenn clas=1
         2nd partition       clas ==2
*/
ActiveRun *getClasses(int runnumber){
 int nclasses;
 int i,flag;
 char line[MAXLINE];
 char msg[200];
 char *environ;
 ActiveRun *ar;
 int buffer[NCLASS];

 for(i=0;i<NCLASS;i++)buffer[i]=0;
 ar = (ActiveRun *) malloc(sizeof(ActiveRun));
 ar->runnumber=runnumber;
 ar->timetot=0;
 ar->period_counter=0;
 ar->timesec_last=0;
 ar->timeusec_last=0;
 ar->orbit_last=0;
 strcpy(ar->partitionname,"NOT FOUND");
 strcpy(ar->rcfgname,"NOT FOUND");
 strcpy(ar->filename,"NOT FOUND");

 environ=getenv("VMEWORKDIR");
 strcpy(ar->rcfgname,environ);
 sprintf(ar->rcfgname,"%s/WORK/RCFG/r%d.rcfg",ar->rcfgname,ar->runnumber);
 ar->rcfg=fopen(ar->rcfgname,"r");
 //printActiveRuns();
 if(ar->rcfg == NULL){
  printf("getClasses warning: cannot open file %s\n",ar->rcfgname);
  printf("getClasses warning: counters for all classes used.\n");
  nclasses=NCLASS;
  for(i=0;i<NCLASS;i++)buffer[i]=i+1;
  goto ERR;
 } 
 printf("getClasses: file %s open succesfully.\n",ar->rcfgname);
 flag=0;
 while((fgets(line,MAXLINE,ar->rcfg)!=NULL)){
   //printf("%s\n",line);
   if(line[0] == '#') continue;
   if(line[0] == '\0') continue;  //end of file
   if(strncmp("PARTITION",line,9) == 0) getName(ar->partitionname,line);
   if(strncmp("CLASSES",line,7) == 0){
    flag=1;
    break;
   }
 }
 //printf("%s",line);
 if(flag==0){
    sprintf(msg,"getClasses: classes not found.\n");
    intError(msg);
    nclasses=NCLASS;
    for(i=0;i<NCLASS;i++)buffer[i]=i+1;
    goto ERR;
 }
 nclasses=0;
 while((fgets(line,MAXLINE,ar->rcfg)!=NULL)){
    //printf("%s",line);
    if(line[0] == '#') continue;
    int classk;
    classk = getclas(line);
    if(classk == 0){
      sprintf(msg,"getClasses error; no class in line: %s \n",line);
      intError(msg);
      nclasses=NCLASS;
      for(i=0;i<NCLASS;i++)buffer[i]=i+1;
      goto ERR;
    }
    if(nclasses<NCLASS){
      buffer[nclasses]=classk;
      nclasses++;
    }else{
      sprintf(msg,"GetClasses error: # of classes>50.\n");
      intError(msg);
      nclasses=NCLASS;
      for(i=0;i<NCLASS;i++)buffer[i]=i+1;
      goto ERR;
    }
    sprintf(msg,"GetClasses: Run %i: active class %i \n",ar->runnumber,classk);
    prtLog(msg);
 }
 printf("# classes=%i \n",nclasses);
 if(nclasses==0){
    sprintf(msg,"GetClasses error: # classes=0\n");
    intError(msg);
    nclasses=NCLASS;
    for(i=0;i<NCLASS;i++)buffer[i]=i+1;
    goto ERR;
 }
 ERR:
 if(ar->rcfg != NULL){
   fclose(ar->rcfg);
 };
 ar->nclasses=nclasses;
 ar->ActiveClasses = (int *) malloc(nclasses*sizeof(int));  
 ar->ActiveCounts  = (w32 *) malloc(nclasses*sizeof(w32));  
 ar->classcount_last = (w32 *) malloc(nclasses*sizeof(w32));
 for(i=0;i<nclasses;i++){
   ar->ActiveClasses[i]=buffer[i];
   ar->ActiveCounts[i]=0;
   ar->classcount_last[i]=0;
 }
 printActiveClasses(ar);
 return ar;
}
/*-------------------------------------------------------openFile()
  Open file at the beginning of run
  Construct ActiveRun
*/
int openFile(int ix,int runnumber){
 ActiveRun *ar;
 char msg[200];
 int i;
 printf("Opening file for run %i \n",runnumber);
 ar=getClasses(runnumber);
 activeruns[ix]=ar;
 sprintf(ar->filename,"run%d.cnt",runnumber);
 ar->file=fopen(ar->filename,"w");
 if(ar->file == NULL){
   sprintf(msg,"openFile error: cannot open file %s \n",ar->filename);
   prtError(msg);
   return 1;
 }else{
   sprintf(msg,"File %s opened.\n",ar->filename);
   prtLog(msg);   
 }
 fprintf(ar->file,"%s\n","1");
 fprintf(ar->file,"%d %d ",ar->runnumber,ar->nclasses);
 // Write list of classes to run file
 for(i=0;i<ar->nclasses;i++){
  fprintf(ar->file,"%d ",ar->ActiveClasses[i]); 
  //1st classs=1
 }
 fprintf(ar->file,"\n");
 printActiveClasses(ar);
 return 0;
}
/*----------------------------------------------------getClassCount()
*/
void getClassCount(ActiveRun *ar,int i,int *counts,w32 *buffer,int notfirst){
 /* classes counted from 0*/
 int iclass;
 // hw class number
 iclass = ar->ActiveClasses[i];
  //L0
 counts[0]=buffer[CSTART_L0+15+iclass];
 counts[1]=buffer[CSTART_L0+99+iclass];
 //L1
 counts[2]=buffer[CSTART_L1+39+iclass];
 counts[3]=buffer[CSTART_L1+89+iclass];
 //L2
 counts[4]=buffer[CSTART_L2+25+iclass];
 counts[5]=buffer[CSTART_L2+75+iclass];
 //printf("%i %i %i %i %i %i %i \n",iclass,counts[0],counts[1],counts[2],counts[3],counts[4],counts[5]);
 if(notfirst){
 // skontroluj velkost counterov, ci dodif parcuje
 // dynamicke runy ?
  ar->ActiveCounts[i]=ar->ActiveCounts[i]+dodif32(ar->classcount_last[i],counts[5]);  
 }
 // onlt l2a for the moment
 //for(k=0;k<6;k++)ar->classcount_last[k][i]=counts[k]; 
 ar->classcount_last[i]=counts[5]; 
}
/*-------------------------------------------------------getTimes()
*/
void getTimes(ActiveRun *ar,w32 *timesec,w32 *timeusec,w32 *orbit,w32 *buffer,int notfirst){
 char msg[200];
 *timesec=buffer[CSTART_SPEC];
 *timeusec=buffer[CSTART_SPEC+1];
 *orbit=buffer[CSTART_SPEC+2];
 if(notfirst){
  w32 timedif;
  timedif = (*timesec-ar->timesec_last)*1000000;
  timedif=timedif+(*timeusec+ar->timeusec_last);
  if(timedif>62*1000000){
   sprintf(msg,"Time between readings more than 1 min: %i secs",timedif/1000000);
   prtLog(msg);
  }
  if(timedif>ORBMAX){
   sprintf(msg,"Time between readings more than %i secs (ORBMAX): %i",ORBMAX/1000000,timedif/1000000);
   prtLog(msg);
   timedif=timedif/ORBMAX;
   ar->period_counter=ar->period_counter+timedif;
  }
  ar->timetot=ar->timetot+timedif;
  if(*orbit < ar->orbit_last)ar->period_counter=ar->period_counter+1;  
 }
 ar->timesec_last=*timesec; 
 ar->timeusec_last=*timeusec;
 ar->orbit_last=*orbit; 
}
/*-------------------------------------------------------writeFile()
 Write counters to the file
 Order of classes is the same as in rcfg file.
*/
int writeFile(int ix,w32 *buffer,int notfirst){
 int ret=0;
 ActiveRun *ar;
 ar=activeruns[ix];
 printActiveRuns();
 //for(i=0;i<20;i++)printf("%u ",buffer[CSTART_SPEC-3+i]);printf("\n"); 
 if(ar->file){
   int i,j;
   int classcount[6];
   w32 timesec,timeusec,orbit;
   getTimes(ar,&timesec,&timeusec,&orbit,buffer,notfirst);
   fprintf(ar->file,"%u %u %u %u\n",orbit,ar->period_counter,timesec,timeusec);
   printf("RUN %d timestamp (orbit periodcount secs usecs): %u %u %u %u\n",ar->runnumber,orbit,ar->period_counter,timesec,timeusec);
   // Print 6 counters per class (L0before,L0after,L1before,...)
   for(i=0;i<ar->nclasses;i++){
     if(ar->ActiveClasses[i]){
      getClassCount(ar,i,classcount,buffer,notfirst);
      for(j=0;j<6;j++)fprintf(ar->file,"%u ",classcount[j]);
      fprintf(ar->file,"\n");
      //for(j=0;j<6;j++)printf("%u ",classcount[j]);
      //printf("\n");
     }else{
      intError("WriteFile");
     }
   }
 }else{
  //printf("writeFile: file %s not opened !\n",ar->filename);
  ret=1;
 }
 return ret;
}
/*-------------------------------------------------------closeFile()
 Close file at the end of run
*/
int closeFile(int ix){
 ActiveRun *ar;
 int rc;
 char cmd[200],msg[200];
 ar=activeruns[ix]; 
 // register with DCS FXS (this executable is started from ~tri home directory):
 // counter file
 if(ar->file){
   fclose(ar->file);
   sprintf(cmd,"./dcsFES_putData.sh %d GRP CTP_xcounters /home/tri/%s", ar->runnumber, ar->filename);
   rc=system(cmd);
   //printf("cmd:%s rc:%d\n", cmd, rc);
   sprintf(cmd,"mv -f %s delme/%s",ar->filename,ar->filename);
   rc=system(cmd);
   sprintf(msg,"cmd:%s rc:%d\n", cmd, rc);
   prtLog(msg);
 }else{
   sprintf(msg,"run %d: xcounter file %s not send to FXS.\n",ar->runnumber,ar->filename);
   prtError(msg);
 }
 // config file
 /*if(ar->rcfg){
   fclose(ar->rcfg);
   char *environ;
   char delme[256];
   sprintf(cmd,"./dcsFES_putData.sh %d GRP CTP_runconfig %s",ar->runnumber,ar->rcfgname);  
   rc=system(cmd);
   //printf("cmd:%s rc:%d\n", cmd, rc);
   environ=getenv("VMEWORKDIR");
   strcpy(delme,environ);
   sprintf(delme,"%s/WORK/RCFG/delme/r%d.rcfg",delme,ar->runnumber);
   sprintf(cmd,"mv -f %s %s",ar->rcfgname,delme);
   rc=system(cmd);
   sprintf(msg,"cmd:%s rc:%d\n", cmd, rc);
   prtLog(msg);
 }else{
   sprintf(msg,"run %d: config file %s not send to FXS.\n",ar->runnumber,ar->rcfgname);
   prtError(msg);
 }
*/
 fflush(stdout);
 // DAQ logbook info
 printL2ascalers(ar);
 activeruns[ix]=freeActiveRun(ar);
 return 0;
}
/*-----------------------------------------------------------------------
*/
void updateCountFiles(w32 *buffer){
 int ix;
 char dt[20]; 
 unsigned int PartRunNumNew[NPARTIT];
 getdatetime(dt);
 //printf("%s Active run numbers: ",dt);
 for(ix=RUNXCOUNTERSSTART;ix<(RUNXCOUNTERSSTART+NPARTIT);ix++){
    //printf("%d ",buffer[ix]);
    PartRunNumNew[ix-RUNXCOUNTERSSTART]=buffer[ix];
 }
 //printf("\n");
 for(ix=0;ix<NPARTIT;ix++){
    if(PartRunNumNew[ix] && (activeruns[ix]==NULL)){
       printf("%s change detected: Run %d started. \n",dt, PartRunNumNew[ix]);
       openFile(ix,PartRunNumNew[ix]);
       writeFile(ix,buffer,0);
    }else if((PartRunNumNew[ix]==0) && activeruns[ix]){
       printf("%s change detected: Run %d stopped. \n",dt, activeruns[ix]->runnumber);
       writeFile(ix,buffer,1);
       closeFile(ix);
    }else if(PartRunNumNew[ix] && activeruns[ix]) writeFile(ix,buffer,1);
 }
}
/*-----------------------*/ void gotcnts(void *tag, void *buffer, int *size) {
//printf("gotcnts tag:%d size:%d\n", *(int *)tag, *size );
if(*size != 4*(realcnts)) {          // zmena rl
  printf("error in gotcnts. First word of message (if any):0x%x actual size=%d expected size=%d \n",*(w32 *)buffer,*size,4*(realcnts)); // zmena rl
    return;
};
/*
printf(" addr    0x abs           diff[secs]\n");
for(ix=0; ix<NCS; ix++) {
  cs[ix].currcs= buffer[cs[ix].reladdr];
  printf(" %3d %8x %10.4f\n", cs[ix].reladdr, cs[ix].currcs, 
    dodif32(cs[ix].prevcs, cs[ix].currcs)/2500000.);
  cs[ix].prevcs= cs[ix].currcs;
};
printf("\n");
*/
// run numbers 
updateCountFiles((w32 *)buffer);
}
/*--------------------------------------------------------------------
*/
int main(int argc, char **argv) {
int inforc;
infolog_SetFacility("CTP");
inforc= dic_info_service("CTPDIM/MONCOUNTERS", MONITORED, 0, 
  cnts,4*(realcnts), gotcnts, 137, &cntsFailed, 4); // zmena rl
printf("CTPDIM/MONCOUNTERS service id:%d\n", inforc);
infolog_trg(LOG_INFO, "xcounters: restart");
//initcntsCTP();
while(1) {
  //char inpline[100];
  sleep(10000000);
/*
  printf("\n\
g     -get counters immediately\n\
q     -quit:");
  fgets(inpline, 100, stdin);
  if(inpline[0]=='q') break;
  if(inpline[0]=='g') {
    int rc;
    rc= dic_cmnd_service("CTPDIM/STARTRUNCOUNTER", 0,0);
    printf("RC from dic_cmnd_service:%d\n", rc);
  };*/
};
dic_release_service(inforc);
return(0);
} 
