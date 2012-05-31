#define MAXLINELENGTH 800
enum Ttokentype{tNONE, tERR, tSYMNAME, tSTRING, tINTNUM, tHEXNUM,
                tVAR, tFUN, tVMEADR, tCHAR,
                tEOCMD, tFLOAT,
             /* operators start */
                tASSIGN, tEQS, tPLUS, tMINUS, tMULT, tDIV,
             /* operators end */
                tLEFT, tRIGHT, tLEFTA, tRIGHTA,
                tCOMMA, tDOT };

void copy2nl(char *dest, char *src, int maxlen);
enum Ttokentype nxtoken(char *line, char *cmd, int *ix);
enum Ttokentype nxtoken1(char *line, char *cmd, int *ix);
void getRestLine(char *line, char delim, char *rest);
int getNextLine(char *str);
int str2int(char *decnumber);
int hex12int(char hexdigit);
int hex2int(char *decnumber);
int gethexdec(char *hexdecnum, unsigned int *num);
void UPPER(char *);
void LOWER(char *);
int stringStarts(char *str, char *strc);
int getNextFunName(char *line, char *name);

