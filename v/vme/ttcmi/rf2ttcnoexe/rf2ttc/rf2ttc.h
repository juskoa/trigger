/****************************************************************/
/*                                              		*/
/*  file: rf2ttc.h 	                           		*/
/*                                              		*/
/*  Author: Markus Joos, CERN-ECP               		*/
/*                                              		*/
/*  14.Aug.06  MAJO  created                   		  	*/
/*****C 2007 - The software with that certain something**********/

#ifndef _RF2TTC_H 
#define _RF2TTC_H


#include "common.h"


/*************/
/* Constants */
/*************/
#define TWO_MILLISECONDS 2000
#define RF2TTC_MAP_SIZE  0x80000
#define TEST_MODE        0
#define RUNNING_MODE     1
#define LAB_MODE         2
#define FIFOSIZE         255
#define DACRANGE         256
#define ORBRANGE         64
#define INVALID          9999


/*************/
/* Registers */
/*************/
#define MANUFACTURER_ID            0x00000
#define BOARD_ID                   0x00004
#define REVISION_ID                0x00008
#define PROGRAM_ID                 0x0000C
#define BSET                       0x00010
#define BCLEAR                     0x00014
#define BC_DELAY25_BC1             0x7D000
#define BC_DELAY25_BC2             0x7D004
#define BC_DELAY25_BCREF           0x7D008
#define BC_DELAY25_BCMAIN          0x7D00C
#define BC_DELAY25_GCR             0x7D014
#define ORBIN_DELAY25_ORB1         0x7D020
#define ORBIN_DELAY25_ORB2         0x7D024
#define ORBIN_DELAY25_GCR          0x7D034
#define ORBOUT_DELAY25_ORB1        0x7D040
#define ORBOUT_DELAY25_ORB2        0x7D044
#define ORBOUT_DELAY25_ORBMAIN     0x7D048
#define ORBOUT_DELAY25_GCR         0x7D054
#define DELAY25_REG                0x7D200
#define TTCRX_POINTER              0x7E000
#define TTCRX_DATA                 0x7E004
#define TTCRX_REG                  0x7E200
#define ORB_COUNTER_RESET          0x7FA44
#define PERIOD_COUNTER_RESET       0x7FA48
#define ORB_INT_RESET              0x7FA4C
#define PERIOD_COUNTER_ENABLE      0x7FA64
#define ORB_COUNTER_ENABLE         0x7FA68
#define ORB_INT_ENABLE             0x7FA6C
#define WORKING_MODE               0x7FA78
#define BEAM_NO_BEAM_DEF           0x7FA7C
#define BST_MACHINE_MODE           0x7FA9C 
#define TTCRX_STATUS               0x7FAA0 
#define ORBMAIN_PERIOD_FIFO_RD     0x7FAC0
#define ORBMAIN_PERIOD_FIFO_STATUS 0x7FAC4
#define ORBMAIN_PERIOD_RD          0x7FAC8
#define ORBMAIN_COUNTER            0x7FACC
#define ORBMAIN_INT_PERIOD_COUNTER 0x7FAD0
#define ORBMAIN_INT_PERIOD_SET     0x7FAD4
#define ORBMAIN_LENGTH             0x7FAD8
#define ORBMAIN_COURSE_DELAY       0x7FADC
#define ORBMAIN_POLARITY           0x7FAE0
#define ORBMAIN_NOBEAM_SELECT      0x7FAE4
#define ORBMAIN_BEAM_SELECT        0x7FAE8
#define ORBMAIN_MAN_SELECT         0x7FAEC 
#define ORB2_DAC                   0x7FAFC
#define ORB2_PERIOD_FIFO_RD        0x7FB00
#define ORB2_PERIOD_FIFO_STATUS    0x7FB04
#define ORB2_PERIOD_RD             0x7FB08
#define ORB2_COUNTER               0x7FB0C
#define ORB2_INT_PERIOD_COUNTER    0x7FB10
#define ORB2_INT_PERIOD_SET        0x7FB14
#define ORB2_LENGTH                0x7FB18
#define ORB2_COURSE_DELAY          0x7FB1C
#define ORB2_POLARITY              0x7FB20
#define ORB2_NOBEAM_SELECT         0x7FB24
#define ORB2_BEAM_SELECT           0x7FB28
#define ORB2_MAN_SELECT            0x7FB2C 
#define ORB1_DAC                   0x7FB3C
#define ORB1_PERIOD_FIFO_RD        0x7FB40
#define ORB1_PERIOD_FIFO_STATUS    0x7FB44
#define ORB1_PERIOD_RD             0x7FB48
#define ORB1_COUNTER               0x7FB4C
#define ORB1_INT_PERIOD_COUNTER    0x7FB50
#define ORB1_INT_PERIOD_SET        0x7FB54
#define ORB1_LENGTH                0x7FB58
#define ORB1_COURSE_DELAY          0x7FB5C
#define ORB1_POLARITY              0x7FB60
#define ORB1_NOBEAM_SELECT         0x7FB64
#define ORB1_BEAM_SELECT           0x7FB68
#define ORB1_MAN_SELECT            0x7FB6C
#define BCMAIN_QPLL_STATUS         0x7FB7C
#define BCMAIN_QPLL_MODE           0x7FB80
#define BCMAIN_NOBEAM_SELECT       0x7FB84
#define BCMAIN_BEAM_SELECT         0x7FB88
#define BCMAIN_MAN_SELECT          0x7FB8C
#define BCREF_QPLL_STATUS          0x7FB98
#define BCREF_DAQ                  0x7FB9C
#define BCREF_QPLL_MODE            0x7FBA0
#define BCREF_NOBEAM_SELECT        0x7FBA4
#define BCREF_BEAM_SELECT          0x7FBA8
#define BCREF_MAN_SELECT           0x7FBAC
#define BC2_QPLL_STATUS            0x7FBB8
#define BC2_DAQ                    0x7FBBC
#define BC2_QPLL_MODE              0x7FBC0
#define BC2_NOBEAM_SELECT          0x7FBC4
#define BC2_BEAM_SELECT            0x7FBC8
#define BC2_MAN_SELECT             0x7FBCC
#define BC1_QPLL_STATUS            0x7FBE8
#define BC1_DAQ                    0x7FBEC
#define BC1_QPLL_MODE              0x7FBF0
#define BC1_NOBEAM_SELECT          0x7FBF4
#define BC1_BEAM_SELECT            0x7FBF8
#define BC1_MAN_SELECT             0x7FBFC


