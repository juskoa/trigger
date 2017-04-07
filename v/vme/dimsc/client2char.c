#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef CPLUSPLUS
extern "C" {
#include <dic.hxx>
}
#else
#include <dic.h>
#endif
 
char cmd[80];

#define MXLINE 100
char StatusChar[MXLINE];
char StatusFailed[]="failed";

void uroutine(void *tag, void *buffer, int *size) {
printf("uroutine tag:%d buffer:%s size:%d\n", *(int *)tag, (char *)buffer, *size);
}

int main(int argc, char **argv) {
int i,ixslash;
unsigned int inforc;
if((argc<2) || (argc>2)) {
  printf("Start:\n\
client2 service/name\n\
\n\
Using TTCMI/QPLL\n\
\n");
  //exit(4);
  strcpy(cmd, "TTCMI/QPLL");
} else {
  strncpy(cmd, argv[1], 80);
};
/* service */
inforc= dic_info_service(cmd, MONITORED, 0, StatusChar, MXLINE,
//  NULL, 3488, &StatusFailed, 4);
  uroutine, 3488, StatusFailed, sizeof(StatusFailed));
//printf("%s %s length:%d\n",cmd, StatusChar, sizeof(StatusChar));
pause();
//while(1) { sleep(20); }; 
dic_release_service(inforc);
}
