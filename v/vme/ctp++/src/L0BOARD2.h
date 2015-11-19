#ifndef _L0BOARD2_h_
#define _L0BOARD2_h_
#include "L0BOARD.h"
#include <cmath>
class L0BOARD2: public L0BOARD
{
 public:
	L0BOARD2(int vsp);
	~L0BOARD2();
	void setClassVetoes(w32 index,w32 cluster,w32 bcm,w32 rare,w32 clsmask); // obsolete
	void setClassVetoesL0(w32 index,w32 cluster,w32 lml0busy,w32 clsmask,w32 alrare=1,w32 pf=0xf);
	void setClassVetoesLM(w32 index,w32 cluster,w32 lmdeadtime,w32 clsmask,w32 alrare=1,w32 pf=0xf);
	void setClassVetoes(w32 index,w32 cluster);
	void setClassConditionL0(w32 index,w32 inputs,w32 rndtrg,w32 bctrg,w32 bcmask,w32 l0fun=0xf);
	void setClassConditionLM(w32 index,w32 inputs,w32 rndtrg,w32 bctrg,w32 bcmask,w32 lmfun=0xf);
	void setClassInvert(w32 index,w32 invert=0xfff);
	void setBC1(w32 T){vmew(SCALED_1,T);};
	void setBC2(w32 T){vmew(SCALED_2,T);};
 	void setTCSET(w32 w){vmew(TCSET,w);};
 	void setTCCLEAR(){vmew(TCCLEAR,0);};
	void writeL0Function(int i,w32 word);
	void writeLMFunction(int i,w32 word){vmew(LM_FUNCTION+(i-1)*4,word);};
        void writeFunction(int i,w32 word);
        void setFunction(int ifun,bool* mask);
	void writeL0INTfunction(int i,w32 word);
 	int calcLUT(string& fun,bool* mask);
	void setOrbitOffset(w32 off){vmew(ORBIT_OFFSET,off);};
	void setBCOffset(w32 off){vmew(BCOFFSET,off);};
	void setL0INTSEL(w32 word){vmew(L0_INTERACTSEL,word);};
	void setLMINTSEL(w32 word){vmew(LM_INTERACTSEL,word);};
	void setINRND1_24(w32 inp){vmew(RND1_EN_FOR_INPUTS,1<<(inp-1));};
	void setINRND24_48(w32 inp){vmew(RND1_EN_FOR_INPUTS+0x4,1<<(inp-25));};
	void setLMRND1rate(w32 word){vmew(LM_RANDOM_1,word);};
	w32 getBC1(){return vmer(SCALED_1);};
	w32 getBC2(){return vmer(SCALED_2);};
	w32 getTCSTATUS(){return vmer(TCSTATUS);};
	w32 getL0rqst(){return (vmer(TCSTATUS)&0x4)/0x4;};
	w32 getl0ackn(){return (vmer(TCSTATUS)&0x8)/0x8;};
        w32 getOrbitOffset(){return vmer(ORBIT_OFFSET);};
	w32 getBCOffset(){return vmer(BCOFFSET);};
	// lm level
	void configL0classesonly();
	void printClass(w32 i);
	void readHWClass(w32 i);
	void writeHWClass(w32 i);
	void printClassConfiguration();
	void readHWClasses();
	void writeHWClasses();
	void convertL02LMClass(w32 i);
	void convertL02LMClassAll();
	// Inputs
	void setSwitch(int i48,int i24);
	void setSwitchAll0();
	void setLMSwitch(int in12, int out12);
	void printSwitch();
	// ssm methods from ctp
        void ddr3_reset();
        void ddr3_status();
        int ddr3_wrdone();
        int ddr3_rddone();
	int ddr3_read(w32 ddr3_ad, w32 *mem_ad, int nws);
        int ddr3_write(w32 ddr3_ad, w32 *mem_ad, int nws);
	int ddr3_ssmread();
	void ddr3_ssmstart(int sec);
        void SetSSM1(w32 *ssm){ssm1=ssm;};
	//
	int DumpSSM(const char *name,int issm);
	int DumpSSMLM(const char *name);
	int AnalSSM();
	void printClasses();
	void readBCMASKS();
	void writeBCMASKS(w32* pat);
        int getOrbits();
        deque<IRDda>& getIRs(){return irs;}
	enum {NCLASS=100};
 private:
	enum{DDR3_TO=30, DDR3_BLKL=16, 
	     DDR3_rd_done=0x1000000,
	     DDR3_wr_done=0x0800000};
         // SSM is special for L)m board
         w32 *ssm1,*ssm2,*ssm3,*ssm4,*ssm5,*ssm6,*ssm7;
         // vme addresses
         w32 const TCSET;
         w32 const TCSTATUS;
         w32 const TCCLEAR;
         w32 const TCSTART;
         w32 const MASK_DATA;
	 w32 const MASK_CLEARADD;
	 w32 const MASK_MODE;
         w32 const LM_INTERACT1;w32 const LM_INTERACT2;w32 const LM_INTERACT_T;w32 const LM_INTERACTSEL;
	 w32 const SCALED_1;w32 const SCALED_2;
	 w32 const LM_RANDOM_1;w32 const LM_RANDOM_2;
	 w32 const DDR3_CONF_REG0;w32 const DDR3_CONF_REG1;w32 const DDR3_CONF_REG2;
	 w32 const DDR3_CONF_REG3;w32 const DDR3_CONF_REG4;w32 const DDR3_BUFF_DATA; 
	 w32 const SYNCAL;   // Tono:SYNCH_ADD
         w32 const L0_INTERACT1;w32 const L0_INTERACT2;w32 const L0_INTERACT_T;w32 const L0_INTERACTSEL;
	 w32 const RND1_EN_FOR_INPUTS;
	 //w32 const L0_CONDITION; declared in L0BOARD
	 w32 const L0_VETO;
	 w32 const LM_CONDITION;
	 w32 const LM_INVERT;
	 w32 const LM_VETO;
	 w32 const LM_FUNCTION;
	 w32 const L0_FUNCTION;
   	 w32 const ORBIT_OFFSET;
   	 w32 const BCOFFSET;
	 // Configuration
	 w32 lmcond[NCLASS], lmveto[NCLASS],l0cond[NCLASS],l0veto[NCLASS];
	 deque<IRDda> irs;
};
#endif
