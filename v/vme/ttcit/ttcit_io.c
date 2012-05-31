/*
  TTCIT Input/Output routines
 */
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

#include "ttcit_io.h"

static struct ttcit_io_counters IOcounters;

int ttcit_io_open(const char *file, int mode){
  int irc = 0;
  int fd;  

  unsigned long magic;
  size_t howmany;
  ssize_t bytes_read;

  do{

    printf("ttcit_io_open: Opening file %s\n",file);

    IOcounters.opened = FALSE;
    IOcounters.mode = 0;
    IOcounters.magic = 0;
    IOcounters.bytes = 0;
    IOcounters.events = 0;
    IOcounters.headers = 0;
    IOcounters.footers = 0;
    IOcounters.SSM_stored = FALSE;
    IOcounters.bc_stored = FALSE;

    switch(mode){
    case TTCIT_IO_READ:
      fd = open(file, O_RDONLY);
      if(fd == -1){
	printf("ttcit_io_open: ERROR opening file %s for READ\n",file);
      }else{
	/*
	  1-st unsigned long is a MAGIC number, which can be used to determine
	  whether some byte swapped 
	*/
	howmany = sizeof(unsigned long);
	bytes_read = read(fd, &magic, howmany);
	if(bytes_read == -1){
	  printf("ttcit_io_iopen: ERROR reading MAGIC from file %s\n",file);
	  irc = -1;
	}else{
	  IOcounters.magic = magic;
	  printf("File %s opened for READING, MAGIC = %lX\n",file,magic);
	}
      }
      break;
    case TTCIT_IO_WRITE:
      fd = open(file, O_WRONLY | O_CREAT, S_IRWXU );
      if(fd == -1){
	printf("ttcit_io_open: ERROR opening file %s for WRITE\n",file);
      }
      break;
    default:
      printf("ttcit_io_open: Wrong I/O mode %X \n",mode);
      fd = -1;
      break;
    }

    irc = fd;
    IOcounters.opened = TRUE;
    IOcounters.mode = mode;

  }while(0);

  return irc;
}

int ttcit_io_close(int fd){
  int irc = 0;

  do{
    printf("ttcit_io_close: Closing IO file\n");
    irc = close(fd);
    IOcounters.opened = FALSE;
  }while(0);

  return irc;
}

int ttcit_io_new_event(struct ttcit_io_event *event){
  int irc = 0;

  do{
    if(event == NULL){
      irc = -1;
      TTCIT_IO_NEWEV_ERR("event == NULL");
      break;
    }

    event->header.size = sizeof(struct ttcit_io_event_header);
    event->header.type = TTCIT_IO_EVENT;
    event->header.event = (++IOcounters.events);
    event->header.ssm = 0;
    event->header.ssm_items = 0;
    event->header.bc = 0;
    event->header.bc_items = 0;
    event->header.ssm_mode = 0;

    IOcounters.SSM_stored = FALSE;
    IOcounters.bc_stored = FALSE;

  }while(0);

  return irc;
}

int ttcit_io_store_event(int fd, struct ttcit_io_event *event){
  int irc = 0;

  unsigned long size;
  unsigned long ssm_size;
  unsigned long bc_size;
  unsigned long header_size;
  unsigned long total_size;

  int nbytes;

  do{

    if(event == NULL){
      irc = -1;
      TTCIT_IO_STORE_ERR("event == NULL");
      break;
    }

    /*
      Check consistency of the EVENT
     */
    header_size = sizeof(struct ttcit_io_event_header);
    ssm_size = event->header.ssm_items * sizeof(struct ttcit_io_ssm_scope);
    bc_size = event->header.bc_items * sizeof(unsigned long);
    size = event->header.size;
    total_size = header_size + ssm_size + bc_size;
    if(total_size != size){
      irc = -2;
      TTCIT_IO_STORE_ERR("Inconsistent SIZE of EVENT record");
      printf("           Indiated = %lu      Detected = %lu\n",
	     size,total_size);
      break;
    }

    nbytes = (int)size;
    irc = ttcit_io_write(fd, event, nbytes);
    if(irc <= 0){
      irc = -3;
      TTCIT_IO_STORE_ERR("Canna write the EVENT!");
      break;
    }
    if(irc != nbytes){
      irc = -4;
      TTCIT_IO_STORE_ERR("Incomplete event written\n");
      printf("            Requested: %d   Written: %d\n",
	     nbytes,irc);
      break;
    }

  }while(0);

  return irc;
}

