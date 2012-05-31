
/*
  Manipulation with Flash Memory and FPGA
  (abridged version of my original vme2fpga.c )
 */

#include "vmewrap.h"
#include "ttcit.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>


/*
  At the moment the status functions are here more or less as a 
  hook for the future possibilities 
  For time being there is no CONFIG_STATUS nor FLASH_STATUS registers 
  available in the VME controller code
 */

static struct bufferFM wrtBuffer; /* data to be written into FM */
static struct bufferFM readBuffer; /* data read from FM */

int GetStatusFPGA(){

  int fpgaStat;
  char statReady[] = "READY\0";
  char statBusy[] = "BUSY\0";
  char statUnknown[] = "UNKNOWN\0";
  char *st;

  fpgaStat = GetStatFPGA();
  switch(fpgaStat){
  case READYFP:
    st = &statReady[0];
    break;
  case BUSYFP:
    st = &statBusy[0];
    break;
  default:
    st = &statUnknown[0];
    break;
  }
  printf("FPGA status is %i = %s \n",fpgaStat,st);

  return fpgaStat;
}

int GetStatFPGA(){
  int stat;
  w32 status;
  w32 bit0 = 1;

#ifdef FPGA_NOSTAT
  status = bit0;
#else
  status = VMER32(CONFIG_STATUS);
#endif
  stat = ( (status & bit0 ) == bit0 );
  return stat;
}

int GetStatusFM(){
  int flMemStat;
  char statReady[] = "READY\0";
  char statBusy[] = "BUSY\0";
  char statUnknown[] = "Unknown\0";
  char *st;

  flMemStat = GetStatFM();
  switch(flMemStat){
  case READYFM:
    st = &statReady[0];
    break;
  case BUSYFM:
    st = &statBusy[0];
    break;
  default:
    st = &statUnknown[0];
    break;
  }
  printf("Flash Memory status   %i = %s \n",flMemStat,st);

  return flMemStat; 
}

int GetStatFM(){
  int stat;
  w32 status;
  w32 ryby_mask = FM_RYBY_MASK;

#ifdef FLASH_NOSTAT
  status = ryby_mask;
#else
  status = VMER32(FLASHACCESS_NOINCR);
#endif
  stat = ( (status & ryby_mask ) == ryby_mask );
  return stat;
}

/*xxFGROUP REMOVE_ME
  Resets Flah Memory (this is not to be used after the fm_fpga code
  is debuged and must disappear from the GUI
 */
int ResetFM(){
  int irc = 0;
  irc = resetFM();
  switch(irc){
  case 0:
    printf("Reset FM: SUCCESSFULL\n");
    break;
  case 1:
    printf("Reset FM: TIMEOUT\n");
    break;
  default:
    printf("Reset FM: Unexpected IRC\n");
    break;
  }
  return irc;
}

int resetFM(){
  int irc = 0;
  int st;

  st = writeFM(FLASHACCESS_NOINCR,0xF0);
  if(st != 0){
    irc = 1;
  }else{
    irc = 0;
  }

  return irc;
}

/*xxFGROUP REMOVE_ME
Erase contents of the Flash Memory (must be done before writing new contents)

This function must disappear from the GUI
*/
int EraseFM(){
  int irc = 0;
  int st = 0;

  st = eraseFM();
  switch(st){
  case 0:
    printf("Chip Erase SUCCESSFULL\n");
    break;
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
    printf("Chip Erase ERROR: Timeout after command word %i\n",st);
    break;
  case 7:
    printf("Chip Erase ERROR: Erase operation canna start, FM busy\n");
    break;
  case 11:
    printf("Chip Erase ERROR: Timeout after the whole command sequence\n");
    break;
  default:
    printf("Chip Erase ERROR: Unexpected return code\n");
    break;
  }
  irc = st;

  return irc;
}

