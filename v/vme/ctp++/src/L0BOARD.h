#ifndef _L0BOARD_h_
#define _L0BOARD_h_
#include "BOARD.h"
#include <cmath>
class L0BOARD: public BOARD
{
 public:
	L0BOARD(int vsp);
	void setClass(w32 index,w32 inputs,w32 l0f,w32 rn,w32 bc);
	void setClass(w32 index,w32 inputs);
	void SetClass(w32 index,w32 inputs,w32 cluster);
	void setClassesToZero();
	void setClassInvert(w32 index,w32 invert=0xffffff);
	virtual void setClassVetoes(w32 index,w32 cluster,w32 bcm,w32 rare,w32 clsmask)=0;
	virtual void setClassVetoes(w32 index,w32 cluster)=0;
	virtual void setBC1(w32 T)=0;
	virtual void setBC2(w32 T)=0;
	virtual w32 getBC1()=0;
	virtual w32 getBC2()=0;
        virtual void readBCMASKS()=0;
        virtual void writeBCMASKS(w32* pat)=0;
	virtual void printClasses()=0;
	int CheckCountersNoTriggers();
	virtual void configL0classesonly(){error();};
	enum{CL0TIME=15,CL0CLSB=19,CL0STR=171,CL0CLSA=187,CL0CLST=289};
        // SSM
        virtual int AnalSSM();
        virtual void ddr3_reset(){error();};
        virtual void ddr3_status(){error();};
        virtual int ddr3_wrdone(){error(); return 1;};
        virtual int ddr3_rddone(){error(); return 1;};
	virtual int ddr3_read(w32 ddr3_ad, w32 *mem_ad, int nws){return 1;};
        virtual int ddr3_write(w32 ddr3_ad, w32 *mem_ad, int nws){return 1;};
	virtual int ddr3_ssmread(){return 1;};
        virtual void ddr3_ssmstart(int sec){error();};
 	virtual int DumpSSM(const char *name,int issm){error();return 1;};
 	virtual int DumpSSMLM(const char *name){error();return 1;};

 private:
        void error(){printf("Error: this method should be never called\n");exit(1);};
 protected:
 
         // vme addresses
         w32 const L0_CONDITION;
         w32 const L0_INVERT;
};
#endif