int ttcit_io_make_event(int fd,
			struct ttcit_io_event *event,
			struct SSMbuffer *ssm, unsigned long SSMmode,
			struct BCfifo *bc){
  int irc = 0;

  do{
    if(event == NULL){
      irc = -1;
      TTCIT_IO_MKEV_ERR("event == NULL");
      break;
    }
    if(ssm == NULL){
      irc = -2;
      TTCIT_IO_MKEV_ERR("ssm == NULL");
      break;
    }
    if(bc == NULL){
      irc = -3;
      TTCIT_IO_MKEV_ERR("bc == NULL");
      break;
    }

    irc = ttcit_io_new_event(event);
    if(irc != 0){
      irc = -10 + irc;
      TTCIT_IO_MKEV_ERR("_new_event error -10+irc");
      break;
    }

    irc = ttcit_io_write_ssm(event, ssm, SSMmode);
    if(irc < 0){
      irc = -100 + irc;
      TTCIT_IO_MKEV_ERR("_write_ssm error -100+irc");
      break;
    }

    irc = ttcit_io_write_bcfifo(event,bc);
    if(irc <= 0){
      irc = -1000 + irc;
      TTCIT_IO_MKEV_ERR("_write_bc error -1000+irc");
      break;
    }

    irc = ttcit_io_store_event(fd, event);
    if(irc <= 0){
      irc = -10000 + irc;
      TTCIT_IO_MKEV_ERR("_store_event error -10000+irc");
      break;
    }

    IOcounters.bytes += irc;

  }while(0);

  return irc;
}

int ttcit_io_write_ssm(struct ttcit_io_event *event, 
		       struct SSMbuffer *ssm, unsigned long SSMmode){
  int irc = 0;

  int start;
  int items;

  struct ttcit_io_ssm_scope item;
  int i;
  int ptr1, ptr2;
  int size_in_ulongs;
  unsigned long mask;

  do{

    if(event == NULL){
      irc = -1;
      TTCIT_IO_WSSM_ERR("event == NULL");
      break;
    }

    if(ssm == NULL){
      irc = -2;
      TTCIT_IO_WSSM_ERR("ssm == NULL");
      break;
    }

    event->header.ssm_mode = SSMmode;

    if(IOcounters.SSM_stored){
      irc = -3;
      TTCIT_IO_WSSM_ERR("SSM already stored in this event");
      break;
    }

    if(IOcounters.bc_stored){
      event->header.ssm = event->header.bc + event->header.bc_items;
      event->header.ssm_items = 0;
    }

    start = event->header.ssm;
    items = 0;

    if(ssm->top <= 0){
      irc = -4;
      TTCIT_IO_WSSM_ERR("SSM empty - cannot be!");
      break;
    }

    size_in_ulongs = sizeof(struct ttcit_io_ssm_scope) /
      sizeof(unsigned long);

    /*
      Write only those items, that have at least one bit set
     */
    mask = MASK_L1ACCEPT | MASK_DOUTSTR | MASK_SUBADR | MASK_DOUT;
    for(i = 0; i < ssm->top; i++){
      item.count = i;
      item.word = (mask & ssm->buf[i]);  /* Mask it before trying */
      if(item.word == 0x0){
	continue;
      }
      /* Relevant item, can be stored */
      ptr1 = start + items * size_in_ulongs;
      ptr2 = ptr1 + 1;
      /*
	Check the available memory in event buffer
       */
      if(ptr2 > EVENT_BUFFER){
	irc = -5;
	TTCIT_IO_WSSM_ERR("Not enough memory in EVENT buffer");
	printf("       NEEDED = %d     PRESENT = %d\n",
	       ptr2, EVENT_BUFFER);
	break;
      }
      event->buf[ptr1] = item.count;
      event->buf[ptr2] = item.word;
      items++;
    }
    if(irc < 0){
      break;
    }
    event->header.ssm_items = items;
    irc = sizeof(struct ttcit_io_ssm_scope) * items;
    event->header.size += irc;
    IOcounters.SSM_stored = TRUE;

  }while(0);

  return irc;
}

