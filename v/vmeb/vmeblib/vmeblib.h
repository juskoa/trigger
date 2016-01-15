#include "vmewrapdefs.h"
//prtError.c
void prtError(char *msg);
void prtWarning(char *msg);
void prtLog(char *msg);
void intError(char *msg);
void printenvironment();

//ranlux.c
void setseeds(long seed, int lux);
double rnlx();
void prtfnames(char *dirname, char *suffix);

//
w32 loadFPGA(int boardvsp);

//timeroutines.c
void getdatetime(char *dmyhms);
void GetMicSec(w32 *tsec, w32 *tusec);
void AddSecUsec(w32 *tsec,w32 *tusec,w32 plussec,w32 plususec);
void SubSecUsec(w32 *tsec,w32 *tusec,w32 plussec,w32 plususec);
w32 DiffSecUsec(w32 tsec,w32 tusec,w32 prevtsec,w32 prevtusec);
void DiffSecUsecFrom(w32 fromsec,w32 fromtusec, w32 *diff_s,w32 *diff_u);

void micwait(int micsecs);
void prtProfTime(char *name);

//dodif32.c
w32 dodif32(w32 before, w32 now); 
w32 rounddown(float f);
int w32toint(w32 w);

// lexan.c
int remove_char_from_string(char c, char *str);
unsigned int hex4(char *m4);
int hex12int(char c);
char int12hex(int numint);
void UPPER(char *);

//detectfile.c, environment.c
int detectfile(char *name, int secs);
int readfile(char *fname, char *mem, int maxlen);
void readpw(char *facility, char *mem);
void printenvironment();
int envcmp(char *name, char *value);
int popenread(char *cmd, char *output, int leng);
int isArg(int argc, char **argv, char *isin);
void do_partitionCtpConfigItem(char *pname, char *partitionCtpConfigItem);

//act routines:

//dimwrap.c
int getCALIBBC(char *detname);
int get_DIMW32(char *service);

// daq
int daqlogbook_open();
int daqlogbook_close();
int daqlogbook_update_triggerClusterCounter(unsigned int run, unsigned int cluster, unsigned long L2aCount);
int daqlogbook_update_triggerClassCounter(unsigned int run, unsigned char classId, unsigned long long LMbCount, unsigned long long LMaCount, unsigned long long L0bCount, unsigned long long L0aCount, unsigned long L1bCount, unsigned long L1aCount, unsigned long L2bCount, unsigned long L2aCount, float ctpDuration);
int daqlogbook_update_triggerDetectorCounter(unsigned int run, const char *detector, unsigned long L2aCount);
int daqlogbook_update_triggerInputCounter(unsigned int run, unsigned int inputId, unsigned int inputLevel, unsigned long long inputCount);
int daqlogbook_update_triggerGlobalCounter(unsigned int run, unsigned long L2aCount, float ctpDuration);

//ctp database
void mydbDisconnect();
int mydbConnect();
void red_update_detsinrun(int runn, unsigned int detpattern);
void red_clear_detsinrun(int runn);
unsigned int red_get_detsinrun(int runn);
void red_get_runs(int *runs, int *dets);

