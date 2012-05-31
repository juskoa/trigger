#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef CPLUSPLUS
#include <dic.hxx>
#else
#include <dic.h>
#endif

#define TAGstartcount 333
#define TAGstopcount 334
#define TAGprintruns 335
void readCounters();

char mygetchar(){
 char c,d;
 c=getchar();
 while((d=getchar()) != '\n');
 //printf("myget %c %i \n",d,d);
 return c;
}
unsigned int  enterrunnum(){
 char runc[100];
 unsigned int run;
 int i,d;
 printf("Enter run number:");
 fgets(runc,100,stdin); 
 //printf("%s %i\n",runc,strlen(runc));
 run=0;
 d=1;
 for(i=strlen(runc)-2;i>=0;i--){
  //printf("0x%x \n",runc[i]);
  run=run+(runc[i]-0x30)*d;
  d=d*10;  
 }
 return run;
}
void callback(void *tag, int *retcode){
 char command[100];
 printf("callback: %li %i \n",*(long *)tag,*retcode);
 switch(*(long *)tag){
   case(TAGstartcount):
        strcpy(command,"STARTRUNCOUNT");
        break;
   case(TAGstopcount):
        strcpy(command,"STOPRUNCOUNT");
        break;
   default:
        printf("callback: Unknown tag %li \n",*(long *)tag);
        return;
 }
 if(*retcode)printf("%s succesful. \n",command);
 else printf("%s failed. \n",command);
}
void printHelp() {
printf("Commands: s(start) f(stop) p(print runs) q(quit) \n");
};
//-------------------------------------------
int main(){
 char c,startcom[100],stopcom[100],printcom[100];
 unsigned int run;
 int size;
char prt[]="PRINT";
 strcpy(startcom,"CTPDIM/STARTRUNCOUNTER");
 strcpy(stopcom,"CTPDIM/STOPRUNCOUNTER");
 strcpy(printcom,"CTPDIM/PRINTRUNS");
 size=sizeof(run);
 printf("dim server with counters TESTER \n");
 while(1) {
  printHelp();
  if((c = mygetchar()) == 'q') break;
  switch(c){
   case 's':
        run=enterrunnum();
        printf("Starting partition %i 0x%x \n",run,run);
        //dic_cmnd_service(startcom,&run, size);
        dic_cmnd_callback(startcom,&run, size,&callback,TAGstartcount);
        break;
   case 'f':
        run=enterrunnum();
        printf("Stoping partition %i 0x%x\n",run,run);
        //dic_cmnd_service(stopcom,&run, size);
        dic_cmnd_callback(stopcom,&run, size, &callback,TAGstopcount);
        break;
   case 'p':
        dic_cmnd_service(printcom, prt,5);
        break;
   default: 
         printf("Unknown command %c \n",c);
  }
 }
 return 0;
}