int ttcit_io_write_bcfifo(struct ttcit_io_event *event, 
			  struct BCfifo *bc){
  int irc = 0;

  int start;
  int items;

  int i;

  int max_index_needed = 0;
  int size_in_ulongs;

  do{

    if(event == NULL){
      irc = -1;
      TTCIT_IO_WBC_ERR("event == NULL");
      break;
    }

    if(bc == NULL){
      irc = -2;
      TTCIT_IO_WBC_ERR("bc == NULL");
      break;
    }

    /*
      If BC fifo has already been stored, raise alarm
     */
    if(IOcounters.bc_stored){
      irc = -3;
      TTCIT_IO_WBC_ERR("BC fifo already stored in this event");
      break;
    }

    /*
      If SSM is already in, put the BC fifo after it
     */
    if(IOcounters.SSM_stored){
      size_in_ulongs = sizeof(struct ttcit_io_ssm_scope) / 
	sizeof(unsigned long);
      event->header.bc = event->header.ssm + 
	(event->header.ssm_items * size_in_ulongs);
      event->header.bc_items = 0;
    }

    start = event->header.bc;
    if(bc->top <= 0){              /* Empty BC is an ERROR */
      irc = -4;
      TTCIT_IO_WBC_ERR("BC FIFO EMPTY! Cannot be!");
      break;
    }

    /*
      Make sure that all items can be stored in existing buffer
     */
    max_index_needed = start + bc->top - 1;
    if(max_index_needed > EVENT_BUFFER){
      irc = -5;
      TTCIT_IO_WBC_ERR("Not enough memory in EVENT buffer");
      printf("             NEEDED = %d     PRESENT = %d\n",
	     max_index_needed, EVENT_BUFFER);
      break;
    }


    items = 0;
    for(i = 0; i < bc->top; i++){
      event->buf[start + items++] = bc->buf[i];
    }
    if(items != bc->top){
      irc = -5;
      TTCIT_IO_WBC_ERR("Entries in BC fifo and EVENT dont match");
      break;
    }
    event->header.bc_items = items;
    irc = sizeof(unsigned long) * items;
    event->header.size += irc;
    IOcounters.bc_stored = TRUE;

  }while(0);

  return irc;
}

int ttcit_io_write_header(int fd, struct ttcit_io_header *header){
  int irc = 0;
  int nbytes = 0;

  struct file_header {
    unsigned long magic;
    struct ttcit_io_header header;
  } f_header;

  size_t to_copy;

  header->magic = TTCIT_IO_MAGIC;
  header->type = TTCIT_IO_FILEHEAD;
  header->size = sizeof(struct ttcit_io_header);
  header->long_size = sizeof(unsigned long);
  header->date = (unsigned long)time(NULL);
  header->version = 100;

  f_header.magic = TTCIT_IO_MAGIC;
  to_copy = sizeof(struct ttcit_io_header);
  memcpy(&(f_header.header), header, to_copy);

  nbytes = sizeof(struct file_header);

  irc = ttcit_io_write(fd, &f_header, nbytes);
  if(irc > 0){
    IOcounters.bytes += irc;
    IOcounters.headers++;
  }

  return irc;
}

int ttcit_io_write_footer(int fd, struct ttcit_io_footer *foot){
  int irc = 0;
  int nbytes = 0;

  do{
    if(foot == NULL){
      printf("ttcit_io_footer: foot == NULL\n");
      irc = -1;
      break;
    }

    foot->size = sizeof(struct ttcit_io_footer);
    foot->type = TTCIT_IO_FOOTER;
    foot->bytes_written = IOcounters.bytes + foot->size;
    foot->events_written = IOcounters.events;
    nbytes = foot->size;

    irc = ttcit_io_write(fd, foot, nbytes);
    if(irc > 0){
      IOcounters.bytes += irc;
      IOcounters.footers++;
    }
  }while(0);

  return irc;
}

int ttcit_io_write(int fd, void *data, int nbytes){
  int irc = 0;

  ssize_t written = 0;
  size_t count = 0;
  size_t remains;
  char *buf;
  int buf_ptr;
  int total_written;

  do{
    if(data == NULL){
      printf("ttcit_io_write: No data, pointer to data NULL\n");
      irc = -1;
      break;
    }
    if((!IOcounters.opened)||(IOcounters.mode != TTCIT_IO_WRITE)){
      printf("ttcit_io_write: No file opened for write\n");
      irc = -2;
      break;
    }

    remains = (size_t)nbytes;
    buf = (char *)data;
    buf_ptr = 0;
    total_written = 0;

    while(remains > 0){
      count = (remains > TTCIT_IO_BLKSIZ) ? TTCIT_IO_BLKSIZ : (size_t)remains;
      remains -= count;

      written = write(fd, buf, count);
      if(written != (ssize_t)count){
	printf("ttcit_io_write: Count %d is not the same as written %d\n",
	       count, written);
	irc = -3;
	break;
      }
      total_written += (int)written;

      buf_ptr = (int)count;
      buf = (buf + buf_ptr);
    }
    if(irc < 0){
      break;
    }

    irc = total_written;

  }while(0);

  return irc;
}

