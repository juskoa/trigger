#ifndef w81632
typedef unsigned int w32;
typedef unsigned short int w16;
typedef unsigned char w8;
#define w81632
#endif
#define VMEDARWMASK 0x3
#define VMEDAR      0x1
#define VMEDAW      0x2
#define VMEDAWIDTHMASK 0x70
#define VMEDA8     0x10
#define VMEDA16    0x20
#define VMEDA32    0x40
typedef int (*TspecVmeF)(w8 *vmespace,w32 siz,int offset, w32 rw81632flags);
/*
#define specVmeFR 1
#define specVmeFW 2
*/
int vmesimOpen(int vmespace, w32 baseaddr, w32 size, w32 am);
int vmesimClose(int vmespace);
int vmesimreg(TspecVmeF pf, int vmespace, w32 vmeaddr, w32);
w8  vmesimr8(int vmespace,w32 offset);
w16 vmesimr16(int vmespace,w32 offset);
w32 vmesimr32(int vmespace,w32 offset);
void vmesimw8(int vmespace,w32 offset, w8 value);
void vmesimw16(int vmespace,w32 offset, w16 value);
void vmesimw32(int vmespace,w32 offset, w32 value);

