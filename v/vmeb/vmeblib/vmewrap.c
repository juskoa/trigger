/* 
13.10.2004 VMECCT modified
17.10. multiple vme spaces now possible:
vmxopen(int vmespace,char *base, char *size)  (vmespace:0..9)
vmeopen(...) == vmxopen(0,...)
vmeclose() == vmxclose(0)
vmer32(...) == vmxr32(0,...)

vmxclose(int vmespace)
vmx[rw][8,16,32]
value= vmxr32(vmespace, offset)
vmxw32(vmespace, offset, value)

9. 4. 2005 vmxopen(int *vsp, ...) was: vmxopen(int vsp)
   if vsp==-1: free vme spaces are searched. 
               The vmespace is returned in vsp parameter

22.7.2005
rccret = VME_BusErrorRegisterSignal(0)   -added in vmxclose()

---------------------------
1 of the following (defined with gcc -D$VMEDRIVER option):
AIX
VMECCT
VMERCC
SIMVME
CAENVME (added by P. Antonioli)
TSI148 (added by Y.Kharlov 30.06.2013)
*/
/* #define linux */
#if defined(linux) || defined(AIX)
  #include <stdio.h>
  /* #include <stdlib.h> */
  #include <unistd.h>
  #include <fcntl.h>
  #include <signal.h>
  #include <errno.h>
#endif 

#include "vmewrap.h"
#ifdef SIMVME 
#include "vmesim.h"
#endif
#ifdef VMERCC 
#include "rcc_error/rcc_error.h"
#include "vme_rcc/vme_rcc.h"
// #define HOST64    -defined in cmd line: -DHOST64
#endif
#ifdef VMECCT 
#include "vme_api.h"
#define DEVICE_LSI      "lsi0"
#define DEVICE_CTL      "ctl"
#define PCI_ADDRESS     0xC0000000
/*#define WINDOW_SIZE     1024*1024*7     0x1000 = 4KB */
int lsi_fd;
#endif
#ifdef CAENVME 
#include "CAENVMElib.h"

typedef struct {
  CVBoardTypes BdType;
  int handle;
  short Link;
  short BdNum;
  unsigned long BaseAddr;
  long int  status;
  unsigned long adr;
  long int value;
} cv_t; 
cv_t cv;
#endif

#ifdef TSI148
  /* Include files needed for VME driver TSI148 */
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <fcntl.h> 
  #include <unistd.h> 
  #include <stdio.h> 
  #include <string.h> 
  #include <errno.h> 
  #include <sys/ioctl.h> 
  #include <asm/byteorder.h>
  #include "vmedrv.h"
static int fTSI148;
#endif

#include <stdlib.h>
#include <string.h> 

#if defined(AIX) || defined(VMERCC) 
int handle;
#endif
u_long vmeptr=0;

/* max. 10 for VMERCC (or increase VME_MAX_BERR_PROCS in VMERCC driver srcs) */
#define MAXVMESPACES 10    
typedef struct {
  w32 baseaddr;
  int size;
  w32 am;
  int handle;
  u_long vmeptr;    /* 0: this item in vxsp[] is empty (not opened) */
} Tvmespace;
Tvmespace vxsp[MAXVMESPACES]=
{{0,0,0,0,0},
 {0,0,0,0,0},
 {0,0,0,0,0},
 {0,0,0,0,0},
 {0,0,0,0,0},
 {0,0,0,0,0},
 {0,0,0,0,0},
 /*
 {0,0,0,0,NULL},
 {0,0,0,0,NULL},*/
 {0,0,0,0,0},
 {0,0,0,0,0},
 {0,0,0,0,0}};

#ifdef VMERCC 
/*-------------------------------*/int VME_MasterMapVirtualDummyLongAddress(int lochandle, u_long *locvmeptr){
int ret;
/*#ifdef HOST64 this is to be called for the new driver*/
ret = VME_MasterMapVirtualLongAddress((int)lochandle, locvmeptr);
/* #else this is to be called for the old driver 
unsigned int ptr;
ret = VME_MasterMapVirtualAddress((int)lochandle, &ptr);
*locvmeptr = ptr;
#endif */
return ret;
};
#endif
/*---------------------------------------------*/ w32 hex2ui(char *ich) {
/* input: 023d or 023D rc: 573
   operation: convert ich to binary */
int i=0; w32 var4=0; char c;
while( (c=ich[i]) != '\0') {
  w32 ic=17;
  if( c>='0' && c<='9') ic= c-'0';
  if( c>='a' && c<='f') ic= c-'a'+ 10;
  if( c>='A' && c<='F') ic= c-'A'+ 10;
  if(ic!=17) var4= (var4<<4) | ic;
  else break;
  i++;
};
return(var4);
}

