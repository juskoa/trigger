#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include "vmewrap.h"
#include "ctp.h"
#include "ssmctp.h"

#define DEBFLG 0 
#define TEST NCTPBOARDS+4
#define L0INP 24
#define L1INP 24
#define NCLASS 50
#define D11 2048
#define D12 4096

// size of long int is same as for int
typedef unsigned long long int lint;

//w32 IntPF[Mega];
w32 TestSSM[Mega];
//
//  Hardware table
//
struct{
 int loop;
 // Scaled BC
 w32 sbc1,sbc2;
 w32 intsel;
 //
 // PF protection
 //
 // pfcommon
 w32 delayedINTlut;
 w32 delayINT;
 w32 lut12D;
 // PF circuit
 w32 ipf;
 // interaction1/2/T
 w32 int1,int2,intt;
 // interaction look-up tables A/B
 w32 luta,lutb; 
 // Block A
 w32 deltaTa,THa1,THa2,delayA,nodelayAf,scaleA;
 // Block B
 w32 deltaTb,THb1,THb2,delayB,nodelayBf,scaleB;
 //
 // L0 classes
 //
 int l0input[L0INP][NCLASS]; 
 // L0 invert
 int l0invert[NCLASS];
 // L0 function -> goes to l0 inputs
 int l0function[NCLASS]; 
 // L0 rnd trigger
 int l0rndtrig[NCLASS];
 // L0 scale down
 int l0scldown[NCLASS];
 // Cluster code
 int l0clst[NCLASS];
 // BC mask
 int l0bcmask[NCLASS];
 // p/f protection
 int l0pfprot[NCLASS];
 // all/rare input
 int l0allrare[NCLASS];
 // class mask
 int l0clmask[NCLASS];
 //
 // L1 classes
 // 
 int l1input[L1INP][NCLASS];
 // p/f protection
 int l1pfprot[NCLASS];
 // Cluster code
 int l1clst[NCLASS];
 // RoI Vetp flag
 int l1roiv[NCLASS];
}HardWare;
//
// List
//
typedef struct List{
 struct List *next;
 struct List *first;
 int intnum;
}List;
List *addnum(List *last,int num);
List *freenums(List *list);
void printlist(List *list);
//ssmconnection.c
//
// Main program
//
int A2BCD(int ,...);
int initA2B(char bomodes[][FILENAMESIZE],char modes[][NAMESIZE],
		int *boards,w32 *modecodes);
//
// Input/Output
//
char *ConnectionNames[MAXNAMES]; /* used to store names of backplane 
				and Front Pannel connections. 
				The order number in array is used as
		                numerical name of the connection. */
void initNamesCon();
int nofnames; 
int ParseNames(int n,char bomodes[][FILENAMESIZE],char modes[][NAMESIZE],int *boards);
int readMODE(int board,char *filename,w32 *modecode);
int PrintConnections(int n, int *boards, int mode);
int nonemptyline(char *line);
int removespace(char *line);
int parsemode(char *s,w32 *modecode);
int setoffsetsBR(int n,int *boards);
//
// Signal manipulation
//
void setseeds(long seed,int lux);
double rnlx();
int GetSignals(int n, char bomodes[][FILENAMESIZE],int *boards,w32 *modecodes);
int FindOrbitChannel(int n,int *boards);
int syncSSM(int n, int *boards);
int syncSSM2(int n, int *boards);
int syncSIG1(int *sm,int channel);
int syncSIG1a(int *sm,int channel,int start);
int syncSIG2(int board,int channel,char *p);
int compSIG(int board1,int chan1,int board2,int chan2,int offset2,int start);
int compSIG1(int board1,int chan1,int board2,int chan2,int offset2,int start);
int compSIG2(int *board1,int chan1,int board2,int chan2,int offset2,int start);
int compSIG3(int *board1,int chan1,int board2,int chan2,int offset2,int start);
int compSIG4(int *board1,int chan1,int board2,int chan2,int offset2,int start,int flag);
int compSIG5(int *sm,int board,w32 mask,int offset,int start);
void compMessage(char *name,int board,int channel,int j);
int scan(int board, int *offset);
int bit(w32 num,int channel);
int wbit(w32 num,w32 bit,int channel);
//
// Pattern generation
// 
int genSeq(int board,int Period,int Start);
int writeSPn(int board,int Period,int Start,w32 n);
int writeSPb(int board,int Period,int Start,int Channel);
int writeSPP(int board,int Start,int Channel,char *Pattern);
int writeSPF(int board,int Start,int Channel,char *Pattern);
int form2i(char *patin,int *patout,int *nperiod);
int char2i(char a);
char i2char(int i);
char *getPatfromF(w32 *file,int channel,int length);
//
//Hardware
//
int readSSM(int); 
int writeSSM(int);
int WriteBoards(int n,int *boards,w32 modecode,w32 submode);
int StartBoards(int n,char bomodes[][FILENAMESIZE],int *boards,w32 *modecodes,w32 submode);
int ReadBoards(int n, int *boards);
int startSSM(int n, int *boards);   // Start from 0->n ; rc=0->OK
//
//  Logic Modes
//  
int InitMode(int n,int *boards, w32 modecode,w32 submode); 
int SetBoardHW(int n,int *boards,w32 modecode,w32 submode);
int CheckMode(int n,int *boards, w32 modecode,w32 submode); 
// end of ssmconnection.c

