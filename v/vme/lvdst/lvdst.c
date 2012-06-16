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
#include "lvdst.h"
extern int quit;

void loadFPGA();
void rndtestA(int *values);

w32 Gltuver;   // should be 0xe?
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
void getCounters(int N, int increments);
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
int i, j, array[32], number=100,w;
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
rndtestA(array);
//Find the transition
for (i=1; i<32; i++) {
        w = array[i];
        if((w<array[i-1]) && (w<number)) {
                number = w;
                j  = i;
        }
}
if (j != 17) printf("Warning: Tp is not 17 !!! \n");
if (print == 0 ) {
	printf("*****************************************\n");
	printf("****  STEP 1: Finding the exclusion zone - For PATTERN Syn. \n");
	printf("****\n");
	if (j != 0) printf("A transition has been observed in %d ns \n",j);
	printf("This suggest that the exclusion zone of the BC_DELAY_ADD register should be +- 4 ns that value.\n");
	printf("This exclusion zone is a hardware dependence constant.\n");
	printf("It is recommend it to confirm this result making NOW a ADC_SCAN.\n");
}
return(j);
}
/*
Tc - transition of the Synchronised Cable Signal
From here you will be able to do a ADC scan for the cable_1 signal. 
The pattern is setted to be TOGGLE. This result is a cable dependence. 
For a different cable, you might find a different number.
 */