/*----------------------------------------------*/ w8 vmer8(w32 offset) {
w8 retval=0xde;
#ifdef SIMVME
  retval=vmesimr8(0,offset);
#endif

#ifdef CAENVME
cv.adr=cv.BaseAddr+offset;
cv.status= CAENVME_ReadCycle(cv.handle, cv.adr,  &(cv.value), cvA24_U_DATA,cvD8);
if (cv.status < 0) printf("CAENVME_ReadCycle32 error:%li\n", cv.status);
retval= (w8)cv.value;
#endif

#if defined(AIX) || defined(VMERCC) || defined(VMECCT)
retval= *(w8 *)(vmeptr+offset);
#endif
/*printf("vmer8 from %x value:%x\n", offset, retval);*/
return(retval);
}

#ifdef CAENVME
int getCaenVmeConf(int *type, int *link, int *num)
{
  char stconf[120];
  char bridgeDir[120];
  char *envptr,*configFile;
  FILE *caenvme_conf;
  char LinkTypeDesc[2][7]={"V1718\0","V2718\0"};
  envptr=getenv("VMEBDIR");
  if (envptr==NULL) {
    (void)fprintf(stdout,"Undefined VMEBDIR variable\n");
    exit(1);
  } else strcpy(bridgeDir,getenv("VMEBDIR"));
  configFile=strcat( bridgeDir,"/caenvme.setup");
  if (envptr!=NULL)
  errno=0;
  caenvme_conf=fopen(configFile,"r");
  if (errno) {
    (void)fprintf(stdout,"Failed to open %s file\n",configFile);
    perror("fopen"); fflush(stderr); exit(1);
  }
  while ( fgets(stconf,sizeof(stconf),caenvme_conf) != NULL ) {
    //    printf("CANVME config file %s",stconf);
    if (strncmp(stconf,"#",1) != 0) {
      (void)sscanf(stconf,"%d %d %d",type,link,num);
      printf("CAENVME Setup on %s bridge: Card #: %d Link #: %d\n",
	     LinkTypeDesc[*type],*link,*num);
    }
  }
  (void)fclose(caenvme_conf);
  return 0;
}
#endif


/*----------------------------------------------*/ w16 vmer16(w32 offset) {
w16 retval=0xdead;
#ifdef SIMVME
  retval=vmesimr16(0,offset);
#endif
#ifdef CAENVME
cv.adr=cv.BaseAddr+offset;
cv.status= CAENVME_ReadCycle(cv.handle, cv.adr,  &(cv.value), cvA24_U_DATA,cvD16);
if (cv.status < 0) printf("CAENVME_ReadCycle16 error:%li at%lx\n", cv.status,cv.adr);
retval= (w16)cv.value;
#endif

#if defined(AIX) || defined(VMERCC) || defined(VMECCT)
retval= *(w16 *)(vmeptr+offset);
#endif
/*printf("vmer16 from %x value:%x\n", offset, retval);*/
return(retval);
}

