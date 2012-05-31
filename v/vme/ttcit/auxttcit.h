#ifndef _AUXTTCIT_H__
#define _AUXTTCIT_H__

/*
       Header file for the file and string manipulations
 */

/*
       Constants and typedefs
 */

#include <time.h>

#define BLKSIZE 1024
#define BLKOUNT 256
#define CONF_SIZE 349525
#define HEADER_SIZE 65536
#define FILENAME_SIZE 255
#define MAX_CONFIONS 2

struct confHeader {
  char filename[FILENAME_SIZE];   /* FPGA code filename */
  unsigned char header[HEADER_SIZE]; /* Descriptive header for FPGA file */
  int filled;

  int nbytes; /* Nr. of bytes of code */
  unsigned char code[CONF_SIZE];  /* FPGA code, Max 256 MB = 2Mb */
};

#define MAX_FM_BUFFER 1048576  /* 1M byte  = 8 Mbit capacity of Flash Mem */

struct bufferFM {
  unsigned char buf[MAX_FM_BUFFER];
  int nbytes;
  int count;
};

struct confHeader configData[3];

/*
       Functions
*/

int getConfigFile();
int storeConfData(struct bufferFM *buffer);
int readCodeFPGA(char *filename, struct confHeader *h);
void printCodeFPGAdoc(void *s);
clock_t timeInfoPerSec();

#endif /* _AUXTTCIT_H__ */
