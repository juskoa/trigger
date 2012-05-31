#include "ssmconnection.h"
/*--------------------------------------------------------------setPFBLOCKA()
 */ 
void setPFBLOCKA(int board,int ipf){
 w32 pfba,bb;
 bb=BSP*ctpboards[board].dial;
 pfba=HardWare.THa1+(HardWare.THa2<<6)+(HardWare.deltaTa<<12);
 pfba=pfba+(HardWare.delayA<<20)+(HardWare.nodelayAf<<31);
 vmew32(bb+PFBLOCK_A+3*(ipf-1)*4,pfba);
}
void setPFBLOCKB(int board,int ipf){
 w32 pfba,bb;
 bb=BSP*ctpboards[board].dial;
 pfba=HardWare.THb1+(HardWare.THb2<<6)+(HardWare.deltaTb<<12);
 pfba=pfba+(HardWare.delayB<<20)+(HardWare.nodelayBf<<31); 
 vmew32(bb+PFBLOCK_A+4+3*(ipf-1)*4,pfba);	
}
void setPFCOMMON(int board){
 w32 pfcommon,bb;
 bb=BSP*ctpboards[board].dial;
 pfcommon=HardWare.luta + (HardWare.lutb<<4)+ (HardWare.delayedINTlut<<8);
 pfcommon=pfcommon + (HardWare.delayINT<<12);
 vmew32(bb+PF_COMMON,pfcommon);
}
void setPFLUT(int board,int ipf){
 w32 pflut,bb;
 bb=BSP*ctpboards[board].dial;
 pflut=HardWare.lut12D+(HardWare.scaleA<<8)+(HardWare.scaleB<<13);
 vmew32(bb+PFBLOCK_A+8+3*(ipf-1)*4,pflut);  
}
/*------------------------------------------------------------------lookup4()
 * 2 bits look up table
 */ 
int lookup4(int in1,int in2,int lupt){
 if(lupt){
  int out;
  out=in1+2*in2;
  out=1<<out;
  return ((lupt & out ) == out);
 }else return 0;
}
/*------------------------------------------------------------------lookup8()
 * 3 bits lookup table
*/ 
int lookup8(int in1,int in2,int in3,int lupt){
 if(lupt){
  int out;
  out=in1+2*in2+4*in3;
  out=1<<out;
  return ((lupt & out ) == out);
 }else return 0;
}
/*FGROUP DebCon---------------------------------------getPFHW()
 */
int getPFHW(int board,int ipf){
 w32 bb,word;
 bb=BSP*ctpboards[board].dial;
 printf("PF circiut %i \n",ipf);
 //PFCOMMON
 printf("bb=%x \n",bb);
 word=vmer32(bb+PF_COMMON);
 HardWare.luta=word&0xf;
 HardWare.lutb=(word>>4)&0xf;
 HardWare.delayedINTlut=(word>>8)&0xf;
 HardWare.delayINT=(word>>12)&0xfff;
 //PFBLOCK_A
 word=vmer32(bb+PFBLOCK_A+3*(ipf-1)*4);
 HardWare.THa1=word&0x3f;
 HardWare.THa2=(word>>6)&0x3f;
 HardWare.deltaTa=(word>>12)&0xff;
 HardWare.delayA=(word>>20)&0x7ff;
 HardWare.nodelayAf=(word>>31)&0x1;
 //PFBLOCK_B
 word=vmer32(bb+PFBLOCK_A+4+3*(ipf-1)*4);
 HardWare.THb1=word&0x3f;
 HardWare.THb2=(word>>6)&0x3f;
 HardWare.deltaTb=(word>>12)&0xff;
 HardWare.delayB=(word>>20)&0x7ff;
 HardWare.nodelayBf=(word>>31)&0x1;
 // PFLUT
 word=vmer32(bb+PFBLOCK_A+8+3*(ipf-1)*4);
 HardWare.lut12D=word&0xff;
 HardWare.scaleA=(word>>8)&0x1f;
 HardWare.scaleB=(word>>13)&0x1f;
 // INTERACT_1/2/T
 HardWare.int1=vmer32(L0_INTERACT1);
 HardWare.int2=vmer32(L0_INTERACT2);
 HardWare.intt=vmer32(L0_INTERACTT);
 HardWare.intsel=vmer32(L0_INTERACTSEL);
 return 0;
}
/*FGROUP DebCon------------------------------------------------------printPFHW()
 */