/*----------------------------------------------*/ w32 vmer32(w32 offset) {
w32 retval=0xdeaddead;

#ifdef SIMVME
  //printf("vmer32 sim: returning offset:%x\n", offset);
  //retval=vmesimr32(0,offset);
  retval=offset;
  if(offset==0x4) { // CODE_ADD
    retval=0x56;   // always ltu
  } else if(offset==0x80) { //LTUVERSION_ADD
    retval=0xb6;
  };
#endif

#ifdef CAENVME
cv.adr=cv.BaseAddr+offset;
cv.status= CAENVME_ReadCycle(cv.handle, cv.adr,  &(cv.value),cvA24_U_DATA,cvD32);
if (cv.status < 0) printf("CAENVME_ReadCycle32 error:%li at %lx\n", cv.status,cv.adr);
retval= (w32)cv.value;
#endif

#ifdef TSI148
  int n, readvalue;
  lseek(fTSI148, offset, SEEK_SET);
  n = read(fTSI148, &retval, 4);
  if(n != 4){
    printf("TSI148 vmer32 read failed at 0x%0X.  Errno = %d\n",offset, errno);
    _exit(1);
  }
  retval = (w32)(__swab32(retval));
#endif
  
#if defined(AIX) || defined(VMERCC) || defined(VMECCT)
retval= *(w32 *)(vmeptr+offset);
/*printf("vmer32 vmeptr:%x from %x value:%x\n", vmeptr, offset, retval); */
#endif
return(retval);
}
/*----------------------------------*/ void vmew8(w32 offset, w8 value) {
#ifdef SIMVME
  vmesimw8(0,offset,value);
#endif
#ifdef CAENVME
cv.adr=cv.BaseAddr+offset;
cv.status= CAENVME_WriteCycle(cv.handle, cv.adr,  &value, cvA24_U_DATA,cvD8);
if (cv.status < 0) printf("CAENVME_WriteCycle8 error:%li\n", cv.status);
 printf("CAENVME_WriteCycle8:%li %lx %lx\n", cv.status, cv.adr, cv.value);
#endif
#if defined(AIX) || defined(VMERCC) || defined(VMECCT)
*(w8 *)(vmeptr+offset)=value;
#endif
return;
}
/*----------------------------------*/ void vmew16(w32 offset, w16 value) {
#ifdef SIMVME
  vmesimw16(0,offset,value);
#endif

#ifdef CAENVME
cv.adr=cv.BaseAddr+offset;
cv.status= CAENVME_WriteCycle(cv.handle, cv.adr,  &value, cvA24_U_DATA,cvD16);
if (cv.status < 0) printf("CAENVME_WriteCycle16 error:%li\n", cv.status);
#endif

#if defined(AIX) || defined(VMERCC) || defined(VMECCT)
*(w16 *)(vmeptr+offset)=value;
#endif
return;
}
/*----------------------------------*/ void vmew32(w32 offset, w32 value) {
#ifdef SIMVME
  //printf("vmew32 sim: doing nothing:\n");
  vmesimw32(0,offset,value);
#endif
#ifdef CAENVME
cv.adr=cv.BaseAddr+offset;
cv.status= CAENVME_WriteCycle(cv.handle,cv.adr,  &value, cvA24_U_DATA,cvD32);
if (cv.status < 0) printf("CAENVME_WriteCycle32 error:%li at %lx\n", cv.status,cv.adr);
#endif

#ifdef TSI148
  int n;
  lseek(fTSI148, offset, SEEK_SET);
  value = __swab32(value);
  n = write(fTSI148, &value, 4);
  if(n != 4){
    printf("TSI148 vmew32 write failed at 0x%0X.  Errno = %d\n",offset, errno);
    _exit(1);
  }
#endif

#if defined(AIX) || defined(VMERCC) || defined(VMECCT)
*(w32 *)(vmeptr+offset)=value;
#endif
return;
}

/*---------------------*/ void vmeish() {
/* this routine should not be called inside interrupt handler (gotsignal)
It would kill the process using VMERCC driver.
*/
#if defined(VMERCC)
int rccret=0;
rccret = VME_BusErrorRegisterSignal(SIGBUS);
if (rccret != VME_SUCCESS) {
  printf("VME_BusErrorRegisterSignal(SIGB) error: %d\n",rccret);
  VME_ErrorPrint(rccret); fflush(stdout);
  exit(8);
};
#endif
}

u_int getvmeaddmod(char *vam) {
u_int rc;
#if defined(VMERCC) || defined(VMECCT)
if(strcmp(vam,"A32")==0) {rc= VME_A32; }
else if(strcmp(vam,"A24")==0) {rc= VME_A24; }
else if(strcmp(vam,"A16")==0) {rc= VME_A16; }
else {
  printf("getvmemodadd: bad VME address modifier. Using A24\n");
  rc= VME_A24;
};
#else
  printf("getvmemodadd: VME address modifier not supported for this driver\n");
  rc=-1;
#endif
return(rc);
}
#ifdef VMECCT
w32 BoBaLengthcct;
#endif

#ifdef TSI148
/* ======================================================================== */
int getTSI148Info()
{
  vmeInfoCfg_t myVmeInfo;
  int   fd, status;
  fd = open("/dev/vme_ctl", 0);
  if (fd < 0) return(1);
  memset(&myVmeInfo, 0, sizeof(myVmeInfo));
  status = ioctl(fd, VME_IOCTL_GET_SLOT_VME_INFO, &myVmeInfo);
  if (status < 0) return(1);
  close(fd);
  return(0);
}
#endif