int eraseFM(){
  int irc = 0;

  int command[6] = { 0xaa, 0x55, 0x80, 0xaa, 0x55, 0x10 };
  int i;
  int st = 0;

  int x_adr = 0x555;
  int x_incr = 0;
  int dummyd;

  ClearAddressFM();   /* Set address coonter to 0 */  
  while((x_incr++) != x_adr){ 
    dummyd = VMER32(FLASHACCESS_INCR);
  }

  ResetFM();
  if( GetStatFM() == READYFM ){
    for(i=0; i<5; i++){
      VMEW32(FLASHACCESS_NOINCR, command[i]);
      if(st != 0){
	irc = i+1;
	break;
      }
      /*      usleep(usl_cmd); */
    }
    /*
      The last command word starts erasing the memory. The process may take
      a bit longer
     */
    if(irc == 0){
      VMEW32(FLASHACCESS_NOINCR,command[5]);
#ifdef FLASH_NOSTAT
      printf("ERASING - Sleeping\n");
      sleep(ERASE_SLEEP);
#else
      printf("ERASING - Sleeping\n");
      sleep(ERASE_SLEEP);
#endif
      i = 0;
      while( (i < MAX_E_ATTEMPTS) && ((st = GetStatFM()) == BUSYFM )){
	usleep(ERASE_USLEEP);
	printf("%i sec: Erasing FM in progress, status = %i\n",i+1,st);
	i++;
      }
      printf("%i sec: Erasing FM in progress, status = %i\n",i+1,st);
      if( st == BUSYFM ){
	printf("Chip Erase operation not successfull\n");
	irc = 11;
      }
    }
  }else{ 
    /*
      Flash memory is busy, -> ERROR 7
    */
    irc = 7;
  }

  return irc;
}

int writeFM(w32 address, int dato){
  /*
      Writes to FM via VME controller

      address = FLASHACCESS_INCR          VME controller register
                FLASHACCESS_NOINCR
                FLASHADD_CLEAR

                (other adresses have no effect on FM)
  */
  int irc = 0;
  int attempts = 0;

  /*      Write data into a given address
   */
  VMEW32(address, dato);
  /*
         For a while the Flash Memory will be BUSY, but after a 
         reasonable time it must go back to READY
   */
  while( (GetStatFM() == BUSYFM) && (attempts < MAX_W_ATTEMPTS ) ){
    usleep(1);
    attempts++;
  }
  /*
         If the loop terminated without seeing the READY status
         sound an allarm
   */
  if( attempts >= MAX_W_ATTEMPTS ){
    printf("Unable to write (a:%x  data:%x) in Flash Memory \n",address,dato);
    printf("       after %i attempts\n",attempts);
    irc = 1;
  }else{
    irc = 0;
  }

  return irc;
}

/*xxFGROUP REMOVE_ME
  Write nbytes from wrtBuffer into the FM
 */
int blockWriteFM(int nbytes){
  /*
    Writes nbytes bytes from wrtBuffer to Flash memory 
    to FLASHACCESS_INCR

    Erasing the Flash memory chip and positioning the Address counter
    to the desired addres by seekFM() must be done by caller

    If the number of stored bytes in the wrtBuffer is less then nbytes,
    only the actual number of bytes is written into the FM
   */
  int irc = 0;
  int nb;
  int st;
  int dato;
  int i;

  int unlock_bypass[3] = { 0xaa, 0x55, 0x20 };
  int command_prog = 0xa0;
  int bypass_reset[2] = { 0x90, 0x00 };

int nmod = 100;

  if(nbytes > MAX_FM_BUFFER){
    nb = MAX_FM_BUFFER;
  }else{
    nb = nbytes;
  }
  if(nb > wrtBuffer.nbytes){
    nb = wrtBuffer.nbytes;
  }
  if(nb <= 0){
    irc = 1;
    goto lastone;
  }

  wrtBuffer.count = 0;

  /*
    3 words: UNLOCK BYPASS: 0xAA 0x55 0x20
   */
  for(i = 0; i < 3; i++){
    st = writeFM(FLASHACCESS_NOINCR, unlock_bypass[i]);
    if(st != 0){
      irc = 11;
      break;
    }
  }
  if(irc != 0){
    goto lastone;
  }

  for(i=0; i<nb; i++){

    /*
      Send the command, 1-st byte must go to the Address 0, 
      so no address increment is needed
      the 2-nd word must be written with increment
     */
    if(i == 0){
      st = writeFM(FLASHACCESS_NOINCR, command_prog);
    }else{
      st = writeFM(FLASHACCESS_INCR, command_prog);
    }
    if(st != 0){
      irc = -10*i - 1;
      break;
    }
    /*
      Send the dato
     */
    dato = wrtBuffer.buf[i];
    st = writeFM(FLASHACCESS_NOINCR,dato);
    if(st != 0){
      irc = -i-1;
      break;
    }
    wrtBuffer.count++;
if(i < 10){
nmod = 2;
}else if(i < 1000){
nmod = 100;
}else if(i < 10000){
nmod = 100;
}else{
nmod = 100;
}
if((i % nmod) == 0){
printf("%d -th dato written\n",i);
}
  }
  /*
    We must send UNLOCK BYPASS RESET 2 bytes
   */
  for(i = 0; i < 2; i++){
    st = writeFM(FLASHACCESS_NOINCR,bypass_reset[i]);
    if(st != 0){
      irc = -22;
      break;
    }
  }

 lastone:
  return irc;
}

