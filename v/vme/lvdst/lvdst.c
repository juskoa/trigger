/*BOARD lvdst 0x810000 0x800 */
/*
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "vmewrap.h"
#include <iostream>
#include <fstream>

using namespace std;

#define LTUMAIN

#include "lvdst.h"

extern int quit;

//------------------------ karim ---------------------------//
#include <sys/time.h>
#include "../ltu/ltu.h"

void myscan(int micseconds, int* & buf,int print)
{
 int i;
 if(!buf) buf = new int(32);
 int imax=0,imin=0;
 int val;
 if(print==0) printf("************** ADC scan *****************\n");
 for(i=0;i<32;i++){
  setbcdelay(i, 1); //Do not change TTC_INTERFACE word during scan !
  vmew32(PLL_RESET, DUMMYVAL);
   //usleep(10000);
  if(micseconds>0) usleep(micseconds);
  while(1) {
    w32 bcs;
    bcs= vmer32(BC_STATUS);
    if(bcs & BC_STATUSpll) break;
   };
   val=readadc_s();
   buf[i]=val;
   if(val>buf[imax]) imax=i;
   if(val<buf[imin]) imin=i;
   //printf("i=%i, val=%i \n",i,buf[i]);
  if(print==0){
	printf("%d", buf[i]);
	if(i<31) printf(", ");
	else printf("\n");
  }
 }; 
 //printf("max=%i %i    min=%i %i \n",imax,buf[imax],imin,buf[imin]);
 if(print==0) printf("*****************************************\n");
}
//----------------------------------------------------------//

//void loadFPGA();
void rndtestA(int *values)
{
 // do what necessary
 printf("fix me \n");
}

//w32 Gltuver;   // should be 0xe?
w32 getbcstatus();

/*FGROUP TOP GUI ScopeAB "Scope Signals"
Signal selection for front panel 
A,B outputs
*/
/*FGROUP TOP GUI Counters
Counters monitor */

/* moved to ltulib FGROUP FrontPanel 
A,B: 2x5bits for Scope A,B outputs
setAB(23,23) -no output selected
void setAB(w32 A, w32 B) {
vmew32(SCOPE_SELECT, (B<<5) | A);
} 
*/
/*FGROUP FrontPanel 
*/
void getCounters(int N, int increments,int baker);
/*FGROUP FrontPanel 
*/
void clearCounters();

void openRoot() {
printf("Test !\n");


}

//**********************************************
/*FGROUP LVDST tests 
Tp - Transition of the Synchronised Pattern Signal
From here you will be able to make a ADC scan for the 
pattern signal. The pattern is setted to be TOGGLE. 
The aim of this function is to find something known 
as the exclusion zone of the BC_DELAY_ADD register. 
This zone correspond to the worst possible setting for that 
register. This is an hardware dependence constant.

print option
	0  --> Print some comments
	1  --> Don't print anything

 */
int Tp_transition(int print) {
if (print == 0 ) {
	printf("*****************************************\n");
	printf("****  STEP 1: Finding the exclusion zone - For PATTERN Syn. \n");
	printf("****\n");
}
int i, j, number=100,w;
int * array = new int(32);
//For scope
//setAB(2,9);
//PATTERN_SEL, TOGGLE option
vmew32(PATTERN_SEL,0x3);
//Select synchronisation with a positive BC edge
vmew32(SYN_EDGE,0);
//ADC_pattern signal as the ADC input
vmew32(ADC_SELECT,0x0);
//ADC_SCAN
getbcstatus();
myscan(1,array,print);

//Find the transition
for (i=1; i<32; i++) {
        w = array[i];
        if(0<= w && w<array[i-1] && w<number) {
                number = w;
                j  = i;
        }
}
if (j != 17) printf("Warning: Tp is not 17 !!! \n");
if (print == 0 ) {
	if (j != 0) printf("A transition has been observed in %d ns \n",j);
	printf("This suggest that the exclusion zone of the BC_DELAY_ADD register should be +- 4 ns that value.\n");
	printf("This exclusion zone is a hardware dependence constant.\n");
//	printf("It is recommend it to confirm this result making NOW a ADC_SCAN.\n");
}
delete [] array;
return(j);
}
/*
Tc - transition of the Synchronised Cable Signal
From here you will be able to do a ADC scan for the cable_1 signal. 
The pattern is setted to be TOGGLE. This result is a cable dependence. 
For a different cable, you might find a different number.
 */