void printPFHW(){
 printf("INTERACT_1/2/T: \n");
 printf("%x %x %x \n",HardWare.int1,HardWare.int2,HardWare.intt);
 printf("INTERACT_SEL: \n");
 printf("%x \n",HardWare.intsel);
 printf("BLOCK A: \n");
 printf("THa1=%i Tha2=%i deltaTa=%i ",HardWare.THa1,HardWare.THa2,HardWare.deltaTa);
 printf(" NodelflagA=%i DelayA=%i \n",HardWare.nodelayAf,HardWare.delayA);
 printf("BLOCK B: \n");
 printf("THb1=%i Thb2=%i deltaTb=%i ",HardWare.THb1,HardWare.THb2,HardWare.deltaTb);
 printf(" NodelflagB=%i DelayB=%i \n",HardWare.nodelayBf,HardWare.delayB);

printf("PFLUT:\n");
 printf("LUT12D=%x scaleA=%i scaleB=%i \n",HardWare.lut12D,HardWare.scaleA,HardWare.scaleB);

 printf("PFCOMMON: \n");
 printf("LUTa=%x LUTb=%x LUTd=%x \n",HardWare.luta,HardWare.lutb,HardWare.delayedINTlut);
 printf("delay INT=%i \n",HardWare.delayINT);
}
/*FGROUP DebCon
 * Read HW from board and prints
*/
int printPFHWs(int board){
 int i;
 for(i=1;i<6;i++){
  printf("PF %i \n",i);
  getPFHW(board,i);
  printPFHW();
 }
return 0;
}	
/*FGROUP DebCon
 * Write current HardWare PF to board
 */
int setPFHW(int board,int ipf){
 setPFBLOCKA(board,ipf);	
 setPFBLOCKB(board,ipf);
 setPFCOMMON(board);
 setPFLUT(board,ipf);
 return 0;
}
/*----------------------------------------------------------PFcircuit()
 * Original L0 version
 * No delay a/b available
 * Works with L0 - etensively tested
 * int1chan - channel of signal int1
 * int2chan - channel of signal int2 
 */