/*xxFGROUP REMOVE_ME 
Clears Flash Memory address counter (set it to 0)

After debug this function must disappear from GUI
*/
void ClearAddressFM(){
  int irc;

  printf("ClearAddressFM: Resetting the FM Address counter to 0\n");
  irc = writeFM(FLASHADD_CLEAR,0x0);
  if(irc != 0){
    printf("ClearAddressFM: Operation finished with IRC = %i\n",irc);
  }
}

int readFM(w32 address, int* dato){
  /*
    Reads from the VME register returning the value from the Flash memory

    address = FLASHACCESS_NOINCR
              FLASHACCESS_INCR
   */
  int irc = 0;
  int attempts = 0;
  int dataRead = 0;

  /*
    Read from VME register
   */

  dataRead = VMER32(address);

  while( (GetStatFM() == BUSYFM) && (attempts < MAX_R_ATTEMPTS) ){
    attempts++;
  }
  if(attempts >= MAX_R_ATTEMPTS){
    printf("Unable to read from addr:%x after %i attempts\n",address,attempts);
    irc = 1;
  }else{
    irc = 0;
    *dato = dataRead & 0xff;  /* Only 8 bits are relevant */
  }

  return irc;
}

/*xxFGROUP REMOVE_ME
 */
int blockReadFM(int nbytes){ 
  /*
    Reads a block of data from FLAHACCESS_INCR and stores them to the
    readBuffer

    The address from where the data are to be read must be set by 
    seekFM(...) before calling this
  */
  int irc = 0;
  int i;
  int dato;
  int nb;
  int st;
  int mask = 0xff; /* Only 8 bit are relevant */

  if(nbytes > MAX_FM_BUFFER){
    nb = MAX_FM_BUFFER;
  }else{
    nb = nbytes;
  }
  readBuffer.count = 0;

  readBuffer.nbytes = 0; /* Clear the readBuffer */
  for(i = 0; i < MAX_FM_BUFFER; i++){ readBuffer.buf[i] = 0; };

  for(i=0; i<nb; i++){
    if(i > 0){
      st = readFM(FLASHACCESS_INCR, &dato);  /* Read the dato */
    }else{
      st = readFM(FLASHACCESS_NOINCR, &dato);
    }
    if(st == 0){
      readBuffer.buf[readBuffer.nbytes++] = ( dato & mask );
      readBuffer.count++;
    }else{
      irc = -i-1;
      break;
    }
  }

  return irc;
}

/*xxFGROUP REMOVE_ME
  Test of file reading operation
 */
void TestReadFile(){
  int irc = 0;
  char *filename = "ttcit_fpga_code";

  do{
    irc = readCodeFile(filename, &wrtBuffer);
    if(irc != 0){
      printf("TestReadFile: File reading sucks! Or the input is corrupted\n");
      break;
    }
    /*
      Print 20 lines a 16 3 digits numbers separated by comma.
    */
    printAnyBuffer(&wrtBuffer, 20);
  }while(0);

}

/*xxFGROUP REMOVE_ME
 */
void DumpReadBuffer(int nbytes){
  int nl;
  nl = (nbytes / 16 ) + 1;
  printAnyBuffer(&readBuffer, nl);
}

void printAnyBuffer(struct bufferFM *b, int nLines){

  int line, l;
  char fline[80];
  int i, j;
  int c;

  for(line = 0; line < nLines; line++){
    l = line * 16;                    /* starting position in buffer */
    for(i = 0; i < 16; i++){
      c = (int)b->buf[l + i];
      j = i * 4;                      /* Starting position in a line */
      sprintf(&fline[j],"%03d,",c);
    }
    j += 5;
    fline[j] = '\0';
    printf("Line %03d : %s\n",line, &fline[0]);
  }

}

/*xxFGROUP REMOVE_ME
  Dumps the contents of the Flash Memory

  Should not appear in the GUI
 */
