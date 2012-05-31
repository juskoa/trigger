#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include "auxttcit.h"

int getConfigFile(){
  int irc = 0;

  FILE *conf;
  int stat;
  char *ptrLine = NULL;
  char line[FILENAME_SIZE];
  size_t lineLength;
  int i, j;
  int fnamSize;
  int isIt;
  int iConf = -1;
  int count;                 /* Points to 1-st free position in cH->header */
  struct confHeader *cH = NULL;
  char filename[FILENAME_SIZE];
  char *f;
  char *ct;
  char *s;
  char curdir[1000];

  for(i=0; i<MAX_CONFIONS; i++){
    configData[i].filled = 0;
    for(j=0; j<FILENAME_SIZE; j++){
      configData[i].filename[j] = '\0';
    }
    for(j=0; j<HEADER_SIZE; j++){
      configData[i].header[j] = 0xff;
    }
    configData[i].filled = 0;
    configData[i].nbytes = 0;
    for(j=0; j<CONF_SIZE; j++){
      configData[i].code[j] = 0xff;  /* Like an empty FM */
    }
  }

  do{
    /*
      Open the ttcit.con in the current directory
     */

    conf = fopen("ttcit.con","r");
    if(conf == NULL){
      printf("TTCIT logic configuration file cannot be opened\n\n");
      printf("HELP: If you are using the PyTk GUI, you cannot access\n");
      printf("      the files in your current directory\n");
      printf("      The ttcit.con file must be located in the \n");
      printf("      $VMECFDIR directory with paths to the FPGA code files\n");
      printf("      set relative to the $VMECFDIR\n\n");
      s = getcwd(&curdir[0], 1000);
      printf("PWD = %s\n",curdir);
      irc = 1;
      break;
    }

    /*
      Read the file, parse the lines
     */

    while((ptrLine = fgets(&line[0], FILENAME_SIZE, conf)) != NULL){
      /* 
	 Skip the comment lines
       */
      if(line[0] == '#'){
	continue;
      }
      /* 
	 Process the filename
       */
      if(line[0] == '$'){
	lineLength = strlen(&line[0]);
	fnamSize = 0;
	for(i=1; (size_t)i<lineLength; i++){
	  isIt = isspace(line[i]);
	  if(isIt == 0){
	    fnamSize++;
	  }else{
	    line[i] = '\0';
	    break;
	  }
	}
	fnamSize = strlen(line);

	if(fnamSize == 0){
	  printf("Zero length filename found in line \n%s\n",line);
	  irc = 2;
	  break;
	}
	/*
	  Valid filename found, i.e. we have a new FPGA configuration
	 */
	iConf++;
	if(iConf >= MAX_CONFIONS){
	  printf("Founf more FPGA code files than can be stored into FM\n");
	  printf("File \n%s\n will not be stored into the Flash Memory\n\n",
		 line);
	  continue;
	}
	cH = &(configData[iConf]);
	count = 0;
	/*
	  Store filename in confHeader structure
	 */
	cH->header[count++] = (unsigned char)fnamSize;
	for(i=0; i<fnamSize; i++){
	  cH->filename[i] = line[i+1];
	  cH->header[count++] = (unsigned char)line[i+1];
	}
	cH->filename[fnamSize] = '\0';
	cH->filled = 1;
      }

      /*
	Process the documentation line, add to the current 
	configuration, unless the iCount has overflew
       */
      if(line[0] == '&'){
	if(iConf >= MAX_CONFIONS){
	  printf("Documentation line\n%s\n is ignored",line);
	  continue;
	}
	/*
	  Store the line
	 */
	lineLength = strlen(&line[0]);
	fnamSize = 0;
	for(i=0; (size_t)i<lineLength; i++){
	  isIt = isspace(line[i]);
	  if((isIt == 0) || (line[i] == ' ')){
	    fnamSize++;
	  }else{
	    line[i] = '\0';
	    break;
	  }
	}
	fnamSize = strlen(line);

	if(fnamSize > 0){
	  cH->header[count++] = (unsigned char)fnamSize - 1;
	  for(i=0; i < fnamSize; i++){
	    cH->header[count++] = (unsigned char)line[i+1];
	  }
	  cH->header[count] = 0;
	}
      }
    }
  }while(0);

  do{

    if(irc != 0){
      break;
    }

    /*
      Close the configuration file
     */
    stat = fclose(conf);
    if(stat == EOF){
      printf("TTCIT logic configuration file incorrectly closed\n");
      irc = 99;
      break;
    }

    /*
      Read all code files 
     */

    for(i=0; i<MAX_CONFIONS; i++){
      if(configData[i].filled == 1){
	s = &filename[0];
	ct = &(configData[i].filename[0]);
	f = strncpy(s,ct,FILENAME_SIZE);
	printCodeFPGAdoc(&(configData[i].header));
	cH = &configData[i];
	stat = readCodeFPGA(&filename[0], cH);
	if(stat != 0){
	  printf("The FPGA code from file %s cannot be read\n",filename);
	  irc = 100 + i;
	  break;
	}
      }
    }
    if(irc != 0){
      printf("Error reading FPGA code(s)\n");
      break;
    }

    printf("TTCIT logic configuration file processed successfully\n");

  }while(0);

  return irc;
}

