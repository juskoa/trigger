#define MAXNAMEL 32
typedef struct Tswtrg{
int roc;               // 4 readout control bits
int N;                 // number of triggers (<=10)
char name[MAXNAMEL];   // detector name
char pf[MAXNAMEL];     // past/future definition, symb. name
char bcmask[MAXNAMEL]; // symb. name for BC mask for 'a' triggers IGNORED now
                       // "": BCmask is not used
char type;        // 'a':Asynchronous sw trigger 'c':Calibration
} Tswtrg;