void DumpFM(int nbytes){
  int r = 0;
  int nlines = 0;

  do{
    r = blockReadFM(nbytes);
    if(r != 0){
      printf("blockReadFM : ERROR : Cannot read FM, IRC = %d\n",r);
      break;
    }
    nlines = ( nbytes / 16 ) + 1;
    printAnyBuffer( &readBuffer, nlines);

  }while(0);
}

int readCodeFile(char *filename, struct bufferFM *b){
  int irc = 0;

  FILE *in = NULL;
  int i;
  int c;
  int c_byte;
  char dec_digit[4];
  int j = -1;
  int BLANK = 0x20;
  int r;

  do{
    /*
      Open file with the FPGA code
     */
    in = fopen(filename,"rb");
    if(in == NULL){
      printf("FPGA code file %s cannot be opened\n",filename);
      irc = 1;
      break;
    }

    /*
      Clear the data buffer
     */
    for(i=0; i < MAX_FM_BUFFER; i++){
      b->buf[i] = 0;
    }
    wrtBuffer.nbytes = 0;
    wrtBuffer.count = 0;

    /*
      Read the data into the buffer b
     */
    while((c = fgetc(in)) != EOF){

#ifdef HAVE_ASCII_TTF_FILE
      if((c == 0x0d) | (c == 0x0a) | (c == ',' )){  /* New item, EoL */
	j = -1;
	continue;
      }else{
	if(isdigit(c) || (c == BLANK)){
	  dec_digit[++j] = (char)c;
	}else{
	  printf("ERROR: TTF format error, offending character %X\n",c);
	  irc = -70;
	  goto lastone;
	}
	if(j == 2){
	  dec_digit[3] = '\0';
	  r = sscanf(&dec_digit[0], "%d", &c_byte);
	  if(r == 0){
	    printf("ERROR: Number conversion error, offending nr = %s\n",
		   &dec_digit[0]);
	    irc = -77;
	    goto lastone;
	  }
	}else{
	  continue;
	}
      }
#else
      c_byte = c;
#endif

      b->buf[b->nbytes++] = (unsigned char)c_byte;
      if(b->nbytes > MAX_FM_BUFFER){
	printf("FPGA code file too large, more than %d bytes\n",b->nbytes);
	irc = 2;
	break;
      }
    }

    if(irc != 0){
      break;
    }
    if(b->nbytes <= 0){
      printf("No FPGA code found in the file, NBYTES = %d\n",b->nbytes);
      irc = 3;
      break;
    }

    /*
      Close the FPGA code file
    */
    c = fclose(in);
    if(c == EOF){
      printf("Error closing the file %s \n",filename);
      irc = 4;
      break;
    }

    printf("%d bytes of FPGA code read from %s\n",b->nbytes,filename);

  }while(0);

 lastone:
  return irc;
}

/*FGROUP Configuration
  Reads the file <ttcit_fpga_code> - link to the actual file - , 
  write it into the Flash Memory, Read it back and compare with the 
  read original. If all bytes agree return 0 otherwise <> 0.
 */
int WriteCodeFM(){
  int irc = 0;

  char *defaultFilename = "ttcit_fpga_code";
  int r;
  int i;
  int n_nomatch = 0;

  do{
    /*
      Read the file containg the new FPGA code
     */
    r = readCodeFile(defaultFilename, &wrtBuffer);
    if(r != 0){
      printf("Error reading FPGA code file, IRC = %d\n",r);
      irc = -1;
      break;
    }

    /*
      Erase and reset the Flash Memory and clear the address counter
     */
    r = eraseFM();
    if(r != 0){
      printf("ERROR while erasing the Flash Memory, IRC = %d\n",r);
      irc = -2;
      break;
    }
    r = resetFM();
    if(r != 0){
      printf("ERROR resetting the Flash Memory, IRC = %d\n",r);
      irc = -3;
      break;
    }
    ClearAddressFM();

    /*
      Store data from wrtBuffer into the Flash Memory
     */
printf("Start block write\n");
    r = blockWriteFM(MAX_FM_BUFFER);
printf("Block write finished\n");
    if(r != 0){
      printf("ERROR writing data into the Flash Memory, IRC = %d\n",r);
      irc = -4;
      break;
    }

    /*
      Reset the FM address counter and read back all data you have 
      just written (read only the bytes you stored 
     */
    ClearAddressFM();
    r = blockReadFM(wrtBuffer.nbytes);
    if(r != 0){
      printf("ERROR reading back the Flash Memory, IRC = %d\n",r);
      irc = -5;
      break;
    }
    ClearAddressFM();

    /*
      Compare read and write buffers, Do they match?
     */
    if(wrtBuffer.nbytes != readBuffer.nbytes){
      printf("ERROR: Nr %d bytes written, %d bytes read - DO NOT MATCH\n",
	     wrtBuffer.nbytes, readBuffer.nbytes);
      irc = -6;
      break;
    }
    for(i = 0; i < wrtBuffer.nbytes; i++){
      if(wrtBuffer.buf[i] != readBuffer.buf[i]){
	printf("%d-th bytes do not match: W: %X R: %X\n",i,
	       wrtBuffer.buf[i], readBuffer.buf[i]);
	irc = -7;
	if(++n_nomatch > 10){
	  break;
	}
      }
    }

  }while(0);

  if(irc != 0){
    printf("ERROR storing the FPGA code into the Flash Memory, IRC = %d\n",
	   irc);
  }

  return irc;
}