int PFcircuit(int board,int ipf,int scale_offseta,int scale_offsetb,int int1chan,int int2chan){
 int sum=0;
 int ism=0,ismascale=0,ismbscale=0;
 w32 dpma[256],dpmb[256],deldpm[10];
 w32 inta=0,intb=0,counta=0,countb=0,intd=0;
 w32 pa1,pa2,pb1,pb2,p1,p2,pp=0;

 for(ism=0;ism<256;ism++){dpma[ism]=0;dpmb[ism]=0;} 
 
 ism=0; // here was bug
 while(ism < Mega){
 
  // Write pf to ssm at channel ipf-1  
  sms[TEST].sm[ism]=wbit(sms[TEST].sm[ism],pp,ipf-1);
  //sms[TEST].sm[ism]=pp;
  
  sum=sum+pp;
  
  // Block a 
  if((counta-dpma[(ismascale-HardWare.deltaTa-1)%256]) > HardWare.THa1)pa1=1;
  else pa1=0;
  if((counta-dpma[(ismascale-HardWare.deltaTa-1)%256]) > HardWare.THa2)pa2=1;
  else pa2=0;  
  if(!((ism+scale_offseta)%(HardWare.scaleA+1))){
    dpma[ismascale%256]=counta;    
    ismascale++;
  }
  // Block b 
  //sms[TEST].sm[ism]=pa1;
  if((countb-dpmb[(ismbscale-HardWare.deltaTb-1)%256]) > HardWare.THb1)pb1=1;
  else pb1=0;
  if((countb-dpmb[(ismbscale-HardWare.deltaTb-1)%256]) > HardWare.THb2)pb2=1;
  else pb2=0;  
  if(!((ism+scale_offsetb)%(HardWare.scaleB+1))){
    dpmb[ismbscale%256]=countb;    
    ismbscale++;  
  }
  // BC clock
  if(++ism >= Mega) break;
  // Look-up table Dealyed INT
  // (For the L0 case delay=0, for L1 and L2 boards delay has to be added!)
  intd=lookup4(bit(sms[board].sm[ism],int1chan),
	       bit(sms[board].sm[ism],int2chan),
	       HardWare.delayedINTlut);
  deldpm[(ism)%10]=intd;
  // OR for P1 and P2
  //if((pb1+pb2))printf("ism,pa1,pb1,pa2,pb2 %i %i %i %i %i\n",ism,pa1,pb1,pa2,pb2);
  p1 = pa1 || pb1;
  p2 = pa2 || pb2;	  
  //pp = p1;
  pp=lookup8(p1,p2,deldpm[(ism)%10],HardWare.lut12D);
  //printf("p1,p2,,intd,pp %i %i %i %i\n",p1,p2,intd,pp);
  // Look-up table P1,P2,Delayed INT
  
  // Look-up table INTa
  inta=lookup4(bit(sms[board].sm[ism],int1chan),
	       bit(sms[board].sm[ism],int2chan),
	       HardWare.luta);
  counta=counta+inta;
  // Look-up table INTb
  intb=lookup4(bit(sms[board].sm[ism],int1chan),
	       bit(sms[board].sm[ism],int2chan),
	       HardWare.lutb);
  countb=countb+intb;
 }
 return sum;
}
/*----------------------------------------------------------PFcircuit1()
 * Original L0 version
 * No delay a/b available
 * Works with L1,L2 and no del flag only !
 * int a,b,d read from ssm memory - 
 * int1chan - channel of signal int1
 * int2chan - channel of signal int2 
 */
