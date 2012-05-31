/*
7.4.2005 - the name of flash memory file is devised from CODE_ADD
           vme register. I.e. there is less probability of loading
	   wrong file by mistake
26.5.2008 
CTP: .rbf file is taken from $VMECFDIR (before it was current working dir)
LTU: .rbf is taken from FlashMem.cfg   (i.e. v/vme/FlashMem.cfg)
*/
#include "vmefpga.h"
#include "vmewrap.h"
#include <stdio.h>
#include <stdlib.h>     // malloc
#include <unistd.h>     // usleep
#include <string.h>
#include <sys/time.h>


#define Mega 1024*1024
#define kilo 1024
#define FMsize Mega
#define wDelay 4000
#define rDelay 1
#define wAttempts 4000
#define prt100 100000

#define BUSYFM 0       /* FMem Ready/Busy status */
#define READYFM 1
#define BUSYFP 1       /* FPGA Ready/Busy status */
#define READYFP 0
#define MAXCYC 0x80
#define CUT 0

extern int quit;
static w32 FlMem[FMsize];
static w32 ConfigFile[FMsize];
static int AddErr[FMsize];
static int DatErr[FMsize];
int FileSize;
int errcount;
w32 Pattern;

void PrintMon(int n,w32 *m,double time);
int loadFPGA();
int resetFM();
int MonitorStatus(w32 adresa, short **bity);
int GetStatFPGA();
int writeFM(w32 address, w32 data);
int loadFM();
int GetStatFM();
int ReadInputFile();
int checkcontent();
void EraseFM();
int eraseFM();
int ReadInputFileS(int *size);
int ReadInputFileF(int size);
int ReadFlashMem();

/*****************************************************************************/
/*------------------------FPGA-----------------------------------------------*/
/*****************************************************************************/

/*FGROUP Monitor GUI MonitLoad
 Graphical interface to MonitorLoad. Measure FPGA signals 'Ntimes' waiting
 before measurement 'Tscale' times in arbitrary units, so Tscale should be 
 set up by set and try. Time interval between measurements is estimated
 as (Time after measurement - time at start)/Ntimes. 
*/

int GetStatFPGA() {
 int stat;
 w32 status,bit0=1;
 status=vmer32(ConfigStatus);
 stat= ( (status&bit0) == bit0 );
 return stat;
}
/*FGROUP FPGA GetStatusFPGA
Get Status of FPGA. 

*/
void GetStatusFPGA() {
 printf("FPGA status is <%i> \n",GetStatFPGA());
}
/*FGROUP Monitor MonitorLoad
  C code to MonitLoad. Can be called also indepedently.
  It loads FPGA and monitor signals without outputing them,
  which leads to maximum time information.
 */
void MonitorLoad(int Ntimes,int Tscale)
{
 int i,j;
 double time;
 struct timeval tval0,tval;
 struct timezone tz;
 w32 *mon= (w32 *) malloc(Ntimes*sizeof(w32));
 if(GetStatFPGA() == READYFP ){
   gettimeofday(&tval0,&tz);                         // Start time
   vmew32(ConfigStart,0x0);                          // Start Configuration
   for(i=0;i<Ntimes;i++){
	   mon[i]= vmer32(ConfigStatus);// Monitor
	   for(j=0;j<Tscale;j++)time=0.; // Time delay
   }	   
   gettimeofday(&tval,&tz);                          // End time
   time= (tval.tv_sec-tval0.tv_sec)*1000.+(tval.tv_usec-tval0.tv_usec)/1000.;
   PrintMon(Ntimes,mon,time);
   printf("%i values read during <%f> msecs \n",Ntimes,time);
   free(mon);   
 }else{
   printf("FPGA is nor READY \n");
 }
}
/*--------------------------------------------------------------------*/
/*FGROUP FPGA LoadFPGA
Loads FPGA from Flash memory

*/
void LoadFPGA()
{
 switch(loadFPGA()){
  case BUSYFP : printf("Error:FPGA loading unsuccesful. Timeout in 400 milisecs. \n");break;
  case READYFP: printf("FPGA loading succesful!. \n");break;
  case       2: printf("Error: Cannot load FPGA. Flash Memory busy, check status.\n");break;
  case       3: printf("Error: Cannot load FPGA. FPGA busy. \n");break;
  case       4: printf("Error: Cannot load FPGA, CONF_DONE LOW ! \n");break;
  case       5: printf("Error: Cannot load FPGS, INIT_DONE LOW !");break;
  default     : printf("Error: Unexpected return from loadFPGA. Check code. \n");
 }
}
int loadFPGA(){
 int i=0,j,stat,notfinished=1;
 struct timeval tval0,tval;
 struct timezone tz;
 double time;
 short *bit;
 if(resetFM() == 0){
   printf("Rset FM ok \n");	 
   if(GetStatFPGA() == READYFP ){
     printf("FPGA configuration started. \n");
     /* while( (i<10) && ((stat=GetStatFPGA())== BUSYFP) ) */
     printf("RY/BY   nSTATUS  CONF_DONE INIT_DONE  nCONFIG    DCLK \n"); 
     vmew32(ConfigStart,0x0);
     gettimeofday(&tval0,&tz);
     MonitorStatus(ConfigStatus,&bit);
     for(j=0; j<6; j++)printf("%5i     ",bit[j]);printf("\n");
     while(i<1000 && notfinished)
     {
      usleep(1);
      MonitorStatus(ConfigStatus,&bit);\
      /* 
        Checking CONF_DONE
        Should go down after 0.8 us and after configuration up.  
      */
      if( (bit[2] == 1))notfinished=0;  
      i++;
     }
     if(bit[2] == 0){
      gettimeofday(&tval,&tz);
      for(j=0; j<6; j++)printf("%5i     ",bit[j]);printf("\n");
      time= (tval.tv_sec-tval0.tv_sec)*1000.+(tval.tv_usec-tval0.tv_usec)/1000.;
      printf("Time duration from vmew32(ConfigStart,dummy) untill now %f milisec.\n",time);
      return 4;
     }
     /*
       bit[3] = INIT_DONE
       After CONF_DONE goes up INIT_DONE needs another 20us to go up.
     */
     i=0;
     notfinished=1;
     while(i<3000 && notfinished){
       usleep(1);    
       MonitorStatus(ConfigStatus,&bit);
       if(bit[3]== 1)notfinished=0;
       i++;
     }    
     if( bit[3] == 0){
        gettimeofday(&tval,&tz);
        for(j=0; j<6; j++)printf("%5i     ",bit[j]);printf("\n");
        time= (tval.tv_sec-tval0.tv_sec)*1000.+(tval.tv_usec-tval0.tv_usec)/1000.;
        printf("Time duration from vmew32(ConfigStart,dummy) untill now %f milisec.\n",time);  
        return 5;      
     }      
     i=0;
     notfinished=1;
     while(i<500000 && notfinished){
       usleep(1);
       stat=GetStatFPGA();
       if(stat == READYFP) notfinished =0;
     }   
     gettimeofday(&tval,&tz);
     time= (tval.tv_sec-tval0.tv_sec)*1000.+(tval.tv_usec-tval0.tv_usec)/1000.;
     for(j=0; j<6; j++)printf("%5i     ",bit[j]);printf("\n");
     printf("Time duration from vmew32(ConfigStart,dummy) untill now %f milisec.\n",time);  
     return stat;
   }else{ 
         MonitorStatus(ConfigStatus,&bit);
         for(j=0; j<6; j++)printf("%5i     ",bit[j]);printf("\n");
         return 3;
   }      
 }else return 2;
}
/*FGROUP Monitor MonitorStatusFPGA
 Monitors FPGA by reading status word.
 Ntimes: number of times you want to read status word.
 Tinterval: time interval between reading status word in microseconds. 

*/
void MonitorStatusFPGA(int Ntimes, int Tinterval)
{
 int i,j;
 short *bit;
 for(j=0;j<Ntimes;j++){
   if((j/20)*20 -j == 0)
/*       12345678901234567890123456789012345678901234567890 */
   printf("TIME   RY/BY   nSTATUS  CONF_DONE INIT_DONE  nCONFIG    DCLK \n");   
   MonitorStatus(ConfigStatus,&bit);
   printf("%4i",j);
   for(i=0; i<6; i++)printf("%5i     ",bit[i]);
   printf("\n");
   free(bit);
   usleep(Tinterval); 
 }
/*  printf("MonitorStatusFPGA finished.\n"); */
}
/*FGROUP Monitor MonitorChange
 Loops infinitelly reading status and detect change
 */
