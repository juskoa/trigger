#define MAXNAMELENGTH 80        // paths
#define MAXCTPINPUTLENGTH 40    // ctp input name length
#define MAXPARTNAME 12
#define MAXDETNAME 12
#define ERRMSGL 300
// max. line length in .pcfg file: BCMASKS: 'BCMASKS '+3*3564+'\0' = 10701
// or 'L0F34 '+ ~ 4*1024 + symb.definitions
#define MAXLINECFG 10800
#define MAXNLINES 256      // not more lines allowed in .pcfg
#define NCLASS 50
#define NCLUST 6
#define NFO 6
#define NCON 4        // # of connectors at FO
#define NDETEC 24
#define NCTPINPUTS 60    // 24+24+12
#ifndef MNPART
#define MNPART 6
#endif
#define BCMASKN 12      // always 12 (also for fy <=AB, was 4 before)
// l0f1/2 rnd1/2 bc1/2 for L0 fimware AB, bits: 24 25 26 27 28 29
#define L0CONBIT0 24   
#define L0CONBITL 29
// 24-25: l0f1/2  26-27:l0f3/4 28-29:rnd1/2 30-31:bc1/2 for L0 fimware >= AC
#define L0CONBITLac 31
/* debug prints: 1-print, 0-no print.
*/
#define DBGcfgin 0     // input file .pcfg processing
#define DBGmask  0     // input file .pcfg proc -masking (applyMask)
#define DBGpfs  0     // input file .pcfg proc -masking (applyMask)
#define DBGparts 0     // adding/deleting partitions  in Partitions
#define DBGac2HW 0     // addClasses2HW()
#define DBGaf2HW 0     // addFO2HW()
#define DBGbusy  0     // busy handling
#define DBGswtrg 0     // SW trigger (generateXOD)
#define DBGcnts  0     // read and print counters (LTU, CTP)
#define DBGlogbook 0   // updateDAQClusters()
#define DBGgetInputDets 0   // getInputDets()
#define DBGcumRBIF 1   // cumRBIF()
#define DBGCLGROUPS 1  // class groups (time slots on/off for class groups)
#define DBGrbif 0

#include "bakery.h"
/* the following symbol defined only for development:
see dims.c dimservices.c
*/
//#define DEVELOP
//#define TEST

// where .pcfg files are (relative to VMECFDIR directory)
#define PARTDBDIR "CFG/ctp/pardefs/"
// databse files (valid.ltus,...):
// not used. Instead: $VMECFDIR/CFG/ctp/DB
//#define CFGDBDIR "/home/alice/aj/v/vme/CFG/ctp/DB/"
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
typedef struct Tinput{
  char name[MAXCTPINPUTLENGTH];   // "" empty item
  int detector;      // ECS/DAQ Detector number (0..) or -1 if not found
  int level;         // 0,1,2
  w32 signature;
  int inputnum;      // 1..24 (for level 0)
  int dimnum;        // order number of cable. 1 if only 1 input cable given
  int configured;    // 1:configured   0: not configured
  int edge;          // 0:positive  1: negative -1: notdefined
  int delay;         // 0-15 in BCs   -1:not defined
  int deltamin;
  int deltamax;
}Tinput;

typedef struct Tdetector {
char name[MAXDETNAME];  // "": empty item
int fo;       //1-6   0: detector not connected
int foc;      //1-4   FO connector
int busyinp;  //1-24   0: no busy connected
int detnum;   //0...  the same as position of this item in validLTUs
int i2cchan;  //0-7. -1:not connected
int i2cbran;  //0-7  valid only if i2cbran != -1
// following items for debbugging (following counters ctp,ltu,ddg,daq...)
int ltuvsp;   //  -2:default, -1:don't try to open it, >0: LTU vme opened
char ltubasea[12];// ltu base address or LTUDIM name. "":not present
w32 ctpl2strosod,ctpl2stro, //run statistics
    ctpl1outsod,ctpl1out,  
    ctpl0outsod,ctpl0out, 
    ltul2asod,ltul2a,
    ddgl2a;                 //not used
} Tdetector;

