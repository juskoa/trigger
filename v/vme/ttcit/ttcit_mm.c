/*
  TTCit software logic Memory Management tools
 */

#include <stdio.h>
#include <stdlib.h>

#include "ttcit_mm.h"


int ttcit_mm_FIFO_create(struct ttcit_mm_FIFO *fifo,
			 size_t ObjSize, size_t NrObjects){
  int irc = 0;

  int same_alloc;

  do{

    /*
      It may happen, that we are trying to allocate memory for FIFO
      that has already been allocated with the same arguments.

      In that case no reallocation is necessary, just check the 
      parameters and reset it to empty state
     */

    if(fifo->mem != NULL){  /* FIFO has already been allocated */

      same_alloc = (ObjSize == fifo->size); /* The same objects stored */
      same_alloc = same_alloc && 
	((ObjSize*NrObjects) == fifo->memtop);
      same_alloc = same_alloc && 
	(NrObjects == (fifo->n + 1));

      if(same_alloc){ /* All elements match, just empty it and leave */
	ttcit_mm_FIFO_empty(fifo);
	break;
      }else{      /* Not all elements match, free and allocate as fresh */
	ttcit_mm_FIFO_free(fifo);
      }
    }

    fifo->mem = calloc(NrObjects, ObjSize);
    if(fifo->mem == NULL){
      printf("Not enough memory to allocate %u objects of size %u\n",
	     ObjSize, NrObjects);
      irc = -1;
      break;
    }
    fifo->size = ObjSize;
    fifo->memtop = ObjSize * NrObjects;
    fifo->n = (int)NrObjects - 1;        /* Max index for TOP/BOTTOM */

    ttcit_mm_FIFO_empty(fifo);
  }while(0);

  return irc;
}

void ttcit_mm_FIFO_free(struct ttcit_mm_FIFO *fifo){
  if(fifo->mem != NULL){
    free(fifo->mem);
  }
  fifo->mem = NULL;
  fifo->size = 0;
  fifo->memtop = 0;
  fifo->infifo = 0;
  fifo->top = 0;
  fifo->bottom = 0;
  fifo->empty = TRUE;
}

void ttcit_mm_FIFO_empty(struct ttcit_mm_FIFO *fifo){
  fifo->infifo = 0;
  fifo->top = -1;
  fifo->bottom = -1;
  fifo->empty = TRUE;
}

int ttcit_mm_FIFO_put(struct ttcit_mm_FIFO *fifo, void *obj){
  int irc = -2;

  int i;
  char *m;
  int diff, ceiling;
  int  newtop = 0;
  size_t A;

  do{

    if(fifo->mem == NULL){   /* No memory has been allocated */
      irc = 0;
      break;
    }

    if(fifo->size == 0){ /* Zero sized objects, maybe not initialized FIFO */
      irc = 0;
      break;
    }

    diff = fifo->top - fifo->bottom;

    do{

      if((fifo->infifo == 0) || fifo->empty){    /* FIFO is empty */
	fifo->empty = FALSE;
	fifo->top++;         /* Only in this case top == bottom */
	fifo->bottom++;
	A = 0;                    /* Start copying at address 0 */
	break;
      }

      if(diff >= 0){
	ceiling = fifo->n;
	newtop = (fifo->top < ceiling) ? fifo->top + 1 : 0;
      }else{
	ceiling = fifo->bottom;
	newtop = (fifo->top < ceiling) ? fifo->top + 1 : fifo->bottom;
      }
      if(newtop == fifo->bottom){
	newtop = -1;
	break;
      }

      fifo->top = newtop;      

      /*
	Here we know the index, now we must calculate its address
       */
      A = fifo->top * fifo->size;

    }while(0);

    if(newtop < 0){    /* It looks like we have no memory left */
      irc = -1;
      break;
    }

    /*
      Copy the Object into FIFO memory, starting with address A
     */

    m = (char *)fifo->mem;
    for(i = 0; (size_t)i < fifo->size; i++){
      *(m + A + i) = *((char *)obj + i);
    }

    fifo->infifo++;
    irc = fifo->top + 1;

  }while(0);

  return irc;
}