void Tc_transition(int cable, int *values1, int print) {
int kTransitionValue;
kTransitionValue=Tp_transition(print);

if (print == 0) {
	printf("*****************************************\n");
	printf("**** STEP 2: Find the best setting for the BC_DELAY_ADD register.\n");
	printf("****\n");
}

int i, j, w=-1, number=100, count=0;
int * array = new int(32);
//scope
//setAB(2,9);
//Select the toggling pattern, PATTERN_SEL
vmew32(PATTERN_SEL,0x3);
//Note: the BC edge is irrelevant for this measurement.
if (print==0) {
	cout << "///////////////////////////////////////////" << endl;
	cout << "// \t Analysing cable #" << cable << "...    \t //" << endl;
	cout << "///////////////////////////////////////////" << endl;
}
//Select the ADC_CABLE signal as the ADC input. Select 1(2) for cable 1(2).
//vmew32(ADC_SELECT,1);//for cable#1 
//vmew32(ADC_SELECT,2);//for cable#2
vmew32(ADC_SELECT,cable);
//ADC_SCAN - the phase shift of the toggling cable 1(2) input 
//in respect to the BC clock
getbcstatus();
myscan(1,array,print);
//Find a transition
for (i=1; i<32; i++) {
        w = array[i];
        if(0<=w && w<array[i-1] && w<number) {
                number = w;
                j  = i;
        }
	if (w == 0) {
		count = count + 1;
		if (count == 1) values1[1] = i;
		if (count == 2 && (i<values1[1]-3 || i>values1[1]+3)) { 
				  values1[2] = i;
				  values1[0] = 2;
		} else { 
			values1[0] = 1;
			count = 1;
		  }
	}
}
if (print == 0) {
	if (j != 0)  printf("A transition has been observed in %d ns\n",values1[1]); 
//	printf("It is recommend it to confirm this result making NOW a ADC_SCAN.\n\n");
}
//return(j);
 delete [] array;
 return;
}
/*FGROUP LVDST tests
Ts - The best setting for the BC_DELAY_ADD register
 */
int Ts_transition(int cable, int print) {
int Tc=0, kTs=0, values1[2];
Tc_transition(cable,values1,print);
Tc = values1[1];
if (Tc >=12) { kTs = Tc - 12;
} else kTs = Tc + 12;
return(kTs);
}
/* 
Function to get the Edge Sign.
The aim of this function is to find if the setting is in or out the exclusion zone.
Point inside the exclusion zone are defined with a negative edge (1)
Note: For this the Tp value is needed.
 */