/*---------------------------------------------------------------
   TKlas declarations
-----------------------------------------------------------------*/
/*CLA.02 0xffffffff 0x0 0x1fff0 0x0 0xfffffff 0x0 0xf000fff
      class # (01-50)
         L0condition    L0veto      L1definition  L2definition
                    L0invert    L0scaler      L1invert
Note:
bit17 (0x10000) of L0veto is CLASS MASK bit written into HW as
bit31 for firmAC. 
bit0 of L0_MASK word ->see setClass() in ctp.c
L0veto:
bits[2..0] Cluster code
bit12      All/Rare flag (1: All  0: Rare)
#define mskCLAMASK 0x10000   -we better throw it out (is bit31 with firmAC)
*/

typedef struct TKlas{
 w32 l0inputs;
 w32 l0inverted;
 w32 l0vetos;      // bit17: 1:class not choosen  
                   //bit[2:0] hwcluster (1-6) ONLY IN HW variable
 w32 scaler;
 w32 l1definition;
 w32 l1inverted;
 w32 l2definition;
 int hwclass;     // in Tpartition: 0..49 (valid if hwallocated ==2 or 3)
                  // in Hardware: 0 not allocated, 1: allocated
 char *partname;  // !can be not valid (in load2HW when STOP partition)
 int classgroup;  // index to ClassGroup[]. 0: no classgroup assigned
 int sdg;         // index into SDGS. -1 if not SDG class
}TKlas;

/* old definition (w.r.t. level, never used):
typedef struct TPastFut{
 w32 pf_common;
 w32 pf_blockA[5];
 w32 pf_blockB[5];
 w32 pflut[5];
}TPastFut;   */
/* new definition from 26.9.2011: 
PF_N_BCS THa1 dTa THb1 dTb INTa INTb Delayed_INT THa2 THb2 P_signal 
                  63   62  0xa  0x0  0x0         63   63   2
*/
#define ixTHa1 0
#define ixdTa 1
#define ixTHb1 2
#define ixdTb  3
#define ixTHa2 4
#define ixTHb2 5
#define ixP_signal 6
// 9 values (x=L0/1/2): PFBLOCK_Ax PFBLOCK_Bx PFLUTx PFBLOCK_Ax... (was 7)
#define ixMaxpfdefs 9
#define ixINTa 0
#define ixINTb 1
#define ixDelayed_INT 2
// 3 values (x=L0/1/2) PF_COMMONx (was 3 before 23.10.2011)
#define ixMaxpfdefsCommon 3
typedef struct TPastFut{
 w32 pfdefs[ixMaxpfdefs];
}TPastFut;
typedef struct TPastFutCommon{
 w32 pfdefsCommon[ixMaxpfdefsCommon];
}TPastFutCommon;

/*-----------------------------------------------------------------
Strucutre TRBIF declarations  (RandomBcdownscaledInteractionsFunctions)
-------------------------------------------------------------------*/
// RandomBCdownscaledIntFuncs=TRBIF
//                                 # inputs in lookup table (lut)
//        rnd1   rnd2bc1 bc2   char  !  !- lut  
//rbif 0x2828282:0x0:0x1:0x0:['0x0', 4, 0]:['0x0', 4, 0]:['0x0', 4, 0]:
//                                 intfun1     intfun2        inttfun  
//['0x0', 4, 0]:['0x0', 4, 0]
//   l0fun1         l0fun2
// TODO check order with parted syntax
#define ixrnd1 0
#define ixrnd2 1
#define ixbc1 2
#define ixbc2 3
#define ixl0fun1 4 
#define ixl0fun2 5
#define ixintfun1 6
#define ixintfun2 7
#define ixintfunt 8
#define ixlut3132 9
#define ixlut4142 10
#define ixrbifdim 11 
#define notused 51
#define nothwal 61
#define L0INTFSMAX 64
#define L0F34SDMAX 200

