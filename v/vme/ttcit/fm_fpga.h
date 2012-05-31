#ifndef _FM_FPGA_H__
#define _FM_FPGA_H__

#define FPGA_NOSTAT     /* No FPGA status check */
#undef FLASH_NOSTAT    /* No Flash Memory status check */

#define MAX_FM_BUFFER  1048576   /* 1MB = 8 Mbit Flash memory */

#define MAX_E_ATTEMPTS 50
#define MAX_W_ATTEMPTS 50
#define MAX_R_ATTEMPTS 50

#ifdef FLASH_NOSTAT
#define ERASE_SLEEP    2000000    /* Sleep for 2 sec */
#else 
#define ERASE_SLEEP    30     /* Sleep for > 11 sec */ 
#endif
#define CONFIG_SLEEP   3 /*  30 sec after FPGA config */
#define ERASE_USLEEP   999999 /* 1 sec */

#define READYFP 1    /* FPGA status READY/BUSY = CONFIG_STATUS  */
#define BUSYFP  0
#define READYFM 1    /* Flash Memory READY/BUSY = FLASH_STATUS */
#define BUSYFM  0

#define FM_DATA_MASK 0xff   /* bits 0...7 are data */
#define FM_RYBY_MASK 0x100  /* bit 8 is Ready/Busy flag for FM */


/*
  If you are going to read TTF files which are just formatted dump of
  3 decimal digits 8bit bytes separated by comma, 16 data items in a line,
  ended by 0x0D 0x0A (CR LF), last line is not complete, ended by EOF.
 */

#define HAVE_ASCII_TTF_FILE     /* If this is the used code format */

/*
  Structures and data types
 */

struct bufferFM {
  unsigned char buf[MAX_FM_BUFFER];
  int nbytes;
  int count;
};

/*
  Variables
 */


/*  Functions in fm_fpga */

int readCodeFile(char *filename, struct bufferFM *b);
int monitorStatusFPGA(int Ntimes, int Tinterval);
int monitorStatus(w32 address, short *bit);
void TestReadFile();
void printAnyBuffer(struct bufferFM *b, int nLines);
void DumpFM(int nbytes);
void DumpReadBuffer(int nbytes);
void dummyDebugStop();

#endif /* _FM_FPGA_H__ */