int Edge_selection(int print, int Setting, int Tp) {
int kSignOfZone = -1;
if (Setting >= 0 && Setting <=32) {
	if (Tp != 17) printf("Warning: Tp is not 17 !!! (current value: %i)\n",Tp);
	if (Setting>=(Tp-3) && Setting<=(Tp+3)) { kSignOfZone = 1; } 
	else kSignOfZone = 0;
	if (print == 0) {
		if (kSignOfZone == 1) printf("The edge is negative \n");
		else printf("The edge is positive\n"); 
	}
} else printf("WARNING: Incorrect setting value (=%i)\n",Setting);
return(kSignOfZone);
}
/*FGROUP LVDST tests
For Pattern delay measurement. 
The aim of this function is to find the correct settings for the
DELAY_1 register. This function uses the results from previous Syn functions.
*/
int D_pattern_delay(int cable, int print) {

int kTp = -1, kTs =-1, kDelay=0, kSignOfZone=-1;
w32 mystatus[12], mystatus1[12];
//w32, kCableErrors, kSequenceStrobes;
int k;
float number, arrayCableErrors[33],arraySequenceStrobes[33];
float arrayRatio[33];
kTs = Ts_transition(cable,print);
kTp = Tp_transition(1);

if (print == 0) {
	printf("*****************************************\n");
	printf("**** STEP 3: Pattern Delay Measurement. \n");
	printf("****\n");
	printf("The aim of this program is to find the correct settings for the DELAY_%i register.\n",cable);
}

kSignOfZone = Edge_selection(1,kTs,kTp);
//Select the SEQUENCE pattern
vmew32(PATTERN_SEL,0x1);
//Select the ADC_cable signal as the ADC input. Note: 1 for cable 1
vmew32(ADC_SELECT,cable);
//Select the Sequence with all 24 bits asserted
vmew32(SEQ_DATA,0xffffff);
//Select the sequence period - about 1.5 microSeconds (60 BC intervals)
//The period =60 is long enough to avoid the sequence overlapping for cable length
//of up to 160m
//vmew32(SEQ_PERIOD,60);
vmew32(SEQ_PERIOD,31); //karim
//vmew32(SEQ_PERIOD,23); //karim
//Scope signal. A=Sequence strobe and B=delayed_pattern1
//setAB(18,4);
//Set BC_DELAY_ADD - Value found previously on Ts_transition
vmew32(BC_DELAY_ADD,kTs);
//Apply the Edge rule to select the BC edge
vmew32(SYN_EDGE,kSignOfZone);
//Delay is done in BC units
for (k=0; k<32; k++) {
	if(cable==1) vmew32(DELAY_1,k);
	else if(cable==2) vmew32(DELAY_2,k);
	else cout << "WARNING: wrong cable number (=" << cable << ")" << endl;
	//wait for 100 ms
        usleep(100000);
	//clearCounters();
	readCounters(mystatus1,12,0);
	//Sleep a bit - get more statistics
	usleep(100000);
	readCounters(mystatus,12,0);
	if(cable==1) arrayCableErrors[k]=mystatus[8] - mystatus1[8]; // Cable1-errors
	else if(cable==2) arrayCableErrors[k]=mystatus[9] - mystatus1[9]; // Cable2-errors
	else cout << "WARNING: wrong cable number (=" << cable << ")" << endl;
	//Number of sequence strobes - number of patterns generated
        arraySequenceStrobes[k]=mystatus[11] - mystatus1[11];
	//cout << arrayCableErrors[k]<< " " << " " << arraySequenceStrobes[k] << endl;
	arrayRatio[k] = (1.0*arrayCableErrors[k])/arraySequenceStrobes[k];
	if (print == 0) {
		/*
		printf("For setting %d\n",k-1);
		printf("CableError %d\n",arrayCableErrors[k]);
		printf("SequenceStrobes %d\n",arraySequenceStrobes[k]);
		printf("ratio is %f\n ",arrayRatio[k]);
		printf("\n");
		*/
		//printf("<%i> <%f> \n",k,arrayRatio[k]);
		printf("%f", arrayRatio[k]);
		if(k<31) printf(", ");
		else printf("\n");
	}
}
number=10000.0;
for (k=0; k<32; k++) {
        if(arrayRatio[k]<number) {
		number = arrayRatio[k];
                kDelay  = k;
        }
}
if (print == 0) {
	printf("DELAY_%i should be %i\n",cable, kDelay);
	printf("Number of errors %f\n",number);
}

const int real_delay = 28;
if(cable==1 && kDelay!=real_delay) {
	cout << "WARNING: delay is not "<< real_delay <<"! forcing it to "<< real_delay << endl;
	kDelay=real_delay;
}

//Set the correct value for DELAY_1/2 register
if(cable==1) vmew32(DELAY_1,kDelay);
else if(cable==2) vmew32(DELAY_2,kDelay);
else cout << "WARNING: wrong cable number (=" << cable << ")" << endl;

return(kDelay);
}
//}
void selection (int setting, int delay, int Tp, int cable){
 int kEdge; 
 kEdge = Edge_selection(1,setting,Tp);
 vmew32(SYN_EDGE,kEdge);
 if(cable==1) vmew32(DELAY_1,delay);
 else if(cable==2) vmew32(DELAY_2,delay);
 else cout << "WARNING: wrong cable number (=" << cable << ")" << endl;
 vmew32(BC_DELAY_ADD,setting);
 printf("Setting for %d\n",setting);
 printf("Edge %d\n",kEdge);
 printf("Delay: %d\n",delay);
}