/*********/
/* Types */
/*********/
typedef struct 
{
  volatile u_int manufacturer_id;               /* 0x0000 */
  volatile u_int board_id;                      /* 0x0004 */
  volatile u_int revision_id;                   /* 0x0008 */
  volatile u_int program_id;                    /* 0x000C */
  volatile u_int bset;                          /* 0x0010 */
  volatile u_int bclear;                        /* 0x0014 */
  u_int dummy0[127994];

  //Delay 25 registers
  volatile u_int bc_delay25_bc1;		/* 0x7d000 */
  volatile u_int bc_delay25_bc2;		/* 0x7d004 */
  volatile u_int bc_delay25_bcref;		/* 0x7d008 */
  volatile u_int bc_delay25_bcmain;		/* 0x7d00C */
  u_int dummy0b[1];
  volatile u_int bc_delay25_gcr;		/* 0x7d014 */
  u_int dummy1[2];
  volatile u_int orbin_delay25_orb1;		/* 0x7d020 */
  volatile u_int orbin_delay25_orb2;		/* 0x7d024 */
  u_int dummy1b[3];
  volatile u_int orbin_delay25_gcr;		/* 0x7d034 */
  u_int dummy2[2];
  volatile u_int orbout_delay25_orb1;		/* 0x7d040 */
  volatile u_int orbout_delay25_orb2;		/* 0x7d044 */
  volatile u_int orbout_delay25_orbmain;	/* 0x7d048 */
  u_int dummy2b[2];
  volatile u_int orbout_delay25_gcr;		/* 0x7d054 */
  u_int dummy3[106]; 
  volatile u_int delay25_reg;			/* 0x7d200 */
  u_int dummy4[895];

  //TTCrx
  volatile u_int ttcrx_pointer;		        /* 0x7e000 */
  volatile u_int ttcrx_data;		        /* 0x7e004 */
  u_int dummy27[126];
  volatile u_int ttcrx_reg;			/* 0x7e200 */
  u_int dummy28[1552];

  //Reset space
  volatile u_int orb_counter_reset;		/* 0x7fa44 */
  volatile u_int period_counter_reset;		/* 0x7fa48 */
  volatile u_int orb_int_reset;			/* 0x7fa4c */
  u_int dummy29[5];

  //Comman space
  volatile u_int period_counter_enable;		/* 0x7fa64 */
  volatile u_int orb_counter_enable;		/* 0x7fa68 */
  volatile u_int orb_int_enable;	        /* 0x7fa6c */
  u_int dummy30[2];

  //Working modes
  volatile u_int working_mode;		        /* 0x7fa78 */
  volatile u_int beam_no_beam_def;		/* 0x7fa7c */
  u_int dummy31[7];

  //BST
  volatile u_int bst_machine_mode;		/* 0x7fa9c */
  volatile u_int ttcrx_status;		        /* 0x7faa0 */
  u_int dummy32[7];

  //Main orbit 
  volatile u_int orbmain_period_fifo_rd;	/* 0x7fac0 */
  volatile u_int orbmain_period_fifo_status;	/* 0x7fac4 */
  volatile u_int orbmain_period_rd;		/* 0x7fac8 */
  volatile u_int orbmain_counter;		/* 0x7facc */
  volatile u_int orbmain_int_period_counter;	/* 0x7fad0 */
  volatile u_int orbmain_int_period_set;	/* 0x7fad4 */
  volatile u_int orbmain_length;		/* 0x7fad8 */
  volatile u_int orbmain_course_delay;		/* 0x7fadc */
  volatile u_int orbmain_polarity;		/* 0x7fae0 */
  volatile u_int orbmain_nobeam_select;		/* 0x7fae4 */
  volatile u_int orbmain_beam_select;		/* 0x7fae8 */
  volatile u_int orbmain_man_select;		/* 0x7faec */ 
  u_int dummy33[3];

  //Orbit 2
  volatile u_int orb2_dac;			/* 0x7fafc */
  volatile u_int orb2_period_fifo_rd;		/* 0x7fb00 */
  volatile u_int orb2_period_fifo_status;	/* 0x7fb04 */
  volatile u_int orb2_period_rd;		/* 0x7fb08 */
  volatile u_int orb2_counter;			/* 0x7fb0c */
  volatile u_int orb2_int_period_counter;	/* 0x7fb10 */
  volatile u_int orb2_int_period_set;		/* 0x7fb14 */
  volatile u_int orb2_length;			/* 0x7fb18 */
  volatile u_int orb2_course_delay;		/* 0x7fb1c */
  volatile u_int orb2_polarity;			/* 0x7fb20 */
  volatile u_int orb2_nobeam_select;		/* 0x7fb24 */
  volatile u_int orb2_beam_select;		/* 0x7fb28 */
  volatile u_int orb2_man_select;		/* 0x7fb2c */ 
  u_int dummy34[3];

  //Orbit 1
  volatile u_int orb1_dac;			/* 0x7fb3c */
  volatile u_int orb1_period_fifo_rd;		/* 0x7fb40 */
  volatile u_int orb1_period_fifo_status;	/* 0x7fb44 */
  volatile u_int orb1_period_rd;		/* 0x7fb48 */
  volatile u_int orb1_counter;			/* 0x7fb4c */
  volatile u_int orb1_int_period_counter;	/* 0x7fb50 */
  volatile u_int orb1_int_period_set;		/* 0x7fb54 */
  volatile u_int orb1_length;			/* 0x7fb58 */
  volatile u_int orb1_course_delay;		/* 0x7fb5c */
  volatile u_int orb1_polarity;			/* 0x7fb60 */
  volatile u_int orb1_nobeam_select;		/* 0x7fb64 */
  volatile u_int orb1_beam_select;		/* 0x7fb68 */
  volatile u_int orb1_man_select;		/* 0x7fb6c */
  u_int dummy35[3];

  //BC-main
  volatile u_int bcmain_qpll_status;		/* 0x7fb7c */
  volatile u_int bcmain_qpll_mode;		/* 0x7fb80 */
  volatile u_int bcmain_nobeam_select;		/* 0x7fb84 */
  volatile u_int bcmain_beam_select;		/* 0x7fb88 */
  volatile u_int bcmain_man_select;		/* 0x7fb8c */
  u_int dummy36[2];

  //BC-ref
  volatile u_int bcref_qpll_status;		/* 0x7fb98 */
  volatile u_int bcref_daq;		        /* 0x7fb9c */
  volatile u_int bcref_qpll_mode;		/* 0x7fba0 */
  volatile u_int bcref_nobeam_select;		/* 0x7fba4 */
  volatile u_int bcref_beam_select;		/* 0x7fba8 */
  volatile u_int bcref_man_select;		/* 0x7fbac */
  u_int dummy37[2];
  
  //BC1
  volatile u_int bc2_qpll_status;		/* 0x7fbb8 */
  volatile u_int bc2_daq;		        /* 0x7fbbc */
  volatile u_int bc2_qpll_mode;			/* 0x7fbc0 */
  volatile u_int bc2_nobeam_select;		/* 0x7fbc4 */
  volatile u_int bc2_beam_select;		/* 0x7fbc8 */
  volatile u_int bc2_man_select;		/* 0x7fbcc */
  u_int dummy38[6];

  //BC2
  volatile u_int bc1_qpll_status;		/* 0x7fbe8 */
  volatile u_int bc1_daq;		        /* 0x7fbec */
  volatile u_int bc1_qpll_mode;			/* 0x7fbf0 */
  volatile u_int bc1_nobeam_select;		/* 0x7fbf4 */
  volatile u_int bc1_beam_select;		/* 0x7fbf8 */
  volatile u_int bc1_man_select;		/* 0x7fbfc */
} rf2ttc_regs_t;


/**************/
/* Prototypes */
/**************/
u_int RF2TTC_Open(u_int vmeabase);
u_int RF2TTC_Close(void);
u_int RF2TTC_I2C_Read(u_int offset, u_int *data);
u_int RF2TTC_I2C_Write(u_int offset, u_int data);
u_int RF2TTC_GetHandle(u_int *vmehandle);
u_int RF2TTC_CalVolt(u_int dac, u_int mode);
u_int RF2TTC_CalOrbit(u_int orbit, u_int mode);
u_int RF2TTC_FindCalib(u_int *data, u_int size, u_int mode);
u_int RF2TTC_Dump(void);
u_int RF2TTC_Slot2VME(u_int slot, u_int *addr);

//To be removed
u_int RF2TTC_Open(u_int vmeabase, u_int slot);

#endif
