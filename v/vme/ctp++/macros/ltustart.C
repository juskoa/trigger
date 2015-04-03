#include "TTCITBOARD.h"
#include "LTUBOARD.h"
/*
 * Starts and stops ltu to generate data for ssm
 */
int main(int argc,char *argv[]){
 w32 vmeadd;
 if(argc == 2){
  if(argv[1][0] != '0' && argv[1][1] != 'x'){
   printf("Expecting hex number starting with 0x \n");
   return 1;
  }
  string ss(argv[1]);
  if(convertS2H(vmeadd,ss)) return 1;
 }else{
  printf("Wrong number of arguments: %i \n",argc);
  return 1;
 }
 printf("Starting ltu= 0x%x \n",vmeadd);
 int vmesp=-1;
 string ltuname("ltu");
 LTUBOARD *ltu= new LTUBOARD(ltuname.c_str(),vmeadd,vmesp);
 //ltu->TTCinit();
 ltu->ClearFIFOs();
 if(ltu->SLMstart()) return 1;
 usleep(24000);
 ltu->SLMquit();
 return 0;
}
