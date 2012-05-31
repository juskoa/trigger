#include <stdio.h>

#define Mega 1024*1024
#define kilo 1024
#define FMsize Mega
#define wAttempts 4000

static int ConfigFile[FMsize];
/*****************************************************************************/
/*-------------Flash Memory--------------------------------------------------*/
/****************************************************************************/
/* Read Input File, find its size */
int ReadInputFileS(int *size)
{
 FILE *InputFile;
 int i=0,end; unsigned char word;
 size_t nmemb=1,nread;
 InputFile=fopen("FlashMem.cfg","rb");/* File is in directory vme */
 if(InputFile == NULL){
  printf("ReadInputFileS:Input File can not be opened. \n");
  *size=0;
  return 1;
 }else{
  printf("ReadInputFileS:Input File succesfully opened. \n");
  i=0;
  while((nread=fread(&word,1,nmemb,InputFile)) == nmemb){
   printf("ReadInputFileS:i=%i j=%i word=%2x \n",i,nread,word); 
   i++;
   if(i > FMsize){
     printf("ReadInputFileS:Input File seems too large! \n");
     return 2;
   }  
  }
  end=feof(InputFile);
  if(end != 1){
     printf("ReadInputFileS:Not a real end of file. Check the input file. \n ");
     return 3;
  }
  *size=i;
  /*  printf("*size=%i, size=%u \n",*size,size); */
  }
  close(InputFile);
  return 0;
}  
/*****************************************************************************/
/* Read Input File. */
int ReadInputFileF(int size)
{
 int i;
 FILE *InputFile;
 unsigned char word;
 size_t nmemb=1,nread;
  
 InputFile=fopen("FlashMem.cfg","rb");/* File is in directory vme */
 if(InputFile == NULL){
  printf("ReadInputFileF:Input File can not be opened. \n");
  return 1;
 } 
 
 /* printf("Address of file is %u \n",ConfFile); */
 for(i=0;i < size;i++){
   nread=fread(&word,1,nmemb,InputFile);
   if(nread != nmemb){
      printf("ReadInputFileF:Error in Reading Config File from LoadFM \n");
      return 3;
   }
   ConfigFile[i]=0;
   ConfigFile[i]=word;
   /*printf("ReadInputFileF:ConfigFile[%i]=%i \n",i,ConfigFile[i]);*/ 
 }
 close(InputFile);
 return 0;
}
/***************************************************************************/
int main() {
int size;
ReadInputFileS(&size);
printf("Size=%i",size);
}