//ssmconfo.c :
int setBoardFO(int n,int *boards,w32 modecode,w32 submode);
int checkModeFO(int n,int *boards,w32 modecode,w32 submode);
int writeBoardsFO(int n,int *boards,w32 modecode,w32 submode);
int Connect(int n, int *boards);
int ConnectPat(int n, int *boards);
int FOL0L1(int n, int *boards);
int FOL2(int n,int *boards);
void getCluster(int board);
void warnmess(char *mode,int board,char *signame,char *message);
//ssmconl0.c
void setlinksl0();
int initModeL0(int n,int *boards,w32 modecode,w32 submode);
int startBoardsL0(int n,int *boards,w32 modecode,w32 submode);
int setBoardL0(int n,int *boards,w32 modecode,w32 submode);
int checkModeL0(int n,int *boards,w32 modecode,w32 submode);
int writeBoardsL0(int n,int *boards,w32 modecode,w32 submode);

int alignStrobes();
int L0ingen(int n,int *boards);
int genl0clustDT(int ctpbusy,int l0strobe,int l0classes[][],int board,w32 *sm,int *start);
int findactivel0(int l0classes[][]);
int findchans(int board,int *smclst,int *sml0s,int *smdt,int *smdata);
int getPaFu(int board,int l0class,int ibc);
int inputsAND(int board,int l0class,int ism);
int functionL0(int ism,int l0class, int board0);

int calcPF(int board1,int board2,int ipf);
//int calcPFnodel0(int board,w32 scale,w32 delay);
//int calcPFnodel1(int board,w32 scale);
int settestPF(int board,int ipf);
int setrndPF(int board,int ipf);
int setrndPF2(int board);
int initInt12(int board);
int setallPF(int board);
int testallPF(int board);
//  ssmconl1.c
void setlinksl1();
int initModeL1(int n,int *boards,w32 modecode,w32 submode);
int checkModeL1(int n,int *boards,w32 modecode,w32 submode);
int setBoardL1(int n,int *boards,w32 modecode,w32 submode);
int writeBoardsL1(int n,int *boards,w32 modecode, w32 submode);
int calcPFL1a(int board1,int ipf);
int testallPFL12(int board);
int l1class(int *boards);
//ssmconl2.c
int initModeL2(int n,int *boards,w32 modecode,w32 submode);
int checkModeL2(int n,int *boards,w32 modecode,w32 submode);
int setBoardL2(int n,int *boards,w32 modecode,w32 submode);
int writeBoardsL2(int n,int *boards,w32 modecode,w32 submode);
//ssmconbu.c
int initModeBU(int n,int *boards,w32 modecode,w32 submode);
int checkModeBU(int n,int *boards,w32 modecode,w32 submode);
int setBoardBU(int n,int *boards,w32 modecode,w32 submode);
int writeBoardsBU(int n,int *boards,w32 modecode,w32 submode);
//none.c
int initModeN(int n,int *boards,w32 modecode,w32 submode);
int startBoardsN(int n,int *boards,w32 modecode,w32 submode);
int checkModeN(int n,int *boards,w32 modecode,w32 submode);
int setBoardN(int n,int *boards,w32 modecode,w32 submode);
int writeBoardsN(int n,int *boards,w32 modecode,w32 submode);
int calcPFTL1(int board1,int board2,int ipf,int boffset);
int L2a2Interface(int boardl1,int boardl2,int boardint);
int bcoffset(int boardl2,int boardint);
int searchL0();
//CTPRIRDList *getL2alist(int boardl2,CTPRIRDList *L2alist);
//CTPRIRDList *getCTPRIRDList(int boardint,CTPRIRDList *list);
int dumpIntSsm(int board);
//int getL2alist();
//ssmconint.c
int initModeINT(int n,int *boards,w32 modecode,w32 submode);
int checkModeINT(int n,int *boards,w32 modecode,w32 submode);
int setBoardINT(int n,int *boards,w32 modecode,w32 submode);
int writeBoardsINT(int n,int *boards,w32 modecode,w32 submode);
// data.c

int printALL();
int alignStrobes(int n, int *boards);
//  pastfun.c
void setPFCOMMON(int board);
void setPFBLOCKA(int board,int ipf);
void setPFBLOCKB(int board,int ipf);
void setPFLUT(int board,int ipf);
int lookup4(int in1,int in2,int lupt);  
int lookup8(int in1,int in2,int in3,int lupt);
int getPFHW(int board,int ipf);
void printPFHW();
int PFcircuit(int board,int ipf,int scale_offseta,int scale_offsetb,int int1chan,int int2chan);
int PFcircuit1(int board,int ipf,int scale_offseta,int scale_offsetb,int int1chan,int int2chan,int int3chan);
int PFcircuit2(int board,int ipf,int scale_offseta,int scale_offsetb,int int1chan,int int2chan,int int3chan,int start,int ic);
int PFcircuit3(int board,int ipf,int scale_offseta,int scale_offsetb,int int1chan,int int2chan,int start,int ic);

//
// Signal list methods
// 
/* Signal is basic structure.
 * Signals for each board are saved as list:
 * sig1->sig2->sig3->null
 * looping over signals:
 *  sig=sms[i].signal->first;
 *  while(sig)sig->next;
*/ 
Signal *addSignal(Signal *last,int channel,int namenum, char *namechar);
Signal *findSignal(int board, int namenum,char *name);
Signal *findSignalS(int board, int namenum,char *name);
int freeSignals(int n,int *boards);
char *findChannel(int board,int channel);