int ttcit_mm_FIFO_get(struct ttcit_mm_FIFO *fifo, void *obj,
		      enum ttcit_mm_getype action){
  int irc = -2;

  char *m;
  size_t A;
  int i;
  int newbottom;
  int diff;

  do{

    /*
      If FIFO is empty, there is nothing to copy/delete
     */
    if(fifo->mem == NULL){
      irc = 0;
      break;
    }

    if((fifo->infifo == 0) || fifo->empty){
      irc = 0;
      ttcit_mm_FIFO_empty(fifo);
      break;
    }

    /*
      Copy the Object at the bottom. Do nothing if you just want to
      delete it
     */

    if(action != DELETE){
      if(obj == NULL){    /* If you want to delete only, obj may be NULL */
	irc = 0;
	break;
      }
      m = (char *)fifo->mem;
      A = fifo->bottom * fifo->size;
      for(i = 0; (size_t)i < fifo->size; i++){
	*((char *)obj + i) = *(m + A + i);
      }
    }

    /*
      If we want to PICK or DELETE we must remove the copy of the object from
      FIFO
     */
    if(action == COPY){  /* We do not touch the BOTTOM !!! */
      irc = fifo->bottom + 1;     
      break;
    }

    /* We must properly update the new index to BOTTOM */

    /*
      If there is just one Object in FIFO, after PICK the FIFO becomes
      EMPTY
     */

    if(fifo->infifo == 1){
      irc = fifo->bottom + 1;
      ttcit_mm_FIFO_empty(fifo);
      break;
    }

    /*
      We must handle separately 2 cases:   TOP > BOTTOM
                                           TOP < BOTTOM
     */

    diff = fifo->top - fifo->bottom;

    if(diff >= 0){    /* This is simple case TOP > BOTTOM */
      newbottom = fifo->bottom + 1;
    }else{
      newbottom = (fifo->bottom < (int)fifo->n) ? fifo->bottom + 1 : 0;
    }
    irc = fifo->bottom + 1;
    fifo->bottom = newbottom;
    fifo->infifo--;

  }while(0);

  return irc;
}

void *ttcit_mm_FIFO_get_addr(struct ttcit_mm_FIFO *fifo){
  void *ptr = NULL;
  char *m;
  size_t A;

  do{

    if(fifo->mem == NULL){
      break;
    }

    if((fifo->infifo == 0) || (fifo->empty)){
      break;
    }

    m = (char *)fifo->mem;
    A = fifo->bottom * fifo->size;
    ptr = m + A;

  }while(0);

  return ptr;
}

void ttcit_mm_FIFO_print(struct ttcit_mm_FIFO *fifo, const char *s){
  printf("Printing FIFO : %s\n",s);
  printf("--------------\n");
  printf("FIFO.mem           = %p    Void pointer to memory\n",fifo->mem);
  printf("FIFO.size          = %u    Object size in bytes\n",fifo->size);
  printf("FIFO.n             = %d    Max. internal index\n",fifo->n);
  printf("FIFO.memtop        = %u    Top of the memory\n",fifo->memtop);
  printf("FIFO.infifo        = %u    Number of objects stored\n",fifo->infifo);
  printf("FIFO.top           = %d    Index to the TOP\n",fifo->top);
  printf("FIFO.bottom        = %d    Index to the Bottom\n",fifo->bottom);
  printf("FIFO.empty         = %d    Is FIFO empty???\n",fifo->empty);
  printf("_____________________________________________________\n");
}

void ttcit_mm_test(){

  DECL_FIFO(test_fifo);

  size_t osize;
  size_t no;

  unsigned long Object;
  unsigned long i;
  int irc;
  unsigned long maxtry = 1000;

  /*
    Initialize small test FIFO: Object    = unsigned long (size = 4 bytes)
                                N objects = 7 
   */

  ttcit_mm_FIFO_print(&test_fifo, "test_fifo : uninitiated\0");

  do{

  osize = sizeof Object;
  no = 7;
  irc = ttcit_mm_FIFO_create(&test_fifo, osize, no);
  if(irc != 0){
    printf("Canna create FIFO: test_fifo  IRC = %d\n",irc);
    break;
  }
  ttcit_mm_FIFO_print(&test_fifo, "test_fifo: After allocating memory\0");

  printf("Allocate the same FIFO once more, check with DDD \n");
  irc = ttcit_mm_FIFO_create(&test_fifo, osize, no);
  if(irc != 0){
    printf("Canna create FIFO: test_fifo  IRC = %d\n",irc);
    break;
  }
  ttcit_mm_FIFO_print(&test_fifo, "test_fifo: After allocating memory\0");

  /*
    TEST: 1

    Fill the FIFO until FULL, then print the contents
   */
  ttcit_mm_FIFO_empty(&test_fifo);

  for(i = 1; i < maxtry; i++){
    Object = i;
    irc = ttcit_mm_FIFO_put(&test_fifo, &Object);
    if(irc <= 0){
      printf("Filled %lu Objects, IRC = %d\n",i,irc);
      break;
    }else{
      printf("Putting Object %lu     IRC = FIFO.top = %d\n",Object,irc);
    }
    ttcit_mm_FIFO_print(&test_fifo, "test_fifo ina loop\0");
  }
  ttcit_mm_FIFO_print(&test_fifo, "test_fifo : After first FULL fill\0");

  /*
    Now we can try to pick one by one all objects from the FIFO
   */

  do{
    irc = ttcit_mm_FIFO_get(&test_fifo, &Object, PICK);
    if(irc > 0){
      printf("After removing Object = %lu       IRC = %d\n",Object,irc);
    }else{
      printf("FIFO seem to be empty IRC = %d\n",irc);
    }
    ttcit_mm_FIFO_print(&test_fifo,"test_fifo - after PICK\0");
  }while(irc > 0);
  ttcit_mm_FIFO_print(&test_fifo,"test_fifo - Must be EMPTY\n");

  /*
    TEST: 2 

    1) Fill Fifo until FULL
    2) PICK half of the Objects
    3) Fill again until FULL
    4) Pick all objects 
   */

  Object = 10;    /* Filling full */
  do{
    Object++;
    irc = ttcit_mm_FIFO_put(&test_fifo, &Object);
    printf("I put Object %lu     with IRC = %d\n",Object,irc);
  }while(irc > 0);

  ttcit_mm_FIFO_print(&test_fifo, "test_fifo - after filling FULL\0");

  /* PICK-int half of the Objects */

  for(i = 0; i < 4; i++){
    irc = ttcit_mm_FIFO_get(&test_fifo, &Object, PICK);
    if(irc >0){
      printf("Picked Object %lu     IRC = %d\n",Object,irc);
    }else{
      printf("An attept to pick an object lead to IRC = %d\n",irc);
      break;
    }
  }
  ttcit_mm_FIFO_print(&test_fifo, "test_fifo - after half Object picked\0");

  /* Fill till full */

  Object = 50;
  do{
    Object++;
    irc = ttcit_mm_FIFO_put(&test_fifo, &Object);
    printf("Putting Object %lu      leads to IRC = %d\n",Object,irc);
    ttcit_mm_FIFO_print(&test_fifo,"test_fifo  - being filled again\n");
  }while(irc > 0);

  /* Read all elements from FIFO, empty it */

  do{
    irc = ttcit_mm_FIFO_get(&test_fifo, &Object, PICK);
    if(irc > 0){
      printf("Picked object %lu     got IRC = %d\n",Object,irc);
    }else{
      printf("Picked NOTHING, got IRC = %d\n",irc);
    }
    ttcit_mm_FIFO_print(&test_fifo, "test_fifo: PICK \0");
  }while(irc > 0);


  }while(0);

  ttcit_mm_FIFO_free(&test_fifo);

}

