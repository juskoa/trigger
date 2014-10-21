/*BOARD ltu 0x813000 0x800 */

#include "ltut.h"


#define DEFAULT_BC_DELAY 0
#define DEFAULT_L0	1250	// ns
#define DEFAULT_L1	6500	// ns
#define DEFAULT_WINDOW	50	// ns
#define DEFAULT_LMSA	1125	// ns
#define DEFAULT_FREQ	10	// kHz


void endmain()
{
}


void initmain()
{
	PrintInfos();
	MeasureTemp();
}

void boardInit()
{
}

/*FGROUP SimpleTests
Print some informations
about the LTU.
*/
void PrintInfos() {
	BTCode = 0xff & vmer32(CODE_ADD);
	BSN = 0xff & vmer32(SERIAL_NUMBER);
	verVME = 0xff & vmer32(VERSION_ADD);
	verLOG = 0x3f & vmer32(L_VERSION_ADD);	

	printf("The LTU Board Type code is: 0x%x.\n", BTCode);
	printf("The LTU board Serial Number is: 0x%x.\n", BSN);
	printf("The VME controller FPGA Firmware Version is: 0x%x.\n", verVME);
	printf("The Logic FPGA Firmware Version is: %.2f (0x%x).\n", verLOG/10., verLOG);
}

/*FGROUP SimpleTests
Show the status of the 
test mode.
*/
void PrintTestStatus(){
	leds_test = 0x1 & vmer32(TEST_ADD);
	printf("The test mode of the LEDs is %d.\n", leds_test);
}

/*FGROUP SimpleTests
Turn on/off test mode.
*/
void SetTestMode(int mode){
	if (mode > 1) mode = 1;
	if (mode < 0) mode = 0;
	vmew32(TEST_ADD, mode);
	printf("The test mode of the LEDs is set to %d.\n", mode);
}


/*FGROUP SimpleTests 
Click to start/stop blinking
front panel LEDs
*/
void TestLEDS() {
	leds_test = 0x1 & vmer32(TEST_ADD);
	if (leds_test)
		printf("The test mode of the LEDs is now off.\n");
	else
		printf("The test mode of the LEDs os now on.\n");
	
	leds_test = ~leds_test;
	vmew32(TEST_ADD, leds_test);
}

/*FGROUP SimpleTests 
Click to measure the board
temperature.
*/
void MeasureTemp() {
	vmew32(TEMP_START,dummy);
	busy_temp = 0x1 & vmer32(TEMP_STATUS);
//	printf("Busy status: %d\n",busy_temp);
	usleep(1000);
	busy_temp = 0x1 & vmer32(TEMP_STATUS);
//	printf("Busy status: %d\n",busy_temp);
	temp_data = 0xFF & vmer32(TEMP_READ);
	printf("The board has %d°C.\n", temp_data);
}

/*FGROUP Config
Resets all timings to default
values.
*/
void ResetTimes() {

	SetTimer(DEFAULT_WINDOW,DEFAULT_L0,DEFAULT_L1);
	SetGeneratorTimes(DEFAULT_LMSA,DEFAULT_FREQ);	

	vmew32(BC_DELAY,DEFAULT_BC_DELAY);
	printf("BC Delay has been reset to %d.\n", 0x1f & vmer32(BC_DELAY));
}


/*FGROUP Config
Prepare a soft reset
of the LTU.
*/
void SoftReset() {
	printf("Preparing a soft reset...\n");
	vmew32(SOFT_RESET,dummy);
}

/*FGROUP Config
Show current delay of the delayed
BC clock.
*/
void PrintDelay() {
	delay = 0x1f & vmer32(BC_DELAY);
	printf("The current delay of the delayed BC clock is %d ns.\n", delay);
}

/*FGROUP Config
Set the delay of the programmable
delay line to delay the BC clock before
it is used in the FPGA.

The delay range is 0 to 31 ns.
*/
void SetDelay(int set_delay) {
	if (set_delay < 0) set_delay = 0;
	if (set_delay > 31) set_delay = 31;

	delay = 0x1f & vmer32(BC_DELAY);
	vmew32(BC_DELAY,set_delay);
	//set_delay = 0x1f & vmer32(BC_DELAY);
	printf("The delay was %d ns and is set now to %d ns.\n", delay, set_delay);
}

