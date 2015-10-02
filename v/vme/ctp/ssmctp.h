#ifdef SSMCTP
#define EXTRN
#else
#define EXTRN extern
#endif

#define Mega 1024*1024
#define Orbit 3564
/* the following number defines max. number of snapshot memories
 * analysed in 1 time (including LTUs plugged in other crates)
 */
#define NLM0SSM 6  //  number of used blocks of LM0 ssm
#define NSSMBOARDS (NCTPBOARDS+6+NLM0SSM)   // + 4xltu + test + none
#define MAXNAMES 300      /* max. number of all names in CFG/ctp/ssmsigs/backplanefp.names */
#define NAMESIZE 20       /* length of name in char*/
#define MAXSSMMODE 20       /* length of mode in char*/
#define FILENAMESIZE 2*NAMESIZE+3

typedef struct Signal{
 struct Signal *next;
 //struct Signal *prev;
 struct Signal *first;
 int channel;            // channel
 int signamenum;         // numerical name according ConnectionNames
 char signame[NAMESIZE]; // char name
 //char pattern[200];     // pattern of the signal (max 200/4=50 hexa digits)
 int patlen;              // length of pattern
 int offset;              // beginning of signal on receiving board (>orbit off)
}Signal;

typedef struct {
  char name[NAMESIZE];   /*  name of the board */
  char mode[MAXSSMMODE]; /* Mode examples: fo_igl0l1 ltu_i1 */
  char ltubase[9];     /* '\0' if not ltu (or ltu not in the crate) */
  int syncmode; /* sync of HW and sms[].sm. We may not use it. */
  w32 *sm;    /* SSM content (Mega words). NULL - SSM not read. */
  w32 offset;  /* after syncSSM */
  int syncflag;/* set by syncSSM, set to -1 by readSSM */
  w32 orbit;   /* channel or mask for orbit for this board */
  Signal *signal; /* signal list */
  w32 lopmode;    /* last op. mode (set in setomSSM).The must for LM0 board! */
} Tsms;

EXTRN Tsms sms[NSSMBOARDS];
#ifdef SSMCTP
int SYNCFLAG=1;
#else
EXTRN int SYNCFLAG;  /* used to set sms[].syncflag for synced SSMs. 
                    This global variable is increased by syncSSM always 
                    after being used */
#endif

#undef EXTRN

/*int setSSM(int board,char *mode, w32 omiocs); */
void initSSM();
void gettableSSM(int flag);
w32 getswSSM(int board);
void setsmssw(int board, char *newmode);
int setomSSM(int board, w32 omiocs);    // rc=0 -> OK
int startSSM(int n, int *boards);
int startSSM1(int board);
int stopSSM(int board);
int condstopSSM(int board, int cntpos, int maxloops, 
  int sleepafter, int customer);
int readSSM(int board);
int writeSSM(int board);
int dumpSSM(int board, char *filename);
void printSSM(int board, int fromadr);
int readSSMDump(int board,char *filename);