/* ======================================================================== */
/*----*/ int vmxopenam(int *vsp, char *BoardBaseAddress, 
             char *BoardSpaceLength, char *VMEAM) {
/* 
Inputs:
   BoardBaseAddress: "0xa00000" or "VXI0::450" (VISA)
   BoardSpaceLength  "0xCB"
   VMEAM: "A32" "A24" "A16"  (only for VMERCC/CCT !)
vsp: 0,1,2,..., -> open given vmespace (0, 1, 2, ...)
     -1         -> find out unopen vmespace, open it and return
                   its number in vsp
 rc: 0 -OK, vme opened
     4 -*vsp is already opened
     8 -cannot open vme device
    10 - *vsp==-1 on input, but all the vme spaces opened
    16 -*vsp>MAXVMESPACES
 */
int i;
int rccret=0; /* For checking errors */
w32 BoBaAd,BoBaLength;

#ifdef VMERCC
int lochandle;
u_long locvmeptr;
static VME_MasterMap_t master_map = {0xa00000, 0xCB, VME_A24, 0};
#endif
#ifdef VMECCT
w8 *locvmeptr;
int lsifdi, ctl_fd, loclsi_fd;
char version[200];
w8 swapMode;
PCI_IMAGE_DATA idata;
#endif

if(*vsp>(MAXVMESPACES-1)) {
  printf("vmxopen(*vsp,...), vsp>%d\n",MAXVMESPACES-1);
  rccret= 16; goto RCRET;
};
if(*vsp==-1) {
  /* find free one: */
  for(i=1; i<MAXVMESPACES; i++) {
    if(vxsp[i].vmeptr==0) {
      *vsp= i; break;
    };
  };
  if(*vsp==-1) {
    printf("All vme %d spaces opened \n",MAXVMESPACES);
    rccret= 10; goto RCRET;
  };
} else {
  if(vxsp[*vsp].vmeptr != 0) {
    printf("Trying to open already opened vme space %d\n",*vsp);
    rccret= 4; goto RCRET;
  };
};
BoBaAd=hex2ui(&BoardBaseAddress[2]);
BoBaLength=hex2ui(&BoardSpaceLength[2]);
/*printf("base:%x Length:%x\n",BoBaAd,BoBaLength); */
#ifdef SIMVME
  if( (rccret= vmesimOpen(*vsp, BoBaAd, BoBaLength, 1))!=0 ) {
    printf("vmesimOpen() error: %d\n",rccret);
    goto EXIT8;
  };
vxsp[*vsp].vmeptr= BoBaAd;   //just for vmxclose()
#endif
//von #else 
#ifdef VMERCC
if((rccret=VME_Open()) != VME_SUCCESS) {
  printf("VME_Open() error: %d\n",rccret);
  goto EXIT8;
};
master_map.vmebus_address= BoBaAd;
master_map.window_size= BoBaLength;
master_map.address_modifier= getvmeaddmod(VMEAM);
rccret = VME_MasterMap(&master_map, &lochandle);
if (rccret != VME_SUCCESS) {
  printf("VME_MasterMap() error: %d\n",rccret);
  VME_ErrorPrint(rccret);
  goto EXIT8;
};
//rccret = VME_MasterMapVirtualAddress((int)lochandle, (u_int *)&locvmeptr);
rccret = VME_MasterMapVirtualDummyLongAddress((int)lochandle, &locvmeptr);

if (rccret != VME_SUCCESS) {
  printf("VME_MasterMapVirtualAddress() error: %d\n",rccret);
  goto EXIT8;
};
rccret = VME_BusErrorRegisterSignal(SIGBUS);
if (rccret != VME_SUCCESS) {
  printf("VME_BusErrorRegisterSignal(SIGBUS) error: %d\n",rccret);
  VME_ErrorPrint(rccret);
  goto EXIT8;
};
vxsp[*vsp].baseaddr= BoBaAd;
vxsp[*vsp].size= BoBaLength;
vxsp[*vsp].am= getvmeaddmod(VMEAM);
vxsp[*vsp].handle= lochandle;
vxsp[*vsp].vmeptr= locvmeptr;
if( *vsp==0) {
  handle= vxsp[0].handle;
  vmeptr= vxsp[0].vmeptr;
};
#endif     // --------------------------------------------------- VMERCC
#ifdef VMECCT
/* modified for tb2004:
- only ltu, ttcvi,top boards used in tb2004, i.e.
    ltu: 0x81X000
  ttcvi: 0x80X000
top: 0x800000  (actually: 0x800200)
Always open device with the length 0x10000 (64KB) and modify then
vmeptr accordingly
*/
/* BoBaLength= 0x10000;   4K with pciBusSpace 0 or 4. 64KB with 1,2,3,5,6,7 */
rccret= vme_getApiVersion(version);
/* printf("CCT API version:%s\n", version); */
  /* open CTL device */
ctl_fd= vme_openDevice(DEVICE_CTL);
if (ctl_fd < 0) {
  printf( "Cannot open device %s \n", DEVICE_CTL );
  goto EXIT8;
};
swapMode = SWAP_MASTER; // 0x0 SWAP_MASTER SWAP_SLAVE SWAP_FAST SWAP_MASK SWAP_PORT
rccret = vme_setByteSwap( ctl_fd, swapMode );
vme_closeDevice( ctl_fd  );
/*printf( "Byte swap: %x with return %d \n", swapMode, rccret );*/

/* find out, if 64K window already mapped: */
for(i=0; i<MAXVMESPACES; i++) {
  if((vxsp[i].vmeptr!=NULL) && 
    ((vxsp[i].baseaddr&0xffff0000) == (BoBaAd&0xffff0000))) {
    /* 0x0081.... LTU,   0x0080.... TTCvi */
    vxsp[*vsp].baseaddr= BoBaAd;
    vxsp[*vsp].size= BoBaLength;
    vxsp[*vsp].am= getvmeaddmod(VMEAM);
    vxsp[*vsp].handle= vxsp[i].handle;
    vxsp[*vsp].vmeptr= vxsp[i].vmeptr -
      (vxsp[i].baseaddr-(vxsp[i].baseaddr&0xffff0000)) +
      BoBaAd - (BoBaAd&0xffff0000); 
    /* vxsp[*vsp].vmeptr= vxsp[i].vmeptr -
      vxsp[i].baseaddr&0x0000f000 + BoBaAd&0x0000f000; */
    /* printf("DBG vmxop..(%d):lsi_fd:%d vmeptr:%x BoBaAd:%x\n",
        *vsp,vxsp[*vsp].handle,vxsp[*vsp].vmeptr, BoBaAd); */
    rccret=0;
    goto RCRET;
  };
};

/* prepare PCI image window (different for different type of boards) */
idata.pciAddress = PCI_ADDRESS | (BoBaAd&0xf0000);
idata.vmeAddress = BoBaAd & 0xffff0000;
//BoBaLengthcct= BoBaAd - idata.vmeAddress + 0x1000;
if(idata.vmeAddress== 0x00810000)
  idata.size = 0x10000;   /* always 0x10000 for 16 LTU (x4096) */
else
  idata.size = 0x10000;   /* always 0x10000 for TOP,ttcvi */
idata.dataWidth = VME_D32;        // data width 
idata.addrSpace = getvmeaddmod(VMEAM);  // A32
idata.postedWrites = 1;
idata.type = LSI_DATA;
idata.mode = LSI_USER;
idata.vmeCycle = 0;
idata.pciBusSpace = 0;
idata.ioremap = 1;      // 1 needed for vme_read and vme_write
  /* open the LSI device:
  Search from lsi0->lsi7 for 0x80... and 
              lsi7->lsi0 for 0x81... 
   - if 0x80 opened after 0x81 was opened, the 0x81 opened device doesn't work
     properly with VMECCT driver ! */
if( (BoBaAd & 0xff0000) == 0x800000) {
/* if(1) { */
 for(lsifdi=0; lsifdi<=7; lsifdi++) {
  char lsidev[5];
  if(lsifdi==0 || lsifdi==4) continue; 
  sprintf(lsidev,"lsi%d",lsifdi);
  loclsi_fd = vme_openDevice( lsidev );
  if( loclsi_fd < 0 ) {
    /* printf( "Cannot open %s \n", lsidev ); */
  } else {
    //printf("Using device %s loclsi_fd:%d\n", lsidev, loclsi_fd);
    goto OKLSI;
  };
 };
} else {
 for(lsifdi=7; lsifdi>=0; lsifdi--) {
  char lsidev[5];
  /*  if(lsifdi==0 || lsifdi==4) continue;  */
  sprintf(lsidev,"lsi%d",lsifdi);
  loclsi_fd = vme_openDevice( lsidev );
  if( loclsi_fd < 0 ) {
    /* printf( "Cannot open %s \n", lsidev ); */
  } else {
    printf("Using device %s loclsi_fd:%d\n", lsidev, loclsi_fd);
    goto OKLSI;
  };
 };
};
printf( "Cannot open vme. Device lsi* not available\n");
goto EXIT8;
OKLSI:
    /* enable PCI image window */
rccret = vme_enablePciImage( loclsi_fd, &idata );
if( rccret < 0 ) {
  printf( "Cannot enable PCI image window: %d \n", rccret );
  goto EXIT8;
};
/*printf("PCI image enable, vmeAddress:%x\n", idata.vmeAddress);*/
/* rccret = vme_mmap( loclsi_fd, 0, BoBaLengthcct, (u_int *)&locvmeptr); */
rccret = vme_mmap( loclsi_fd, 0, idata.size, (u_int *)&locvmeptr); 
if( rccret < 0 ) {
  printf( "Cannot map the device: %d \n", rccret );
  goto EXIT8;
};
locvmeptr= locvmeptr + (BoBaAd - (BoBaAd & 0xffff0000)); 
/*locvmeptr= locvmeptr + (BoBaAd - (BoBaAd & 0x0000f000));*/
vxsp[*vsp].baseaddr= BoBaAd;
vxsp[*vsp].size= BoBaLength;
vxsp[*vsp].am= getvmeaddmod(VMEAM);
vxsp[*vsp].handle= loclsi_fd;
vxsp[*vsp].vmeptr= locvmeptr;
if( *vsp==0) {
  vmeptr= vxsp[0].vmeptr;
};
/*printf("DBG vmxopen(%d):lsi_fd:%d vmeptr:%x BoBaAd:%x\n",
		*vsp,loclsi_fd, locvmeptr, BoBaAd); */
#endif    //------------------------------------------------------- VMECCT
#ifdef AIX
vmeptr=(w8 *)MapVME(BoBaAd,BoBaLength);
/*void (*signal(int signum, void (*handler)(int)))(int); 
 typedef void (*sighandler_t)(int);
       sighandler_t signal(int signum, sighandler_t handler);
*/
/*
printf("AIX MapVME() (length 0x1000) ok, boardbase:%x(%s) vmeptr:%x\n",
  BoBaAd, BoardBaseAddress, vmeptr); */
#endif
//von #endif   /* SIMVME */

#ifdef CAENVME
  int type,link,num,rc;
  rc=getCaenVmeConf(&type,&link,&num); 
  cv.BdType=(CVBoardTypes)type; 
  cv.Link=(short)link;              
  cv.BdNum=(short)num;
  cv.handle=0; 
  cv.status=0;
  rccret=0;
  rccret= CAENVME_Init(cv.BdType, cv.Link, cv.BdNum, (int32_t*)&(cv.handle));
  printf( "CAENVME_Init: %d, %d %d %d %d \n", rccret, cv.BdType, cv.Link, cv.BdNum, cv.handle );   
  if (rccret<0){
    printf( "CAENVME_Init: %d \n", rccret );    
    goto EXIT8;
  }
  printf( "CAENVME_Init: %d, %d %d %d %d \n", rccret, cv.BdType, cv.Link, cv.BdNum, cv.handle ); 
 cv.BaseAddr=BoBaAd;
#endif

#ifdef TSI148
/* Interface to VME chip Tundra Tsi148 */
/* Contact: Yuri Kharlov. 30.06.2013   */
//----------------------------------------------------------------------
  int fdCtl, status, window;
  vmeOutWindowCfg_t vmeOut;
    
  if(getTSI148Info()){
    printf("getTSI148Info failed.  Errno = %d\n", errno);
    _exit(1);
  }

  fTSI148 = open("/dev/vme_m0", O_RDWR);
  if(fTSI148 < 0){
    printf("TSI148 open /dev/vme_m0 failed.  Errno = %d\n", errno);
    _exit(1);
  }

  window = 0;
  memset(&vmeOut, 0, sizeof(vmeOutWindowCfg_t));

  vmeOut.windowNbr        = 0;
  vmeOut.windowEnable     = 1;
  vmeOut.windowSizeU      = 0;
  vmeOut.windowSizeL      = 1<<16; // VME window size, assume 16 bitsh
  vmeOut.xlatedAddrU      = 0;
  vmeOut.xlatedAddrL      = hex2ui(&BoardBaseAddress[2]); // LTU base address
  vmeOut.bcastSelect2esst = VME_SSTNONE;
  vmeOut.wrPostEnable     = 1;
  vmeOut.prefetchEnable   = 1;
  vmeOut.xferRate2esst    = VME_SSTNONE;
  vmeOut.addrSpace        = VME_A24;
  vmeOut.maxDataWidth     = VME_D32;
  vmeOut.xferProtocol     = VME_SCT;
  vmeOut.userAccessType   = VME_SUPER;
  vmeOut.dataAccessType   = VME_DATA;

  status = ioctl(fTSI148, VME_IOCTL_SET_OUTBOUND, &vmeOut);
  if(status < 0){
    printf("TSI148 ioctl failed.  Errno = %d\n", errno);
    return(status);
  }
  printf("TSI148 successfully opened and configured\n");
  rccret = status;
#endif
RCRET:
//printf("vmxopen rccret:%d vsp: %d\n", rccret, *vsp);
return(rccret);
EXIT8:
#ifndef CAENVME
printf("Base address:%x Length: %d\n",BoBaAd, BoBaLength);
#endif
return(8);
}
/*----*/ int vmxopen(int *vsp, char *BoardBaseAddress, 
             char *BoardSpaceLength) {
char a24[]="A24";
#ifdef TSI148
  printf("\t YK-1\n");
#endif
return( vmxopenam(vsp, BoardBaseAddress, BoardSpaceLength, a24));
}