#define LEN_l0f34 4096
//debug case: corresponds to [a b c d e f] in Ctpcfg.__init__(
// Ctpconfig.dbgbits=6 (real:12)
//#define LEN_l0f34 64 

#define ORBITLENGTH 3564

typedef struct TRBIF{
 w32 rbif[ixrbifdim];    // value (valid according to rbifuse[])
 w32 rbifuse[ixrbifdim]; 
 //notused  :not used by this partition , rbif[ix] irrelevant
 //nothwal  : used but not allocated to hw, requested value in rbif[ix]
 // in hw allocated at hw.rbif[rbifuse[ix]]
 w32 intsel;
 //w32 bcmask[4];
 char l0intfs[5*L0INTFSMAX];  // l0f1/2 int1/2/t as a text string
 //char BCMASK[ORBITLENGTH+1];  // '1','2',...,'f'
 w16 BCMASK[ORBITLENGTH+1];  // '1','2',...,'f' ... 'fff' for 12 BC masks
 w8 BCMASKuse[12];             // same as rbif 0:not used, 1..12: bcm1..12 used
 TPastFut pf[5];
 w8 PFuse[5];             // 0:not used, 1..5: pf1..4 used [4]==5:PFT used
 TPastFutCommon pfCommon;
 w8 PFCuse;               // 0 not used (PF not used at all), 1: used
 //w8 rare;   //all_rare_flag word is actually not real RBIF
 w8 lut34[LEN_l0f34];   // pointer to 2*2x1024 bytes (lut31+lut32 + lut41+lut42)
 // 1byte: 4 rigthmost bits contain 4 bits of lut
 char l0f3sym[L0F34SDMAX];  // symbolic representation
 char l0f4sym[L0F34SDMAX];
}TRBIF;

// TRBIF:
TRBIF *allocTRBIF();
void cleanTRBIF(TRBIF *rbif, int leaveint);
void printTRBIF(TRBIF *rbif);
/*-----------------------------------------------------------------
    TFO,TBUSY,TpPastFut declarations
-------------------------------------------------------------------*/

typedef struct TFO{
 w32 cluster;
 w32 test_cluster;
}TFO;

typedef struct TBUSY{
/* 1-6:cluster1-6, 0:Test cluster
   set_cluster[][23..0] -> 1: subdetector is in this cluster
*/
 w32 set_cluster[NCLUST+1];  
}TBUSY;

typedef struct TSDGS{
  char name[MAXPARTNAME];   // symbolic SDG name. "": empty field
  char pname[MAXPARTNAME];  // symbolic SDG name
  w32  l0pr;                // calculated from n% (always rnd downscale)
  int firstclass;  //1..50, 0: not allocated yet
}TSDGS;

// Clean TFO
void cleanTFO(TFO *fo);
void copyTFO(TFO *to,TFO *from);
// Clean TBUSY
void cleanTBUSY(TBUSY *busy);
void copyTBUSY(TBUSY *to,TBUSY *from);

