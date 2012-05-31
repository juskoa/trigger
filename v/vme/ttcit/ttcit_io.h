#ifndef _TTCIT_IO_H__
#define _TTCIT_IO_H__

/*
  Header file for the TTCIT Input/Output
*/

#include "ttcit_logic.h"

/*
  Definitions that may become system dependent
 */

#define TTCIT_IO_BLKSIZ    32767

/*
  Macros
 */

#define TTCIT_IO_CPCOUNT(A) c->A = IOcounters.A

#define TTCIT_IO_MKEV_ERR(A) printf(	\
				      "ttcit_io_make_event : %s %s %d\n ", \
				      A ," IRC = ",	       \
				      irc)
#define TTCIT_IO_NEWEV_ERR(A) printf( \
				     "ttcit_io_new_event : %s %s %d\n ", \
				     A ," IRC = ", irc)

#define TTCIT_IO_WBC_ERR(A) printf( \
				   "ttcit_io_write_bcfifo : %s %s %d\n", \
				   A ," IRC = ",irc)
#define TTCIT_IO_RREC_ERR(A) printf( \
				    "ttcit_io_read_record : %s %s %d\n", \
				    A ," IRC = ",irc)

#define TTCIT_IO_RD_ERR(A) printf( \
				  "ttcit_io_read : %s %s %d\n", \
				  A ," IRC = ",irc)

#define TTCIT_IO_RNXT_ERR(A) printf( \
				    "ttcit_io_next_event : %s %s %d\n", \
				    A ,"IRC = ",irc)

#define TTCIT_IO_DUMP_ERR(A) printf( \
				    "ttcit_io_read_dump: %s %s %d\n", \
				    A ,"IRC = ",irc)

#define TTCIT_IO_WSSM_ERR(A) printf( \
				    "ttcit_io_write_ssm: %s %s %d\n", \
				    A ,"IRC = ",irc)

#define TTCIT_IO_STORE_ERR(A) printf( \
				     "ttcit_io_store_event: %s %s %d\n", \
				     A ,"IRC  = ",irc)
#define TTCIT_IO_NEXT_PRT(A,B) printf( \
				    "ttcit_io_next_event: %s %lu\n", \
				    A, B)

#define TTCIT_IO_NEXT_PRTX(A,B) printf( \
				       "ttcit_io_mext_event: %s %lX\n", \
				       A, B)

#define TTCIT_IO_NEXT_ERR(A) printf( \
				    "ttcit_io_next_event: %s %s %d\n", \
				    A, "IRC  = ",irc)


/*
  I/O file structure:

  0) Magic number         = first word is a magic number showing how to
                            swap bytes on our platform

  1) FILE HEADER

  2) Record_1             = EVENT HEADER
                            SSM_data
                            BC_FIFO_data

     Record_2

     Record_3

     ...

     Record_N

  3) FILE FOOTER

*/

#define TTCIT_IO_MAGIC      0xa1b2c3d4  /* Magic word, needed for byte swaps */

#define TTCIT_IO_MAGIC_1    0xd4c3b2a1  /* Bytes swapped */

#define TTCIT_IO_RECLEN     8192        /* Record length - must be optimized */

#define TTCIT_IO_SSM_SCOPE  0xdead0001  /* SSM, scope mode (with timing) */
#define TTCIT_IO_SSM_SEQ    0xdead0002  /* SSM, sequencer mode, no timing */
#define TTCIT_IO_FIFO_SEQ   0xdead0003  /* FPGA FIFO, BC sequence, no timing */
#define TTCIT_IO_FOOTER     0xdead1000  /* Footer record */
#define TTCIT_IO_EVENT      0xdead0004  /* Event record */
#define TTCIT_IO_FILEHEAD   0xdead2000  /* File header */
#define TTCIT_IO_UNKNOWN    0xdeadbead  /* Unknown record type */

#define TTCIT_IO_READ       0xadaa0001  /* Open mode READ */
#define TTCIT_IO_WRITE      0xadaa0002  /* Open file for WRITE */


/*
  Here we assume:   sizeof unsigned long = 4 bytes
 */

struct ttcit_io_header {

  unsigned long size;         /* Header size in bytes */
  unsigned long type;         /* Records type - FILE HEADER */
  unsigned long magic;        /* Magic number, let us keep it */
  unsigned long long_size;   /* Size of long */
  unsigned long date;        /* Date of the file creation, time since 
				0:0:0 UTC, 1 January 1970
			        Can be converted to something better  */
  unsigned long version;     /* Version of the I/O package */

};

