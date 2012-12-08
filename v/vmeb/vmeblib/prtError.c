#include <stdio.h>
void getdatetime(char *);
/* prtError.c */
/*------------------------------------------------------prtWarning(char *msg)
Print error.
*/
void prtWarning(char *msg) {
char dt[20];
getdatetime(dt);
printf("%s: ***  Warning:%s\n",dt,msg); fflush(stdout);
}
/*------------------------------------------------------prtError(char *msg)
Print error.
*/
void prtError(char *msg) {
char dt[20];
getdatetime(dt);
printf("%s: ***  Error:%s\n",dt,msg); fflush(stdout);
}
/*----------------------------------------------------*/ 
void prtLog(char *msg) {
char dt[20];
getdatetime(dt);
printf("%s: %s\n", dt, msg); fflush(stdout);
}
/*------------------------------------------------------intError(char *msg)
Internal error.
*/
void intError(char *msg) {
char dt[20];
getdatetime(dt);
printf("%s: *** Internal Error:\n%s\n\n",dt,msg); fflush(stdout);
}