int ttcit_io_read_dump(int fd, int nwords, void *data){
  int irc = 0;

  int nbytes = 0;
  unsigned long *ulong;
  int nread;

  int i;
  unsigned long word;
  div_t d;
  int nlines;

  do{

    if(data == NULL){
      irc = -1;
      TTCIT_IO_DUMP_ERR("data == NULL");
      break;
    }

    if(nbytes < 0){
      irc = 0;
      break;
    }

    /*
      Try to read nwords words
     */
    nbytes = nwords * sizeof(unsigned long);
    irc = ttcit_io_read(fd, data, nbytes);
    if(irc <= 0){
      irc = -3;
      TTCIT_IO_DUMP_ERR("Canna read data from opened file\n");
      break;
    }

    /*
      Print the already read data as unsigned longs
     */

    nread = irc / sizeof(unsigned long);
    if(nread <= 0){
      printf("No full unsigned long dato has been read\n");
      break;
    }

    ulong = (long unsigned int *)data;
    ttcit_io_swap_ulong(ulong, nread, IOcounters.magic);

    printf("ttcit_io_read_dump: READ %d LONG WORDS\n",nread);
    printf("                    dump follows ...\n\n");


    nlines = 0;
    for(i = 0; i < nread; i++){
      word = *(ulong + i);
      d = div(i,10);
      if(d.rem == 0){
	printf(" %6d :  ",i);
      }
      printf(" %4lX ",word);

      if(d.rem == 9){
	printf("\n");
	nlines++;
      }
      if((nlines % 10) == 0){
	printf("\n");
      }
    }
    printf("\n");

    /*
      Close the file
     */

    irc = ttcit_io_close(fd);

  }while(0);

  return irc;
}

int ttcit_io_read(int fd, void *data, int nbytes){
  int irc = 0;

  ssize_t readed = 0;
  size_t count = 0;
  size_t remains;
  char *buf;
  int buf_ptr;
  int total_read = 0;

  do{

    if(data == NULL){
      irc = -1;
      TTCIT_IO_RD_ERR("data == NULL");
      break;
    }

    if((!IOcounters.opened) || (IOcounters.mode != TTCIT_IO_READ)){
      irc = -51;
      TTCIT_IO_RD_ERR("I/O file not opened (for reading)");
      break;
    }

    if(nbytes < 0){
      irc = -2;
      TTCIT_IO_RD_ERR("Nr. of bytes to be read is NEGATIVE");
      break;
    }

    if(nbytes == 0){
      irc = 0;
      break;
    }

    remains = (size_t)nbytes;
    buf = (char *)data;
    buf_ptr = 0;

    while(remains > 0){
      count = (remains > TTCIT_IO_BLKSIZ) ? TTCIT_IO_BLKSIZ : remains;
      remains -= count;

      readed = read(fd, buf, count);
      if(readed != (ssize_t)count){
	printf("ttcit_io_read: Count %d is not the same as readed %d\n",
	       count, readed);
	irc = -3;
	break;
      }
      total_read += (int)readed;

      buf_ptr = (int)count;
      buf = (buf + buf_ptr);
    }
    if(irc < 0){
      break;
    }

    irc = total_read;

  }while(0);

  return irc;
}