void MonitorChange()
{
 struct timeval tval0,tval;
 struct timezone tz;
 double time;
 w32 mask=0xff;
 int i,change=1;
 w32 status1,status2;
 gettimeofday(&tval0,&tz);
 status1 = vmer32(ConfigStatus) & mask;
 printf("time=0         ");
 /* pit=64;
 for(i=0; i<7; i++){
   bit = ((status1 & pit) == pit);  
   printf("%1i",bit);  
   pit=pit/2;
 }  */ 
 printf(" %2x \n",status1); 
 //printf("time=0 status=%b \n",status1);
 while(change){
  for(i=0;i<100000;i++){	 
   status2= vmer32(ConfigStatus) & mask;	 
   if(status2 != status1){	  
    gettimeofday(&tval,&tz);	  
    time = (tval.tv_sec-tval0.tv_sec)*1000.+(tval.tv_usec-tval0.tv_usec)/1000.;
    printf("time=%9.1f  ",time);
    /* pit=64;
    for(i=0; i<7; i++){
      bit = ((status2 & pit) == pit);  
      printf("%1i",bit);  
      pit=pit/2;
    }; */  
    //gettimeofday(&tval,&tz);	  
    //time2 =(tval.tv_sec-tval0.tv_sec)*1000.+(tval.tv_usec-tval0.tv_usec)/1000.;
    //printf(" %2x time2:%9.1f\n",status2,time2); 
    printf(" %2x\n",status2);
    status1=status2;   
   } 
   //printf("time=%9f.1 status=%b \n",time,status2);
   fflush(stdout);
  }
 }
 //printf("Change detected \n"); 
}
/*****************************************************************************/
/*-------------Flash Memory--------------------------------------------------*/
/******************************************************************************/
/*FGROUP FlMem GetStatus
Get Status of Flash memory.
*/
void GetStatusFM() {
 printf("Flash memory status is %i \n",GetStatFM()); 
}
int GetStatFM() {
 int stat;
 w32 status,bit7=128;
 status=vmer32(FlashStatus);
 stat = ( (status&bit7) == bit7 );
 return stat;
}
/*****************************************************************************/
/*FGROUP FlMem ResetFM
Resets Flash Memory.
*/
void ResetFM(){
 switch(resetFM()){
  case  0:printf("ResetFM: succes. \n");break;
  case  1:printf("Error: ResetFM: timeout.\n");break;
  case  2:printf("Error: ResetFM: FM busy.\n");break;
  default:printf("Error: ResetFM: Unexpected return code. Check code. \n");
 }
}
int resetFM()
{
 if( 1 /* GetStatFM() == READYFM*/ ){
    if(writeFM(FlashAccessNoIncr,0xF0) !=0 )return 1; else return 0;
 }else return 2;
}
/******************************************************************************/
char FileName[99]="notinitialised";
int doFileName() {
w32 boardcode;
char *environ;
char bname[120];
environ= getenv("VMECFDIR"); strcpy(bname, environ);
 boardcode= 0xff&vmer32(CODE_ADD);
 if( (boardcode>=0x50) && (boardcode<=0x56)) {
   printf("boardcode:%x\n",boardcode);
 } else return 11;
  if( boardcode==0x50) {
   strcpy(bname,"l0");
 } else if( boardcode==0x51) {
   strcpy(bname,"l1");
 } else if( boardcode==0x52) {
   strcpy(bname,"l2");
 } else if( boardcode==0x53) {
   strcpy(bname,"fo");
 } else if( boardcode==0x54) {
   strcpy(bname,"busy");
 } else if( boardcode==0x55) {
   strcpy(bname,"int");
 } else if( boardcode==0x56) {
   strcpy(bname,"ltu");
 } else {
   printf("Unknown board code:%x.\n", boardcode);
   return 11;
 };
if( boardcode == 0x56) {
  strcpy(FileName,"FlashMem.cfg");
} else {
  sprintf(FileName,"%s/CFG/%s/%s.rbf", environ, bname, bname);
};
printf("Flash memory file name in WORKDIR:%s\n",FileName);
return 0;
}
void printErrMsg(int errnum) {
 switch(errnum){
  case 0: printf("FM loading succesful ! \n");break;
  case 2: printf("The Input File for Flash memory can not be read.\n");break;
  case 1: printf("Problem reading Input File \n");break;
  case 3: printf("Flash memory busy. Write not allowed. \n");break;
  case 4: printf("LoadFM: problem to clear address \n");break;
  case 5: printf("LoadFM: problem in 1st word \n");break;
  case 6: printf("LoadFM: problem in 2nd word \n");break;
  case 7: printf("LoadFM: problem in 3rd word \n");break;
  case 8: printf("LoadFM: problem in 4th word \n");break;
  case 9: printf("LoadFM: interrupted by user. \n");break;
  case 10: printf("LoadFM: attempt to load wrong file\n");break;
  case 11: printf("LoadFM: Unknown board\n");break;
  default:printf("LoadFM: unexpected return code. Check code. \n");
 }
}

