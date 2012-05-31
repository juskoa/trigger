#ifndef VMEWRAP_H_INCLUDED
#define VMEWRAP_H_INCLUDED

#ifndef w81632
typedef unsigned int w32;
typedef unsigned short int w16;
typedef unsigned char w8;
#define DUMMYVAL 0xffffffff   /* recommended for DUMMY writes */
#define w81632
#endif

void Delay(int halfmicsecs);
w32 hex2ui(char *hexn);
void GetMicSec(w32 *tsec, w32 *tusec);   /* get time in seconds, micseconds */
w32 DiffSecUsec(w32 tsec,w32 tusec,w32 prevtsec,w32 prevtusec); /* rc: diff. in micsecs */
void micwait(int micsecs);   /* CPU wait */
void vmeish();
w8  vmer8(w32 offset);
w16 vmer16(w32 offset);
w32 vmer32(w32 offset);
void vmew8(w32 offset, w8 value);
void vmew16(w32 offset, w16 value);
void vmew32(w32 offset, w32 value);
int vmeopen(char *baseaddr, char *length);
int vmeclose();
w8  vmxr8(int vmesp, w32 offset);
w16 vmxr16(int vmesp, w32 offset);
w32 vmxr32(int vmesp, w32 offset);
void vmxw8(int vmesp, w32 offset, w8 value);
void vmxw16(int vmesp, w32 offset, w16 value);
void vmxw32(int vmesp, w32 offset, w32 value);
int vmxopen(int *vmesp, char *base, char *length);
int vmxopenam(int *vmesp, char *base, char *length, char *am);
int vmxclose(int vmesp);
void vmesysfail(int s);   // 0:reset !=0: set
int cctopen();
int cctclose();
#endif
