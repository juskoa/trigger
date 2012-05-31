%module clientpy
%{
#include <string.h>

extern void dicxinit();
extern int dicxcmnd_callback(int tag, char *cmd, char *message);
extern int dicxinfo_service(char *cmd);
extern char *waitinfocall(int tag, int secs);
extern void dicxrelease_service(int tag);
extern char *test(char *instr);
%}

extern void dicxinit();
extern int dicxcmnd_callback(int tag, char *cmd, char *message);
extern int dicxinfo_service(char *cmd);
extern char *waitinfocall(int tag, int secs);
extern void dicxrelease_service(int tag);
extern char *test(char *instr);