/*FGROUP FlMem LoadFM
Loads Flash Memory from the file on disk.
*/
void LoadFM() {
 printErrMsg(loadFM());
}
int loadFM()
{int rc;
 int i,idum,size;
 /* before loading, find file_name */
 rc= doFileName();
 if(rc !=0) return(rc);
 
 if(ReadInputFileS(&size) != 0) return 2;
 if(ReadInputFileF(size) != 0)return 1;
 
 if( GetStatFM() == READYFM ){
   if(writeFM(FlashAddClear,0x0) !=0) return 4;
   for (i=0; i<size; i++){
   if(writeFM(FlashAccessNoIncr,0xAA) !=0) return 5;
   if(writeFM(FlashAccessNoIncr,0x55) !=0) return 6;
   if(writeFM(FlashAccessNoIncr,0xA0) !=0) return 7;
   if(writeFM(FlashAccessIncr,ConfigFile[i]) !=0) return 8; 
   /* printf("LoadFM: ConfigFile[%i]=%c \n",i,ConfigFile[i]); */
   /* if( (i % 10) == 0){ printf(" %i \r",i);fflush(stdout);} */
   if( (i % prt100) == 0) printf("lOadFM: %i words written \n",i);
   for(idum=0;idum<10;idum++)
   if(quit !=0) return 9;
   }; printf("lOadFM: %i words written \n",i);
 return 0;
 }else return 3;  
}
/*FGROUP FlMem ReadInputFile
Reads the input configuration file from disk
and saves it to array.
*/
int ReadInputFile()
{int rc;
 int size;
 rc= doFileName();
 if(rc !=0) {
   printErrMsg(rc); return(rc);
 };
 if(ReadInputFileS(&size) == 0){
 printf("Size of the Input File is %i\n",size);
 return 0;
 }else return 1; 
}
/******************************************************************************/
/*FGROUP FlMem CheckContent
Reads Content of Flash Memory and compares it
with the file on disk.
*/
void CheckContent()
{
 switch(checkcontent()){
  case 0:printf("CheckContent: finished succesfully.\n");break;
  case 2:printf("Error: CheckContent: The Input File for Flash memory ");
         printf("cannot be read.\n");break;
  case 3:printf("Error: CheckContent: Cannot read Flash Memory \n");break;
  case 1:printf("Error: CheckContent: Problem reading Input File \n");break;
  default:printf("Error: CheckContent: unexpected return code. Check the code \n");
 }
}
int checkcontent() {
 int size,diftot=0,diff[8]={0,0,0,0,0,0,0,0};
 int i,j,error,rc;
 w32 difbit,pit; 
 FILE *FMdif;
 rc= doFileName();
 if(rc !=0) {
   printErrMsg(rc); return(rc);
 };
 
 if(ReadInputFileS(&size) != 0) return 2;
 if(ReadInputFileF(size) != 0) return 1;
 if(ReadFlashMem() != 0) return 3;
 FMdif=fopen("FMdif.txt","w");
 for(i=0;i<size;i++){
  error=0;
  difbit=(FlMem[i]^ConfigFile[i]);
  if(FlMem[i] != ConfigFile[i]){
     diftot++;
     error=1;
  } 
  fprintf(FMdif,"%5x File: %2x %2x ",i,ConfigFile[i],FlMem[i]);
  if(error)fprintf(FMdif," Error ");
  fprintf(FMdif,"\n");
  pit=1;
  for(j=0;j<8;j++){
      /*  printf("%i ", difbit&pit );                    */
      diff[j]=diff[j]+( (difbit&pit) == pit );
      pit=2*pit;
  }
  /*  printf("\n");   */
 }
 printf("CheckContent: Total number of different words= %i. \n",diftot);
 printf("Differences bit by bit: \n");
 printf(" Bit0 Bit1 Bit2 Bit3 Bit4 Bit5 Bit6 Bit7 \n");
 for(i=0;i<8;i++)printf(" %.2f",diff[i]/((float) size));
 printf("\n");
 fclose(FMdif);
 return 0;
}
/********************************************************************************/
/*FGROUP FlMem EraseFM
Erases Flash Memory.
*/
void EraseFM() {
 switch(eraseFM()){
  case 1: printf("EraseFM: FM erase  done succesfully. \n");break;
  case 0: printf("Error: EraseFM: timeout at the end. \n");break;
  case 2: printf("Error: EraseFM: timeout at word 1. \n");break;
  case 3: printf("Error: EraseFM: timeout at word 2. \n");break;
  case 4: printf("Error: EraseFM: timeout at word 3. \n");break;
  case 5: printf("Error: EraseFM: timeout at word 4. \n");break;
  case 6: printf("Error: EraseFM: timeout at word 5. \n");break;
  case 7: printf("Error: EraseFM: Erase not started, FM busy \n");break;
  default: printf("Error: EraseFM: unexpected return code. Check code. \n");
 }
}
int eraseFM() {
 int i=0,stat; 
 if( GetStatFM() == READYFM ){
   if(writeFM(FlashAccessNoIncr,0xAA) !=0) return 2;
   if(writeFM(FlashAccessNoIncr,0x55) !=0) return 3;
   if(writeFM(FlashAccessNoIncr,0x80) !=0) return 4;
   if(writeFM(FlashAccessNoIncr,0xAA) !=0) return 5;
   if(writeFM(FlashAccessNoIncr,0x55) !=0) return 6;
   vmew32(FlashAccessNoIncr,0x10);
   while ( (i<50) && ( (stat=GetStatFM()) == BUSYFM ) ){
     usleep(999999);
     if((i%10)==0) {
       printf("%i secs: Erasing FM in progress, status=%i \n",i+1,stat);
     };
     i++;
   };
   printf("%i secs: Erasing FM in progress, status=%i \n",i+1,stat);
   return stat;
 }
 else return 7;
}
/****************************************************************************/
/* Read Input File, find its size */
int ReadInputFileS(int *size) {
 FILE *InputFile;
 int i=0,end; w8 word;
 size_t nmemb=1,nread;
 InputFile=fopen(FileName,"rb");/* File is in directory vme */
 if(InputFile == NULL){
  printf("Error: ReadInputFileS:Input File can not be opened. \n");
  *size=0;
  return 1;
 }else{
  printf("ReadInputFileS:Input File succesfully opened. \n");
  i=0;
  while((nread=fread(&word,1,nmemb,InputFile)) == nmemb){
   /*printf("ReadInputFileS:i=%i j=%i word=%2x \n",i,nread,word); */
   i++;
   if(i > FMsize){
     printf("Error: ReadInputFileS:Input File seems too large! \n");
     return 2;
   }  
  }
  end=feof(InputFile);
  if(end != 1){
     printf("Error: ReadInputFileS:Not a real end of file. Check the input file. \n ");
     return 3;
  }
  *size=i;
  /*  printf("*size=%i, size=%u \n",*size,size); */
  }
  fclose(InputFile);
  return 0;
}  
/*****************************************************************************/
/* Read Input File. */
int ReadInputFileF(int size) {
 int i;
 FILE *InputFile;
 w8 word;
 size_t nmemb=1,nread;
  
 InputFile=fopen(FileName,"rb");/* File is in directory vme */
 if(InputFile == NULL){
  printf("Error: ReadInputFileF:Input File can not be opened. \n");
  return 1;
 } 
 
 /* printf("Address of file is %u \n",ConfFile); */
 for(i=0;i < size;i++){
   nread=fread(&word,1,nmemb,InputFile);
   if(nread != nmemb){
      printf("Error: ReadInputFileF:Error in Reading Config File from LoadFM \n");
      return 3;
   }
   ConfigFile[i]=0;
   ConfigFile[i]=word;
   /*printf("ReadInputFileF:ConfigFile[%i]=%i \n",i,ConfigFile[i]);*/ 
 }
 fclose(InputFile);
 return 0;
}
/*FGROUP FlMem 
Check against 1
*/
int Check1()
{
 int i,dif=0;
 if(ReadFlashMem() != 0) return 3;
 for(i=0;i<Mega;i++){
  if( (i % 100000) == 0)printf("Check1 in progress %d\n",i);
  if(FlMem[i] != 0xff){
     dif++;
     printf("i=%i FM= %x \n",i,FlMem[i]);
     return i;
  }
 } 
 if(dif == 0)
  printf("Check1 success: the number of different words is %i \n",dif);
 else
  printf("Error Check1: the bumber of different words is %i \n",dif);
 return 0;
}
/***************************************************************************/
/**************************TOOLS********************************************/
/***************************************************************************/
int MonitorStatus(w32 adresa, short **bity)
/*   Decompose the first 8bits of the word at address 'adresa' to bits 
     and returns array of 8 bits.
*/
{
 int i;
 w32 pit;
 w32 statword;
 short *bit =(short *) malloc(8*sizeof(short));
 
 statword=vmer32(adresa);

/* printf("statword=%x \n",statword); */
 
 pit=1;
 for(i=0; i<8; i++){
     bit[i] = ((statword & pit) == pit);  /* printf("%1i",bit[i]); */  
     pit=pit*2;
 }   /* printf("\n"); */
 *bity=bit;
 return 0;
}
void PrintMon(int n,w32 *m,double time) {
 FILE *f;
 char *name[]={"RY/BU ","nSTATU","CONF_D","INIT_D","nCONFI","DCLK  "}; 
 int i,j;
 short bit; w32 pit;
 f=fopen("WORK/FPGA.mon","w");
 for(j=0;j<n;j++){
   pit=1;
   printf("<%i> %4.1f ",j,j*time/n*1000.);
   fprintf(f,"%i",j);
   for(i=0; i<8; i++){
     bit = ((m[j] & pit) == pit);  
     fprintf(f," %1i ",bit); 
     if(i<6)printf(" %s <%1i> ",name[i],bit);  
     pit=pit*2;
   }   
   printf("\n"); 
   fprintf(f,"\n");
 }
 fclose(f);
}
/*****************************************************************************/
/*FGROUP FlMem  */
int ReadFlashMem() {
 const int size=FMsize;
 int i;
 printf("Reading of FM started.\n");
 if( resetFM() != 0){
  printf("Error: ReadFlashMem: Cannot resetFM. \n");
  return 2;
 }
 if(GetStatFM() == READYFM){
    if(writeFM(FlashAddClear,0x0) !=0) return 2;
    for(i=0; i<size; i++){
     if((i % 10000) == 0)printf("Reading FM in progress %d \n",i);
     FlMem[i]=vmer32(FlashAccessIncr)&0xff;
     /*printf("ReadFM from board: FlMem[%i]=%x \n",i,FlMem[i]);*/
    }  
    return 0;     
 }else{
  printf("Error: ReadFlashMem: Cannot read FM, FM busy. \n ");
  return 1;
 }
}
/***************************************************************************/
int writeFMdeb(w32 address, w32 data,w32 *addFM)
/* write to FM and checks for timeout */
{
 int attempts=0;
 vmew32(address,data); 
 while ( (GetStatFM() == BUSYFM) && (attempts < wAttempts) )attempts=attempts+1;
 if(attempts >= wAttempts){
  printf("Unable to write (a:%x data:%x)in Flash Memory after %i attempts \n",
          address,data,wAttempts);
  return 1;
 }else{
 /*vmew32(FlashAccessNoIncr,0xF0);*/
 *addFM=vmer32(FlashStatus);
 *addFM = ((*addFM)&0x7f); 
 return 0;
 }
}
/***************************************************************************/
int writeFM(w32 address, w32 data)
/* write to FM and checks for timeout */
{
 int attempts=0;
 vmew32(address,data); 
 while ( (GetStatFM() == BUSYFM) && (attempts < wAttempts) )attempts=attempts+1;
 if(attempts >= wAttempts){
  printf("Unable to write (a:%x data:%x)in Flash Memory after %i attempts \n",
          address,data,wAttempts);
  return 1;
 }else{
   return 0;
 }
}
int readFM(w32 address)
{
 int data,attempts=0;
  data=vmer32(address);
  while ( (GetStatFM() == BUSYFM) && (attempts < wAttempts) )attempts=attempts+1;
  if(attempts >= wAttempts){
      printf("Unable to write (a:%x data:%x)in Flash Memory after %i attempts \n",
                address,data,wAttempts);
      return -1;
   }else{
    return data;
 }
}
                       
