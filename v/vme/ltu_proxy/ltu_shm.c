/* ctp shared memory access main
SHM: ipcs -m, ipcrm shm SHMID
*/
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include "vmewrapdefs.h"
#include "shmaccess.h"
#include "lexan.h"
#include "vmeblib.h"
#define LTUMAIN
#include "ltu.h"
#define PROXY_MAIN
#include "ltu_utils.h"
int quit;
/*----------------------------------------*/ int main(int argc, char **argv) {
int rc=0;
w32 shmkey; int segid;
char action='?';
//cshmInit();
if(argc==1) {
  printf("Usage:\n");
  printf("ltu_shm 81X000 [L | N | F | R]\n");
  return(8);
} else if(argc>=2) { 
  if( strncmp(argv[1], "81", 2)==0) {
    shmkey=hex2int(argv[1]);
    if(argc>2) { 
      if((argv[2][0]=='L') || (argv[2][0]=='N') || (argv[2][0]=='F') || (argv[2][0]=='R')) { 
        action= argv[2][0];
      };
    };
    printf("%s -> %x action:%c.\n", argv[1], shmkey, action);
  } else {
    printf("ltu_shm 81X000     -base address of LTU has to be supplied\n");
    return(8);
  };
};
//  if(strcmp(argv[1], "lock")==0) 

ltushm= (Tltushm *)mallocShared(shmkey, 0, &segid);  //only attch
if(ltushm->id==0) {   //just allocated
  ltushm->id=shmkey;
  printf("ERROR: shared memory alloc problem in ds_register()\n");
} else {
  printf("got shared memory\n");
};
if(action=='L') {
  ltushm->ltucfg.flags= ltushm->ltucfg.flags | FLGlog1sec;
} else if(action=='N') {
  ltushm->ltucfg.flags= ltushm->ltucfg.flags & (~FLGlog1sec);
} else if(action=='F') {
  ltushm->ltucfg.flags= ltushm->ltucfg.flags | FLGfakebusy;
} else if(action=='R') {
  ltushm->ltucfg.flags= ltushm->ltucfg.flags & (~FLGfakebusy);
} else {
  ;
};
printltuDefaults();
shmdt(ltushm);
return(rc);
}
