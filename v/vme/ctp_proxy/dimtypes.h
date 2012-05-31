#include "dimctypes.h"
typedef struct Tswtrgreq{
char cidat[80];   // "" -> free item
int cid;
int bc;       // bc (valid for sync/cal. Always 8 (see dimservices.c)
int N;            // required and generated # of triggers
int Ngenerated;   // -1 before sw trg. generation is started
int roc;          // readout control bits
char name[MAXNAMEL];   // detector name
char pf[MAXNAMEL];     // past/future definition, symb. name
char bcmask[MAXNAMEL]; // symb. name for BC mask -valid for Async. tiggers
char type;        // s a c. 's' not supported from dimcswtrg.c (client)
} Tswtrgreq;

#define DBGCMDS 1           // all commands logged on server
#define DBGRESULTcaba 0
#define DBGfindSwTrgReq 0