/*FGROUP LVDST
WindowFinding function.
This is a general function: functions using pre-set input values are availables (e.g. Find_Sampling_Window).
Delay done in Nano-seconds. We will start from the best possible point according 
to the cable. This value is obtained by the CableSignalSyn function.

kFrom and kTo     this defind the measure range.
		  A full test will be from kFrom=0 to kTo=31

seqTpye 
	--> 1  for sequence pattern  0x800000
	--> 2  for sequence patter   0xaaaaaa
	--> 3  for random pattern

runType 
	-->1    This correspond to a normal run (53 secs) 
		Type time=0 and timeUnits='n'
        --> 0   Time and timeUnits defind by the user.
                timeUnits   's' for seconds
		  	    'h' for hours
e.g. runType=0, time=1 and timeUnits='s' correspond to 1 sec run for each measure


NOTE: To stop this program, please press CONTROL + X
*/
void Find_Window(int cable, int fromSetting, int toSetting, int seqType, int runType, int time, char timeUnits, char * outfile) {
if (runType == 3) {
        time  = 0;
        timeUnits = 'n';
 } 
const char * outfile_const = outfile;
ofstream fw(outfile_const,ios::out);

int kTp, 
    kTc,
    kTs,
    kDelay, 
    count=0,
    sleep, 
    k, 
    //w,
    arrayCableErrors2[32],
    arraySequenceStrobes2[32], 
    arrayElapsedTime[32];
float arrayRatio2[32], 
      arrayTemp[32];
int val1[2], kTch;
w32 mystatus1[12], mystatus2[12], mystatus3[12];
//Call Syn functions
Tc_transition(cable,val1,1);
kTp = Tp_transition(1);
if (kTp != 17 ) printf("W: Tp != 17 - Please check this value !!! \n");
kTc = val1[1];
if (val1[0] == 2)  {kTch = val1[2]; } else kTch = 0;
if (kTc >= 12) { kTs = kTc -12; }
else kTs = kTc + 12;  
//printf("The delay should be in %d\n ",kDelay);
//vmew32(PATTERN_SEL,0x1);
vmew32(ADC_SELECT,cable);
//Sequence - It should be a RANDOM pattern for the final test
// H'AAAAAA'
//vmew32(SEQ_DATA,11184810);
kDelay = D_pattern_delay(cable,0);
if (seqType == 1) { 
vmew32(PATTERN_SEL,0x1);
vmew32(SEQ_DATA,0x800000);
vmew32(SEQ_PERIOD,240);
}
else if (seqType == 2) {
  vmew32(PATTERN_SEL,0x1);
        vmew32(SEQ_DATA,0xaaaaaa);
        vmew32(SEQ_PERIOD,240);
}
else if (seqType == 3) {
		vmew32(PATTERN_SEL,0x2);
		vmew32(RANDOM_RATE,0x3fffffff);
}
else if (seqType == 4) {
  vmew32(PATTERN_SEL,0x1);
        vmew32(SEQ_DATA,0x888888);
        vmew32(SEQ_PERIOD,24);
}

printf("*****************************************\n");
printf("**** STEP 4: Bit Error Rate Measurement. \n");
printf("****\n");
printf("The aim of this program is to measure the BER for the DELAY_%i register values from %i to %i.\n",cable,fromSetting,toSetting);

//Scope signal. A=Sequence strobe and B=delayed_pattern1
//setAB(18,4);
//printf("CHECK THIS: %d  %d  % d\n",kDelay,kTc,kTch);
//for (w=0; w<5; w++){
for (k=fromSetting; k<=toSetting; k++) {
//Two transitions
if (val1[0] == 2) {
	                if (k>=0 && k<=kTc-1)        selection(k,kDelay+1,kTp,cable);
                        if (k>=kTc && k<=kTp+3)      selection(k,kDelay,kTp,cable);
                        if (k>=kTp+4 && k<=kTch-1)   selection(k,kDelay+1,kTp,cable);
			if (k>=kTch && k<=31)       selection(k,kDelay,kTp,cable);
}
if (val1[0] == 1) {
		
     if (kTc < 12 ) {  
  	 	if (kTs <= kTp + 3) {
			if (k>=0 && k<=kTc-1)   selection(k,kDelay+1,kTp,cable);
			if (k>=kTc && k<=kTp+3) selection(k,kDelay,kTp,cable);
			if (k>=kTp+4 && k<=31)  selection(k,kDelay+1,kTp,cable);
		 }
		 if (kTs > kTp + 3) {
			if (k>=0 && k<=kTc-1)   selection(k,kDelay,kTp,cable);
	                if (k>=kTc && k<=kTp+3) selection(k,kDelay-1,kTp,cable);
        	        if (k>=kTp+4 && k<=31)  selection(k,kDelay,kTp,cable);
		 }
     }
     else {
		if (kTc <= kTp + 3) {
			if (k>=0 && k<=(kTc-1))    selection(k,kDelay,kTp,cable);
			if (k>=kTc &&  k<=(kTp+3)) selection(k,kDelay-1,kTp,cable);
                	if (k>=(kTp+4) &&  k<=31)  selection(k,kDelay,kTp,cable);
	        }
		if (kTc > kTp +3 ) {              
		  	if (k>=0 && k<=kTp+3)          selection(k,kDelay,kTp,cable);
                	if (k>=(kTp+4) && k <(kTc-1))  selection(k,kDelay+1,kTp,cable);
	        	if (k>=kTc && k <=31)          selection(k,kDelay,kTp,cable);
        	}
     }//tc selection
}//one transition
	//*******************************************************
	//wait for 100 ms
	usleep(100000);

	//clear counters
        //clearCounters();
	readCounters(mystatus1,12,0);	
	//select time and timeUnits according to user settings
     	switch(timeUnits) {
  
	       case 's' :
	               sleep = time * 1000000;
		       break;
	       case 'h' :
        	       sleep =  time * 3600*1000000;
		       break;
	       case 'n' :
        	       sleep =  53000000;
	               break;
	       default:
        	       sleep =  53000000;
		       break;
	}
	//printf("measure performed in a time of, micro sec  %d\n",sleep);
        usleep(sleep);
	//count again
	readCounters(mystatus2,12,0);
	//Check number of errors
	mystatus3[8]  = mystatus2[8]  - mystatus1[8];
	mystatus3[9]  = mystatus2[9]  - mystatus1[9];
	mystatus3[10]  = mystatus2[10]  - mystatus1[10];
	mystatus3[11] = mystatus2[11] - mystatus1[11];
	mystatus3[0]  = mystatus2[0]  - mystatus1[0];
	if(cable==1) arrayCableErrors2[k]=mystatus3[8]; // Cable1-errors
	else if(cable==2) arrayCableErrors2[k]=mystatus3[9]; // Cable2-errors
	else cout << "WARNING: wrong cable number (=" << cable << ")" << endl;

	if (seqType == 1 || seqType == 2 || seqType == 4) {
		arraySequenceStrobes2[k]=mystatus3[11];
		arrayRatio2[k] = (1.0*arrayCableErrors2[k])/arraySequenceStrobes2[k];
		/*
		printf("index %d\n",k);
		printf("CableError %d\n",arrayCableErrors2[k]);
		printf("SequenceStrobes %d\n",arraySequenceStrobes2[k]);
		printf("ratio is %f\n ",arrayRatio2[k]);
		printf("\n");
		*/
		printf("<%i> <%f> \n",k,arrayRatio2[k]);
		//printf("<%i> <%i>\n",k,arrayCableErrors2[k]);
		if (fw.is_open()) {
			fw << k << " " << arrayRatio2[k] << endl;
			fw.flush();
		}
	} 
	if (seqType ==3) {
		arrayElapsedTime[k]=mystatus3[0];
		arrayTemp[k] = 0.4*arrayElapsedTime[k];
		arrayRatio2[k] = (1.0*arrayCableErrors2[k])/arrayTemp[k];
		/*
		printf("index %d\n",k);
		printf("CableError %d\n",arrayCableErrors2[k]);
		printf("ElapsedTime %d\n",arrayTemp[k]);
		printf("ratio is %f\n ",arrayRatio2[k]);
		printf("\n");
		*/
		printf("<%i> <%f> \n",k,arrayRatio2[k]);
		//printf("<%i> <%i>\n",k,arrayCableErrors2[k]);
		if (fw.is_open()) {
			fw << k << " " << arrayRatio2[k] << endl;
			fw.flush();
		}
	}
	//count total number of errors
	if (arrayCableErrors2[k] !=0) count = count + 1;

/* 
if (quit != 0) { 
  printf("quit volumne %d\n",quit); 
  fflush(stdout); quit = 0; 
  break;
};
*/
}//loop over the entire BC delay range
printf("Total number of settings where errors were found %d\n",count);
printf("Data was saved on WORK directory. The file name is windowData-DOT-txt\n");
if (fw.is_open()) fw.close();
return;
}
//}