// Clean TPastFut
//void cleanTPastFut(TPastFut *pf);
//void copyPF(TPastFut *to,TPastFut *from);
//void copyPFC(TPastFutCommon *to,TPastFutCommon *from);
//
/*---------------------------------------------------------------
   Structure Hardware declaration.
----------------------------------------------------------------*/
typedef struct Hardware{
 //int ClusterCode; // Clusters for ClusterCode
 char name[4];
 TKlas *klas[NCLASS];   // NULL: never (for allocation see klas.hwclass)
 TRBIF *rbif;
 TFO fo[NFO];           // clust. is free if there is no bit ON in fo[0-5]
 TBUSY busy;
 int sdgs[NCLASS];   // Default: 0..49. Different in case if SDG active 
     // for given class used to fill L0_SDSCG registers
}Hardware;
// Clean existing HW structure
void cleanHardware(Hardware *hw, int leaveint);
// Print Hardware
void printHardware(Hardware *part, char *dbgtext);
void copyHardware(Hardware *to,Hardware *from);
/*--------------------------------------------------------------
   Tpartition declarations
*/
/* Shared memory:
15.10.2009: only allocated in ctp_Init()
*/
typedef struct Tpartitionshm{
 char name[MAXNAMELENGTH];   // "" not allocated
 w32 Detector2Clust[NDETEC];   /* logical clusters detector belongs to */
 w32 run_number;  /* unique for each Partition */
 w32 paused;      // 0: running 1: paused
}Tpartitionshm;
#define FLGignoreDAQLOGBOOK 0x1
#define FLGignoreDAQRO 0x2
#define FLGignoreGCALIB 0x4
typedef struct Tctpshm {
char datetime[20];  // '\0' or dd.mm.yyyy hh:mm:ss of last SHM init
Tbakery swtriggers; // see ctplib/swtrigger.c
Tbakery ccread;  // ctp counters readings (see ctplib/readCounters.c)
w32 GlobalFlags;
w32 active_cg;  // active classgroup. 255 during PAUSE
Tdetector validLTUs[NDETEC];
Tinput validCTPINPUTs[NCTPINPUTS];
Tpartitionshm startedParts[MNPART];
w8 lut34s[LEN_l0f34];
}Tctpshm;

/* info for DAQlogbook gathered [mostly] in ctpproxy and passed to the server
and consequently to daq db 
#define MAXRUN1MSG 500
typedef struct TDAQInfo {
w32 daqonoff;           // 0: CTP readout active
w32 masks[NCLUST];      // detectors in each CLUSTER
w32 inpmasks[NCLUST];   // input detectors feeding each CLUSTER
unsigned long long classmasks[NCLUST];  // classes for each CLUSTER
char run1msg[MAXRUN1MSG];  // orig. rcfg msg (as used before LS1)
} TDAQInfo;
*/
/*--------------------------- fixed counters for MONLUMCNTS service 
typedef struct Tfixed_cnts{
int a970pos;  // position in array970 for counter 0,1,2,...
              // -1: not active, -2: end of record
}Tfixed_cnts;
*/

#define MAXCLASSGROUPS 10
#define CG_NEVERACTIVE 9999
#define CLASSGROUPTIMESLOT 1
typedef struct Tpartition{
 char name[MAXPARTNAME];
 char partmode[MAXNAMELENGTH];
/* klas[i]==NULL:class not allocated. Classes are always 
assigned here from the beginning (0,1,2,...).  I.e. after first 
appearance of NULL, there are only NULLs, before applying det. mask.
After applyMask(), there can be holes (i.e. always go through
all classes (if MaskedDetectors !=0) */
 TKlas *klas[NCLASS];       
 TRBIF *rbif;       // private (allocated for each partition)
/* Table HWRBIF - PartRBIF is in TRBIF: rnifuse[]*/
/* Table HW cluster ->> Part Cluster 
pclust=ClusterTable[hwclust]   
hwclust = [0..5],pclust=[1..6] ClusterTable is redefined 
each time partitions are merged. But stays the same over whole 
time when part. is loaded.  */
 w32 ClusterTable[NCLUST];               
/* Detector2Clust[idet] = log. clusters (bits 0..5 correspond to clusters
1..6) for detector idet=0..23
LOGICAL, NOT HW_CLUSTER!
Detector2Clust is created during input file .pcfg reading, 
??? and is be corected in addFO2HW ??? */
 w32 Detector2Clust[NDETEC];
 w32 hwallocated; /* 0:new partition, 
                     0x1: clusters allocated
                     0x2: classes allocated  0x3: both
                  */
 w32 MaskedDetectors;   /* bits 23..0. 1: detector is included in DAQ 
                           -see note in ctp_StartPartition*/
 w32 run_number;  /* unique for each Partition */
 int positionInAllPartitions;   // -1: before included to AllPartitions[]
 // or index in AllPartitions
 int nclassgroups;   // 0: no classgroups for this partition
                     //>0: there are 'class groups' (i.e. only 1 in 1 time)
 int active_cg;   // 0: or pointer to active class group to classgroups[]
 int remseconds;  // remaining seconds in case partition was paused
                  // -1: partition is not in 'paused state'
 int classgroups[MAXCLASSGROUPS];  /* 0: class always IN
 >1 the time (in time slots CLASSGROUPTIMESLOT) when this class is active*/
 w32 lastsecs;   /* start time  of last interval (only 1 group active) */
 w32 lastmics;   /* in secs/mics */
 w32 totalsecs[MAXCLASSGROUPS];   /* active time of this group */
 w32 totalmics[MAXCLASSGROUPS];   /* in secs/mics */
 Tpartitionshm *cshmpart;
 /*Tfixed_cnts *fixed_cnts; not needed (fixed_cnts ONLY for PHYSICS_1)
 perhaps later, if we decide fixed_cnts in any partition!
 */
}Tpartition;

