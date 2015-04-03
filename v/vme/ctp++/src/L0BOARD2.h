#ifndef _L0BOARD2_h_
#define _L0BOARD2_h_
#include "L0BOARD.h"
#include <cmath>
class L0BOARD2: public L0BOARD
{
 public:
	L0BOARD2(int vsp);
	void setClassVetoes(w32 index,w32 cluster,w32 bcm,w32 rare,w32 clsmask);
	void setClassVetoes(w32 index,w32 cluster);
	void setBC1(w32 T){vmew(SCALED_1,T);};
	void setBC2(w32 T){vmew(SCALED_2,T);};
	w32 getBC1(){return vmer(SCALED_1);};
	w32 getBC2(){return vmer(SCALED_2);};
        void ddr3_reset();
        void ddr3_status();
        int ddr3_wrdone();
        int ddr3_rddone();
	int ddr3_read(w32 ddr3_ad, w32 *mem_ad, int nws);
        int ddr3_write(w32 ddr3_ad, w32 *mem_ad, int nws);
	int ddr3_ssmread();
	void ddr3_ssmstart(int sec);
	int DumpSSM(const char *name,int issm);
	void printClasses();

 private:
	enum{DDR3_TO=30, DDR3_BLKL=16, 
	     DDR3_rd_done=0x1000000,
	     DDR3_wr_done=0x0800000};
         // SSM is special for L)m board
         w32 *ssm1,*ssm2;
         // vme addresses
	 w32 const L0VETO;
	 w32 const SCALED_1;
	 w32 const SCALED_2;
	 w32 const DDR3_CONF_REG0;
	 w32 const DDR3_CONF_REG1;
	 w32 const DDR3_CONF_REG2;
	 w32 const DDR3_CONF_REG3;
	 w32 const DDR3_CONF_REG4;
	 w32 const DDR3_BUFF_DATA; 
};
#endif