/***************************************************************************/
/*FGROUP FlMemChecks
write to FM at address address.
*/
int WriteFM(w32 address,w32 word)
{
   w32 i;
   if(writeFM(FlashAddClear,0x0) !=0) return -1;
   for(i=0;i<address;i++)vmer32(FlashAccessIncr);
   if(writeFM(FlashAccessNoIncr,0xAA) !=0) return -5;
   if(writeFM(FlashAccessNoIncr,0x55) !=0) return -6;
   if(writeFM(FlashAccessNoIncr,0xA0) !=0) return -7;
   if(writeFM(FlashAccessIncr,word) !=0) return -8;
   return 1;   
}
/*FGROUP FlMemChecks
INCR=1 - read from address drom actual address
INCR=0 - read address
*/
int ReadFM(w32 address,int INCR)
{
/* Read Address in FM
*/

   w32 word,ix;
   if(INCR){
   }else{ 
    if(writeFM(FlashAddClear,0x0) !=0) return -1;
    for(ix=0;ix<address;ix++)vmer32(FlashAccessIncr);
   } 
   word=vmer32(FlashAccessIncr)&0xff;
   printf("ADDRESS=%d DATA=%x \n",address,word);
   return address;
}
/*FGROUP FlMemChecks
Writing pattern pat.
pat>0xff : pat goes from 0 to 255 in cycle.
file = 1 - read from config file
file = 0 - uses generated pattern
*/
int LoadFM0(int pat,int Nwords,int file)
{
 int i,size,addSW;
 int rc,error;
 w32 pat2,addFM, add1;
 FILE *load=fopen("LOAD.txt","w");
 errcount=0;
 rc= doFileName();
 if(rc !=0) {
   printErrMsg(rc); return(rc);
 };
 if(file){
   if(ReadInputFileS(&size) != 0) return 2;
   if(ReadInputFileF(size) != 0)return 1;
   Nwords=size;
 }else{
   Pattern=pat; 
   if(pat>0xff)Pattern=0;
 }  

 if(writeFM(FlashAccessNoIncr,0xF0) !=0 )return 3;
  if( GetStatFM() == READYFM ){
   if(writeFM(FlashAddClear,0x0) !=0) return 10;
   for (i=0; i<Nwords; i++){
     addSW=i&0x7f; 
     error=0;
     if(file) pat2=ConfigFile[i]; 
       else{
        pat2=Pattern;
        if(Pattern<CUT) pat2=0xaa;
     }   
     fprintf(load," Pattern: %2x Address: %5x %2x ",pat2,i,addSW);
     if(writeFMdeb(FlashAccessNoIncr,0xAA,&addFM) !=0){
        printf("Error at address: %i %2x  Status addr= %x\n",i,i,addFM); 
        return 5;
     }
     add1=addFM;
     fprintf(load,"Word #1: %2x ",addFM);      
     if(writeFMdeb(FlashAccessNoIncr,0x55,&addFM) !=0){
        printf("Error at address: %i %2x  Status addr= %x\n",i,i,addFM); 
        return 6;
     }
     if(add1 != addFM) return 333;
     fprintf(load,"#2: %2x ",addFM);      
     if(writeFMdeb(FlashAccessNoIncr,0xA0,&addFM) !=0){
        printf("Error at address: %i %2x  Status addr= %x\n",i,i,addFM); 
        return 7;
     }   
     if(add1 != addFM) return 334;
     fprintf(load,"#3: %2x ",addFM);

     if(writeFMdeb(FlashAccessIncr,pat2,&addFM) !=0){
        printf("Error at address: %i %2x Status addr= %x\n",i,i,addFM); 
        return 8;
     }
     if( ((add1+1)%0x80) != addFM){
       error=1;
       errcount=errcount+error;
     }  
     fprintf(load,"#4: %2x ",addFM);   
     if(pat>0xff)Pattern= (Pattern+1) % MAXCYC; 
     if( (i % prt100) == 0) printf("LOadFM0: %i words written \n",i);
     if(error)fprintf(load," Error %i \n",errcount);
       else fprintf(load,"\n");  
     if(quit !=0) return 9;
   }
 if(pat>0xff)Pattern=0xfff;  
 return 0;
 }else return 3;  
}
/*FGROUP FlMemChecks
Writing pattern pat.
pat>0xff : pat goes from 0 to 255 in cycle.
file = 1 - read from config file
file = 0 - uses generated pattern
Different way of writing to FM:
 I do not increase at last step.
 I do increase with reset.
*/
int LoadFMErr(int pat,int Nwords,int file)
{
 int i,size,addSW;
 int rc,error;
 w32 pat2,addFM,add1;
 FILE *load=fopen("LOAD.txt","w");
 errcount=0;
 rc= doFileName();
 if(rc !=0) {
   printErrMsg(rc); return(rc);
 };

 if(file){
   if(ReadInputFileS(&size) != 0) return 2;
   if(ReadInputFileF(size) != 0)return 1;
   Nwords=size;
   FileSize=size;
 }else{
   Pattern=pat; 
   if(pat>0xff)Pattern=0;
 }  

 if(writeFM(FlashAccessNoIncr,0xF0) !=0 )return 3;
  if( GetStatFM() == READYFM ){
   if(writeFM(FlashAddClear,0x0) !=0) return 10;
   for (i=0; i<Nwords; i++){
     addSW=i&0x7f; 
     error=0;
     if(file) pat2=ConfigFile[i]; 
       else{
        pat2=Pattern;
        if(Pattern<CUT) pat2=0xaa;
     }   
     fprintf(load," Pattern: %2x Address: %5x %2x ",pat2,i,addSW);
     if(writeFMdeb(FlashAccessNoIncr,0xAA,&addFM) !=0){
        printf("Error at address: %i %2x  Status addr= %x\n",i,i,addFM); 
        return 5;
     }
     add1=addFM;
     fprintf(load,"Word #1: %2x ",addFM);      
     if(writeFMdeb(FlashAccessNoIncr,0x55,&addFM) !=0){
        printf("Error at address: %i %2x  Status addr= %x\n",i,i,addFM); 
        return 6;
     }
     if(add1 != addFM) return 333;
     fprintf(load,"#2: %2x ",addFM);      
     if(writeFMdeb(FlashAccessNoIncr,0xA0,&addFM) !=0){
        printf("Error at address: %i %2x  Status addr= %x\n",i,i,addFM); 
        return 7;
     }   
     if(add1 != addFM) return 334;
     fprintf(load,"#3: %2x ",addFM);

     if(writeFMdeb(FlashAccessNoIncr,pat2,&addFM) !=0){
        printf("Error at address: %i %2x Status addr= %x\n",i,i,addFM); 
        return 8;
     }
     if(add1 != addFM) return 335;
     fprintf(load,"#4: %2x ",addFM);   
     if(writeFMdeb(FlashAccessIncr,0xF0,&addFM) !=0)return 88;
     fprintf(load,"#5: %2x ",addFM);          
     if( ((add1+1)%0x80) != addFM){
       error=1;
       AddErr[errcount]=i+1;
       DatErr[errcount]=ConfigFile[i+1];
       fprintf(load," Error %i \n",errcount);
       fprintf(load,"     Address=%5x, Data=%2x \n",AddErr[errcount],DatErr[errcount]);
       errcount=errcount+error;
       i=i+1;
     }  
     if(pat>0xff)Pattern= (Pattern+1) % MAXCYC; 
     if( (i % prt100) == 0) printf("LOadFMErr: %i words written \n",i);
     if(!error)fprintf(load,"\n");  
     if(quit !=0) return 9;
   }
 if(pat>0xff)Pattern=0xfff;
 printf("Number od errors detected is %i \n",errcount);  
 return 0;
 }else return 3;  
}
/*---------------------------------------------------------*/
void throwit(int k){
 int i;
 for(i=k;i<errcount-1;i++){
  AddErr[i]=AddErr[i+1];
  DatErr[i]=DatErr[i+1];
 }
}
/*FGROUP FlMemChecks
  Correct errors stored in AddErr and DatErr.
*/
int Correct()
{
 int i,j,working,cycle=0;
 w32 addSW,addFM;
 if(writeFM(FlashAccessNoIncr,0xF0) !=0 )return 1;
 if(writeFM(FlashAddClear,0xF0) !=0 )return 2;
 i=0;
 while(errcount>0){
  addSW=i&0x7f;
  j=0;
  working=1;
  while(j<errcount && working){
   if(i == AddErr[j]){
    writeFMdeb(FlashAccessNoIncr,DatErr[j],&addFM);
    if(addFM != addSW){
      printf("AddSW=%2x addFM=%2x \n",addSW,addFM);
      return 100;
    }  
    throwit(j);
    errcount=errcount-1;
    working=0;
   }
   j++;
  }
  if(writeFMdeb(FlashAccessIncr,0xF0,&addFM) !=0)return -i;
  if( ((addSW+1)%0x80) != addFM) i++;
  i=(i+1)%FileSize;
  if(i==0){
   printf("Cycle %i done errcount=%i \n",cycle,errcount);
   if(writeFM(FlashAddClear,0xF0) !=0 )return 3;
   cycle++;
  } 
 }
 return 0; 
}
/*FGROUP FlMemChecks
Uses the Unlock Bypass Mode
*/
int LoadFMUB(int pat,int Nwords)
{
 int i,addSW,error;
 w32 pat2,addFM0, addFM;
 FILE *load=fopen("LOAD.txt","w");
 Pattern=pat; 
 if(pat>0xff)Pattern=0;
 if(writeFM(FlashAccessNoIncr,0xF0) !=0 )return 1;

 if( GetStatFM() == READYFM ){
   if(writeFM(FlashAddClear,0x0) !=0) return 10;
   writeFM(FlashAccessNoIncr,0xaa);
   writeFM(FlashAccessNoIncr,0x55);
   writeFM(FlashAccessNoIncr,0x20);
   for (i=0; i<Nwords; i++){
     addSW=i&0x7f; 
     error=0;
     pat2=Pattern;
     if(Pattern<CUT) pat2=0xaa   ;
     fprintf(load," Pattern: %2x Address: %5x %2x ",pat2,i,addSW);
     if(writeFMdeb(FlashAccessNoIncr,0xA0,&addFM) !=0){
        printf("Error at address: %i %2x  Status addr= %x\n",i,i,addFM); 
        return 5;
     }
     addFM0=addFM;
     fprintf(load,"#1: %2x ",addFM);      
     if(writeFMdeb(FlashAccessIncr,pat2,&addFM) !=0){
        printf("Error at address: %i %2x Status addr= %x\n",i,i,addFM); 
        return 8;
     }
     fprintf(load,"#2: %2x ",addFM);
     if((addFM0+1) != addFM)fprintf(load," Error ");   
     if(pat>0xff)Pattern= (Pattern+1) % MAXCYC; 
     if( (i % prt100) == 0) printf("LOadFMUB: %i words written \n",i);
     if(error)fprintf(load," Error %i \n",error);
       else fprintf(load,"\n");  
     if(quit !=0) return 9;
   }
 writeFM(FlashAccessNoIncr,0x90);  
 writeFM(FlashAccessNoIncr,0x00);  
 if(pat>0xff)Pattern=0xfff;  
 return 0;
 }else return 3;  
}