/*FGROUP LVDST
SamplingWindowFinding function.
No input is needed. It automatically starts a test ~ 53sec long and creates the text file 'windowData_53s_karim.txt' as output.

NOTE: To stop this program, please press CONTROL + X
*/
void Find_Sampling_Window(int cable) {
char * myoutfile = "/usr/local/trigger/devel/v/vme/lvdst/windowData_53s_karim.txt";
Find_Window(cable, 0, 31, 3, 1, 0, 'n', myoutfile);
return;
}
//}

/*FGROUP LVDST
BERmeasurement function for given BC delay (=Ts).
Time and timeUnits defind by the user.
                timeUnits   's' for seconds
		  	    'h' for hours
e.g. time=1 and timeUnits='s' correspond to 1 sec run for each measure

NOTE: To stop this program, please press CONTROL + X
*/
void BER_Measurement(int cable, int time, char timeUnits) {
char * myoutfile = "/usr/local/trigger/devel/v/vme/lvdst/BERmeasurement_karim.txt";
const int kTs = Ts_transition(cable, 1);
Find_Window(cable, kTs, kTs, 4, 0, time, timeUnits, myoutfile);
return;
}
//}

/*FGROUP LVDST
check the pattern with the scope
*/
void check_pattern(int cable, long pattern){
//Select the SEQUENCE pattern
vmew32(PATTERN_SEL,0x1);
//Select the ADC_cable signal as the ADC input. Note: 1 for cable 1
vmew32(ADC_SELECT,cable);
//Select the Sequence with all 24 bits asserted
vmew32(SEQ_DATA,pattern);
//Select the sequence period - about 1.5 microSeconds (60 BC intervals)
//The period =60 is long enough to avoid the sequence overlapping for cable length
//of up to 160m
//vmew32(SEQ_PERIOD,60);
vmew32(SEQ_PERIOD,64); //karim
//vmew32(SEQ_PERIOD,23); //karim
return;
}

