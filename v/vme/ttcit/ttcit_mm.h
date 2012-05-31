#ifndef TTCIT_MM_H__
#define TTCIT_MM_H__

/*
  Memory manegement for TTCit software logic
 */

#ifndef TRUE
#define TRUE  (1==1)
#endif
#ifndef FALSE
#define FALSE (1==0)
#endif

/*
                        FIFO
                        ==== 
*/

struct ttcit_mm_FIFO {

  void *mem;               /* Pointer to FIFO memory, must be allocated
                              by ttcit_mm_FIFO_create call */

  size_t size;     /* Size of the object to be stored / retrieved */

  size_t n;           /* Number of objects that can be stored in memory */

  size_t memtop;   /* TOP of the memory, going beyond this will lead to
                      segmentation violation error */

  size_t infifo;   /* Number of data held in FIFO */

  int top;         /* Index of the YOUNGEST object (last inserted) */

  int bottom;      /* Index of the OLDEST object (first inserted ) */

  int empty;       /* == TRUE if FIFO is empty, FALSE is holding data */
};

/*
  Always declare FIFO structures using macro DECL_FIFO in order to have them
  properly filled with nulls at the begining.

  Further use of such unallocated FIFO will be detected as error thus
  avoiding the problems with segmentation violation
 */

#define DECL_FIFO(A) struct ttcit_mm_FIFO A = {NULL,0,0,0,0,0,0,1}

enum ttcit_mm_age { YOUNGEST, OLDEST };

/*
  Action to be done by _get_ function: 

  PICK      = return the oldest Object copy, delete copy in FIFO

  COPY      = return the oldest Object copy, leave copy in FIFO as oldest

  DELETE    = delete oldest Object copy from FIFO, do not return anything
 */
enum ttcit_mm_getype { PICK, COPY, DELETE };

/*
  Member functions for FIFO
 */

/*
  Create a FIFO:
                  fifo      = pointer to structure where the FIFO is
                              to be placed
                  ObjSize   = size of 1 object to be stored in FIFO
                  NrObjects = Max. number of objects we want to store
                              in FIFO

  returns:
                  0         = FIFO created successfully
                 -1         = the necessary memory cannot be allocated
*/
int ttcit_mm_FIFO_create(struct ttcit_mm_FIFO *fifo, size_t ObjSize, 
			 size_t NrObjects);

/*
  Frees allocated memory, makes fifo unusable
 */
void ttcit_mm_FIFO_free(struct ttcit_mm_FIFO *fifo);

/*
  Empty the FIFO, set number of help objects to 0
 */
void ttcit_mm_FIFO_empty(struct ttcit_mm_FIFO *fifo);

/*
  Puts an object into FIFO:
                           fifo   = pointer to structure where FIFO has been
                                    allocated
                           obj    = ponter to the object to be inserted
  returns:
                > 0   = index of the object in memory (absolute)
                  0   = zero size object not stored
                 -1   = not enough memory in FIFO

 */
int ttcit_mm_FIFO_put(struct ttcit_mm_FIFO *fifo, void *obj);

/*
  Gets the oldest object from the FIFO:
                                        fifo   = pointer to FIFO storage
                                        obj    = pointer to object to be placed
                                                 in FIFO
                                        action = what exactly has to be done
                                                 PICK, COPY, DELETE
  returns:
                > 0   = index of the retrieved object (absolute)
                  0   = FIFO is empty, nothing has been retrieved
 */
int ttcit_mm_FIFO_get(struct ttcit_mm_FIFO *fifo, void *obj,
		      enum ttcit_mm_getype action);

/*
  Gets the address of the oldest object. Does not change the FIFO. It is
  meant for cases where one has to modigy the oldest item stored
 */
void *ttcit_mm_FIFO_get_addr(struct ttcit_mm_FIFO *fifo);

/*
  Print the contents of the struct ttcit_mm_FIFO:

                           fifo    = Pointer to FIFO structure to be printed
                           s       = string to be printed (e.g. FIFO name)
 */
void ttcit_mm_FIFO_print(struct ttcit_mm_FIFO *fifo, const char *s);

/*
  Simple test to verify the code independently of the rest of ttcit 
  software.
 */
void ttcit_mm_test();

/*
                     H I S T O G R A M
 */

/*
  Simple, 1D histogram of integer values (type 1Di1)

  It has 1 cell per integer from range min...max
 */
struct ttcit_mm_I1Di1 {
  int entries;
  int channels;
  int min;
  int max;
  int underflow;
  int overflow;

  int *h;
};

/*
  Create one simle 1D histogram of the type I1Di1
 */
int ttcit_mm_I1Di1_create(struct ttcit_mm_I1Di1 *h, int min, int max);

/*
  Fill one simple 1D histogram of the type 1Di1
 */
void ttcit_mm_I1Di1_fill(struct ttcit_mm_I1Di1 *h, int val);

/*
  Empty/Delete one simple 1D histogram of the type I1Di1
 */
void ttcit_mm_I1Di1_delete(struct ttcit_mm_I1Di1 *h);

/*
  Print contents of one simple 1D histogram of the type I1Di1
 */
void ttcit_mm_I1Di1_print(struct ttcit_mm_I1Di1 *h, const char *s);

#endif /* TTCIT_MM_H__ */