/*-----------*/ int vmeopen(char *BoardBaseAddress, char *BoardSpaceLength) {
  int rc;
#ifdef TSI148
  printf("\t YK-0: BoardBaseAddress=%s, BoardSpaceLength=%s\n",BoardBaseAddress,BoardSpaceLength);
#endif
#ifndef CAENVME
 int vsp0=0;
#endif
#ifdef SIMVME
rc= vmxopen(&vsp0, BoardBaseAddress, BoardSpaceLength);
#endif
#ifdef VMERCC
rc= vmxopen(&vsp0, BoardBaseAddress, BoardSpaceLength);
handle= vxsp[0].handle;
vmeptr= vxsp[0].vmeptr;
#endif
#ifdef VMECCT
rc= vmxopen(&vsp0, BoardBaseAddress, BoardSpaceLength);
vmeptr= vxsp[0].vmeptr;
#endif
#ifdef CAENVME
  int type,link,num;
  rc=getCaenVmeConf(&type,&link,&num); 
  cv.status=0;
  cv.BdType=(CVBoardTypes)type; 
  cv.Link=(short)link;              
  cv.BdNum=(short)num;
  cv.handle=0; 
  rc= CAENVME_Init(cv.BdType, cv.Link, cv.BdNum, (int32_t*)&(cv.handle));
  if(rc<0){
    printf( "CAENVME_Init: %d \n", rc );    
    //    goto EXIT8;
    return(rc);
  }
  printf( "CAENVME_Init: %d, %d %d %d %d \n", rc, cv.BdType, cv.Link, cv.BdNum, cv.handle ); 
#endif
return(rc);
}

