#include "../../vmeb/vmeblib/lexan.h"
#include "vmewrap.h"
#include <sys/types.h>
#include <stdio.h>
#include <string.h>


/*REGSTART32 */
/* VME FPGA: */
#define CODE_ADD      0x4     /* board type (0x56 for LTU) */
#define SERIAL_NUMBER 0x8     /* unique serial number  of the board */
#define VERSION_ADD   0xC     /* VME FPGA firmware version */
#define SOFT_RESET    0x28
#define CONFIG_STATUS 0x50
#define CONFIG_START  0x54

#define TEMP_START    0x58   /* LTU temperature: */
#define TEMP_STATUS   0x5c
#define TEMP_READ     0x60
/*REGEND */

/* LTU logic FPGA: */
/*REGSTART32 */
/*REGSTART32 */
#define TEST_ADD	0xc0
#define SWITCH_A	0xc4
#define SWITCH_B	0xc8
#define SEQGEN_LMSA	0xcc
#define SEQGEN_FREQ	0xd0
#define SEQGEN_SEED	0xd4
#define SEQGEN_ENABLE	0xd8
#define SEQGEN_MODE	0xdc
#define TIMER_L0	0xe0
#define TIMER_L1	0xe4
#define TIMER_WINDOW	0xe8
#define BC_DELAY	0xec
#define COUNTER_RESET		0xf0
#define COUNTER_LOCK	0xf4
#define COUNTER_BC	0xf8
#define COUNTER_LM	0xfc
#define COUNTER_L0	0x100
#define COUNTER_L1	0x104
#define COUNTER_TTCA	0x108
#define COUNTER_LMGL	0x10c
#define L_VERSION_ADD	0x110
#define INP_PHASE	0x114
#define BUSY_DISABLE	0x118
#define LM_GL_DISABLE	0x11c
#define SSH_START	0x120
#define SSH_MODE	0x124
/*REGEND */
/*REGEND */

int dummy = 1;

void endmain();
void initmain();
void boardInit();

/* Simple Tests */
void PrintInfos();
	w32 BTCode, BSN, verVME, verLOG;
void PrintTestStatus();
void TestLEDS();
	int leds_test;
void MeasureTemp();
	int busy_temp, temp_data;


/* Config */
void SoftReset();
void PrintDelay();
	int delay;
void SetDelay(int set_delay);
void PrintPhase(int repeat);
	int phase;
void SetOutputsAB(int A, int B);
	int A_old, B_old;
void EnableDisableLMGL();
	int endisLMGL;
void EnableDisableBUSY();
	int endisBUSY;


/* Timings */
void PrintTimer();
	int time_L0, time_L1, width;
void SetTimer(int Window, int L0_time, int L1_time);
	int L0_cycles, L1_cycles;


/* Sequence Generator */
void PrintGenerator();
	int seq_enable;
void EnableDisableGenerator();
void ShowGeneratorMode();
	int seq_mode;
void SetModePulser();
void SetModeBC();
void SetModeRandom();
void PrintSGTimes();
	int time_LMSA, frequency;
void SetGeneratorTimes(int LMSA, int Freq);

/* Counter */
void CounterReset();
void CounterPrint();
	unsigned int counter_bc, counter_lm, counter_l0, counter_l1, counter_ttca, counter_lmgl;

/* SSH */
#define MEM_SIZE 10000//1048575
void SetSSHMode(int mode);
	int mode_cont;
void WriteMemory();
void ReadMemory(int amount);
	int mem_cont;
