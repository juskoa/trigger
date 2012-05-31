#ifdef SSMCTP
#define EXTRN
#else
#define EXTRN extern
#endif
EXTRN Tctpboards ctpboards[NCTPBOARDS];

#define Mega 1024*1024
#define Orbit 3600
#define OrbitMask 1   
#define NSSMBOARDS (NCTPBOARDS+4)
#define MAXLINE 256
#define MAXNAMES 100
#define NAMESIZE 10

typedef struct {
  char name[NAMESIZE];   /*  name of the board */
  char modeHW[NAMESIZE]; /* Mode of last recording*/
  char modeSW[NAMESIZE]; /* mode of the data in CPU memory */
  w32 *sm;    /* SSM content (Mega words). NULL - SSM not read. */
  w32 offset;  /* after syncSSM */
  int syncflag;/* set by syncSSM, set to -1 by readSSM */
} Tsms;
EXTRN Tsms sms[NSSMBOARDS];
#ifdef SSMCTP
int SYNCFLAG=1;
#else
EXTRN int SYNCFLAG;  /* used to set sms[].syncflag for synced SSMs. 
                    This global variable is increased by syncSSM always 
                    after being used */
#endif
