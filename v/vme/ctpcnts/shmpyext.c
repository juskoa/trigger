#include <Python.h>
#include <stdio.h>

#include "vmewrapdefs.h"
#include "shmaccess.h"

static PyObject * getcnt(PyObject *self, PyObject *args);
static PyObject * startstopfw(PyObject *self, PyObject *args);
static PyObject * getcnts(PyObject *self, PyObject *args);
static PyMethodDef shmpyextMethods[] = {
    {"getcnt",  getcnt, METH_VARARGS,
     "return the value of 1 counter."},
    {"startstopfw",  startstopfw, METH_VARARGS,
     "Start/Stop fifo write 1:start 0:stop"},
    {"getcnts",  getcnts, METH_VARARGS,
     "return list of values of requested counters."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};
 
/* PyMODINIT_FUNC initshmpyext(void) { */
void initshmpyext(void) {
(void) Py_InitModule("shmpyext", shmpyextMethods);
shm= (Tshm *)mallocShared(CTPSHMKEY, 0, &shmsegid);
/*freeShared() */
}

/*--------------------------------------------------------------- startstopfw
O: start (1) or stop (0) writing to fifo by hwreader
RC: requested value which was set or None if shm not allocated
*/
static PyObject *startstopfw(PyObject *self, PyObject *args) {
int ireq;
w32 cntval;
if (!PyArg_ParseTuple(args, "i", &ireq)) {
  return NULL;
};
if(shm==NULL) {
/* (for void function) */
  Py_INCREF(Py_None); return Py_None;
} else {
  shm->request= ireq;
/*printf("getcnt:%d:%x\n",ireq,cntval); */
return Py_BuildValue("i", ireq);
}
}
/*--------------------------------------------------------------- getcnt
O: read counter value (counters are named by numbers 0... -relative
   position in Tbuf1 structure) from active SHM buffer
RC: counter value or NULL if bad counter index given
*/
static PyObject *getcnt(PyObject *self, PyObject *args) {
int icnt;
w32 cntval;
if (!PyArg_ParseTuple(args, "i", &icnt)) {
  return NULL;
};
if(shm==NULL) {
/* (for void function) */
Py_INCREF(Py_None); return Py_None;
} else {
cntval= getcnt1(icnt);
/*printf("getcnt:%d:%x\n",icnt,cntval); */
return Py_BuildValue("i", cntval);
}
}
/*--------------------------------------------------------------- getcnts */
static PyObject *getcnts(PyObject *self, PyObject *args) {
const char *command;
int iarr[4]={2,3,4};
if (!PyArg_ParseTuple(args, "s(iii)", &command, 
  &iarr[0], &iarr[1], &iarr[2]  //, &iarr[3], &iarr[4], 
  //&iarr[5], &iarr[6], &iarr[7], &iarr[8], &iarr[9]
  )) {
  return NULL;
};
printf("getcnts:%s\n",command);
return Py_BuildValue("iiii", 
  iarr[0], iarr[1], iarr[2] //, iarr[3] //, iarr[4], 
  //iarr[5], iarr[6], iarr[7], iarr[8], iarr[9]
  );
/* or (for void function)
Py_INCREF(Py_None); return Py_None;
*/
}
/* int main() { printf("main...\n"); } */

