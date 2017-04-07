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

int StatusInt;
int StatusFailed= -2;

void uroutine(void *tag, void *rc, int *size) {
printf("uroutine tag:%d rc:%d size:%d\n", *(int *)tag, *(int *)rc, *size);
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
inforc= dic_info_service(cmd, MONITORED, 0, &StatusInt, 4,
//  NULL, 3488, &StatusFailed, 4);
  uroutine, 3488, &StatusFailed, 4);
//printf("%s %d length:%d\n",cmd, StatusInt, sizeof(StatusInt));
pause();
//while(1) { sleep(20); }; 
dic_release_service(inforc);
}