/*FGROUP LVDST
findHistory function.
Following up the previous measurement, i.e. the sampling window determination, this function
reads the error counter in a regular interval of 1 minute. The idea here is to show whether 
the errors occurred in a burst or they were spread evenly over the entire period of measurement.

kFrom and kTo     this defind the measure range.
                  A full test will be from kFrom=0 to kTo=31

seqTpye
        --> 1  for sequence pattern  0x800000
        --> 2  for sequence patter   0xaaaaaa
        --> 3  for random pattern

timeSteps is the entire period of measurement. If timeSteps is equal to 3,
the measurement will last for 3 minutes.

NOTE: To stop this program, please press CONTROL + X
*/
void Find_History(int cable, int fromSetting, int toSetting, int seqType, int timeSteps) {
//if (runType == 3) {
//        time  = 0;
//        timeUnits = 'n';
//} 
ofstream fh("/usr/local/trigger/devel/v/vme/lvdst/historyData_karim.txt",ios::out);

//The following were added for the findHistory function
//
int hk;
//
int kTp, 
    kTc,
    kTs,
    kDelay, 
    count=0,
    sleep, 
    k, 
    //w,
    arrayCableErrors2[32],
    arraySequenceStrobes2[32], 
    arrayElapsedTime[32];
float arrayRatio2[32], 
      arrayTemp[32];
int values1[2], kTch;
w32 mystatus1[12], mystatus2[12], mystatus3[12];
//Call Syn functions
kTp = Tp_transition(1);
if (kTp != 17 ) printf("W: Tp != 17 - Please check this value !!! \n");
Tc_transition(cable,values1,1);
kTc = values1[1];
if (values1[0] == 2)  {kTch = values1[2]; } else kTch = 0;
if (kTc >= 12) { kTs = kTc -12; }
else kTs = kTc + 12;  
kDelay = D_pattern_delay(cable,1);
//printf("The delay should be in %d\n ",kDelay1);
//vmew32(PATTERN_SEL,0x1);
vmew32(ADC_SELECT,cable);
//Sequence - It should be a RANDOM pattern for the final test
// H'AAAAAA'
//vmew32(SEQ_DATA,11184810);
if (seqType == 1 ) { 
	vmew32(PATTERN_SEL,0x1);
	vmew32(SEQ_DATA,0x800000);
	vmew32(SEQ_PERIOD,240);
}
if (seqType == 2) {
  vmew32(PATTERN_SEL,0x1);
        vmew32(SEQ_DATA,0xaaaaaa);
        vmew32(SEQ_PERIOD,240);
}
if (seqType == 3) {
		vmew32(PATTERN_SEL,0x2);
		vmew32(RANDOM_RATE,0x3fffffff);
}
//Scope signal. A=Sequence strobe and B=delayed_pattern1
//setAB(18,4);
//printf("CHECK THIS: %d  %d  % d\n",kDelay1,kTc,kTch);
//for (w=0; w<5; w++){
for (k=fromSetting; k<=toSetting; k++) {

//Two transitions
if (values1[0] == 2) {
	                if (k>=0 && k<=kTc-1)        selection(k,kDelay+1,kTp,cable);
                        if (k>=kTc && k<=kTp+3)      selection(k,kDelay,kTp,cable);
                        if (k>=kTp+4 && k<=kTch-1)   selection(k,kDelay+1,kTp,cable);
			if (k>=kTch && k<=31)       selection(k,kDelay,kTp,cable);
}
if (values1[0] == 1) {
		
     if (kTc < 12 ) {  
  	 	if (kTs <= kTp + 3) {
			if (k>=0 && k<=kTc-1)   selection(k,kDelay+1,kTp,cable);
			if (k>=kTc && k<=kTp+3) selection(k,kDelay,kTp,cable);
			if (k>=kTp+4 && k<=31)  selection(k,kDelay+1,kTp,cable);
		 }
		 if (kTs > kTp + 3) {
			if (k>=0 && k<=kTc-1)   selection(k,kDelay,kTp,cable);
	                if (k>=kTc && k<=kTp+3) selection(k,kDelay-1,kTp,cable);
        	        if (k>=kTp+4 && k<=31)  selection(k,kDelay,kTp,cable);
		 }
     }
     else {
		if (kTc <= kTp + 3) {
			if (k>=0 && k<=(kTc-1))    selection(k,kDelay,kTp,cable);
			if (k>=kTc &&  k<=(kTp+3)) selection(k,kDelay-1,kTp,cable);
                	if (k>=(kTp+4) &&  k<=31)  selection(k,kDelay,kTp,cable);
	        }
		if (kTc > kTp +3 ) {              
		  	if (k>=0 && k<=kTp+3)          selection(k,kDelay,kTp,cable);
                	if (k>=(kTp+4) && k <(kTc-1))  selection(k,kDelay+1,kTp,cable);
	        	if (k>=kTc && k <=31)          selection(k,kDelay,kTp,cable);
        	}
     }//tc selection
}//one transition
	//*******************************************************
	//wait for 100 ms
	usleep(100000);

	//clear counters
        //clearCounters();
for (hk=1; hk<=timeSteps; hk++) {

	//wait for 100 ms
        //usleep(100000);

	readCounters(mystatus1,12,0);	

        //sleep = 60 * 1000000;
	sleep = 1000000;//1 sec (karim)
	//printf("measure performed in a time of, micro sec  %d\n",sleep);
        usleep(sleep);

	//count again
	readCounters(mystatus2,12,0);

	//Check number of errors
	mystatus3[8]  = mystatus2[8]  - mystatus1[8];
	mystatus3[9]  = mystatus2[9]  - mystatus1[9];
	mystatus3[11] = mystatus2[11] - mystatus1[11];
	mystatus3[0]  = mystatus2[0]  - mystatus1[0];
	if(cable==1) arrayCableErrors2[k]=mystatus3[8]; // Cable1-errors
	else if(cable==2) arrayCableErrors2[k]=mystatus3[9]; // Cable2-errors
	else cout << "WARNING: wrong cable number (=" << cable << ")" << endl;

	if (seqType == 1 || seqType == 2 ) {
		arraySequenceStrobes2[k]=mystatus3[11];
		arrayRatio2[k] = (1.0*arrayCableErrors2[k])/arraySequenceStrobes2[k];
		/*
		printf("index %d\n",k);
		printf("CableError %d\n",arrayCableErrors2[k]);
		printf("SequenceStrobes %d\n",arraySequenceStrobes2[k]);
		printf("ratio is %f\n ",arrayRatio2[k]);
		printf("\n");
		*/
		//printf("<%i> <%f> \n",k,arrayRatio2[k]);
		printf("<%i> <%i> \n",hk,arrayCableErrors2[k]);
 		if (fh.is_open()) {
			fh << hk << " " << arrayCableErrors2[k] << endl;
			fh.flush();
		}
	} 
	if (seqType ==3) {
		arrayElapsedTime[k]=mystatus3[0];
		arrayTemp[k] = 0.4*arrayElapsedTime[k];
		arrayRatio2[k] = (1.0*arrayCableErrors2[k])/arrayTemp[k];
		/*
		printf("index %d\n",k);
		printf("CableError %d\n",arrayCableErrors2[k]);
		printf("ElapsedTime %d\n",arrayTemp[k]);
		printf("ratio is %f\n ",arrayRatio2[k]);
		printf("\n");
		*/
		printf("<%i> <%i> \n",hk,arrayCableErrors2[k]);
 		if (fh.is_open()) {
			fh << hk << " " << arrayCableErrors2[k] << endl;
			fh.flush();
		}
	}

	//count total number of errors
	if (arrayCableErrors2[k] !=0) count = count + 1;

	//Find Window
	mystatus1[8]  = mystatus3[8];
	mystatus1[9]  = mystatus3[9];
        mystatus1[11] = mystatus3[11];
        mystatus1[0]  = mystatus3[0];

}//find window
/* 
if (quit != 0) { 
  printf("quit volumne %d\n",quit); 
  fflush(stdout); quit = 0; 
  break;
};
*/
}//loop over the entire BC delay range
printf("Total number of settings where erros were found %d\n",count);
printf("Data was saved on WORK directory. The file name is historyData-DOT-txt\n");
if (fh.is_open()) fh.close();
return;
}
//}

