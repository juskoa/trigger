#include "CTP.h"
#include <cmath>
void printhelp()
{
 printf("First argument: number of reads in loop, default=1000\n");
 printf("Second argument: time between 2 reads (vme accesses), default= 5 usec\n");
}
int main(int argc,char **argv){
 int numreads=1000;
 w32 delta=5;
 if(argc == 1){
   printf("Running with defaulte arguments\n");
   printf(" # reads per loop=1000 \n");
   printf(" # Time between 2 reads delta= 5 usec \n");
   printf("Type readBCstatus.e -h for help\n");
   printhelp();
 }else if(argc==2){
  if((argv[1][0]='-') && (argv[1][1]=='h')){
    printhelp();
    return 0;
  }
  else numreads=atoi(argv[1]);
 }else if(argc==3){
    numreads=atoi(argv[1]);
    delta=atoi(argv[2]);
 }else{
   printf("Expected number of arguments 1 or 2\n");
   printf("Type readBCstatus.e -h for help\n");
   printhelp();
   return 1;
 }
  printf("numreads= %i delta= %i \n",numreads,delta);
  int loop=0;
  CTP* ctp = new CTP;
  while(1){
   printf("loop = %i -----------------------------------------------\n",loop);
   printhelp();
   printf("Current values: %i %i \n",numreads, delta);
   printf("loop = %i -----------------------------------------------\n",loop);
   //if(ctp->readBCStatus(1000000)) break;
   ctp->readBCStatus(numreads,delta) ;
   //usleep(10); is inside readBCStatus
   loop++;
   fflush(stdout);
  }
  return 0;
}