int ttcit_io_read_record(int fd, void *data, unsigned long *type, int nmax){
  int irc = 0;

  int ret = 0;
  int nwords;
  unsigned long rec_size;
  enum { RREC_SIZE, RREC_REST, RREC_STOP } rrec_state;
  int stop_it;

  unsigned long *u;

  union {
    unsigned long *ul;
    struct ttcit_io_header *hdr;
    struct ttcit_io_footer *foot;
    struct ttcit_io_event *eve;
  } ptr;

  do{

    if(data == NULL){
      irc = -1;
      TTCIT_IO_RREC_ERR("data == NULL");
      break;
    }

    if(type == NULL){
      irc = -2;
      TTCIT_IO_RREC_ERR("type == NULL");
      break;
    }

    *type = TTCIT_IO_UNKNOWN;
    if(nmax < 0){
      irc = -3;
      TTCIT_IO_RREC_ERR("Max nr. of data to be read NEGATIVE");
      break;
    }

    ptr.ul = (unsigned long *)data;

    stop_it = FALSE;
    rrec_state = RREC_SIZE;
    while(rrec_state != RREC_STOP){
      switch(rrec_state){
      case RREC_SIZE:
	nwords = sizeof(unsigned long);
	u = (unsigned long *)data;
	rrec_state = RREC_REST;
	break;
      case RREC_REST:
	rec_size = *u++;
	if(rec_size > (unsigned long)nmax){
	  irc = -7;
	  TTCIT_IO_RREC_ERR("Record size too large");
	  printf("           Record size: %lu      Max size: %d\n",
		 rec_size, nmax);
	  stop_it = TRUE;
	  break;
	}
	nwords = (int)rec_size - sizeof(unsigned long); /* Remaining bytes */
	rrec_state = RREC_STOP;
	break;
      default:
	stop_it = TRUE;
	break;
      }
      if(stop_it){
	break;
      }
      ret = ttcit_io_read(fd, u, nwords);
      if(ret != nwords){
	if(ret < 0){
	  irc = -4;
	  TTCIT_IO_RREC_ERR("READ error");
	  break;
	}
	if(ret == 0){
	  irc = -5;
	  TTCIT_IO_RREC_ERR("No records size read");
	  break;
	}
	irc = -6;
	TTCIT_IO_RREC_ERR("Requested and Read bytes dont match");
	printf("            Requested: %d      Read: %d\n",nwords,ret);
	break;
      }
      /*
	If byte swapping is needed, do it now, i.e. before some of the
	bytes are evaluated
       */
      ttcit_io_swap_ulong(u, nwords, IOcounters.magic);
    }
    if(stop_it){
      break;
    }

    /*
      Here we have more or less proper record: File HEADER
                                                    EVENT
                                                    FOOTER
     */
    *type = *u;
    switch(*type){
    case TTCIT_IO_FILEHEAD:
      break;
    case TTCIT_IO_FOOTER:
      break;
    case TTCIT_IO_EVENT:
      break;
    default:
      *type = TTCIT_IO_UNKNOWN;
      break;
    }
    irc = (int)rec_size;

  }while(0);

  return irc;
}