struct ttcit_io_record {

  unsigned long size;                              /* Record size in bytes */
  unsigned long type;                              /* Record type */
  unsigned long seqNr;                             /* Sequential number */
  unsigned long items;                            /* Nr. of items in record */
  unsigned long buf[TTCIT_IO_RECLEN];    /* Record data */

};

struct ttcit_io_footer {

  unsigned long size;                /* Size of the footer */
  unsigned long type;                /* Footer identifier TTCIT_IO_FOOTER */
  unsigned long bytes_written;       /* Number of bytes written */
  unsigned long events_written;      /* Number of events written */

};

/* ITEMS FOR DIFFERENT RECORD TYPES */

/*
  Item for record type: TTCIT_IO_SSM_SCOPE

  An item is written when the word read from the SSM is different from the
  last read one.

  An item consists of: { SSM_address , SSM word }
*/

struct ttcit_io_ssm_scope {
  unsigned long count;                /* SSM address */
  unsigned long word;                 /* SSM word */
};

/*
  Item for record type: TTCIT_IO_SSM_SEQ

  Only sequences are seen, no timing info.

  An item consists of { SSM word }
 */

struct ttcit_io_ssm_seq {
  unsigned long word;                 /* SSM word */
};

/*
  Item for record type: TTCIT_IO_FIFO_SEQ:

  Only BC numbers are stored
 */

struct ttcit_io_fifo_seq {
  unsigned long BC;                  /* BC count */
};

/*
  Event header
 */

struct ttcit_io_event_header {
  unsigned long size;              /* Event records size in Bytes */
  unsigned long type;              /* Record type */
  unsigned long event;             /* Event number */
  unsigned long ssm_mode;          /* SSM mode
				      TTCIT_IO_SSM_SCOPE : set_ss_mode(0)
				      TTCIT_IO_SSM_SEQ   : set_ss_mode(1) 
				    */
  unsigned long ssm;               /* Pointer to start of the SSM data within
				      the structure */
  unsigned long ssm_items;         /* Nr. of SSM snap shot items */
  unsigned long bc;                /* Pointer to the BF FIFO data within 
				      structure */
  unsigned long bc_items;          /* Nr. of BC FIFO items in the event */
};

/*
  Full event data structure

  Increase the buffer size if 1MB is not enough
 */

struct ttcit_io_event {
  struct ttcit_io_event_header header;   /* Event header */
  unsigned long buf[EVENT_BUFFER];
};

/*
  Structure keeping internal statistics - how many bytes, records and 
  items are written
*/

struct ttcit_io_counters {
  int opened;                 /* TRUE if file opened, FALSE if closed */
  unsigned long mode;
  unsigned long magic;        /* Magic number as seen after READ */

  int SSM_stored;
  int bc_stored;

  unsigned long bytes;        /* Number of bytes read/written */
  unsigned long events;       /* Number of events WRITTEN/READ */
  unsigned long headers;      /* Number of file headers WRITTEN/READ */
  unsigned long footers;      /* Number of file footers WRITTEN/READ */

};

/*
  I/O functions
*/

/*
  Open I/O file: 
                 file       = filename
                 mode       = TTCIT_IO_READ or TTCIT_IO_WRITE (read || write)

		 returns:   file descriptor
 */
int ttcit_io_open(const char *file, int mode);

/*
  Close I/O file:
                  fd        = file desctiptor

		  returns:    0  if all O.K.
                              error code if not
 */

int ttcit_io_close(int fd);

/*
  Start a new event:
                     event     = event data structure to be initialized

                     returns:
                                 0   = O.K.
                                 1   = event == NULL
 */

int ttcit_io_new_event(struct ttcit_io_event *event);

/*
  Write an event:
                   fd           = file descriptor
                   event        = event data structure to be written
                                  for new event

		   returns:       0  = if O.K.
                                 -1  = event == NULL

 */

int ttcit_io_store_event(int fd, struct ttcit_io_event *event);

/*
  Write SSM buffer:
                    fd           = file descriptor
                    ssm          = SSM buffer 
                    SSMmode      = TTCIT_IO_SCOPE or TTCIT_IO_SEQ

		    returns:     number of bytes written
                                 0    = Empty buffer
                                 <0   = error
 */