#ifdef DBMAIN
#define EXTERN
Tctpshm *ctpshmbase=NULL; 
Tinput *validCTPINPUTs=NULL;
int clg_defaults[MAXCLASSGROUPS]= {0,1,2,3,4,5,6,7,8,9};
#else
#define EXTERN extern
extern Tctpshm *ctpshmbase;
extern Tinput *validCTPINPUTs;
extern int clg_defaults[MAXCLASSGROUPS];
#endif
EXTERN int ctpsegid;
//extern Tdetector validLTUs[NDETEC];
//extern Tinput validCTPINPUTs[NCTPINPUTS];
EXTERN Tdetector *validLTUs;
EXTERN int NSDGS;
EXTERN TSDGS SDGS[NCLASS];
EXTERN Hardware HW;
EXTERN Tpartition *AllPartitions[MNPART];  //=Started + Loaded
EXTERN Tpartition *StartedPartitions[MNPART];  //=Started only
EXTERN char partmode[40];  // alternative name of the partition file

// Tpartition.c:
//--------------------------------------------------------
int copycheckPF(TRBIF *rbifnew, TRBIF *rbif, int ixpf);
int initHW(Hardware *hw);
// load HW to hw
int load2HW(Hardware *hw, char *tsname);
int readHW(Hardware *hw);

void prtLog(char *msg);
void prtError(char *msg);
void prtWarning(char *msg);
void intError(char *msg);
void cleanTKlas(TKlas *klas);
void copyTRBIF(TRBIF *dst, TRBIF *src);
void copyTKlas(TKlas *toklas,TKlas *fromklas);
void copymodTKlas(TKlas *toklas,TKlas *fromklas,Tpartition *part);
void printTKlas(TKlas *klas,int i);
// Check if Klas has all clusters nonzero (Clu Versus 0)
int checkCluV0TKlas(TKlas *klas);
// Check if Klas has compatible cluster l0=l1=l2 (Clu Versus L0,L1,L2)
int checkCluVL012TKlas(TKlas *klas);
// delete Tpartition (make it free)
Tpartition *deleteTpartition(Tpartition *part);
// Set name
int setnameTpartition(Tpartition *part, char *name);
// Print different structures
void printTpartition(char *headtext, Tpartition *part);
int getNAllPartitions();
void printStartedTp();
void printAllTp();
// Set pointers to NULL, variables to zero Tpartitition != NULL
int applyMask(Tpartition *part, char *detmask);
void initTpartition(Tpartition *part,char *name);
// Get number of clusters in partition
int nofclustTpartition(Tpartition *part,int *nofclus); 
// Get number of detectors in partition
int nofdetecTpartition(Tpartition *part,int *nofdet); 
// Check if detectors in existing partitions
int checkdetTpartition(Tpartition *part,int idet,int *flag);
// Check cluster versus 0
int checkClustV0Tpartition(Tpartition *part);
// Check cluster L0,L1,L2 compatibility
int checkClustVl012Tpartition(Tpartition *part);
// Check if Clusters from triggers are compatible with clusters
// from detectors
int checkTClustVDClustTpartition(Tpartition *part);
// get busymask for pause/resume
w32 getBusyMaskPartition(Tpartition *part);
//int getDAQClusterInfo(Tpartition *part, TDAQInfo *daqi);
w32 findHWCluster(Tpartition *part, w32 pcluster);
int l0condition2rbif(int bit, int *ix);