/*FGROUP FlMemChecks
Check against 0 or what is written in LoadFM0.
*/
int Check0(int Nwords) {
 int diftot=0,diff[8]={0,0,0,0,0,0,0,0};
 int i,j,rc=-1,error,cycl=0,addSW;
 w32 difbit,pit,flword,status,addST,pat2; 
 FILE *dump;
 
 if(writeFM(FlashAddClear,0x0) !=0) return 3;
 dump=fopen("FM.txt","w");
 if(Pattern==0xfff){
  cycl=1;
  Pattern=0;
 }
 for(i=0;i<Nwords;i++){
  error=0;
  status=vmer32(FlashStatus);
  flword=vmer32(FlashAccessIncr)&0xff;
  addST=status&0x7f;
  status=status&0x80;
  addSW=i&0x7f;
  pat2=Pattern;
  if(Pattern<CUT)pat2=0xaa;
  fprintf(dump,"Address: %5x %2x Status Address: %2x %x",i,addSW,addST,status);
  difbit=(flword^pat2);
  if(flword != pat2){
     if(rc == -1)rc=i;
     diftot++;
     error=1;
     fprintf(dump," Error   :%2x %2x \n",pat2,flword);
  }else fprintf(dump," Pattern :%2x %2x \n",pat2,flword);
     
  if(cycl) Pattern=(Pattern+1)% MAXCYC;
  pit=1;
  for(j=0;j<8;j++){
      /*  printf("%i ", difbit&pit );                    */
      diff[j]=diff[j]+( (difbit&pit) == pit );
      pit=2*pit;
  }
  /*  printf("\n");   */
 }
 printf("CheckContent: Total number of different words= %i. \n",diftot);
 printf("First different word at address %i \n",rc);
 printf("Differences bit by bit: \n");
 printf(" Bit0 Bit1 Bit2 Bit3 Bit4 Bit5 Bit6 Bit7 \n");
 for(i=0;i<8;i++)printf(" %.2f",diff[i]/((float) Nwords));
 printf("\n");
 fclose(dump);
 if(rc>0)rc=-rc;
 return (-diftot);
}
/*FGROUP FlMemChecks
*/
int Comb()
{
 int i,dif, ngen=100000;
 FILE *comb=fopen("COMB.txt","w");
 for(i=0;i<0xff;i++){
  if(eraseFM() != 1){
    fclose(comb);
    return 1;
  }  
  LoadFM0(i,ngen,0);
  dif=Check0(ngen);
  if(dif<=0){
   fprintf(comb,"Pattern= %2x Errors/Gen= %i/ %i \n",i,dif,ngen);
  }else{
    fclose(comb);
    return 2;
  }  
 }
 fclose(comb);
 return 0;
}
/* FGROUP FlMemChecks

int FindIT(int Nwords) {
 int i,ncount=0,first=1;
 int wor,wand;
 FILE *adr;
 if(ReadFlashMem() != 0) return 3;
 adr=fopen("ADRer.txt","w");
 for(i=0;i<Nwords;i++){
  if(FlMem[i]==0xff){  // was: if(FlMem[i]=0xff){
   if(first){
    first=0;
    wand=i;wor=i;
    ncount++;
   }else{
    wand=wand&i;
    wor=wor|i;
    ncount++;
   }; 
   fprintf(adr,"%5x\n");
  }
 }
 fclose(adr);
 printf("WAND=%x WOR=%x ncount=%i \n",wand,wor,ncount);
 return 0;
} */