void Tc_transition(int *values1) {
int kTransitionValue;
kTransitionValue=Tp_transition(1);
int i, j, array[32], w=-1, number=100, count=0;
//scope
//setAB(2,9);
//Select the toggling pattern, PATTERN_SEL
vmew32(PATTERN_SEL,0x3);
//Note: the BC edge is irrelevant for this measurement.
//Select the ADC_CABLE signal as the ADC input. Select 1(2) for cable 1(2).
vmew32(ADC_SELECT,1);
//ADC_SCAN - the phase shift of the toggling cable 1(2) input 
//in respect to the BC clock
getbcstatus();
rndtestA(array);
//Find a transition
for (i=1; i<32; i++) {
        w = array[i];
        if((w<array[i-1]) && (w<number)) {
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
/*
if (print == 0) {
	printf("*****************************************\n");
	printf("**** STEP 2: Find the best setting for the BC_DELAY_ADD register.\n");
	printf("****\n");
	if (j != 0)  printf("A transition has been observed in %d ns \n",j); 
	printf("It is recommend it to confirm this result making NOW a ADC_SCAN.\n\n");
}
return(j);
*/
}
/*FGROUP LVDST tests
Ts - The best setting for the BC_DELAY_ADD register
 */
int Ts_transition() {
int Tc=0, kTs=0, values1[2];
Tc_transition(values1);
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
	if (Tp != 17) printf("Warning: Tp is not 17 !!! \n");
	if (Setting>=(Tp-3) && Setting<=(Tp+3)) { kSignOfZone = 1; } 
	else kSignOfZone = 0;
	if (print == 0) {
		if (kSignOfZone == 1) printf("The edge is negative \n");
		else printf("The edge is positive\n"); 
	}
} else printf("WARNING: Incorrect setting value \n");
return(kSignOfZone);
}
/*FGROUP LVDST tests
For Pattern delay measurement. 
The aim of this function is to find the correct settings for the
DELAY_1 register. This function uses the results from previous Syn functions.
*/
int D_pattern_delay(int print) {
int kTp = -1, kTs =-1, kDelay1=-1, kSignOfZone=-1;
w32 mystatus[12], mystatus1[12];
//w32, kCableErrors, kSequenceStrobes;
int k;
float i, number, arrayCableErrors[33],arraySequenceStrobes[33];
float arrayRatio[33];
kTp = Tp_transition(1);
//kTs = Ts_transition(1);
kTs = Ts_transition();
kSignOfZone = Edge_selection(1,kTs,kTp);
//Select the SEQUENCE pattern
vmew32(PATTERN_SEL,0x1);
//Select the ADC_cable signal as the ADC input. Note: 1 for cable 1
vmew32(ADC_SELECT,0x1);
//Select the Sequence with all 24 bits asserted
vmew32(SEQ_DATA,0xffffff);
//Select the sequence period - about 1.5 microSeconds (60 BC intervals)
//The period =60 is long enough to avoid the sequence overlapping for cable length
//of up to 160m
vmew32(SEQ_PERIOD,60);
//Scope signal. A=Sequence strobe and B=delayed_pattern1
//setAB(18,4);
//Set BC_DELAY_ADD - Value found previously on Ts_transition
vmew32(BC_DELAY_ADD,kTs);
//Apply the Edge rule to select the BC edge
vmew32(SYN_EDGE,kSignOfZone);
//Delay is done in BC units
for (k=0; k<32; k++) {
	//wait for 100 ms
        usleep(100000);
	//clearCounters();
	readCounters(mystatus1,12,0);
	//Sleep a bit - get more statistics
	usleep(100000);
	readCounters(mystatus,12,0);
	// Cable1-errors
        arrayCableErrors[k]=mystatus[8] - mystatus1[8];
	//Number of sequence strobes - number of patterns generated
        arraySequenceStrobes[k]=mystatus[11] - mystatus1[11];
	arrayRatio[k] = (1.0*arrayCableErrors[k])/arraySequenceStrobes[k];
	vmew32(DELAY_1,k);
	if (print == 0) {
		/*
		printf("For setting %d\n",k-1);
		printf("CableError %d\n",arrayCableErrors[k]);
		printf("SequenceStrobes %d\n",arraySequenceStrobes[k]);
		printf("ratio is %f\n ",arrayRatio[k]);
		printf("\n");
		*/
		printf("<%i> <%f> \n",k,arrayRatio[k]);
	}
}
number=10000.0;
for (k=1; k<32; k++) {
        i = arrayRatio[k];
        if((i<=arrayRatio[k-1]) && (i<=number)) {
                number = i;
                kDelay1  = k-1;
        }
}
//Set the correct value for DELAY_1 register
vmew32(DELAY_1,kDelay1);
if (print == 0) {
	printf("*****************************************\n");
	printf("**** STEP 3: Pattern Delay Measurement. \n");
	printf("****\n");
	printf("The aim of this program is to find the correct settings for the DELAY_1 register.\n");
	printf("DELAY_1 should be %d\n",kDelay1);
	//printf("Number of errors %d\n",number);
}
return(kDelay1);
}
void selection (int setting, int delay, int Tp){
int kEdge; 
kEdge = Edge_selection(1,setting,Tp);
vmew32(SYN_EDGE,kEdge);
vmew32(DELAY_1,delay);
vmew32(BC_DELAY_ADD,setting);
/*
printf("Setting for %d\n",setting);
printf("Edge %d\n",kEdge);
printf("Delay: %d\n",delay);
*/
}
/*FGROUP LVDST
WindowFinding function.
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
void Find_Window(int fromSetting, int toSetting, int seqType, int runType, int time, char timeUnits) {
if (runType == 3) {
        time  = 0;
        timeUnits = 'n';
} 
FILE *fh;
fh = fopen("WORK/windowData.txt","w");
int kTp, 
    kTc,
    kTs,
    kDelay1, 
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
Tc_transition(values1);
kTc = values1[1];
if (values1[0] == 2)  {kTch = values1[2]; } else kTch = 0;
if (kTc >= 12) { kTs = kTc -12; }
else kTs = kTc + 12;  
kDelay1 = D_pattern_delay(1);
//printf("The delay should be in %d\n ",kDelay1);
//vmew32(PATTERN_SEL,0x1);
vmew32(ADC_SELECT,0x1);
//Sequence - It should be a RANDOM pattern for the final test
// H'AAAAAA'
//vmew32(SEQ_DATA,11184810);
if (seqType == 1) { 
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
	                if (k>=0 && k<=kTc-1)        selection(k,kDelay1+1,kTp);
                        if (k>=kTc && k<=kTp+3)      selection(k,kDelay1,kTp);
                        if (k>=kTp+4 && k<=kTch-1)   selection(k,kDelay1+1,kTp);    
			if (k>=kTch && k<=31)       selection(k,kDelay1,kTp);
}
if (values1[0] == 1) {
		
     if (kTc < 12 ) {  
  	 	if (kTs <= kTp + 3) {
			if (k>=0 && k<=kTc-1)   selection(k,kDelay1+1,kTp);
			if (k>=kTc && k<=kTp+3) selection(k,kDelay1,kTp);
			if (k>=kTp+4 && k<=31)  selection(k,kDelay1+1,kTp);
		 }
		 if (kTs > kTp + 3) {
			if (k>=0 && k<=kTc-1)   selection(k,kDelay1,kTp);
	                if (k>=kTc && k<=kTp+3) selection(k,kDelay1-1,kTp);
        	        if (k>=kTp+4 && k<=31)  selection(k,kDelay1,kTp);
		 }
     }
     else {
		if (kTc <= kTp + 3) {
			if (k>=0 && k<=(kTc-1))    selection(k,kDelay1,kTp);
			if (k>=kTc &&  k<=(kTp+3)) selection(k,kDelay1-1,kTp);
                	if (k>=(kTp+4) &&  k<=31)  selection(k,kDelay1,kTp);
	        }
		if (kTc > kTp +3 ) {              
		  	if (k>=0 && k<=kTp+3)          selection(k,kDelay1,kTp);
                	if (k>=(kTp+4) && k <(kTc-1))  selection(k,kDelay1+1,kTp);
	        	if (k>=kTc && k <=31)          selection(k,kDelay1,kTp);
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
	mystatus3[11] = mystatus2[11] - mystatus1[11];
	mystatus3[0]  = mystatus2[0]  - mystatus1[0];
	arrayCableErrors2[k]=mystatus3[8];

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
		printf("<%i> <%i> \n",k,arrayCableErrors2[k]);
 		fprintf(fh,"%i %2d \n",k,arrayCableErrors2[k]);

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
		printf("<%i> <%i> \n",k,arrayCableErrors2[k]);
		fprintf(fh,"%i %2d \n",k,arrayCableErrors2[k]);
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
printf("Total number of settings where erros were found %d\n",count);
printf("Data was saved on WORK directory. The file name is windowData-DOT-txt\n");
fclose(fh);
}
//}
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
void Find_History(int fromSetting, int toSetting, int seqType, int timeSteps) {
//if (runType == 3) {
//        time  = 0;
//        timeUnits = 'n';
//} 
FILE *fh;
fh = fopen("WORK/historyData.txt","w");
//The following were added for the findHistory function
//
int hk;
//
int kTp, 
    kTc,
    kTs,
    kDelay1, 
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
Tc_transition(values1);
kTc = values1[1];
if (values1[0] == 2)  {kTch = values1[2]; } else kTch = 0;
if (kTc >= 12) { kTs = kTc -12; }
else kTs = kTc + 12;  
kDelay1 = D_pattern_delay(1);
//printf("The delay should be in %d\n ",kDelay1);
//vmew32(PATTERN_SEL,0x1);
vmew32(ADC_SELECT,0x1);
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
	                if (k>=0 && k<=kTc-1)        selection(k,kDelay1+1,kTp);
                        if (k>=kTc && k<=kTp+3)      selection(k,kDelay1,kTp);
                        if (k>=kTp+4 && k<=kTch-1)   selection(k,kDelay1+1,kTp);    
			if (k>=kTch && k<=31)       selection(k,kDelay1,kTp);
}
if (values1[0] == 1) {
		
     if (kTc < 12 ) {  
  	 	if (kTs <= kTp + 3) {
			if (k>=0 && k<=kTc-1)   selection(k,kDelay1+1,kTp);
			if (k>=kTc && k<=kTp+3) selection(k,kDelay1,kTp);
			if (k>=kTp+4 && k<=31)  selection(k,kDelay1+1,kTp);
		 }
		 if (kTs > kTp + 3) {
			if (k>=0 && k<=kTc-1)   selection(k,kDelay1,kTp);
	                if (k>=kTc && k<=kTp+3) selection(k,kDelay1-1,kTp);
        	        if (k>=kTp+4 && k<=31)  selection(k,kDelay1,kTp);
		 }
     }
     else {
		if (kTc <= kTp + 3) {
			if (k>=0 && k<=(kTc-1))    selection(k,kDelay1,kTp);
			if (k>=kTc &&  k<=(kTp+3)) selection(k,kDelay1-1,kTp);
                	if (k>=(kTp+4) &&  k<=31)  selection(k,kDelay1,kTp);
	        }
		if (kTc > kTp +3 ) {              
		  	if (k>=0 && k<=kTp+3)          selection(k,kDelay1,kTp);
                	if (k>=(kTp+4) && k <(kTc-1))  selection(k,kDelay1+1,kTp);
	        	if (k>=kTc && k <=31)          selection(k,kDelay1,kTp);
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

        sleep = 60 * 1000000;
	//printf("measure performed in a time of, micro sec  %d\n",sleep);
        usleep(sleep);

	//count again
	readCounters(mystatus2,12,0);

	//Check number of errors
	mystatus3[8]  = mystatus2[8]  - mystatus1[8];
	mystatus3[11] = mystatus2[11] - mystatus1[11];
	mystatus3[0]  = mystatus2[0]  - mystatus1[0];
	arrayCableErrors2[k]=mystatus3[8];

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
 		fprintf(fh,"%i %2d \n",hk,arrayCableErrors2[k]);

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
		fprintf(fh,"%i %2d \n",hk,arrayCableErrors2[k]);
	}

	//count total number of errors
	if (arrayCableErrors2[k] !=0) count = count + 1;

	//Find Window
	mystatus1[8]  = mystatus3[8];
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
fclose(fh);
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
    loadFPGA();
    Gltuver= 0xff&vmer32(LTUVERSION_ADD);
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

