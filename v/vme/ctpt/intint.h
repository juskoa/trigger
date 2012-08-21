//inint.c
// CTPReadout structure
//
typedef unsigned long long int lint;
typedef struct CTPR{
 int l2clusters;
 lint l2classes;
 int bcid;
 int orbit;
 int eob;   // used as eob flag, other items shoyld be zero
 int esr;
 int clt;    // calibration trigger
 int swc;    // software class
 int issm;   // position of the first word in ssm
}CTPR;

//Interaction Record
typedef struct IRDda{
 int error;
 int orbit;
 int Inter[251];
 int bc[251];
 int issm;
}IRDa;

//
typedef struct CTPRIRDList{
 struct CTPRIRDList *next;
 struct CTPRIRDList *first;
 CTPR *ctpr;
 IRDa *irda;
}CTPRIRDList;

// intint.c
CTPRIRDList *getCTPRIRDList(int boardint,CTPRIRDList *list);
void printBCs(CTPRIRDList *list,FILE *file);
CTPRIRDList *freenumsN(CTPRIRDList *list);
void initprintBCs();
/*FGROUP IntBoard
Do the following for l1, l2, int boards:
Start After 27ms, Read into ssm[] 
Out: message issued if time between 1st board start and last board start
too high (>80us).
rc: 0: ok
   >0: error (printed to stdout)
*/
int startRead3SSM();

/*FGROUP IntBoard
 *  analyze interface board data
 *  L2alist - list of CTP readout from L2 board
 *  INTlist - list of CTP readout and IR data from Interface board
-dump to $VMEWORKDIR/IntList.txt
in: 2 3 4  (l1 l2 int board index in ssm[])
Note: start startRead3SSM() before
*/
int L2a2Interface(int boardl1,int boardl2,int boardint);
/*FGROUP IntBoard
 * Dumps Interface board ssm. Word is in output if any bit nonzero.
 * SSM should be read before
*/
int dumpIntSsm(int board);
/*FGROUP IntBoard
 * get L2 message from L2 board
- dump something to file $VMEWORKDIR/L2amessageList.txt
 */
int dumpL2amesage(int board);