/*FGROUP Config
This function prints the current
phase of the input synchronizer
several times.
*/
void PrintPhase(int repeat) {
	int i;
	for (i=0; i<repeat; i++) {
		printf("%010u:\tCurrent phase: %d.\n",vmer32(COUNTER_BC), 0x3 & vmer32(INP_PHASE));
	}
}

/*FGROUP Oscilloscope
Set the Oscilloscope outputs.
A:
0 = disable
1 = DL_BC
2 = LM_GL
3 = OUT_L1 (=TTCtrd_A)
4 = clk (fast clock)
5 = mux_LM
6 = seq_LM
7 = seq_LM | OUT_SPARE (OUT_SPARE = LM_SA)
8 = OUT_SPARE
9 = seq_LM | OUT_SPARE | TTC_A
10 = OUT_SPARE | OUT_L1
11 = TTC_A
12 = STROBE
13 = BUSY_dis & IN_BUSY_1
14 = disable
15 = disable
B:
0 = disable
1 = clk (fast clock)
2 = OUT_SPARE (=LM_SA)
3 = OUT_ORBIT (=TTC_B)
4 = OUT_BUSY (=IN_BUSY1 = BUSY from GTU; at the moment set to 0) 
5 = IN_BC
6 = TTC_A
7 = OUT_L1
8 = TTC_A_synchr
9 = TTC_B
10 = WRITE
11 = test_LEDs
12 = IN_BUSY1
13 = LM_dis & LM_GL
14 = disable
15 = disable
*/

void SetOutputsAB(int A, int B){
	if (A > 15) A = 15;
	if (A < 0) A = 0;
	if (B > 15) B = 15;
	if (B < 0) B = 0;
	
	A_old = 0xf & vmer32(SWITCH_A);
	B_old = 0xf & vmer32(SWITCH_B);

	printf("Output A is set to number %d, it was %d before.\n", A, A_old);
	printf("Output B is set to number %d, it was %d before.\n", B, B_old);

	if (A != A_old) vmew32(SWITCH_A,A);
	if (B != B_old) vmew32(SWITCH_B,B);
}

/*FGROUP Config
Disables and enables the LM_GL
input. If the connector is unplugged,
you have to disable this input,
otherwise it will be recogniced as
a continuous LM signal
*/
void EnableDisableLMGL(){
	endisLMGL = 0x1 & vmer32(LM_GL_DISABLE);
	vmew32(LM_GL_DISABLE,!endisLMGL);
	if (endisLMGL)
		printf("The LM-GL input is now ENABLED.\n");
	else
		printf("The LM-GL input is now DISABLED.\n");
}

/*FGROUP Config
Disables and enables the BUSY
input. If the connector is unplugged,
you have to disable this input,
otherwise it will be recogniced as
a continuous BUSY signal
*/
void EnableDisableBUSY(){
	endisBUSY = 0x1 & vmer32(BUSY_DISABLE);
	vmew32(BUSY_DISABLE,!endisBUSY);
	if (endisBUSY)
		printf("The BUSY input is now ENABLED.\n");
	else
		printf("The BUSY input is now DISABLED.\n");
}

#define OFFSET_TIMER 2;
/*FGROUP Timings
Shows waiting times between LM <-> L0
and L0 <-> L1 and the length of the
open window for the next trigger signal.
*/
void PrintTimer() {
	time_L0 = 0xff & vmer32(TIMER_L0);
	time_L1 = 0xfff & vmer32(TIMER_L1);
	time_L0 += OFFSET_TIMER;
	time_L1 += OFFSET_TIMER;
	width = 0x1f & vmer32(TIMER_WINDOW);
	time_L0 -= width/2;
	time_L1 -= width/2;
	printf("The time window is set to %d clock cycles -> %.2f ns.\n", width, width*6.25);
	printf("The L0 time is set to %d clock cycles -> %.2f ns.\n", time_L0, time_L0*6.25);
	printf("The L1 time is set to %d clock cycles -> %.2f ns.\n", time_L1, time_L1*6.25);
}


