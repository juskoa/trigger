#include "L1BOARD.h"
L1BOARD::L1BOARD(int vsp)
:
	BOARD("l1",0x82a000,vsp,4),
	L1CONDITION(0x400),
	L1INVERT(0x600),
	L1DELAYL0(0x4cc)
{
  this->AddSSMmode("inmon",0); 
  this->AddSSMmode("outmon",1); 
  this->AddSSMmode("ingen",2); 
  this->AddSSMmode("outgen",3); 
}
//----------------------------------------------------------------------------
/* 
 * Check counters assuming classes are not configured
 */
int L1BOARD::CheckCountersNoTriggers()
{
 int ret=0;
 //w32 time = countdiff[CL1TIME]; 

 if(countdiff[CL1STRIN] != 0){
   printf("L1 strobe IN!= 0 %u \n",countdiff[CL1STRIN] );
   ret=1;
 }
 if(countdiff[CL1STROUT] != 0){
   printf("L1 strobe OUT != 0 %u \n",countdiff[CL1STROUT] );
   ret=1;
 }
 for(int i=0;i<100;i++){
  if(countdiff[i+CL1CLSB] != 0){
    printf("L1classB%02i != 0 %u \n",i,countdiff[i+CL1CLSB]);
    ret=1;
  }
  if(countdiff[i+CL1CLSA] != 0){
    printf("L1classA%02i != 0 %u \n",i,countdiff[i+CL1CLSA] );
    ret=1;
  }
 }
 for(int i=0;i<7;i++){
    if(countdiff[i+CL1CLST] != 0){
      printf("l1clst%1i != 0 %u \n",i,countdiff[i+CL1CLST]);
      ret=1;
    }
 }
 if(ret==0)printf("L1  CheckCountersNoTriggers: NO ERROR detected.\n");
 return ret;
} 

//----------------------------------------------------------------------------
// set single class at index, with input mask inputs and cluster
void L1BOARD::setClass(w32 index,w32 inputs,w32 cluster,w32 vetos)
{
 w32 word=inputs+(cluster<<28) + (vetos<<24)+(1<<31);
 vmew(L1CONDITION+4*index,word);
}
//----------------------------------------------------------------------------
// Set all classes to 0xfffff - dont care
void L1BOARD::setClassesToZero()
{
 for(w32 i=0; i<kNClasses; i++)setClass(i+1,0xffffff,0,0xf);
}
//----------------------------------------------------------------------------
// read and print all classes
void L1BOARD::printClasses()
{
 printf("CTP classes from hardware:\n");
 for(w32 i=0; i<kNClasses; i++){
    w32 word=vmer(L1CONDITION+4*(i+1));
    printf("0x%x ",word);
    if((i+1)%10 == 0)printf("\n");
 }
}
//---------------------------------------------------------------------
// Checking classes against 1 - to be removed ot generalised
//
int L1BOARD::AnalSSM()
{
 int rc=0;
 w32 sl0strobech,sl0datach;
 if((sl0strobech=getChannel("l0strobe"))>32)rc=1;
 if((sl0datach  =getChannel("l0data"))>32)rc=1;
 if(rc){
   printf("Error in L1BOARD:AnalSSM: channels not found.\n");
   return 1;
 }
 printf("L1BOARD:AnalSSM: l0strobe, l1data channels %i %i\n",sl0strobech,sl0datach);

 w32 classlow[50],classhigh[50];
 w32 *sm=GetSSM();
 int i=0;
 int L0L0=260;
 int l0issm=-L0L0;
 while(i<Mega){
   int j=0;
   if(bit(sm[i],sl0strobech)){
     if((i-l0issm) < L0L0){
       printf("L1BOARD::AnalSSM: Error L0L0 time violation at %i \n",i);
       rc=1;
     }
     l0issm=i;
     i++;
     while((j<50) && (i+j)< Mega){
      classlow[j]=bit(sm[i+j],sl0strobech);
      classhigh[j]=bit(sm[i+j],sl0datach);
      //printf("%i %i %i %i\n",i,j,classlow[j],classhigh[j]);
      if((classlow[j] != 1) || (classhigh[j] != 1)){
        //printf("classes !=1 at %i \n",i+j);
        rc=1;
      }
      j++;
     }
   }
   i=i+j+1;
 }
 return rc;
}