// clgroups.c
int nextclassgroup(Tpartition *part);
int enableclassgroup(Tpartition *part, int setclgroup);
void disableNEVERACTIVE(Tpartition *part);
void  startTimer(Tpartition *part, int interval, w32 clgroup);
int  stopTimer(Tpartition *part, w32 clgroup);

//ctplib subroutines:
void  readTables();
void copyDetector2Clust(w32 *dest, w32 *src);
void printDetector2Clust(w32 *src);
int findInputName(char *name);
int findInput(int level, int input);
int getEdgeDelayDB(int level, int input, int *edge, int *delay);
void getctp_alignment(Tpartition *partit, char *af, int leng, w32 l0finputs);
Tdetector *findLTUdaqdet(int daqdetnum);
Tdetector *findLTU(char *ltuname);
int findLTUdetnum(char *ltuname);
void bit2name(w32 ctprodets, char *detname);
int detList2bitpat(char *dlist);
int findINPdaqdet(int level, int input);
w32 findBUSYinputs(w32 ctprodets);
void findLTUNAMESby(w32 busypat, w32 detpat, char *names);
int findDETfocon(int fo,int con,char *name);
int Connector2Detector(int fo,int con,int *det);//should go to ctp.h

// cfg2part subroutines:
int char2i(char a,w32 *num);
int string2int(char *c,int length,w32 *num,char b);
int PFL2Partition(char *line,TPastFut *pf);
int BUSY2Partition(char *line,TBUSY *busy);
int FO2Partition(char *line,Tpartition *part);
TKlas *CLA2Partition(char *line, int *error, char *pname);
TRBIF *L0342Partition(char *line,TRBIF *rbif);
TRBIF *BCMASK2Partition(char *line,TRBIF *rbif);
TRBIF *RBIF2Partition(char *line,TRBIF *rbif);
int SDGadd(char *line, char *pname);
int SDGfind(char *name, char *pname);
void SDGinit();
void SDGclean(char *pname);
int ParseFile(char lines[][MAXLINECFG],Tpartition *part);
void readPartitionErrors(int error,char *filename);
Tpartition *readDatabase2Tpartition(char *filename);
void checkPCFG(char *pname, char *msg, int maxmsg);
int getInputDets(TKlas *klpo, Tpartition *part, w32 *l0finputs);
void setglobalflags(int argc, char **argv);

// ctpshm.c
void cshmInit();
void cshmDetach();
void cshmClear();
int cshmGlobFlag(w32 flag);
void cshmSetGlobFlag(w32 flag);
void cshmClearGlobFlag(w32 flag);
void setglobalflag(int argc,char **argv,char *flagName,int flag);
void cshmPrint();
void cshmAddPartition(Tpartition *part);
void cshmDelPartition(char *part);
void cshmPausePartition(Tpartition *part);
w32 cshmQueryPartition(Tpartition *part);
void cshmResumePartition(Tpartition *part);
int cshmGlobalDets();

// others. swtrigger.c:
//void clearflags();
// int GenSwtrg(int n,char trigtype, int roc, w32 BC, w32 detectors, int customer);