/*FGROUP Timings
Set waiting times between LM <-> L0
and L0 <-> L1 and the length of the
open window for the next trigger signal.
Fill in the times in ns!

For example: The time between LM and L0
has to be 300 ns and the window 50 ns.
Then the L0 can arrive between 275 ns and
325 ns after LM.
*/
void SetTimer(int Window, int L0_time, int L1_time) {
	width = (int) Window/6.25;
	if (width > 31) {
		width = 31;
		printf("Inserted window time was too big...\n");
	} else if (width < 4) {
		width = 4;
		printf("Inserted window time was too low...\n");
	}	

	L0_cycles = L0_time/6.25;
	L1_cycles = L1_time/6.25;

	printf("The time window is set to %d clock cycles -> %.2f ns.\n", width, width*6.25);
	printf("The timer for L0 is set to %d clock cycles -> %.2f ns.\n", L0_cycles, L0_cycles*6.25);
	printf("The timer for L1 is set to %d clock cycles -> %.2f ns.\n", L1_cycles, L1_cycles*6.25);

	time_L0 = L0_cycles + (width/2);
	time_L1 = L1_cycles + (width/2);

	if (time_L0 > 255) {
		time_L0 = 255;
		printf("Inserted L0 time was too big...\n");
	}

	if (time_L1 > 4095) {
		time_L1 = 4095;
		printf("Inserted L1 time was too big...\n");
	}

	time_L0 -= OFFSET_TIMER;
	time_L1 -= OFFSET_TIMER;

//	printf("%d, %d, %d\n", width, time_L0, time_L1);
	vmew32(TIMER_WINDOW,width);
	vmew32(TIMER_L0,time_L0);
	vmew32(TIMER_L1,time_L1);
}

/*FGROUP SequenceGenerator
Display status of the build in sequence
generator.
*/
void PrintGenerator(){
	seq_enable = 0x1 & vmer32(SEQGEN_ENABLE);
	if(seq_enable)
		printf("Sequence generator is ON!\n");
	else
		printf("Sequence generator is OFF!\n");
}

/*FGROUP SequenceGenerator
Enable/Disable the build in sequence
generator. It will produce the LM signal
and the pulse for the standard LTU (LM-SA)
to trigger the remaining sequence.
*/
void EnableDisableGenerator(){
	seq_enable = 0x1 & vmer32(SEQGEN_ENABLE);
	seq_enable = !seq_enable;
	vmew32(SEQGEN_ENABLE,seq_enable);
	if(seq_enable) 	printf("Sequence generator is now ON!\n");
	else		printf("Sequence generator is now OFF!\n");
}

/*FGROUP SequenceGenerator
Display the mode of the sequence generator.
Possible modes are Pulser-mode, BC-mode and
Random-mode. In Pulser-Mode an external pulser
triggers the sequence, in BC-mode the next
sequence starts with the next BC clock cycle and
in Random-mode it starts randomly.
*/
void ShowGeneratorMode() {
	seq_mode = 0x3 & vmer32(SEQGEN_MODE);
	if (seq_mode == 0) {
		printf("No mode specified, no LM will be created!\n");
	} else if (seq_mode == 1) {
		printf("0x%.2x: The generator is in Pulser-mode.\n", seq_mode);
	} else if (seq_mode == 2) {
		printf("0x%.2x: The generator is in BC-mode.\n", seq_mode);
	} else if (seq_mode == 3) {
		printf("0x%.2x: The generator is in Random-mode.\n", seq_mode);
	} else {
		printf("0x%.2x: I don't know this mode...\n", seq_mode);
	}
}

/*FGROUP SequenceGenerator
Set mode to Pulser-mode.
*/
void SetModePulser() {
	vmew32(SEQGEN_MODE,1);
	printf("Set mode to Pulser-mode...\n");
	seq_mode = 0x3 & vmer32(SEQGEN_MODE);
	if (seq_mode != 1)
		printf("Something went wrong...\n");
}

/*FGROUP SequenceGenerator
Set mode to BC-mode.
*/
void SetModeBC() {
	vmew32(SEQGEN_MODE,2);
	printf("Set mode to BC-mode...\n");
	seq_mode = 0x3 & vmer32(SEQGEN_MODE);
	if (seq_mode != 2)
		printf("Something went wrong...\n");
}

/*FGROUP SequenceGenerator
Set mode to Random-mode.
*/
void SetModeRandom() {
	vmew32(SEQGEN_MODE,3);
	printf("Set mode to Random-mode...\n");
	seq_mode = 0x3 & vmer32(SEQGEN_MODE);
	if (seq_mode != 3)
		printf("Something went wrong...\n");
}

#define OFFSET_SEQ 2

