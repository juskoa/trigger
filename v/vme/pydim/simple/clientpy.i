%module clientpy
%{
#include <string.h>

extern int dicxcmnd_callback(char *cmd, char *message);
extern int dicxinfo_service(char *cmd);
extern char *waitinfocall(int secs);
extern void dicxrelease_service(int id);
extern char *test(char *instr);
%}

extern int dicxcmnd_callback(char *cmd, char *message);
extern int dicxinfo_service(char *cmd);
extern char *waitinfocall(int secs);
extern void dicxrelease_service(int id);
extern char *test(char *instr);

