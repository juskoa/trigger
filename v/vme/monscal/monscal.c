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
#define MAX32 4294967296.
typedef struct{
 char partitionname[MAXLINE];
 int runnumber;
 char filename[MAXLINE];   // Counter file
 FILE *file;           // counter file
 char rcfgname[MAXLINE];
 FILE *rcfg;
 int ninputs;
 int ninputsL1;
 w32 *Inputs;
 w32 *InputsL1;
 w32 *InputsCounts;
 w32 *InputsCounts_last;
 int nclasses;
 int *ActiveClasses;  // non zero value is the class number used in run
 w32 *ActiveCountsL2;   // counts of classes as in Active Classes
 w32 *ActiveCountsL0;   // counts of classes as in Active Classes
 // Time parameters
 w32 overflow;
 w32 timetot;
 w32 period_counter;
 w32 timesec_last,timeusec_last,orbit_last;
 double timetotd;
 // last readings
 w32 *classcount_lastL0;
 w32 *classcount_lastL2;
 char **classNames;
}ActiveRun;
ActiveRun *activeruns[NPARTIT];
ActiveRun *defRun;

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
   sprintf(l2a,"[%i,%i] ",ar->ActiveClasses[i],ar->ActiveCountsL2[i]);
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
 if(ar->ActiveCountsL2)free(ar->ActiveCountsL2);
 if(ar->ActiveCountsL0)free(ar->ActiveCountsL0);
 if(ar->classcount_lastL2)free(ar->classcount_lastL2);
 if(ar->classcount_lastL0)free(ar->classcount_lastL0);
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
int getclas(char *line,char *desc){
 int ix,classk=0;
 w32 dig1,dig2;
 ix=0;
 //char desc[10];
 while( line[ix] == ' ')ix++;  //space before trigger descriptor
 int ii=0;
 while(line[ix] != ' '){
      if(ii<5)desc[ii]=line[ix];
      ii++;
      ix++;           // trigger descriptor
 }
 desc[5]='\0';
 // printf("%s \n",desc);
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
 ar->timetotd=0;
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
 char descall[50][10];
 nclasses=0;
 while((fgets(line,MAXLINE,ar->rcfg)!=NULL)){
    //printf("%s",line);
    if(line[0] == '#') continue;
    int classk;
    classk = getclas(line,descall[nclasses]);
    //printf("%s \n",descall[nclasses]);
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
    //prtLog(msg);
 }
 //printf("# classes=%i \n",nclasses);
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
 ar->ActiveCountsL2  = (w32 *) malloc(nclasses*sizeof(w32));  
 ar->ActiveCountsL0  = (w32 *) malloc(nclasses*sizeof(w32));  
 ar->classcount_lastL0 = (w32 *) malloc(nclasses*sizeof(w32));
 ar->classcount_lastL2 = (w32 *) malloc(nclasses*sizeof(w32));
 ar->classNames =  (char**) malloc(nclasses*sizeof(char *));
 for(i=0;i<nclasses;i++){
   ar->ActiveClasses[i]=buffer[i];
   ar->ActiveCountsL2[i]=0;
   ar->ActiveCountsL0[i]=0;
   ar->classcount_lastL0[i]=0;
   ar->classcount_lastL2[i]=0;
   ar->classNames[i]=(char*) malloc(6*sizeof(char));
   strcpy(ar->classNames[i],descall[i]);
   //printf("%s \n",ar->classNames[i]);
 }
 //printActiveClasses(ar);
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
 //printf("Opening file for run %i \n",runnumber);
 ar=getClasses(runnumber);
 activeruns[ix]=ar;
 sprintf(ar->filename,"cnt/run%d.cnt",runnumber);
 ar->file=fopen(ar->filename,"w");
 if(ar->file == NULL){
   sprintf(msg,"openFile error: cannot open file %s \n",ar->filename);
   prtError(msg);
   return 1;
 }else{
   sprintf(msg,"File %s opened.\n",ar->filename);
   //prtLog(msg);   
 }
 fprintf(ar->file,"%s\n","1");
 fprintf(ar->file,"%d %d ",ar->runnumber,ar->nclasses);
 // Write list of classes to run file
 for(i=0;i<ar->nclasses;i++){
  fprintf(ar->file,"%d ",ar->ActiveClasses[i]); 
  //1st classs=1
 }
 fprintf(ar->file,"\n");
 //printActiveClasses(ar);
 return 0;
}
/*----------------------------------------------------------------------
*/
void updateTime(ActiveRun *ar, w32 *buffer, int notfirst)
{
 int time=buffer[CSTART_L0+13];
 if(notfirst){
   ar->timeusec_last=ar->timesec_last;
   ar->timesec_last=time;
 } else {
   ar->timesec_last=time;
 }
}
/*----------------------------------------------------getClassCount()
*/
void getClassCount(ActiveRun *ar,int i,int *counts,w32 *buffer,int notfirst){
 /* classes counted from 0*/
 int iclass;
 // time
 //int time=buffer[CSTART_L0+13];
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
  int timediffi=dodif32(ar->timeusec_last,ar->timesec_last); 
  double timediff=timediffi; 
  if(ar->timetot > (ar->timetot+timediffi))ar->overflow++;
  ar->timetot=ar->timetot+timediffi;
  ar->timetotd = MAX32*ar->overflow + ar->timetot;
  
  ar->ActiveCountsL2[i]=ar->ActiveCountsL2[i]+dodif32(ar->classcount_lastL2[i],counts[5]);  
  ar->ActiveCountsL0[i]=ar->ActiveCountsL0[i]+dodif32(ar->classcount_lastL0[i],counts[0]);  
  // print temporarily here
  double l0diff = dodif32(ar->classcount_lastL0[i],counts[0]); 
  double l2diff = dodif32(ar->classcount_lastL2[i],counts[5]);
  double rateL0= l0diff/timediff/0.4*1.e6; 
  double rateL2= l2diff/timediff/0.4*1.e6; 
  printf("%s: %8.0f %8.0f %10.5f %10.5f %10.5f \n",ar->classNames[i],l0diff,l2diff,rateL0,rateL2,l2diff/l0diff);
 } 
 ar->classcount_lastL0[i]=counts[0]; 
 ar->classcount_lastL2[i]=counts[5]; 
}
/*-------------------------------------------------------writeFile()
 Write counters to the file
 Order of classes is the same as in rcfg file.
*/
int writeFile(int ix,w32 *buffer,int notfirst){
 int ret=0;
 ActiveRun *ar;
 ar=activeruns[ix];
 //printActiveRuns();
 //for(i=0;i<20;i++)printf("%u ",buffer[CSTART_SPEC-3+i]);printf("\n"); 
 if(ar->file){
   int i;
   updateTime(ar,buffer,notfirst);
   int classcount[6];
   // Print 6 counters per class (L0before,L0after,L1before,...)
   for(i=0;i<ar->nclasses;i++){
     if(ar->ActiveClasses[i]){
      getClassCount(ar,i,classcount,buffer,notfirst);
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
 char msg[200];
 ar=activeruns[ix]; 
 // register with DCS FXS (this executable is started from ~tri home directory):
 // counter file
 if(ar->file){
   fclose(ar->file);
   prtLog(msg);
 }else{
   sprintf(msg,"run %d: xcounter file %s not send to FXS.\n",ar->runnumber,ar->filename);
   prtError(msg);
 }
 fflush(stdout);
 // DAQ logbook info
 printL2ascalers(ar);
 activeruns[ix]=freeActiveRun(ar);
 return 0;
}
/* Defaly print for no run:
   - inputs
*/
void printDefault(w32 *buffer)
{
 char *inputsNames[]={"0ASL","0AMU","0OB3","0OCP","0SCO","0HPT"};
 int inputs[]={7,8,14,18,20,17};
 int i,ninp=6;
 if(defRun==0){
   defRun = (ActiveRun *) malloc(sizeof(ActiveRun));
   defRun->ninputs=ninp;
   defRun->Inputs = (w32 *) malloc(ninp*sizeof(w32));  
   defRun->InputsCounts  = (w32 *) malloc(ninp*sizeof(w32));  
   defRun->InputsCounts_last = (w32 *) malloc(ninp*sizeof(w32));
   defRun->timetot=0;
   defRun->timetotd=0;
   for(i=0;i<ninp;i++){
     defRun->Inputs[i]=inputs[i];
     defRun->InputsCounts_last[i]=buffer[CSTART_L0+65+inputs[i]];
     defRun->InputsCounts[i]=0;
   }
   defRun->timesec_last=buffer[CSTART_L0+13];
   //printInputs(defRun);
   return; 
 }
 printf("Name      Counts        Rate        <Rate>  \n");
 int time=0;
 time=buffer[CSTART_L0+13];
 int timediffi=dodif32(defRun->timesec_last,time); 
 double timediff=timediffi; 
 //defRun->timetot=defRun->timetot+timediffi;
 if(defRun->timetot > defRun->timetot+timediffi)defRun->overflow++;
 defRun->timetot=defRun->timetot+timediffi;
 defRun->timetotd = MAX32*defRun->overflow + defRun->timetot;
  
 for(i=0;i<ninp;i++){
    int count=buffer[CSTART_L0+65+inputs[i]];
    defRun->InputsCounts[i]=defRun->InputsCounts[i]+dodif32(defRun->InputsCounts_last[i],count);  
    double diff=dodif32(defRun->InputsCounts_last[i],count);
    double rate=diff/timediff/0.4*1000000.;
    double timetot=defRun->timetot*0.4*1.0e-6;
    double arate=(double)(defRun->InputsCounts[i])/ defRun->timetotd;
    //printf("%s %8u %f %f %f\n",inputsNames[i],counts[i],rate,diff,timediff);
    printf("%s %10u %12.3f %12.3f %12.3f\n",inputsNames[i], defRun->InputsCounts[i] ,rate,arate,timetot);
    defRun->InputsCounts_last[i]=count;
 }
 defRun->timesec_last=time;
}
/*-----------------------------------------------------------------------
*/
void updateCountFiles(w32 *buffer){
 int ix,norun=1;
 char dt[20]; 
 unsigned int PartRunNumNew[NPARTIT];
 getdatetime(dt);
 //printf("%s Active run numbers: ",dt);
 for(ix=RUNXCOUNTERSSTART;ix<(RUNXCOUNTERSSTART+NPARTIT);ix++){
    //printf("%d ",buffer[ix]);
    PartRunNumNew[ix-RUNXCOUNTERSSTART]=buffer[ix];
 }
 printf("#######################################################################\n");
 printf("INPUTS %s \n",dt);
 printf("-----------------------------------------------------------------------\n");
 printDefault(buffer);
 for(ix=0;ix<NPARTIT;ix++){
    if(PartRunNumNew[ix] && (activeruns[ix]==NULL)){
       // start run
       printf("%s change detected: Run %d started. \n",dt, PartRunNumNew[ix]);
       openFile(ix,PartRunNumNew[ix]);
       //printf("RUN %i %s\n",ar->runnumber,ar->partitionname);
       writeFile(ix,buffer,0);
       norun=0;
    }else if((PartRunNumNew[ix]==0) && activeruns[ix]){
       // stop run
       printf("%s change detected: Run %d stopped. \n ",dt, activeruns[ix]->runnumber);
       printf("RUN %i %s",activeruns[ix]->runnumber,activeruns[ix]->partitionname);
       writeFile(ix,buffer,1);
       closeFile(ix);
       norun=0;
    }else if(PartRunNumNew[ix] && activeruns[ix]){
       // running run
       printf("####################################################################\n");
       printf("RUN %i %s",activeruns[ix]->runnumber,activeruns[ix]->partitionname);
       printf("CLASS        L0       L2     L0rate     L2rate   L2/L0 \n");
       printf("--------------------------------------------------------------------\n");
       writeFile(ix,buffer,1);
       norun=0;
    }
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