/*FGROUP SequenceGenerator
Show times of the sequence generator
between LM <-> LM-SA and the frequency
of LM triggers.
*/
void PrintSGTimes() {
        time_LMSA = 0xff & vmer32(SEQGEN_LMSA);
        frequency = 0xfff & vmer32(SEQGEN_FREQ);

	time_LMSA += OFFSET_SEQ;
	
	printf("The time between LM and LM-SA is set to %d clock cycles -> %d ns.\n", time_LMSA, time_LMSA*25);
	printf("The frequency of the Sequence Generator is %d kHz.\n", 40000/frequency);
}

/*FGROUP SequenceGenerator
Set waiting time between LM <-> LM-SA in ns
and frequency of LM trigger in kHz.
*/
void SetGeneratorTimes(int LMSA, int Frequency) {
	time_LMSA = LMSA/25;
	frequency = 40000/Frequency;
	
	if (time_LMSA > 255) { 
		time_LMSA = 255;
		printf("Inserted LMSA time was too big...\n");
	}
	if (frequency < 250) {
		frequency = 250;
		printf("Inserted frequency was too high...\n");
	} else if (frequency > 131071) {
		frequency = 131071;
		printf("Inserted frequency was too low...\n");
	}


	printf("Set waiting time between LM and LM-SA to %d clock cycles -> %d ns.\n", time_LMSA, time_LMSA*25);
	printf("The frequency is set to %d kHz.\n",40000/frequency);

	time_LMSA -= OFFSET_SEQ;

	vmew32(SEQGEN_LMSA,time_LMSA);
	vmew32(SEQGEN_FREQ,frequency-time_LMSA-1);
}

/*FGROUP Counter
Reset all counters.
*/
void CounterReset(){
	vmew32(COUNTER_RESET,dummy);
	printf("All counters are set to 0.\n");
}

/*FGROUP Counter
Print all counters.
*/
void CounterPrint() {

	vmew32(COUNTER_LOCK,dummy);
	counter_bc = vmer32(COUNTER_BC);
	counter_lm = vmer32(COUNTER_LM);
	counter_l0 = vmer32(COUNTER_L0);
	counter_l1 = vmer32(COUNTER_L1);
	counter_ttca = vmer32(COUNTER_TTCA);
	counter_lmgl = vmer32(COUNTER_LMGL);

	printf("BC: %u.\n", counter_bc);
	printf("LM: %u.\n", counter_lm);
	printf("L0: %u.\n", counter_l0);
	printf("L1: %u.\n", counter_l1);
	printf("LM-GL: %u.\n", counter_lmgl);
	printf("TTC-A: %u (L0+L1 = %u, L2 = %u).\n", counter_ttca, counter_l0+counter_l1, counter_ttca-counter_l0-counter_l1);
}

/*FGROUP SSH
 */
void SetSSHMode(int mode) {
	vmew32(SSH_MODE, mode);
	mode_cont = 0x3 & vmer32(SSH_MODE);
	printf("0x%x: Mode was set to 0x%x.\n",(mode_cont>>2) & 0x1, mode_cont & 0x3);
}

/*FGROUP SSH
 */
void WriteMemory() {
	vmew32(SSH_START,dummy);
	usleep(26215);
	printf("Memory was written.\n");
}

FILE* mem;
/*FGROUP SSH
 */
void ReadMemory(int amount) {

	mode_cont = 0x3 & vmer32(SSH_MODE);

	if (mode_cont == 0) mem = fopen("/localhome/trd/ltu_20140716/vme/ltut/memdump_states.dat","w");
	if (mode_cont == 1) mem = fopen("/localhome/trd/ltu_20140716/vme/ltut/memdump_BC_count.dat","w");

	if (mem == NULL) {
		printf("Error opening file!\n");
	} else {
		fprintf(mem,"0x%x\n",mode_cont);
	}

	if (amount == 42) amount = 1048575;

	int i=0;
	for (i=0; i<amount; i++) {
		printf("%d ", i);
		mem_cont = vmer32(SSH_START);
		if (mode_cont == 0) printf("0x%x: mon_phase: %d, mon_state_seqGen: 0x%x, mon_state_PC: 0x%x.\n", mem_cont, 0x3 & (mem_cont>>10),0x1F & (mem_cont>>5), 0x1F & mem_cont);
		if (mode_cont == 1) printf("0x%x\n",mem_cont);
		if (mem != NULL) fprintf(mem,"0x%x\n",mem_cont);
	}
	fclose(mem);
}
