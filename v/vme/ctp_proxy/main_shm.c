/* ctp shared memory access main
SHM: ipcs -m, ipcrm shm SHMID
*/
#include <stdio.h>
#include "vmewrap.h"
#include "shmaccess.h"
#include "ctp.h"
#define DBMAIN
#include "Tpartition.h"

/*----------------------------------------*/ int main(int argc, char **argv) {
int rc=0;
cshmInit();
if(argc==1) {
  cshmPrint();
} else if(argc==2) { 
  if(strcmp(argv[1], "lock")==0) {
    lockBakery(&ctpshmbase->ccread, 3);
  } else if(strcmp(argv[1], "unlock")==0) {
    unlockBakery(&ctpshmbase->ccread, 3);
  };
  printBakery(&ctpshmbase->ccread);
};
/* this should be done only when nobody else is using it! 
rc= freeShared((w8 *)ctpshmbase, ctpsegid);
*/
return(rc);
}