int ttcit_io_write_ssm(struct ttcit_io_event *event, 
		       struct SSMbuffer *ssm, unsigned long SSMmode);

/*
  Write BC fifo:
                  event       = event data structure to be filled
                  bc          = BC fifo buffer

		  returns:      number of bytes written
                                0    = Empty buffer
                               <0    = error
 */
int ttcit_io_write_bcfifo(struct ttcit_io_event *event,
			  struct BCfifo *bc);

/*
  Create an event:
                    fd            = file descriptor for output file
		    event         = event data structure to be filled
		    ssm           = SSM bffer
		    SSMmode       = SSM mode (SCOPE or SEQ)
		    bc            = BC fifo buffer

		    returns:      number of bytes written
                                  0 = empty buffer
                                 -1 = event == NULL
				 -2 = ssm == NULL
				 -3 = bc == NULL
				 -4 = ssm not recognized as SSM mode
				 <0   any other error

 */
int ttcit_io_make_event(int fd, 
			struct ttcit_io_event *event,
			struct SSMbuffer *ssm, unsigned long SSMmode,
			struct BCfifo *bc);

/*
  Write header:
                 event       = event data structure to be filled
                 header      = pointer to header structure to be written

		 returns     = nr. of bytes written
                               0   if empty header
                              <0   on error
 */

int ttcit_io_write_header(int fd, struct ttcit_io_header *header);

/*
  Write footer:
                fd          = file descriptor
                foot        = pointer to footer structure to be written

		returns     = nr. of bytes written
                              0 if empty footer
                              <0 on error
 */

int ttcit_io_write_footer(int fd, struct ttcit_io_footer *foot);

/*
  Generic write routine 
                        fd     = file descriptor
                        data   = data buffer
                        nbytes = number of bytes to be written

			returns:  = nr. of bytes written
                                    0 if nothing is written
                                   <0 on error
 */

int ttcit_io_write(int fd, void *data, int nbytes);

/*
  Read specified number of bytes from the input file and dumps them as a 
  stream of unsigned long numbers (in decimal and hex print). To be used
  for the debugging only.

  The file must be opened by caller

                 fd            = file desriptor
                 nwords:       = number of LongWords to be printed and analyzed
		 data           = caller specified memory area properly
		                 dimensioned (no check on its size performed)

		 returns:      number of bytes actually read
                            0 = if no data has been read
                           <0 = on ERROR
 */
int ttcit_io_read_dump(int fd, int nwords, void *data);

/*
  Read generic:
                fd             = file descriptor
		data           = pointer to data buffer to be filled
		nbytes         = how many bytes should be read

		returns:         number of bytes actually read
		               0 = if nothing has been read
			      <0 = on ERROR

 */
int ttcit_io_read(int fd, void *data, int nbytes);

/*
  Read one record:
                   fd      = file descriptor
		   data    = pointer to data buffer to be filled
		   type    = type of the data record: TTCIT_IO_FILEHEAD
		                                      TTCIT_IO_EVENT
						      TTCIT_IO_FOOTER
						      TTCUT_IO_UNKNOWN
		   nmax    = max number of bytes to be read

		   returns:   number of bytes actually read
		            0 no data read
                           <0 ERROR   
 */
int ttcit_io_read_record(int fd, void *data, unsigned long *type, int nmax);

/*
  Read next event, (skip file header if encountered):

                   fd         = file descriptor
                   ssm        = SSM buffer to be filled
                   bc         = BC fifo to be filled
		   SSMmode    = SSM mode: TTCIT_IO_SCOPE
		                          TTCIT_IO_SEQ

		   returns:      number of bytes read
                              0  nothing read
                             <0  error 

 */
int ttcit_io_next_event(int fd, struct SSMbuffer *ssm, struct BCfifo *bc,
			unsigned long *SSMmode, void *data);

/*
  Make a copy of the IO counters.

                                 c  = pointer to a IO counter structure
                                      where the internal counters are to
                                      be copied. 
 */

void ttcit_io_get_counters(struct ttcit_io_counters *c);

/*
  Perform the byte swapping for the whole array of unsigned long

               u          = array of unsigned long elements
	       nwords     = number of longwords to be corrected
	       magic      = magic word as seen after the READ

 */
void ttcit_io_swap_ulong(unsigned long *u, int nwords, unsigned long magic);

#endif /* !_TTCIT_IO_H__ */