int PFcircuit1(int board,int ipf,int scale_offseta,int scale_offsetb,int int1chan,int int2chan,int int3chan){
 int sum=0;
 int ism=0,ismascale=0,ismbscale=0;
 w32 dpma[256],dpmb[256],deldpm[D12];
 w32 inta=0,intb=0,counta=0,countb=0,intd=0;
 w32 pa1,pa2,pb1,pb2,p1,p2,pp=0;

 for(ism=0;ism<256;ism++){dpma[ism]=0;dpmb[ism]=0;} 
 
 ism=0; // here was bug
 while(ism < Mega){
 
  // Write pf to ssm at channel ipf-1  
  sms[TEST].sm[ism]=wbit(sms[TEST].sm[ism],pp,ipf-1);
  //sms[TEST].sm[ism]=pp;
  
  sum=sum+pp;
  
  // Block a 
  if((counta-dpma[(ismascale-HardWare.deltaTa-1)%256]) > HardWare.THa1)pa1=1;
  else pa1=0;
  if((counta-dpma[(ismascale-HardWare.deltaTa-1)%256]) > HardWare.THa2)pa2=1;
  else pa2=0;  
  if(!((ism+scale_offseta)%(HardWare.scaleA+1))){
    dpma[ismascale%256]=counta;    
    ismascale++;
  }
  // Block b 
  //sms[TEST].sm[ism]=pa1;
  if((countb-dpmb[(ismbscale-HardWare.deltaTb-1)%256]) > HardWare.THb1)pb1=1;
  else pb1=0;
  if((countb-dpmb[(ismbscale-HardWare.deltaTb-1)%256]) > HardWare.THb2)pb2=1;
  else pb2=0;  
  if(!((ism+scale_offsetb)%(HardWare.scaleB+1))){
    dpmb[ismbscale%256]=countb;    
    ismbscale++;  
  }

  // BC clock
  if(++ism >= Mega) break;

  // Look-up table Dealyed INT
  //intd=bit(sms[board].sm[ism],int3chan+1); directly dint_d
  intd=bit(sms[board].sm[ism],int3chan);
  deldpm[(ism)%D12]=intd;
  // OR for P1 and P2
  //if((pb1+pb2))printf("ism,pa1,pb1,pa2,pb2 %i %i %i %i %i\n",ism,pa1,pb1,pa2,pb2);
  p1 = pa1 || pb1;
  p2 = pa2 || pb2;	  
  //P1,P2 delint look up table
  //pp=lookup8(p1,p2,intd,HardWare.lut12D);
  pp=lookup8(p1,p2,deldpm[(ism-HardWare.delayINT-1)%D12],HardWare.lut12D);
  //printf("p1,p2,,intd,pp %i %i %i %i\n",p1,p2,intd,pp);
  // Look-up table P1,P2,Delayed INT
  
  // Look-up table INTa
  inta=bit(sms[board].sm[ism],int1chan);
  counta=counta+inta;
  // Look-up table INTb
  intb=bit(sms[board].sm[ism],int2chan);
  countb=countb+intb;
 }
 return sum;
}
/*----------------------------------------------------------PFcircuit2()
 * int1chan - channel of signal inta
 * int2chan - channel of signal intb
 * int3chan - channel of signal intd 
 * Version for testing everything on L1 board
 * inta,intb and intd signals are taken directly from ssm
*/
int PFcircuit2(int board,int ipf,int scale_offseta,int scale_offsetb,int int1chan,int int2chan,int int3chan,int start,int ic){
 int sum=0;
 //   BC Block A clk Block B clk
 w32 ism,ismascale=0,ismbscale=0;
 // A DP mem   B DP mem
 w32 dpma[256],dpmb[256];
 w32 inta=0,intb=0,counta=0,countb=0,intd=0;
 w32 pa1=0,pa2=0,pb1=0,pb2=0,p1,p2,pp=0;
 w32 pa1o=0,pa2o=0,pb1o=0,pb2o=0;
 w32 dela,delb;
 // delay memories
 w32 delpa1[D11],delpa2[D11],delpb1[D11],delpb2[D11];

 dela = (HardWare.delayA+1);
 delb = (HardWare.delayB+1);
 printf("PFCircuit2: dela delb %i %i \n",dela,delb); 
 for(ism=0;ism<D11;ism++){delpa1[ism]=0;delpa2[ism]=0;delpb1[ism]=0;delpb2[ism]=0;}
 for(ism=0;ism<256;ism++){dpma[ism]=0;dpmb[ism]=0;} 

 ism=0;
 while(ism < start){sms[TEST].sm[ism]=wbit(sms[TEST].sm[ism],pp,5*ic);ism++;}
 while(ism < Mega){ 
  // Write pf to ssm at channel ipf-1  
  //sms[TEST].sm[ism]=wbit(sms[TEST].sm[ism],pp,ipf-1);
  sms[TEST].sm[ism]=wbit(sms[TEST].sm[ism],pp,5*ic);
  //sms[TEST].sm[ism]=pp;
  
  // Dealayed pxy to ssm
  sms[TEST].sm[ism]=wbit(sms[TEST].sm[ism],pa1o,1+5*ic);
  sms[TEST].sm[ism]=wbit(sms[TEST].sm[ism],pa2o,2+5*ic);
  sms[TEST].sm[ism]=wbit(sms[TEST].sm[ism],pb1o,3+5*ic);
  sms[TEST].sm[ism]=wbit(sms[TEST].sm[ism],pb2o,4+5*ic);
  
  sum=sum+pp;

  // Look-up table Dealyed INT
  // (For the L0 case delay=0, for L1 and L2 boards delay has to be added!)
  intd=bit(sms[board].sm[ism+1],int3chan+1);  // delayed intd
  //deldpm[(ism)%10]=intd;
  
  // Look-up table INTa 
  inta=bit(sms[board].sm[ism],int1chan);
  counta=counta+inta;    
  // Look-up table INTb 
  intb=bit(sms[board].sm[ism],int2chan);
  countb=countb+intb;   
  
  // BLOCK A 
  if((counta-dpma[(ismascale-HardWare.deltaTa-1)%256]) > HardWare.THa1)pa1=1;
  else pa1=0;
  if((counta-dpma[(ismascale-HardWare.deltaTa-1)%256]) > HardWare.THa2)pa2=1;
  else pa2=0;  
  
  if(HardWare.nodelayAf){pa1o=pa1;pa2o=pa2;}else
  {pa1o=delpa1[(ismascale-dela)%D11];pa2o=delpa2[(ismascale-dela)%D11];} 
  if(!((ism+scale_offseta)%(HardWare.scaleA+1))){
    dpma[ismascale%256]=counta;    
    delpa1[ismascale%D11]=pa1; delpa2[ismascale%D11]=pa2;
    ismascale++;
  }
  // BLOCK B 
  if((countb-dpmb[(ismbscale-HardWare.deltaTb-1)%256]) > HardWare.THb1)pb1=1;
  else pb1=0;
  if((countb-dpmb[(ismbscale-HardWare.deltaTb-1)%256]) > HardWare.THb2)pb2=1;
  else pb2=0;  
    
  if(HardWare.nodelayBf){pb1o=pb1;pb2o=pb2;}else
  {pb1o=delpb1[(ismbscale-delb)%D11];pb2o=delpb2[(ismbscale-delb)%D11];} 
  if(!((ism+scale_offsetb)%(HardWare.scaleB+1))){
    dpmb[ismbscale%256]=countb;    
    delpb1[ismbscale%D11]=pb1; delpb2[ismbscale%D11]=pb2;    
    ismbscale++;  
  } 

  // OR for P1 and P2
  //if((pb1+pb2))printf("ism,pa1,pb1,pa2,pb2 %i %i %i %i %i\n",ism,pa1,pb1,pa2,pb2);
  p1 = pa1o || pb1o;
  p2 = pa2o || pb2o;	  

  // Look-up table P1,P2,Delayed INT
  //pp=lookup8(p1,p2,deldpm[(ism)%10],HardWare.lut12D);
  pp=lookup8(p1,p2,intd,HardWare.lut12D);
  //printf("p1,p2,,intd,pp %i %i %i %i\n",p1,p2,intd,pp);
  
  // BC clock
  ism++ ;  
 }
 return sum;
}
/*----------------------------------------------------------PFcircuit3()
 * int1chan - channel of signal int1
 * int2chan - channel of signal int2
 * Version for testing on L0 and L1 board
 * 
*/
int PFcircuit3(int board,int ipf,int scale_offseta,int scale_offsetb,int int1chan,int int2chan,int start,int ic){
 int sum=0;
 //   BC Block A clk Block B clk
 w32 ism,ismascale=0,ismbscale=0;
 // A DP mem   B DP mem
 w32 dpma[256],dpmb[256];
 w32 deldpm[D12];
 w32 inta=0,intb=0,counta=0,countb=0,intd=0;
 w32 pa1=0,pa2=0,pb1=0,pb2=0,p1,p2,pp=0;
 w32 pa1o=0,pa2o=0,pb1o=0,pb2o=0;
 w32 dela,delb;
 // delay memories
 w32 delpa1[D11],delpa2[D11],delpb1[D11],delpb2[D11];

 dela = (HardWare.delayA+1);
 delb = (HardWare.delayB+1);
 printf("PFCircuit2: dela delb %i %i \n",dela,delb); 
 for(ism=0;ism<D11;ism++){delpa1[ism]=0;delpa2[ism]=0;delpb1[ism]=0;delpb2[ism]=0;}
 for(ism=0;ism<256;ism++){dpma[ism]=0;dpmb[ism]=0;} 

 ism=0;
 while(ism < start){sms[TEST].sm[ism]=wbit(sms[TEST].sm[ism],pp,5*ic);ism++;}
 while(ism < Mega){ 
  // Write pf to ssm at channel ipf-1  
  //sms[TEST].sm[ism]=wbit(sms[TEST].sm[ism],pp,ipf-1);
  sms[TEST].sm[ism]=wbit(sms[TEST].sm[ism],pp,5*ic);
  //sms[TEST].sm[ism]=pp;
  
  // Dealayed pxy to ssm
  sms[TEST].sm[ism]=wbit(sms[TEST].sm[ism],pa1o,1+5*ic);
  sms[TEST].sm[ism]=wbit(sms[TEST].sm[ism],pa2o,2+5*ic);
  sms[TEST].sm[ism]=wbit(sms[TEST].sm[ism],pb1o,3+5*ic);
  sms[TEST].sm[ism]=wbit(sms[TEST].sm[ism],pb2o,4+5*ic);
  
  sum=sum+pp;

  // Look-up table Dealyed INT
  // (For the L0 case delay=0, for L1 and L2 boards delay has to be added!)
  intd=lookup4(bit(sms[board].sm[ism],int1chan),
	       bit(sms[board].sm[ism],int2chan),
	       HardWare.delayedINTlut);
  deldpm[(ism)%D12]=intd;
  
  // Look-up table INTa 
  inta=lookup4(bit(sms[board].sm[ism],int1chan),
	       bit(sms[board].sm[ism],int2chan),
	       HardWare.luta);
  counta=counta+inta;    
  // Look-up table INTb 
  intb=lookup4(bit(sms[board].sm[ism],int1chan),
	       bit(sms[board].sm[ism],int2chan),
	       HardWare.lutb);
  countb=countb+intb;   
  
  // BLOCK A 
  if((counta-dpma[(ismascale-HardWare.deltaTa-1)%256]) > HardWare.THa1)pa1=1;
  else pa1=0;
  if((counta-dpma[(ismascale-HardWare.deltaTa-1)%256]) > HardWare.THa2)pa2=1;
  else pa2=0;  
  
  if(HardWare.nodelayAf){pa1o=pa1;pa2o=pa2;}else
  {pa1o=delpa1[(ismascale-dela)%D11];pa2o=delpa2[(ismascale-dela)%D11];} 
  if(!((ism+scale_offseta)%(HardWare.scaleA+1))){
    dpma[ismascale%256]=counta;    
    delpa1[ismascale%D11]=pa1; delpa2[ismascale%D11]=pa2;
    ismascale++;
  }
  // BLOCK B 
  if((countb-dpmb[(ismbscale-HardWare.deltaTb-1)%256]) > HardWare.THb1)pb1=1;
  else pb1=0;
  if((countb-dpmb[(ismbscale-HardWare.deltaTb-1)%256]) > HardWare.THb2)pb2=1;
  else pb2=0;  
    
  if(HardWare.nodelayBf){pb1o=pb1;pb2o=pb2;}else
  {pb1o=delpb1[(ismbscale-delb)%D11];pb2o=delpb2[(ismbscale-delb)%D11];} 
  if(!((ism+scale_offsetb)%(HardWare.scaleB+1))){
    dpmb[ismbscale%256]=countb;    
    delpb1[ismbscale%D11]=pb1; delpb2[ismbscale%D11]=pb2;    
    ismbscale++;  
  } 

  // OR for P1 and P2
  //if((pb1+pb2))printf("ism,pa1,pb1,pa2,pb2 %i %i %i %i %i\n",ism,pa1,pb1,pa2,pb2);
  p1 = pa1o || pb1o;
  p2 = pa2o || pb2o;	  

  // Look-up table P1,P2,Delayed INT
  pp=lookup8(p1,p2,deldpm[(ism-HardWare.delayINT)%D12],HardWare.lut12D);
  //pp=lookup8(p1,p2,intd,HardWare.lut12D);
  //printf("p1,p2,,intd,pp %i %i %i %i\n",p1,p2,intd,pp);
  
  // BC clock
  ism++ ;  
 }
 return sum;
}

