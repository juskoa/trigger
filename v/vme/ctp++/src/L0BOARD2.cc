#include "L0BOARD2.h"
L0BOARD2::L0BOARD2(int vsp)
:
	L0BOARD(vsp),
        ssm1(0),ssm2(0),ssm3(0),ssm4(0),ssm5(0),ssm6(0),ssm7(0),
	TCSET(0x3fc),TCSTATUS(0x1c0),TCCLEAR(0x1c8),
	TCSTART(0x1c4),
	MASK_DATA(0x1e4),MASK_CLEARADD(0x1e8),MASK_MODE(0x1ec),
	SCALED_1(0x224),SCALED_2(0x228),
        DDR3_CONF_REG0(0x280),DDR3_CONF_REG1(0x284),DDR3_CONF_REG2(0x288),DDR3_CONF_REG3(0x28c),DDR3_CONF_REG4(0x290),
        DDR3_BUFF_DATA(0x2c0),
        SYNCAL(0x340),
        //L0_CONDITION(0x400),
        //L0_INVERT(0x600),
	L0_VETO(0x800),
        LM_CONDITION(0xa00),
        LM_INVERT(0xc00),
	LM_VETO(0xe00),
	LM_FUNCTION(4*0x9d),L0_FUNCTION(4*0x85)
{
}
L0BOARD2::~L0BOARD2()
{
 if(ssm1) delete [] ssm1;
 if(ssm2) delete [] ssm2;
 if(ssm3) delete [] ssm3;
 if(ssm4) delete [] ssm4;
 if(ssm5) delete [] ssm5;
 if(ssm6) delete [] ssm6;
 if(ssm7) delete [] ssm7;
}
//-------------------------------------------------------------------------------
// write word to L0 function. Problem is that vme adresses are not consecutive
//
void L0BOARD2::writeL0Function(int i,w32 word)
{
 if(i<3){
   vmew(L0_FUNCTION+(i-1)*4,word);
 }else{
   vmew(L0_FUNCTION+10*4+(i-3)*4,word);
 }
}
void L0BOARD2::writeFunction(int i,w32 word)
{
 w32 address;
 if(i<3){
   address = L0_FUNCTION+(i-1);
 }else if(i<5){
   address = L0_FUNCTION+10*4+(i-3)*4;
 }else if(i<9){
   address = LM_FUNCTION+4*(i-5);
 }else{
   printf("writeFunction: function out of range: %i \n",i);
   return ;
 }
 printf("fun=%i address=0x%x word=0x%x \n",i,address,word);
 vmew(address,word);
}
//---------------------------------------------------------------------------
// claculate LUT ising parse from ctplib++
int L0BOARD2::calcLUT(string& fun,bool* mask)
{
 int ll=fun.length();
 for(int i=0;i<256;i++){
  string funeval(fun);
  printf("funeval: %s \n",funeval.c_str());
  for(int j=0;j<ll;j++){
    char inp=funeval[j];
    if((inp>=97) && (inp<=104)){
      bool ii=((1<<(inp-97)) & i) == (1<<(inp-97));
      if(ii==1)funeval[j]='1';else funeval[j]='0';
    }
  }  
  printf("funeval: %s \n",funeval.c_str());
  int ret=parse(funeval.c_str(),0);
  if(ret==48)mask[i]=0;
  else if(ret==49)mask[i]=1;
  else return ret;
 }
 return 0;
}
//----------------------------------------------------------------------------
//
void L0BOARD2::setFunction(int ifun,bool* mask)
{
 //bool mask[256];
 //for(int i=0;i<256;i++){
 //  if((i & 0x3)==0x3) mask[i] = 1; else mask[i] = 0;
   //printf("%i %i \n",i,mask[i]);
 //}
 int maskchar[256/4];
 for(int i=0;i<256/4;i++)maskchar[i]=0;
 for(int i=0;i<256;i++){
   maskchar[i/4] += mask[i]<<(i%4);
 }
 for(int i=0;i<256/4;i++)printf("%x",maskchar[i]);
 printf("\n");
 //printf("LUT: \n %s",maskchar);
 for(int i=0;i<16;i++){
  w32 word=i;
  w32 mm=0;
  for(int j=0;j<16;j++){
     mm+=(mask[16*i+j]<<j);
     //mm2+=(mask[16*(16-i)-j-1]<<(j));
     //printf("mm,mm2, 0x%x 0x%x \n",mm,mm2);
  }
  word+=(mm<<16);
  printf("word 0x%x \n",word);
  writeFunction(ifun,word); 
 }
}
//----------------------------------------------------------------------------
/* 
 * set single class at index with cluster,vetoes, no PF
 * Version before LM - to be obsolete
*/ 
void L0BOARD2::setClassVetoes(w32 index,w32 cluster,w32 bcm,w32 rare,w32 clsmask)
{
 w32 word=0;
 // Downscaling to be added
 word=cluster + (0xf<<4) + (bcm<<8)+(rare<<20) + (clsmask<<23);
 vmew(L0_VETO+4*index,word);
}
/* ------------------------------------------------------------------------------------
 * set single class L0 vetoes at index with cluster,vetoes, no PF
 * Version with LM level
*/ 
void L0BOARD2::setClassVetoesL0(w32 index,w32 cluster,w32 lml0busy,w32 clsmask,w32 alrare,w32 pf)
{
 w32 word=0,bcmask=0;
 bcmask = vmer(L0_VETO+4*index)&0x000fff00;
 // Downscaling to be addeda
 word=cluster + (pf<<4)+(bcmask<<0)+(alrare<<20)+(lml0busy<<21) + (clsmask<<23);
 vmew(L0_VETO+4*index,word);
}
/* 
 * set single class L0 vetoes at index with cluster,vetoes, no PF
 * Version with LM level
*/ 
void L0BOARD2::setClassVetoesLM(w32 index,w32 cluster,w32 lmdeadtime,w32 clsmask,w32 alrare,w32 pf)
{
 w32 word=0;
 // Downscaling to be addeda
 word=cluster+(lmdeadtime<<8)+(alrare<<9)+(pf<<10)+(clsmask<<16);
 vmew(LM_VETO+4*index,word);
}
//----------------------------------------------------------------------------
/* 
 * set single class at index with cluster only, and mask 0x0 (active)
*/
void L0BOARD2::setClassVetoes(w32 index,w32 cluster)
{
 setClassVetoes(index,cluster,0xfff,0x1,0x0);
}
//----------------------------------------------------------------------------
void L0BOARD2::setClassConditionL0(w32 index,w32 inputs,w32 rndtrg, w32 bctrg,w32 bcmask,w32 l0fun)
{
 w32 word=0;
 word=inputs+(l0fun<<24)+(rndtrg<<28)+(bctrg<<30);
 vmew(L0_CONDITION+4*index,word);
 word = vmer(L0_VETO+4*index);
 word = word & 0xfff000ff;
 word = word+(bcmask<<8);
 vmew(L0_VETO+4*index,word);
}
//---------------------------------------------------------------------------------
void L0BOARD2::setClassConditionLM(w32 index,w32 inputs,w32 rndtrg,w32 bctrg,w32 bcmask,w32 lmfun)
{
 w32 word=0;
 word=inputs+(lmfun<<12)+(rndtrg<<16)+(bctrg<<18)+(bcmask<<20);
 //printf("setClassConditionLM: 0x%x 0x%x 0x%x \n",LM_CONDITION+4*index,word,rndtrg);
 vmew(LM_CONDITION+4*index,word);
}
//----------------------------------------------------------------------------
void L0BOARD2::setClassInvert(w32 index, w32 invert)
{
 vmew(LM_INVERT+4*index,invert);
}
//----------------------------------------------------------------------------
void L0BOARD2::writeBCMASKS(w32* pat)
{
 vmew(MASK_MODE,1); // vme access
 vmew(MASK_CLEARADD,0x0);
 for(int i=0;i<3564;i++){
   vmew(MASK_DATA,pat[i]);
 }
 vmew(MASK_MODE,0);
}
//----------------------------------------------------------------------------
void L0BOARD2::readBCMASKS()
{
 vmew(MASK_MODE,1); // vme access
 vmew(MASK_CLEARADD,0x0);
 for(int i=0;i<3564;i++){
   w32 word=vmer(MASK_DATA);
   printf("%03x",word);
   if(((i+1)%66)==0)printf("\n");
 }
 printf("\n");
 vmew(MASK_MODE,0);
}
//----------------------------------------------------------------------------
/* 
 * read and print all classes
*/
void L0BOARD2::printClasses()
{
 printf("CTP classes from hardware:\n");
 for(w32 i=0; i<kNClasses; i++){
    w32 cond=vmer(L0_CONDITION+4*(i+1));
    w32 veto=vmer(L0_VETO+4*(i+1));
    printf("%i  0x%x 0x%x \n",i+1,cond,veto);
    //if((i+1)%10 == 0)printf("\n");
 }
}
//----------------------------------------------------------------------------
void L0BOARD2::ddr3_reset()
{
 w32 streg;
 vmew(DDR3_CONF_REG0, 0x7);   // not needed after power-up
 vmew(DDR3_CONF_REG0, 0x4);   // 0x4: Errors_reset
 vmew(DDR3_CONF_REG0, 0x0);   // 0 to: Errors_reset, Logic_reset, DDR3_reset
 streg= vmer(DDR3_CONF_REG0);   // 0 to: Errors_reset, Logic_reset, DDR3_reset
 printf("DDR3_CONF_REG0: 0x%x (expected: 0xec000000)\n", streg);
}
void L0BOARD2::ddr3_status() {
 int ix; w32 val;
 char txflags[300]="";
 const char *txflg[32]= {"?","?","?","?","?","?","?","?","?","?",
 "?","?","?","?","?","?","?","?","?","?",
 "?","?","?",
 "wr_done", "rd_done", "rst_logic", "ddr3_ext_wr_itf_rdy", "ddr3_ext_rd_itf_rdy",
 "full_flag", "rdi_fifo_has_space", "rdi_fifo_empty", "mem_init"};
 val= vmer(DDR3_CONF_REG0);
 sprintf(txflags, "DDR3_CONF_REG0:0x%x =", val);
 for(ix=31; ix>=23; ix--) {
  w32 valix;
  valix= val>>ix;
  //printf("val:0x%x ix:%d val>>ix:0x%x\n", val, ix, valix);
  if((valix & 0x1)==0x1) {
    sprintf(txflags, "%s\n%2d:%s", txflags, ix, txflg[ix]);
    //strcat(txflags, txflg[ix]); strcat(txflags, " ");
  };
 }; printf("%s\n", txflags);
}
int L0BOARD2::ddr3_wrdone() 
{
int irep; w32 status;
irep=0;
do {
  if(irep>DDR3_TO) {
    printf("wrdone error: status: 0x%x\n", status);
    return(1);
  };
  status= vmer(DDR3_CONF_REG0); irep++;
} while((status & DDR3_wr_done) == 0);
return(0);
};