int storeConfData(struct bufferFM *buffer){
  int irc = 0;

  int i, j;
  int start;
  struct confHeader *H;
  int count;
  int nlen;

  char *empty = "Empty code section\0";

  do{

    buffer->nbytes = 0;
    buffer->count = 0;
    for(i=0; i<MAX_FM_BUFFER; i++){
      buffer->buf[i] = 0xff;        /* Be compatible with empty FM */
    }

    /*
      First 3 code sections are simple, just copy the code sections
     */

    for(i=0; i<MAX_CONFIONS; i++){
      start = i * CONF_SIZE;
      H = &configData[i];
      if(H->filled != 1){
	printf("WARNING !!! %d -th code section is empty !!!!\n",i);
	continue;
      }
      for(j=0; j<H->nbytes; j++){
	buffer->buf[start+j] = H->code[j];
      }
      buffer->nbytes += CONF_SIZE;
    }

    /*
      Last FM block is split into 4 sections of 64 kB holding the 
      documentation of the code sections

      4-th documentation section is left empty, maybe something can be stored
      here in the future
     */

    for(i=0; i<MAX_CONFIONS; i++){
      start = (MAX_CONFIONS * CONF_SIZE) + (i * HEADER_SIZE);
      H = &configData[i];
      if(H->filled != 1){
	nlen = strlen(empty);
	count = 0;
	buffer->buf[start+count++] = nlen;
	for(j=0; j<nlen; j++){
	  buffer->buf[start+count++] = empty[j];
	}
	buffer->buf[start+count] = '\0';
      }else{
	count = 0;
	for(j=0; j<HEADER_SIZE; j++){
	  buffer->buf[start+count++] = H->header[j];
	}
      }
    }
    buffer->nbytes += CONF_SIZE;
    /*
      This must look like full, even if only one code section is filled
     */
    /* buffer->nbytes = (MAX_CONFIONS + 1) * CONF_SIZE; */
    buffer->nbytes = MAX_FM_BUFFER;

  }while(0);

  return irc;
}

int readCodeFPGA(char *filename, struct confHeader *h){
  int irc = 0;

  FILE *in = NULL;
  int c;

  h->nbytes = 0;

  do{

    /*
      Open the FPGA code file
     */

    in = fopen(filename, "rb");
    if(in == NULL){
      printf("FPGA code file %s cannot be opened\n",filename);
      irc = 1;
      break;
    }

    /*
      Read the data from the file to the Conf Header
     */

    while((c = fgetc(in)) != EOF){
      h->code[h->nbytes++] = (unsigned char)c;
      if(h->nbytes > CONF_SIZE){
	printf("FPGA code file too large : %d bytes found\n",h->nbytes);
	irc = 2;
	break;
      } 
    }
    if(irc != 0){
      break;
    }
    h->nbytes--;
    if(h->nbytes <= 0){
      printf("No FPGA code found in the file %s\n",filename);
      irc = 3;
      break;
    }

    /*
      Close the FPGA code file
     */
    c = fclose(in);
    if(c == EOF){
      printf("Error closing the file %s\n",filename);
      irc = 4;
      break;
    }

    printf("Filename %s successfully read\n",filename);

  }while(0);

  return irc;
}

void printCodeFPGAdoc(void *s){
  /*
    Print the documentation for the given FPGA code section

    The documentation has the following form:

    Line 1:     name of the file containing the code
    Line 2...n  Any text that has been found in the ttcit.con file
   */

  int irc = 0;
  int nlen;
  unsigned char c;
  char *b;
  int i;

  unsigned char line[FILENAME_SIZE];

  b = (char *)s;

  printf("\n ************************************\n");
  printf(" ** FPGA CODE DOCUMENTATION RECORD **\n");
  printf(" ************************************\n\n");

  do{

    while((c = *b++) != 0xff){

      nlen = c;
      if(nlen == 0){   /* Avoid endless loops */
	continue;
      }
      if(nlen > FILENAME_SIZE){
	printf("Line too long, probably corrupted documentation data block\n");
	irc = 1;
	break;
      }

      for(i=0; i<nlen; i++){
	line[i] = *b++;
      }
      line[nlen] = '\0';
      printf("%s\n",line);

    }
    if(irc != 0){
      break;
    }

  }while(0);
  printf("\n ************************************\n");


  if(irc != 0){
    printf("\n WARNING!!! Corrupted documentation data block\n");
  }
}

/*
clock_t timeInfoPerSec(){
  long tpc = 0;
  tpc = sysconf(_SC_CLK_TCK);
  return (clock_t)tpc;
}
*/