/*FGROUP FlMemChecks
Monitars data lines after write or read
*/
void Monitor(int Ntimes) {
 int Mon[Ntimes];
 int i;
 vmew32(FlashAddClear,0x0);
 for(i=0;i<100;i++)
 vmew32(FlashAccessNoIncr,0xAA);

 for(i=0;i<Ntimes;i++){
  Mon[i]=vmer32(FlashAccessNoIncr);
 }
 for(i=0;i<Ntimes;i++){
   printf("%i %x \n",i,Mon[i]);
 }     
}
/*FGROUP VMETests
Large number of readings
*/
void VMEwrite(w32 Pedjaddress,int Ntimes,w32 word){
 int i;
 w32 address;
 address=4*Pedjaddress;
 for(i=0;i<Ntimes;i++){
  /*word=(vmer32(address)&0xff);*/
  vmew32(address,word);
 }
}
/*FGROUP VMETests
Increasing address in big loop by reading and checking 
in it jumps.
*/
void ReadLoop(int Nloops) {
 int i;
 w32 address0,address,data;
 address0=readFM(FlashStatus)&0x7f; 
 for(i=0;i<Nloops;i++){
   data=readFM(FlashAccessIncr)&0xff;
   address=readFM(FlashStatus)&0x7f;
   /* printf("%i address0=%2x data=%2x",i,address0,data);*/
   address0=(address0+1) % 0x80;
   if((address0) != address){
    printf(" address error:next address: %x \n",address);
    address0=address-1;
   }else /*printf("\n") */;
 }
}
/*FGROUP VMETests
Increasing address in big loop by writing reset and checking 
in it jumps.
*/
void WriteLoop(int Nloops)
{
 int i,i0,dist;
 double adist,ner;
 w32 address0,address;
 address0=readFM(FlashStatus)&0x7f;
 printf("Starting with address %2x \n",address0);
 i=0; 
 i0=0;
 adist=0.;
 ner=0;
 while(1){
   writeFM(FlashAccessIncr,0xF0);
   /* for(j=0;j<10000;j++); */
   i++;
   address=readFM(FlashStatus)&0x7f;
   address0=(address0+1) % 0x80;
   if((address0) != address){
    ner=ner+1;
    dist=i-i0;
    adist=adist+dist;
    i0=i;
    printf(" %d address error:SW address:%2x, FM address: %2x %f\n",i,address0,address, adist/ner);
    address0=address;
   }else /*printf("\n") */;
 }
}
/*FGROUP VME
VME write. Use Pedja address.
*/
void wvme(w32 address,w32 word){
 address=4*address;
 vmew32(address,word);
}
/*FGROUP VME
VME read.Use Pedja address.
*/
w32 rvme(w32 address){
 address=4*address;
 return (vmer32(address)&0xff);
}
/*FGROUP VME
*/
void testcode()
{
 int code,code1,noerror=1;
 code=rvme(0x1);
 if(code != 0x56){
   noerror = 0;
   printf("Error: Board Type Code not correct:%x \n",code);
 }  
 code1=code;
 code=rvme(0x3);
 if( code != 0xa6){
  noerror = 0;
  printf("Error: VME controller Firmare cersion not correct: %x \n",code);
 }
 if(noerror) printf("tesetcode succes. Board type code and VME controller version correct:%x %x\n",code1,code);
}
/*FGROUP VME
*/
void temp()
{
 int temp;
 wvme(0x16,0x0);  /* TEMP_START */
 usleep(1000);
 if(rvme(0x17) != 0){
  printf("Error: Temperature measurement unsuccesfull, Busy up after 1000us.\n");
  return;
 }
 temp=rvme(0x18);
 temp=temp&0xff;
 printf("Temperature= %i\n",temp);
 if( (temp > 35) || (temp < 18)) printf("Error: Temperature outside range: %i \n",temp);
}
/*FGROUP VME
*/
void testver()
{
 int version;
 version=rvme(0x20)&0xff;
 if(version == 0xb3) printf("LTU logic FPGA Version read succesfully: %x \n",version);
 else printf("Error: LTU logic FPGA version: %x \n",version);
 wvme(0x30,0xff);  /* test the leds */ 
 printf("Serial number succes: =========%d ==========\n",rvme(0x2));
}
/***************************************************************************/
void initmain() {
}
void boardInit() {
}
void endmain() {
}


/*ENDOFCF*/
