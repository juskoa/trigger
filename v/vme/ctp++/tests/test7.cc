#include "vmewrap.h"
#define NCLUST 6
#define MAXRUN1MSG (1020)
#include <cstdio>
typedef struct TDAQInfo {
w32 daqonoff;           // 0: CTP readout active
char run1msg[MAXRUN1MSG]; // orig. rcfg msg, as used before LS1
w32 masks[NCLUST];      // detectors in each CLUSTER
w32 inpmasks[NCLUST];   // input detectors feeding each CLUSTER
/*unsigned long long classmasks[NCLUST];  // classes for each CLUSTER
does not sent correctly to 64 bits. But w32 ok, i.e.:
*/
unsigned long long  classmasks00_063[NCLUST];
unsigned long long  classmasks64_100[NCLUST];
//logbook_triggerClassMask_t classmask[NCLUST];
//unsigned long long classmask[2][NCLUST];
} TDAQInfo;

int main()
{
 printf("sizeof TDAQInfo: %lu \n",sizeof(TDAQInfo));
 printf("sizeof w32: %lu \n",sizeof(w32));
 printf("sizeof w32: %u \n",sizeof(char));
 printf("sizeof llu: %lu \n",sizeof(unsigned long long));
 return 0;
}
 