/* FGROUP SimpleTests
returns temperature on the board in centigrades */
int ReadTemperature() {
w32 temp2,status;
int i;
vmew32(TEMP_START, DUMMYVAL);
for(i=0; i<3; i++) {
  usleep(300);
  status=vmer32(TEMP_STATUS);
  if( (status & 0x1) == 0) goto TEMPOK;
};
printf("ReadTemperature, TEMP_STATUS.BUSY timeout:\n");
return(-100);
TEMPOK:
temp2=vmer32(TEMP_READ)&0xff;
printf("ReadTemperature TEMP_READ:%x\n",temp2);
/* do the conversion from binary 2's complement */
return(temp2);
}

int blinkv=0;
/*FGROUP SimpleTests 
Click to Stop/Start blinking
front panel LEDs
  */
void TestLEDS() {
blinkv=vmer32(TEST_ADD);
blinkv=~blinkv;
vmew32(TEST_ADD, blinkv);
}

w32 slbin=0;
/*FGROUP ConfiguratioH 
Print 1 line string xxxx
where x is the status (0/1) of software LED word
*/
void getSWLEDS() {
int ix;
char sl[5]="0000";
slbin= vmer32(SOFT_LED);
//           slbin=slbin+1;
for(ix=0; ix<4; ix++) {
  if(slbin &(1<<ix)) sl[ix]='1';
};
printf("%s\n", sl);
}