int ttcit_io_next_event(int fd, struct SSMbuffer *ssm, struct BCfifo *bc,
			unsigned long *SSMmode, void *data){
  int irc = -99;

  int try_next;
  int nread = 0;
  unsigned long record_type;
  int nmax = sizeof(struct ttcit_io_event);

  struct ttcit_io_header *hdr = NULL;
  unsigned long canonical_magic = TTCIT_IO_MAGIC;
  struct ttcit_io_event *eve = NULL;
  struct ttcit_io_ssm_scope *scope;
  unsigned long *bc_words;
  struct ttcit_io_footer *foot = NULL;

  int tick;
  unsigned long nticks;

  int i;

  int terminate;

  *SSMmode = 0;

  do{

    if(ssm == NULL){
      irc = -1;
      TTCIT_IO_RNXT_ERR("ssm == NULL");
      break;
    }

    if(bc == NULL){
      irc = -2;
      TTCIT_IO_RNXT_ERR("bc == NULL");
      break;
    }

    if(fd == -1){
      irc = -22;
      TTCIT_IO_RNXT_ERR("I/O file not opened");
      break;
    }

    ssm->top = 0;
    bc->top = 0;

    try_next = TRUE;
    terminate = FALSE;
    while(try_next){

      nread = ttcit_io_read_record(fd, data, &record_type, nmax);
      if(nread <= 0){
	irc = -95;
	TTCIT_IO_NEXT_ERR("No data Read of ERROR");
	break;
      }
      switch(record_type){
      case TTCIT_IO_FILEHEAD:
	try_next = TRUE;       /* File header, next record is EVENT */
	hdr = (struct ttcit_io_header *)data;
	/*
	  Print the header (so that one can check)
	 */
	printf("ttcit_io_next_event: File HEADER found\n");
	printf("======================================\n");
	TTCIT_IO_NEXT_PRT("HEADER->size      = ",hdr->size);
	TTCIT_IO_NEXT_PRTX("HEADER->type      = ",hdr->type);
	TTCIT_IO_NEXT_PRTX("HEADER->magic     = ",hdr->magic);
	TTCIT_IO_NEXT_PRT("HEADER->long_size = ",hdr->long_size);
	TTCIT_IO_NEXT_PRT("HEADER->date      = ",hdr->date);
	TTCIT_IO_NEXT_PRT("HEADER->version   = ",hdr->version);
	printf("\n");
	if(hdr->magic != TTCIT_IO_MAGIC){
	  irc = -3;
	  TTCIT_IO_NEXT_ERR("Magic does not match");
	  printf("           Magic read = %lX  is not %lX\n",
		 hdr->magic, canonical_magic);
	  try_next = FALSE;
	  break;
	}
	irc = nread;
	break;
      case TTCIT_IO_EVENT:
	try_next = FALSE;     /* We found what we were looking for */
	eve = (struct ttcit_io_event *)data;
	/*
	  Copy the SSM data
	 */
	*SSMmode = eve->header.ssm_mode;
	nticks = eve->header.ssm_items;
	if(nticks > TTCIT_MAX_ADDR_SSM){
	  irc = -7;
	  TTCIT_IO_NEXT_ERR("Too many SSM data");
	  printf("           Found: %d     Max SSM capacity = %d\n",
		 (int)nticks, (int)TTCIT_MAX_ADDR_SSM);
	  break;
	}
	scope = (struct ttcit_io_ssm_scope *)&eve->buf[eve->header.ssm];
	tick = (nticks > 0) ? scope->count : 0xffffffff;
	for(i = 0; i < TTCIT_MAX_ADDR_SSM; i++){
	  if(tick == i){
	    ssm->buf[i] = scope->word;
	    if(nticks > 0){
	      scope++;
	      nticks--;
	      tick = scope->count;
	    }else{
	      tick = -1;
	    }
	  }else{
	    ssm->buf[i] = 0;
	  }
	}
	ssm->top = TTCIT_MAX_ADDR_SSM;
	/*
	  Copy the BC fifo data
	 */
	bc_words = (unsigned long *)&eve->buf[eve->header.bc];
	nticks = eve->header.bc_items;
	for(bc->top = 0; (nticks > 0) && (bc->top < TTCIT_MAX_ADDR_BCFIFO);
	    bc->top++){
	  bc->buf[bc->top] = *bc_words++;
	}
	bc->top = nticks;
	irc = nread;
	break;
      case TTCIT_IO_FOOTER:
	ssm->top = -1;        /* Indicate EOF */
	foot = (struct ttcit_io_footer *)data;
	printf("ttcit_io_next_event: FIle FOOTER found\n");
	printf("======================================\n");
	TTCIT_IO_NEXT_PRT("FOOTER->size           = ",foot->size);
	TTCIT_IO_NEXT_PRTX("FOOTER->type           = ",foot->type);
	TTCIT_IO_NEXT_PRT("FOOTER->bytes_written  = ",foot->bytes_written);
	TTCIT_IO_NEXT_PRT("FOOTER->events_written = ",foot->events_written);
	printf("\n");
	try_next = FALSE;     /* Last record, no need to continue */

	/*
	  Here we can close the file, error message is not going to be 
	  shown
	 */
	irc = ttcit_io_close(fd);
	if(irc == -1){
	  TTCIT_IO_NEXT_ERR("Canna close I/O file after FOOTER");
	}
	irc = nread;
	break;
      default:
	irc = -4;
	TTCIT_IO_NEXT_PRT("ERROR: Unknown record type = ",record_type);
	try_next = FALSE;
	break;
      }

    }

  }while(0);

  return irc;
}

void ttcit_io_get_counters(struct ttcit_io_counters *c){

  TTCIT_IO_CPCOUNT(opened);
  TTCIT_IO_CPCOUNT(mode);
  TTCIT_IO_CPCOUNT(magic);
  TTCIT_IO_CPCOUNT(bytes);
  TTCIT_IO_CPCOUNT(events);
  TTCIT_IO_CPCOUNT(headers);
  TTCIT_IO_CPCOUNT(footers);
}

void ttcit_io_swap_ulong(unsigned long *u, int nwords, unsigned long magic){

  int i;

  unsigned char *c;
  unsigned char c1, c2, c3, c4, swap;

  do{
    if(magic == TTCIT_IO_MAGIC){  /* O.K. no need to swap bytes */
      break;
    }

    for(i = 0; i < nwords; i++){
      switch(magic){
      case TTCIT_IO_MAGIC_1:
	c = (unsigned char *)(u + i);

	c1 = *c;
	c2 = *(c + 1);
	c3 = *(c + 2);
	c4 = *(c + 3);

	swap = c1;
	c1 = c4;
	c4 = swap;
	swap = c3;
	c3 = c2;
	c2 = swap;

	*c = c1;
	*(c + 1) = c2;
	*(c + 2) = c3;
	*(c + 3) = c4;
	break;
      default:                         /* Do nothing */
	break;
      }
    }

  }while(0);
}
