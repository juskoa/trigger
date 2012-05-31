/****************************************************************/
/*                                              		*/
/*  file: rfrx.h 	                           		*/
/*                                              		*/
/*  Author: Markus Joos, CERN-ECP               		*/
/*                                              		*/
/*  14.Sep.06  MAJO  created                   		  	*/
/*****C 2008 - The software with that certain something**********/

#ifndef _RFRX_H 
#define _RFRX_H

#include "common.h"


/*************/
/* Constants */
/*************/
#define CARD_ID     0x1382
#define BOARD_ID    0x016c
#define FREQ_BASE   (32 * 11 * 80)
#define MAP_SIZE    0x100

/*********/
/* Types */
/*********/
typedef struct 
{
  volatile u_short dummy1;		   /* 0x00 */
  volatile u_short VMEIRQStatID;	   /* 0x02 */
  volatile u_short VMEIRQLevel;		   /* 0x04 */
  volatile u_short Status;		   /* 0x06 */
  volatile u_short IdentCode;		   /* 0x08 */
  volatile u_short dummy2;		   /* 0x0a */
  volatile u_short SIG_DETEC;		   /* 0x0c */
  volatile u_short dummy3;		   /* 0x0e */
  volatile u_short ReceiverModID;	   /* 0x10 */
  volatile u_short ch1_ref;                /* 0x12 */
  volatile u_short ch2_ref;                /* 0x14 */
  volatile u_short ch3_ref;                /* 0x16 */
  volatile u_short ch1_freq_lo;		   /* 0x18 */
  volatile u_short ch1_freq_hi;		   /* 0x1a */
  volatile u_short ch2_freq_lo;		   /* 0x1c */
  volatile u_short ch2_freq_hi;		   /* 0x1e */
  volatile u_short ch3_freq_lo;		   /* 0x20 */
  volatile u_short ch3_freq_hi;		   /* 0x22 */
  volatile u_short card_id;		   /* 0x24 */
  volatile u_short dummy6[10];		   /* 0x26 - 0x38*/
  volatile u_short board_id;		   /* 0x3a */
  volatile u_short dummy7[90];		   /* 0x3c - 0xee*/
  volatile u_short FirmwareVer_lo;         /* 0xf0 */
  volatile u_short FirmwareVer_hi;	   /* 0xf2 */
} rfrx_regs_t;

typedef struct 
{
  u_short ch1_ref;
  u_short ch2_ref;
  u_short ch3_ref;
} rfrx_init_t;


/**************/
/* Prototypes */
/**************/
u_int RFRX_Open(u_int vmeabase, u_int *handle);
u_int RFRX_Close(u_int handle);
u_int RFRX_Init(u_int handle, rfrx_init_t data);
u_int RFRX_Dump(u_int handle);


#endif
