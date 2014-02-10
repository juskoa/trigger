#include "TTCITBOARD.h"
//---------------------------------------------------------------------
TTCITBOARD::TTCITBOARD(string const name,w32 const boardbase,int vsp)
:
BOARD(name,boardbase,vsp,1),
ssm(0),
VERSION(0x4),
CONTROL(0x8),
STATUS(0xc),
READ_SSM_ADDRESS(0x14),
READ_SSM_WORD(0x18),
RESET(0x28)
{
}
void TTCITBOARD::Print()
{
 //printf("%s:",ttcitname.c_str());
 //printboardinfo("");
}
void TTCITBOARD::start_stopSSM()
{
 vmew(RESET,0);
 printf("# word= 0x%x\n",vmer(READ_SSM_ADDRESS));
 printf("status: 0x%x\n",getStatus());
 usleep(50000);
 //vmew(CONTROL,1);
 printf("status: 0x%x\n",getStatus());
 printf("# word= 0x%x\n",vmer(READ_SSM_ADDRESS));
 ssm=GetSSM();
 for(int i=0;i<Mega;i++){
  ssm[i]=vmer(READ_SSM_WORD);
 }
 printf("# word= 0x%x\n",vmer(READ_SSM_ADDRESS));
 for(int i=0;i<Mega;i++){
  if(ssm[i])printf("%i 0x%x \n",i,ssm[i]);
 }
}
