/* Compile/link toobusy.exe:
g++ -DBUSYEXE -I $VMEBDIR/vmeblib -I $VMECFDIR/ctp -I$VMECFDIR/ctp_proxy toobusy.c -Lctplib -lctp -L $VMEBDIR/vmeblib -lvmeb -L/lib/modules/daq -lvme_rcc -lrcc_error -lio_rcc -lcmem_rcc  -o toobusy.exe
*/ 
#include <stdio.h>
#include <stdlib.h>
#include "vmewrap.h"
#include "ctp.h"
#include "ctplib.h"   //getCounter()
#include "vmeblib.h"
#include "ssmctp.h"
#ifdef BUSYEXE
// is in ctp.c , we need it here if toobusy.exe compiled/linked
#define DBMAIN
#endif
#include "../ctp_proxy/Tpartition.h"
#include <unistd.h>

//void mysleep(w32 delta);

w32 CountTime();

/*FGROUP TooBUSY
Reading the detector or cluster currently selected
*/
void readMINIMAXSel(){
 w32 data;
 data=vmer32(MINIMAX_SELECT);
 //printf("MINIMAX_SELECT value= 0x%x read.\n",data);
}
/*FGROUP TooBUSY
Write the detector or cluster number to select
*/
void writeMINIMAXSel(int word){
 vmew32(MINIMAX_SELECT,word);
 //printf("MINIMAX_SELECT value= 0x%x written.\n",word);
}
/*FGROUP TooBUSY
Clear the readMINMAX
*/
void writeMINIMAXClear(){
  w32 word;
  word = 22;
 vmew32(MINIMAX_CLEAR,word);
 //printf("MINIMAX_CLEAR value= 0x%x written.\n",word);
}
/*FGROUP TooBUSY
Read the min and max busies since the last clear
*/
void readMINMAX(){
 w32 min,max;
 min=rounddown(0.4*vmer32(BUSYMINI_DATA));
 max=rounddown(0.4*vmer32(BUSYMAX_DATA));
  printf("MIN= %i MAX=%i \n",min,max);
}
/*FGROUP TooBUSY
The current maximum busy in microseconds for busylong counter
*/
void readMINIMAXLimit(){
 w32 data;
 data=rounddown(0.4*vmer32(MINIMAX_LIMIT));
 //printf("MINIMAX_LIMIT value= %i read.\n",data);
}
/*FGROUP TooBUSY
Set the maximum busy in microseconds for busylong counter
*/
void writeMINIMAXLimit(w32 word){
  w32 value = rounddown(word/0.4);
 vmew32(MINIMAX_LIMIT,value);
 //printf("MINIMAX_LIMIT value= %i written.\n",word);
}
/*FGROUP TooBUSY
Enter the time you wish to wait between busylong reads in seconds and the counter
 will display how many times the detector selected has exceeded the limit written in that time.
*/
void readBUSYlong(w32 delay){
 w32 counts1,counts2;
counts2 = getCounter(0, 104, ccread_ctp);
usleep(delay*1000000);
counts1 = getCounter(0, 104, ccread_ctp)-counts2;
  printf("There have been %i busies exceeding the current limit \n",counts2);
}