/*---------------------------------------------*/ int vmxclose(int vsp) {
int rccret=0; 
#ifdef VMECCT 
w8 *locvmeptr;
int i, lsi_fd;
#endif
if(vsp>(MAXVMESPACES-1)) {
  printf("vmxclose(vsp,...), vsp>%d\n",MAXVMESPACES-1);
  rccret= 16; goto RCRET;
};
if(vxsp[vsp].vmeptr == 0) {
  printf("vmxclose(vsp,...), vsp:%d was not open.\n",vsp);
  rccret= 16; goto RCRET;
};
#ifdef SIMVME
if( (rccret= vmesimClose(vsp))!=0) {
  printf("vmesimClose() error: %d\n",rccret);
};
#endif
#ifdef VMERCC
rccret = VME_MasterUnmap(vxsp[vsp].handle);
if (rccret != VME_SUCCESS) {
  printf("VME_MasterUnmap() error: %d\n",rccret);
};
rccret = VME_BusErrorRegisterSignal(0);
if (rccret != VME_SUCCESS) {
  printf("VME_BusErrorRegisterSignal(0) error: %d\n",rccret);
  VME_ErrorPrint(rccret);
};
rccret = VME_Close();
if (rccret != VME_SUCCESS) {
  printf("VME_Close() error: %d\n",rccret);
};
#endif
#ifdef AIX
UnmapVME();
#endif

#ifdef VMECCT 
/* find out whether the window is used: */
for(i=0; i<MAXVMESPACES; i++) {
  if(vxsp[i].vmeptr!=NULL && (i != vsp) &&
    ((vxsp[vsp].baseaddr&0xffff0000) == (vxsp[vsp].baseaddr&0xffff0000))) {
    /* the window is still used: */
    /* printf(" vmxclo..(%d):lsi_fd:%d vmeptr:%x\n",
      vsp,vxsp[vsp].handle, vxsp[vsp].vmeptr); */
    goto RCRET;
  };
};
lsi_fd= vxsp[vsp].handle;
locvmeptr= vxsp[vsp].vmeptr - 
  (vxsp[vsp].baseaddr - (vxsp[vsp].baseaddr & 0xffff0000));
/*printf(" vmxclose(%d):lsi_fd:%d vmeptr:%x\n",vsp,lsi_fd, locvmeptr);*/
/* rccret = vme_unmap( lsi_fd, (u_int )locvmeptr, BoBaLength); */
rccret = vme_unmap( vxsp[vsp].handle, (u_int )locvmeptr, 0x10000); 
if( rccret < 0 ) {
  printf( "Cannot unmap PCI image window \n" );
};
rccret = vme_disablePciImage( lsi_fd );
if( rccret != 0 ) {
  printf( "Cannot disable PCI image window \n" );
};
  /* close LSI device */
vme_closeDevice( lsi_fd ); 
/*printf( "Device %s is closed: %d \n", DEVICE_LSI, lsi_fd );*/
//usleep(1000000); -no help
  /* close CTL device */
/* vme_closeDevice( ctl_fd  ); */
/*printf( "Device %s is closed: %d \n", DEVICE_CTL, ctl_fd );*/
#endif

#ifdef CAENVME
 rccret=CAENVME_End(cv.handle);
 if(rccret<0)printf("CAENVME_End ret %d \n",rccret);
 else printf("CAENVME_End ret %d \n",rccret);
#endif

RCRET:
vxsp[vsp].baseaddr= 0;
vxsp[vsp].vmeptr=0;
/*printf("vmxclose rccret:%d vsp: %d\n", rccret, vsp); */
return(rccret);
}
/*---------------------------------------------*/ int vmeclose() {
int rc;
rc= vmxclose(0);
vmeptr=0;
return(rc);
}
/*--------------------------------------*/ int cctopen() {
#ifdef VMECCT
extern char BoardBaseAddress[];
extern char BoardSpaceLength[];
return(vmeopen(BoardBaseAddress,BoardSpaceLength));
#else
return(0);
#endif
}
/*---------------------------------------*/ int cctclose() {
#ifdef VMECCT
return(vmeclose());
#else
return(0);
#endif
}
/*----------------------------------------*/ w8 vmxr8(int vsp, w32 offset) {
w8 retval=0xde;

#if defined(AIX) || defined(VMERCC) || defined(VMECCT)
retval= *(w8 *)(vxsp[vsp].vmeptr+offset);
#endif
return(retval);
}
/*--------------------------------------*/ w16 vmxr16(int vsp, w32 offset) {
w16 retval=0xdead;
#if defined(AIX) || defined(VMERCC) || defined(VMECCT)
retval= *(w16 *)(vxsp[vsp].vmeptr+offset);
#endif
return(retval);
}