int L0BOARD2::ddr3_rddone() 
{
int irep; w32 status;
irep=0;
do {
  if(irep>DDR3_TO) {
    printf("rddone error: status: 0x%x\n", status);
    return(1);
  };
  status= vmer(DDR3_CONF_REG0); irep++;
} while((status & DDR3_rd_done) == 0);
return(0);
}
//--------------------------------------------------------------
int L0BOARD2::ddr3_read(w32 ddr3_ad, w32 *mem_ad, int nws) 
{
int rc, ix, iblks, llb, blocks, block0;
// calculate number of full blocks and length of the last block:
blocks= nws/DDR3_BLKL; llb= nws % DDR3_BLKL; block0= ddr3_ad/2;
for(iblks=0; iblks<blocks; iblks++) {
  int cra;
  cra= block0+iblks*8;
  vmew(DDR3_CONF_REG1, cra);
  vmew(DDR3_CONF_REG2, 1);
  rc= ddr3_rddone(); if(rc!=0) return(rc);
  //printf("ddr3_read: 16 words from ddr3_ad:0x%x cra:%d ...\n", ddr3_ad, cra);
  for(ix=0; ix<DDR3_BLKL; ix++) {
    *mem_ad= vmer(DDR3_BUFF_DATA+ix*4);
    mem_ad++;
  };
};
if( llb>0 ) {   // arrange dummy reads for last block
  int cra;
  cra= block0+blocks*8;
  vmew(DDR3_CONF_REG1, cra);
  vmew(DDR3_CONF_REG2, 1);
  rc= ddr3_rddone(); if(rc!=0) return(rc);
  //printf("ddr3_read:last block len:%d cra:%d ...\n", llb, cra);
  for(ix=0; ix<DDR3_BLKL; ix++) {
    if( ix<llb ) {
      *mem_ad= vmer(DDR3_BUFF_DATA+ix*4);
      mem_ad++;
      //printf("%3d read 0x%x\n", ix, *mem_ad);
    } else {
      w32 dummyr;
      dummyr= vmer(DDR3_BUFF_DATA+ix*4);
      //printf("%3d dummy read 0x%x\n", ix, dummyr);
    };
  };
};
return(rc);
}
//---------------------------------------------------------------
int L0BOARD2::ddr3_write(w32 ddr3_ad, w32 *mem_ad, int nws) 
{
int rc, ix, iblks, llb, blocks, block0;
// calculate number of full blocks and length of the last block:
blocks= nws/DDR3_BLKL; llb= nws % DDR3_BLKL; block0= ddr3_ad/2;
for(iblks=0; iblks<blocks; iblks++) {
  int cra;
  cra= block0+iblks*8;
  vmew(DDR3_CONF_REG3, cra);
  vmew(DDR3_CONF_REG4, 1);
  //printf("ddr3_write:16 words from ddr3_ad:0x%x cra:%d ...\n", ddr3_ad, cra);
  for(ix=0; ix<DDR3_BLKL; ix++) {
    vmew(DDR3_BUFF_DATA+ix*4, *mem_ad);
    mem_ad++;
    //printf("%3d written 0x%x\n", ix, val);
  };
  rc= ddr3_wrdone(); if(rc!=0) return(rc);
};
if( llb>0 ) {   // last block padded with 0s:
  int cra;
  cra= block0+blocks*8;
  vmew(DDR3_CONF_REG3, cra);
  vmew(DDR3_CONF_REG4, 1);
  //printf("ddr3_write:last block len:%d cra:%d ...\n", llb, cra);
  for(ix=0; ix<DDR3_BLKL; ix++) {
    if( ix<llb ) {
      vmew(DDR3_BUFF_DATA+ix*4, *mem_ad);
      mem_ad++;
    } else {
      vmew(DDR3_BUFF_DATA+ix*4, 0);
    };
    //printf("%3d written 0x%x\n", ix, val);
  };
  rc= ddr3_wrdone(); if(rc!=0) return(rc);
};
return(rc);
}
//-----------------------------------------------------------------------------
int L0BOARD2::ddr3_ssmread() {
int ddr3ad, ix, rc;
w32 block[DDR3_BLKL];
if (!ssm1) ssm1 = new w32[Mega];
if (!ssm2) ssm2 = new w32[Mega];
if (!ssm3) ssm3 = new w32[Mega];
if (!ssm4) ssm4 = new w32[Mega];
if (!ssm5) ssm5 = new w32[Mega];
if (!ssm6) ssm6 = new w32[Mega];
if (!ssm7) ssm7 = new w32[Mega];
for(ix=0; ix< Mega; ix++) {
  ddr3ad= ix*16;
  rc = ddr3_read(ddr3ad, block, DDR3_BLKL);
  if(rc!=0) {
    printf("Error:%d reading ddr3ad %d\n", rc, ddr3ad);
    return(rc);
  };
  //ssm1[ix]= block[14];
  //ssm2[ix]= block[15];
  ssm1[ix]= block[15];
  ssm2[ix]= block[14];
  ssm3[ix]= block[13];
  ssm4[ix]= block[12];
  ssm5[ix]= block[11];
  ssm6[ix]= block[10];
  ssm7[ix]= block[9];
};
printf("LM ssm read \n");
return(0);
}
//-----------------------------------------------------------
//secs=0 => 1 pass
//secs!=0 => continueos
void L0BOARD2::ddr3_ssmstart(int secs) {
w32 ssmcmd;
w32 seconds1,micseconds1, seconds2,micseconds2,diff;
if(secs==0) {
  ssmcmd= 0;
} else {
  ssmcmd= 1;
};
w32 status = vmer(SSMstatus);
if(status & 0x100){
  printf("SSMbusy, stop recording");
}
vmew(SSMcommand, ssmcmd);
vmew(SSMaddress, 0);
GetMicSec(&seconds1, &micseconds1);
vmew(SSMstart, DUMMYVAL);
if(secs>0) {
  sleep(secs);
} else {
  while(1) {
    w32 st;
    st= vmer(SSMstatus);
    if((st&0x2)==0) continue;   // wait till whole memory recorded
    break;
  };
};
GetMicSec(&seconds2, &micseconds2);
diff=DiffSecUsec(seconds2, micseconds2, seconds1, micseconds1);
printf("ddr3_ssmstart: %d micsecs\n", diff);
}
int L0BOARD2::DumpSSM(const char *name,int issm)
{
 //if(issm==1)SetSSM(ssm1); else SetSSM(ssm2);
 switch(issm)
 {
  case(1): SetSSM(ssm1);break;
  case(2): SetSSM(ssm2);break;
  case(3): SetSSM(ssm3);break;
  case(4): SetSSM(ssm4);break;
  case(5): SetSSM(ssm5);break;
  case(6): SetSSM(ssm6);break;
  case(7): SetSSM(ssm7);break;
  default:{
    printf("DumpSSM: internal error, issm=%i \n",issm);
    return 1;
  }
 }
 return BOARD::DumpSSM(name);
}
int L0BOARD2::DumpSSMLM(const char *name)
{
 int rc=0;
 //printf("%x %x %x %x \n",ssm1,ssm2,ssm3,ssm4);
 SetSSM(ssm1);
 rc=BOARD::DumpSSM("ssm1"); 
 SetSSM(ssm2); 
 rc+=BOARD::DumpSSM("ssm2"); 
 SetSSM(ssm3); 
 rc+=BOARD::DumpSSM("ssm3"); 
 SetSSM(ssm4); 
 rc+=BOARD::DumpSSM("ssm4"); 
 return rc;
}
//-----------------------------------------------------------------------------
void L0BOARD2::configL0classesonly()
{
 for(w32 i=0;i<NCLASS;i++){
    //printf("seting class %i \n",i);
    setClassConditionLM(i,0xfff,0x3,0x3,0xfff);
    setClassVetoesLM(i,0xff,1,1,1);
    setClassConditionL0(i,0xffffff,0x3,0x3,0xfff);
    setClassVetoesL0(i,0x7,1,1,1);

 }
}
//-------------------------------------------------------------------
int L0BOARD2::AnalSSM()
{
 w32 l0b=0,l0a=0;
 for(w32 i=0;i<Mega;i++){
   if(ssm2[i]&(1<<19))l0b++;
   if(ssm2[i]&(1<<16))l0a++;
 }
 printf("l0b=%i l0a=%i \n",l0b,l0a);
 return 0;
}
//-------------------------------------------------------------------
void L0BOARD2::printClass(w32 i)
{
 //w32 lmv=vmer(LM_VETO+(i+1)*4);
 //w32 l0v=vmer(L0_VETO+(i+1)*4);
 //w32 lmc=vmer(LM_CONDITION+(i+1)*4);
 //w32 l0c=vmer(L0_CONDITION+(i+1)*4);
 //printf("Class %2i LMCONDITION:0x%08x LMVETO:0x%08x  L0CONDITION:0x%08x L0VETO:0x%08x\n",i,lmc,lmv,l0c,l0v); 
 //printf("Class %2i LMCONDITION:0x%08x LMVETO:0x%08x  L0CONDITION:0x%08x L0VETO:0x%08x\n",i,lmc,lmv,l0c,l0v); 
 printf("Class %2i LMCONDITION:0x%08x LMVETO:0x%08x  L0CONDITION:0x%08x L0VETO:0x%08x\n",i,lmcond[i],lmveto[i],l0cond[i],l0veto[i]); 
}
//-------------------------------------------------------------------
void L0BOARD2::readHWClass(w32 i)
{
 lmveto[i]=vmer(LM_VETO+(i+1)*4);
 l0veto[i]=vmer(L0_VETO+(i+1)*4);
 lmcond[i]=vmer(LM_CONDITION+(i+1)*4);
 l0cond[i]=vmer(L0_CONDITION+(i+1)*4);
}
//-------------------------------------------------------------------
void L0BOARD2::writeHWClass(w32 i)
{
 vmew(LM_VETO+(i+1)*4,lmveto[i]);
 vmew(L0_VETO+(i+1)*4,l0veto[i]);
 vmew(LM_CONDITION+(i+1)*4,lmcond[i]);
 vmew(L0_CONDITION+(i+1)*4,l0cond[i]);
}
//-------------------------------------------------------------------
void L0BOARD2::printClassConfiguration()
{
 for(int i=0;i<NCLASS;i++){
  printClass(i);
 }
}
//-------------------------------------------------------------------
void L0BOARD2::readHWClasses()
{
 for(int i=0;i<NCLASS;i++){
  readHWClass(i);
 }
}
//-------------------------------------------------------------------
void L0BOARD2::writeHWClasses()
{
 for(int i=0;i<NCLASS;i++){
  writeHWClass(i);
 }
}
//--------------------------------------------------------------------
// Used for convertinf Didier test file to LM classes
void L0BOARD2::convertL02LMClass(w32 i)
{
 // cluster
 l0veto[i]=l0veto[i]&0xfffffff0;
 l0veto[i]=l0veto[i]|0x2;
 return ;
 w32 bcrnd = l0cond[i]&0xf0000000;
 l0cond[i]=l0cond[i] | 0xf0000000;
 l0veto[i]=l0veto[i] | 0x200000;  // LM-L0 busy
 //printf("bcrnd=0x%x \n",bcrnd);
 lmcond[i]=0xfff0ffff | (bcrnd>>12);
 lmveto[i]=0x7f00feff; 
 // switch off all
 l0cond[i]=0;
 lmcond[i]=0;
}
//--------------------------------------------------------------------
// Used for convertinf Didier test file to LM classes
void L0BOARD2::convertL02LMClassAll()
{
 for(int i=0;i<NCLASS;i++){
  if((i %2)==0)convertL02LMClass(i);
 }
}
//-----------------------------------------------------------------
// Get orbits from ssm
//
int L0BOARD2::getOrbits()
{
 if(ssm1 == 0){printf("ssm1 missing \n");return 1;}
 if(ssm3 == 0){printf("ssm3 missing \n");return 1;}
 if(ssm6 == 0){printf("ssm6 missing \n");return 1;}
 if(ssm7 == 0){printf("ssm7 missing \n");return 1;}
 // 
 w32 orbit=0xffffffff;
 w32 orbit0=0x0;
 w32 ocount=0;
 w32 nint=0;
 w32 orbitssm=0;
 IRDda irda;
 clearIRDda(irda);
 //
 w32 orbit0c=0xf;
 w32 orbitc=0;
 w32 ocountc=0;
 bool deb=1;
 //
 for(int i=0;i<Mega;i++){
  // Input checker - calculate IR
  if(ssm3[i]&(1<<22)){
   irda.Inter[nint]=1;
   irda.bc[nint]=i-orbitssm+11;
   nint++;
   if(deb)printf("Input 3: %i \n",i);
  }
  // Orbit from number
  if(ssm6){
  orbit0=orbit;
  orbit=0;
  for(int j=0;j<20;j++){
   w32 mask=1<<(j+12);
   orbit+=((ssm6[i]&mask)==mask)<<j;
  }
  for(int j=0;j<4;j++){
   w32 mask=1<<(j);
   orbit+=((ssm7[i]&mask)==mask)<<(j+20);
  }
  printf("%i 0x%x %i\n",i,orbit,ssm1[i]&0x1);
  if(orbit0==0xffffffff){
    goto next;
  }else if(orbit0==orbit){
   ocount++;
   goto next;
  }else if((orbit-orbit0)==1){
   if(deb)printf("Orbit new:%i 0x%x ocount=%i \n",i,orbit,ocount);
   ocount=0;
   irs.push_back(irda);
   clearIRDda(irda);
   irda.orbit=orbit;
   irda.issm=i;
   nint=0;
   goto next;
  }else{
   printf("Orbit error:issm=%i  orbitold/orbit: 0x%x 0x%x, %i\n",i,orbit0,orbit,ocount);
   ocount=0;
   //return 1;
   goto next;
  }
  }
  next:
  //printf("%i 0x%x 0x%x 0x%x\n",i,ssm6[i],ssm7[i],ssm1[i]);
  // orbit from channel 0
  orbit0c=orbitc;
  orbitc=0;
  orbitc=ssm1[i]&0x1;
  if(orbit0c==0xf)continue;
  else if(orbit0c==orbitc){
   ocountc++;
   continue;
  }else{
   if(ocountc==39) continue;
   if(deb)printf("Orbit at channel 0 issm=%i Orbit length=%i\n",i,ocountc);
   orbitssm=i;
   ocountc=0;
   continue;
  }
 }
 return 0;
}