/*FGROUP TooBUSY 
Sweep of busylong.
Rangemax is the maximum minimax_limit of the sweep in microseconds.
  -maximum average busy
Rangemin (try zero to start) is the first minimax_limit set.
Stepsize is size of the increase in MINIMAX_limit steps ie the bin size in microseconds.
The sweep time is the time in seconds spent waiting on each datapoint.
Detector is the bit number in MINIMAX_SELECT word:
0       CTP BUSY
1-24    Sub-detector 1 to 24 BUSY (Note 2)
(this is the 4th row in VALID.LTUs)
25-30   Cluster 1 to 6 BUSY (Note 3)
31       Test cluster BUSY

rc: 0: ok
    1: cannot open busysweep
*/
int busytool(int rangemax, int rangemin, int stepsize, int sweeptime, int detector){
  int currmax = 0;
  int currmin = 0; 
  int increase = 0;
    int currmaxi = 0;
    int currmini = 0;
  int resetsugg = 0;
  int i = 0;
  FILE *myfile;
  w32 timetotake = 0;
  w32 busylong = 0;
  w32 busylonga = 0;
  w32 busyprevious = 0;
  w32 sweeptimemicrosec = 0;
  w32 newmax = 0;

writeMINIMAXClear();
writeMINIMAXSel(detector);
readMINIMAXSel();

currmax = rangemax; 
 currmin=rangemin;
 //or do we just set it equal to the default in the definition?
  // make currmax something which is calculated by measuring mean BUSYMAX, or as an input.
   currmaxi=rounddown(1.*currmax/stepsize);
  //again need default stepsize
    currmini=rounddown(1.*currmin/stepsize);
 
//w32 busylongold = getCounter(0, 104);
 
// printf("first busylongold read is: %i \n", busylongold);
// w32 busylong = getCounter(0, 104);

myfile=fopen("busysweep","w");
if(myfile==NULL) {
  printf("cannot open busysweep\n");
  return(1);
}; 
timetotake = (sweeptime*(currmaxi-currmini));
printf("The time for this sweep will be %i s.\n", timetotake);
fprintf(myfile,"%d\n", currmax); 
fprintf(myfile,"%d\n", currmin); 
fprintf(myfile,"%i\n", (currmaxi-currmini)); 

if (timetotake>1800){
  printf("This will take longer than half an hour. Aborting...");
} else {
  printf("   us\t diff\t Max\n");
  for(i = currmini; i<currmaxi; i++){
    w32 limit;
    writeMINIMAXClear();  
    //set limit in increasing steps
    limit= stepsize*i;
    writeMINIMAXLimit(limit);
    readMINIMAXLimit();
    //measure busy increments
    sweeptimemicrosec = sweeptime*1000000;
    //busylong = busylong - getCounter(0, 104, ccread_ctp);
    busyprevious = getCounter(0, 104, ccread_ctp);
    //printf("first busy count %i \n", busyprevious);
    usleep(sweeptimemicrosec); 
    //only, and set a default sweeptime;
    //measure busy increments
    busylonga = getCounter(0, 104, ccread_ctp);
    if(busylonga>=busyprevious){
      busylong = (busylonga-busyprevious);
    } else busylong = busylonga + (0xffffffff-busyprevious)+1;
    //printf("second busy count %i \n", busylonga);
    //printf("diff of busy count %i \n", busylong);

    // printf("BusyLONG counter reads %i \n", busylong);
    //print to file
    // value = busylast - busylong;
    fprintf(myfile,"%i\n",busylong);

    //check if any bigger busies than currmax are seen
    newmax=rounddown(0.4*vmer32(BUSYMAX_DATA));
    printf("%5d\t %d\t %d\n", limit, busylong, newmax); 
    //send warning if busy exceeds sweep range
    //print new busylong - previous busylong into a file....
    if (newmax > (w32)currmax){
      increase=1;
      resetsugg = newmax;
      printf("Warning: Maximum busy %i exceeds limits of sweep range %i \n", newmax, currmax);
    };
  };
  if(increase == 1){
    printf("Maximum busy exceeded limits of sweep range. Suggest to increase to %i \n", resetsugg);
    // insert here a tool to open a window with inputs y or n? So if y, currmax = newmax, else continue? Can I do that?
  }
};
fclose(myfile);
return 0;
}

#ifdef BUSYEXE
/* Compile/link toobusy.exe:
g++ -DBUSYEXE -I $VMEBDIR/vmeblib -I $VMECFDIR/ctp -I$VMECFDIR/ctp_proxy toobusy.c -Lctplib -lctp -L $VMEBDIR/vmeblib -lvmeb -L/lib/modules/daq -lvme_rcc -lrcc_error -lio_rcc -lcmem_rcc  -o toobusy.exe
*/ 

int main(int argn, char **argv) {
int ix,rccret,rcbt,vsp=0; w32 l0ver;
int par[5];
if(argn!=6) {
  exit(12);
};
for(ix=1; ix<argn; ix++) {
  printf("%d: %s\n", ix, argv[ix]);
  par[ix-1]= atoi(argv[ix]);
};
micwait(1);
rccret= vmxopenam(&vsp, "0x820000","0xd000","A24");
if(rccret!=0) {
  printf("vmeopen rc:%d\n", rccret); exit(8);
};
l0ver= vmer32(FPGAVERSION_ADD+0x9000); printf("L0 fpga ver:0x%x\n", l0ver);
cshmInit();
/*for(ix=0; ix<4; ix++) {
  printf("CTPinput[%d]:%s\n", ix, validCTPINPUTs[ix].name);
};*/
rcbt= busytool(par[0], par[1], par[2], par[3], par[4]);
//rcbt=22;
rccret= vmeclose();
return(rcbt);
}
#endif