/*---------------------------------------*/ w32 vmxr32(int vsp, w32 offset) {
w32 retval=0xdeaddead;
if(vxsp[vsp].vmeptr==0) {
  printf("vmxr32: VME space %d not opened\n",vsp);
} else {
#if defined(AIX) || defined(VMERCC) || defined(VMECCT)
retval= *(w32 *)(vxsp[vsp].vmeptr+offset);
#endif
};
return(retval);
}
/*----------------------------*/ void vmxw8(int vsp, w32 offset, w8 value) {
#ifdef SIMVME
  vmesimw8(vsp,offset,value);
#endif
#if defined(AIX) || defined(VMERCC) || defined(VMECCT)
*(w8 *)(vxsp[vsp].vmeptr+offset)=value;
#endif
return;
}
/*----------------------------*/ void vmxw16(int vsp, w32 offset, w16 value) {
#ifdef SIMVME
  vmesimw16(vsp,offset,value);
#endif
#if defined(AIX) || defined(VMERCC) || defined(VMECCT)
*(w16 *)(vxsp[vsp].vmeptr+offset)=value;
#endif
return;
}
/*----------------------------*/ void vmxw32(int vsp, w32 offset, w32 value) {
if(vxsp[vsp].vmeptr==0) {
  printf("vmxw32: VME space %d not opened\n",vsp);
} else {
#ifdef SIMVME
  vmesimw32(vsp,offset,value);
#endif
#if defined(AIX) || defined(VMERCC) || defined(VMECCT)
*(w32 *)(vxsp[vsp].vmeptr+offset)=value;
#endif
};
return;
}
/*----------------*/ void vmesysfail(int s) {
#if defined(VMERCC)
if(s!=0) {
  VME_SysfailSet();
} else {
  VME_SysfailReset();
};
#endif
}
