#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/* Reset TTCvi and set TTCrx Control register (according to ttcvi.c)
TTCviBaseAddress: ttcvi base address hexa, eg: 0x801000
*/
int ttcreset(char *TTCviBaseAddress) {
int rc=0;
char cmd[80];
char *cfdir;
cfdir= getenv("VMECFDIR");
if(cfdir==NULL) {
  printf("VMECFDIR env. variable not defined\n");
  return(-1);
};
/*rc= chdir(VMECFDIR);
if(rc!=0) {
  printf("chdir(VMECFDIR) error\n");
  return(-1);
};*/
strcpy(cmd, "cd $VMECFDIR ; echo q | ttcvi/ttcvi.exe -noprompt ");
strcat(cmd, TTCviBaseAddress);
//strcat(cmd, " </dev/null >/dev/null");
//strcat(cmd, " </dev/null >/dev/null");
/* strcat(cmd, " </dev/null\n"); */
rc= system(cmd);
/*rc= cctopen();*/
//printf("%s   rc:%d\n", cmd, rc); fflush(stdout);
return(rc);
}
