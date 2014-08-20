#include "CTP.h"
int main(int argc,char **argv){
 if(argc != 2){
   printf("Expected number of arguments 1 \n");
   printf("Please, use one argiment x={0,1 ,2} for 0x81x000 \n");
   return 1;
 }
 //printf("%s \n",argv[1]);
 w32 add=0x812000;
 if(argv[1][0]=='0')add=0x810000;
 else if(argv[1][0]=='1')add=0x811000;
 else if(argv[1][0]=='2')add=0x812000;
 else{
  printf("Unknown ltu, please, use x={0,1 ,2} for 0x81x000 \n");
  return 1;
 }
 w32 err=0;
 //CTP ctp;
 w32 vmesp=-1;
 string ltuname("ltu");
 LTUBOARD *ltu= new LTUBOARD(ltuname.c_str(),add,vmesp);
 ltu->Print();
 //ltu->SetStandalone();
 string filename="ssm";
 for(int i=0;i<100000;i++){
   ltu->ClearFIFOs();
   ltu->SetMode("inmon",'a');
   ltu->StartSSM();
   usleep(50000);
   ltu->StopSSM();
   ltu->ReadSSM();
   ///ltu->ReadSSMDump(filename.c_str());
   //ltu->AnalSSM();
   //ltu->AnalTotalSSM();
   ltu->AnalTotalSSM2();
   printf("FIFO MAX: 0x%x \n",ltu->GetFIFOMAX());
   err += ltu->GetErrors();
   if(ltu->GetErrors()){
    ltu->DumpSSM(filename.c_str());
    return 1;
   }
   if((i%1) == 0)printf("------------------------------->%i ssm done. ERRORS: %i\n",i+1,err);
   fflush(stdout);
 }
 printf("-------------------------------> ssm done. ERRORS: %i\n",err);
 return 0;
}
