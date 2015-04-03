#include <stdio.h>
#include <unistd.h>    /* sleep(), usleep() */
#include <string.h>
#include "vmewrap.h"
#include "ctp.h"
//#include "ctplib.h"

#define DDR3_TO 30     // timeout when reading ddr3 (number of loops)
#define DDR3_BLKL 16
#define MEGA 1024*1024

void ddr3_status() {
int ix; w32 val;
char txflags[300]="";
const char *txflg[32]= {"?","?","?","?","?","?","?","?","?","?",
"?","?","?","?","?","?","?","?","?","?",
"?","?","?",
"wr_done", "rd_done", "rst_logic", "ddr3_ext_wr_itf_rdy", "ddr3_ext_rd_itf_rdy",
"full_flag", "rdi_fifo_has_space", "rdi_fifo_empty", "mem_init"};
val= vmer32(DDR3_CONF_REG0);
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
int ddr3_wrdone() {
int irep; w32 status;
irep=0;
do {
  if(irep>DDR3_TO) {
    printf("wrdone error: status: 0x%x\n", status);
    return(1);
  };
  status= vmer32(DDR3_CONF_REG0); irep++;
} while((status & DDR3_wr_done) == 0);
return(0);
};
int ddr3_rddone() {
int irep; w32 status;
irep=0;
do {
  if(irep>DDR3_TO) {
    printf("rddone error: status: 0x%x\n", status);
    return(1);
  };
  status= vmer32(DDR3_CONF_REG0); irep++;
} while((status & DDR3_rd_done) == 0);
return(0);
}

void ddr3_reset() {
w32 streg;
vmew32(DDR3_CONF_REG0, 0x7);   // not needed after power-up
vmew32(DDR3_CONF_REG0, 0x4);   // 0x4: Errors_reset
vmew32(DDR3_CONF_REG0, 0x0);   // 0 to: Errors_reset, Logic_reset, DDR3_reset
vmew32(0x9000+SSMaddress, 0); // to get cleared SSMstatus[1] bit (i.e. no overflow)
streg= vmer32(DDR3_CONF_REG0);   // 0 to: Errors_reset, Logic_reset, DDR3_reset
printf("DDR3_CONF_REG0: 0x%x (expected: 0xec000000)\n", streg);
}

int ddr3_read(w32 ddr3_ad, w32 *mem_ad, int nws) {
int rc, ix, iblks, llb, blocks, block0;
// calculate number of full blocks and length of the last block:
blocks= nws/DDR3_BLKL; llb= nws % DDR3_BLKL; block0= ddr3_ad/2;
for(iblks=0; iblks<blocks; iblks++) {
  int cra;
  cra= block0+iblks*8;
  vmew32(DDR3_CONF_REG1, cra);
  vmew32(DDR3_CONF_REG2, 1);
  rc= ddr3_rddone(); if(rc!=0) return(rc);
  //printf("ddr3_read: 16 words from ddr3_ad:0x%x cra:%d ...\n", ddr3_ad, cra);
  for(ix=0; ix<DDR3_BLKL; ix++) {
    *mem_ad= vmer32(DDR3_BUFF_DATA+ix*4);
    mem_ad++;
  };
};
if( llb>0 ) {   // arrange dummy reads for last block
  int cra;
  cra= block0+blocks*8;
  vmew32(DDR3_CONF_REG1, cra);
  vmew32(DDR3_CONF_REG2, 1);
  rc= ddr3_rddone(); if(rc!=0) return(rc);
  //printf("ddr3_read:last block len:%d cra:%d ...\n", llb, cra);
  for(ix=0; ix<DDR3_BLKL; ix++) {
    if( ix<llb ) {
      *mem_ad= vmer32(DDR3_BUFF_DATA+ix*4);
      mem_ad++;
      //printf("%3d read 0x%x\n", ix, *mem_ad);
    } else {
      w32 dummyr;
      dummyr= vmer32(DDR3_BUFF_DATA+ix*4);
      //printf("%3d dummy read 0x%x\n", ix, dummyr);
    };
  };
};
return(rc);
}
int ddr3_write(w32 ddr3_ad, w32 *mem_ad, int nws) {
int rc, ix, iblks, llb, blocks, block0;
// calculate number of full blocks and length of the last block:
blocks= nws/DDR3_BLKL; llb= nws % DDR3_BLKL; block0= ddr3_ad/2;
for(iblks=0; iblks<blocks; iblks++) {
  int cra;
  cra= block0+iblks*8;
  vmew32(DDR3_CONF_REG3, cra);
  vmew32(DDR3_CONF_REG4, 1);
  //printf("ddr3_write:16 words from ddr3_ad:0x%x cra:%d ...\n", ddr3_ad, cra);
  for(ix=0; ix<DDR3_BLKL; ix++) {
    vmew32(DDR3_BUFF_DATA+ix*4, *mem_ad);
    mem_ad++;
    //printf("%3d written 0x%x\n", ix, val);
  };
  rc= ddr3_wrdone(); if(rc!=0) return(rc);
};
if( llb>0 ) {   // last block padded with 0s:
  int cra;
  cra= block0+blocks*8;
  vmew32(DDR3_CONF_REG3, cra);
  vmew32(DDR3_CONF_REG4, 1);
  //printf("ddr3_write:last block len:%d cra:%d ...\n", llb, cra);
  for(ix=0; ix<DDR3_BLKL; ix++) {
    if( ix<llb ) {
      vmew32(DDR3_BUFF_DATA+ix*4, *mem_ad);
      mem_ad++;
    } else {
      vmew32(DDR3_BUFF_DATA+ix*4, 0);
    };
    //printf("%3d written 0x%x\n", ix, val);
  };
  rc= ddr3_wrdone(); if(rc!=0) return(rc);
};
return(rc);
}
int ddr3_ssmread(w32 *ssm1, w32 *ssm2) {
int ddr3ad, ix, rc;
w32 block[DDR3_BLKL];
//for(ix=0; ix<= MEGA; ix++) {
for(ix=0; ix< MEGA; ix++) {
  ddr3ad= ix*16;
  rc= ddr3_read(ddr3ad, block, DDR3_BLKL);
  if(rc!=0) {
    printf("Error:%d reading ddr3ad %d\n", rc, ddr3ad);
    return(rc);
  };
  if(ssm1!=NULL) ssm1[ix]= block[14];
  if(ssm2!=NULL) ssm2[ix]= block[15];
};
vmew32(SSMaddress+BSP*ctpboards[1].dial, 0);   // clear 0x2 flag in SSMstatus
return(0);
}
int ddr3_ssmdump(w32 opmod, FILE *dump) {
int ddr3ad, ix, rc, opmoix;
w32 block[DDR3_BLKL];
int allbitn=0, ixb; int bits[32];
int lowix=8; int highix=31;
for(ixb=lowix; ixb<=highix; ixb++) bits[ixb]=0;

/*FILE *dump;
char fn[100]="WORK/"; */

//for(ix=0; ix<= MEGA; ix++) {
if(opmod==0xa) {
  opmoix= 14;
} else if(opmod==0xb) {
  opmoix= 15;
} else {
  return(1);
};
/*strcat(fn,fname);
dump= fopen(fn,"w");
if(dump==NULL) {
  printf("ddr3_ssmdump: cannot open file %s\n", fn);
  return(2);
}; */
for(ix=0; ix< MEGA; ix++) {
  w32 data;
  ddr3ad= ix*16;
  rc= ddr3_read(ddr3ad, block, DDR3_BLKL);
  if(rc!=0) {
    printf("Error:%d reading ddr3ad %d\n", rc, ddr3ad);
    return(rc);
  };
  data= block[opmoix];
  for(ixb=lowix; ixb<=highix; ixb++) {
    w32 msk;
    msk= 1<<ixb;
    if(data & msk) bits[ixb]++;
  };
  fwrite(&data, sizeof(w32), 1, dump);
};
vmew32(SSMaddress+BSP*ctpboards[1].dial, 0);   // clear 0x2 flag in SSMstatus
for(ixb=lowix; ixb<=highix; ixb++) {
  if(bits[ixb] >0) {
    allbitn= allbitn+bits[ixb];
    printf("%2d: %d\n", ixb, bits[ixb]);
  };
};
printf("ddr3_ssmdump bits:%d details above\n", allbitn);
return(0);
}
void ddr3_ssmstart(int secs) {
w32 ssmcmd;
w32 seconds1,micseconds1, seconds2,micseconds2,diff;
if(secs==0) {
  ssmcmd= 0;   // 1 pass (after)
} else {
  ssmcmd= 1;   // continuous (before)
};
vmew32(SSMaddress+BSP*ctpboards[1].dial, 0);
vmew32(SSMcommand+BSP*ctpboards[1].dial, ssmcmd);
GetMicSec(&seconds1, &micseconds1);
vmew32(SSMstart+BSP*ctpboards[1].dial, DUMMYVAL);
if(secs>0) {
  sleep(secs);
} else {
  while(1) {
    w32 st;
    st= vmer32(SSMstatus+BSP*ctpboards[1].dial);
    if((st&0x2)==0) continue;   // wait till whole memory recorded
    break;
  };
};
GetMicSec(&seconds2, &micseconds2);
diff=DiffSecUsec(seconds2, micseconds2, seconds1, micseconds1);
printf("%d micsecs\n", diff);
}


