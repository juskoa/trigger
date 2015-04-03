#include "L0BOARD2.h"
L0BOARD2::L0BOARD2(int vsp)
:
	L0BOARD(vsp),
	L0VETO(0x800),
	SCALED_1(0x224),SCALED_2(0x228),
        DDR3_CONF_REG0(0x280),DDR3_CONF_REG1(0x284),DDR3_CONF_REG2(0x288),DDR3_CONF_REG3(0x28c),DDR3_CONF_REG4(0x290),
        DDR3_BUFF_DATA(0x2c0)
{
}
//----------------------------------------------------------------------------
/* 
 * set single class at index with cluster,vetoes, no PF
*/ 
void L0BOARD2::setClassVetoes(w32 index,w32 cluster,w32 bcm,w32 rare,w32 clsmask)
{
 w32 word=0;
 // Downscaling to be added
 word=cluster + (0xf<<4) + (bcm<<8)+(rare<<20) + (clsmask<<23);
 vmew(L0VETO+4*index,word);
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
/* 
 * read and print all classes
*/
void L0BOARD2::printClasses()
{
 printf("CTP classes from hardware:\n");
 for(w32 i=0; i<kNClasses; i++){
    w32 cond=vmer(L0CONDITION+4*(i+1));
    w32 veto=vmer(L0VETO+4*(i+1));
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
int L0BOARD2::ddr3_ssmread() {
int ddr3ad, ix, rc;
w32 block[DDR3_BLKL];
if (!ssm1) ssm1 = new w32[Mega];
if (!ssm2) ssm2 = new w32[Mega];
for(ix=0; ix< Mega; ix++) {
  ddr3ad= ix*16;
  rc= ddr3_read(ddr3ad, block, DDR3_BLKL);
  if(rc!=0) {
    printf("Error:%d reading ddr3ad %d\n", rc, ddr3ad);
    return(rc);
  };
  ssm1[ix]= block[14];
  ssm2[ix]= block[15];
};
return(0);
}
void L0BOARD2::ddr3_ssmstart(int secs) {
w32 ssmcmd;
w32 seconds1,micseconds1, seconds2,micseconds2,diff;
if(secs==0) {
  ssmcmd= 0;
} else {
  ssmcmd= 1;
};
vmew(SSMaddress, 0);
vmew(SSMcommand, ssmcmd);
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
printf("%d micsecs\n", diff);
}
int L0BOARD2::DumpSSM(const char *name,int issm)
{
 if(issm==1)SetSSM(ssm1); else SetSSM(ssm2); 
 return BOARD::DumpSSM(name);
}