//extern char BoardBaseAddress[];
void initmain() {   /* called once, at the very beginning */
//ltuDefaults(&ltucfg);
}
void endmain() {   /* called once, at the very end */
}
void boardInit() {   /* called once, after initmain, if -noboardInit 
                        was not present in Command line */
w32 code, pll;
code= 0xff&vmer32(CODE_ADD);
Gltuver= 0xff&vmer32(LTUVERSION_ADD);
if(code==0x56) {
  if(Gltuver==0xff) {
    /* LTU FPGA configuration, if not configured: */
    //loadFPGA(0);
    //Gltuver= 0xff&vmer32(LTUVERSION_ADD);
    printf("LTU not configured, exiting \n");
    exit(1);
  };
} else {
  printf("Incorrect base address or board. Board:0x%x expected:0x56 ver:0x%x\n",    code, Gltuver);
  return;
};
if(Gltuver!=0xe1) {
  printf("Bad firmware (0xe1 expected), exiting...\n"); exit(8);
} else {
  printf("LTU_VERSION:0xe1: LVDS tester\n");
};
pll= vmer32(BC_STATUS)&0x7;
printf("BC_STATUS: %x: ", pll);
if( pll == BC_STATUSpll ) {
  printf("BC and Orbit input signals OK\n");
};
if( (pll & BC_STATUSpll) == 0) {
  printf("BC input signal not present\n");
};
if( (pll & BC_STATUSerr) == 0x01) {
  printf("errorneous BC -check BC LED\n");
};
if( (pll & BC_STATUSorbiterr) == BC_STATUSorbiterr ) {
  printf("errorneous Orbit input signal\n");
};
}
/*ENDOFCF
*/