int ttcit_mm_I1Di1_create(struct ttcit_mm_I1Di1 *h, int min, int max){
  int irc = 0;
  int i;

  size_t size;
  int no;

  if(max < min){
    irc = -1;
  }
  h->min = min;
  h->max = max;
  h->channels = (max - min + 1);

  h->underflow = 0;
  h->overflow = 0;
  h->entries = 0;

  no = h->channels;
  size = (size_t)no * sizeof(int); 
  h->h = (int *)malloc(size);
  if(h->h == NULL){
    irc = -2;
  }else{
    for(i = 0; i < h->channels; i++){
      *(h->h + i) = 0;
    }
  }

  return irc;
}

void ttcit_mm_I1Di1_fill(struct ttcit_mm_I1Di1 *h, int val){
  int j;

  h->entries++;

  j = val - h->min;
  if(j < 0){
    h->underflow++;
  }
  if(j >= h->channels){
    h->overflow++;
  }
  (*(h->h + j))++;
}

void ttcit_mm_I1Di1_delete(struct ttcit_mm_I1Di1 *h){
  h->entries = 0;
  h->channels = 0;
  h->min = 0;
  h->max = 0;
  h->underflow = 0;
  h->overflow = 0;
  free(h->h);
}

void ttcit_mm_I1Di1_print(struct ttcit_mm_I1Di1 *h, const char *s){

  int i;
  int val;
  int x;

  char *Chnl = "Channel\0";   /* 7 chars */
  char *Val = "Val\0";        /* 3 chars */
  char *ValX = "ValHEX\0";    /* 6 chars */
  char *Event = "Events\0";   /* 6 chars */
  char *Ufl = "UNDERFLOW\0";  /* 9 chars */
  char *Ofl = "OVERFLOW\0";   /* 8 chars */
  char *Bnk = "     \0";     /* 5 chars */

  printf("\n\n");

  printf("********************************************************\n");
  printf("*                                                      *\n");
  printf("* %s \n",s);
  printf("*                                                      *\n");
  printf("********************************************************\n");
  printf("\n");
  printf("::   ENTRIES   = %6d\n",h->entries);
  printf("::   CHANNELS  = %6d\n",h->channels);
  printf("::   MIN       = %6d\n",h->min);
  printf("::   MAX       = %6d\n",h->max);
  printf("\n");
  printf("%10s  %6s  %6s  %10s      Zero suppression print\n",
	 Chnl, Val, ValX, Event);
  printf("-------------------------------------------------------\n");
  printf("%10s  %6s  %6s  %10d\n",Ufl,Bnk,Bnk,h->underflow);
  for(i = 0; i < h->channels; i++){
    val = *(h->h + i);
    if(val == 0){              /* Zero suppression print */
      continue;
    }
    x = h->min + i;
    printf("%10d  %6d  %6X  %10d\n",i,x,x,val);
  }
  printf("%10s  %6s  %6s  %10d\n",Ofl,Bnk,Bnk,h->overflow);
  printf("-------------------------------------------------------\n");
  printf(" \n");

}