/*FGROUP Configuration
  Load FPGA configuration from the Flash Memory. Returns 0 if nothing wrong
  happened
 */
int LoadFPGA(){
  int irc = 0;
  int st;
  int i;
#ifndef FPGA_NOSTAT
  int tries = 4;
  int TimeInterval = 10000;
  int Ntimes = 10;
#endif

  do{
    /*
      Reset the Flash Memory for smooth operation
    */
    st = ResetFM();
    if(st != 0){
      printf("FM memory problem, loading FPGA code terminated\n");
      irc = 1;
      break;
    }

    /*
      Clear address counter (don't know whether Loading process needs this)
    */
    ClearAddressFM();

    /*
      Initiate loading process, watch the progress (if possible, or sleep
      for a while)
    */
    st = GetStatFPGA();
    if(st != READYFP){
      printf("FPGA not ready, loadinf FPGA code cannot proceed\n");
      irc = 2;
      break;
    }

    VMEW32(CONFIG_START,0x0);
    printf("FPGA configuration started\n");
    i = 0;
#ifdef FPGA_NOSTAT
    sleep(CONFIG_SLEEP);
#else
    do{
      monitorStatusFPGA(Ntimes, TimeInterval);
      GetStatusFPGA();
      i++;
      st = GetStatFPGA();
    }while((st == BUSYFP) && (i<tries));
    st = GetStatFPGA();
    if(st != READYFP){
      i = Ntimes * TimeInterval;
      printf("FPGA loading unsuccessfull, TIMEOUT after %d miliseconds \n",i);
      irc = 7;
      break;
    }
#endif    

    printf("FPGA configuration loaded successfully\n");

  }while(0);

  return irc;
}

int monitorStatusFPGA(int Ntimes, int Tinterval){

  int irc = 0;

  int i,j;
  int step;
  short bit[8] = { 0, 0, 0, 0, 0, 0, 0, 0};
#ifdef FPGA_NOSTAT
#define CONFIG_STATUS 0xff
#endif

  for(j=0; j<Ntimes; j++){
    step = j % 20;
    if(step == 0 ){
      /*
              123456789012345678901234567890123456789012345678901234567890 */
      printf("TIME   RY/BY   nSTATUS  CONF_DONE INIT_DONE  nCONFIG   DCLK \n");
    }
#ifndef FPGA_NOSTAT
    MonitorStatus(CONFIG_STATUS, &bit[0]);
#endif
    printf("%4i",j);
    for(i=0; i<6; i++){
      printf("%5i     ",bit[i]);
    }
    printf("\n");
    usleep(Tinterval);
  }

  return irc;
}

int MonitorStatus(w32 address, short *bit){
  /*
          Reads a status word at _ address _ and decompses it into
          8 bits in array bit[8]

          irc = 0      if O.K.
                else   if some error happens
  */
  int irc = 0;
  w32 statusWord;
  w32 tib;
  int j;

  statusWord = VMER32(address);    /* Read the status word at address */
  /*
           Decomposing
   */
  tib = 1;
  for(j=0; j<8; j++){
    bit[j] = ( (statusWord & tib ) == tib );
    tib <<= 1;
  }

  return irc;
}

/*FGROUP Debug
 */
void dummyDebugStop(){
  printf("Dummy debug stop - just set a breakpoint here\n");
}


/* ENDOFCF */
