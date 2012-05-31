#define DUMMY 0xff
/*REGSTART32 */
#define VME_RESET 0x10  /* DUMMY write: */
/*clear L0_COUNTER and start STATUS_CLOCK measurement */
/*STATUS */
#define VERSION_NUMBER 0x20
#define ENABLE_RECEIVER 0x30 /* 7bits: [6..0]
bits 5..0: 1bit per 4 channels
bit 6:     1bit for 1 channel (top one, common one).
0 -> enable driver   (fanout)
1 -> enable receiver (fanin)
i.e. 
0x40 reading indicates fanOUT
0x3f reading indicates fanIN
*/
#define DELAY0 0x40   /* DELAY1: 0x50 DELAYN:0x40+N*0x10  */
#define DELAY1 0x50
#define DELAY2 0x60
#define DELAY3 0x70
#define DELAY4 0x80
#define DELAY5 0x90
#define DELAY6 0xa0
#define DELAY7 0xb0
#define DELAY8 0xc0
#define DELAY9 0xd0
#define DELAY10 0xe0
#define DELAY11 0xf0
#define DELAY12 0x100
#define DELAY13 0x110
#define DELAY14 0x120
#define DELAY15 0x130
#define DELAY16 0x140
#define DELAY17 0x150
#define DELAY18 0x160
#define DELAY19 0x170
#define DELAY20 0x180
#define DELAY21 0x190
#define DELAY22 0x1a0
#define DELAY23 0x1b0

#define CONTROL_REG 0x1c0   
/* default: not known (should be set by sw to 0)
[1:0] FO clock phase in cca 6.25ns steps
      0: c0 clock   1: c1 clock 2: c2 clock 3: c3 clock
[2]   FO 1: DelayEnabled (DELAY* registers are valid). Default:0 
[3]   FI 1: Enable BUSYTEST bit
[4]   FI BUSYTEST bit -propagated to FI output if [3] is set to 1
[8-5] STATUS_CLOCK readonly. -result of phase measurement 
*/
#define BUSY_MASK 0x1d0
/* 0xffffff -masking 23-0 channels. 1:enabled 0: disabled
fanio output: top connector
*/
#define L0_COUNTER  0x1e0   /* number of successfuly received L0 (fanout) */
#define READ_INPUTS 0x1f0   /* [23:0] - status of inputs for FI */
#define c0_counter  0x200   /* counting L0 with clocks c0,1,2,3 */
#define c1_counter  0x210
#define c2_counter  0x220
#define c3_counter  0x230
/*REGEND */
