/* detectfile.c  */
#include <stdio.h>
#include <stdlib.h>    //system()
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

/*-----------------------*/ int detectfile(char *name, int maxsecs) {
/* name: name of the file
   maxsecs: timeout in seconds (0: check only once and return)
   rc:   -1: error or file length (has to be >0)
Problems:
if we are NFS client looking for file written on NFS server,
the file is not seen sometimes: it seems, it happens in
half of the cases after ctpproxy restart.
- fopen/fclose -doesn't refresh
- touch -seems doesn't refresh neither
14.6.
file is (see pydimserver.py scp) put to /tmp directory too,
so we can get it from there (detectfile is called once more)
15.6. opendir/closedir commented out -perhaps not needed with scp
      (we do not use nfs more)
26.7.2014: the problem is in SMI: DIM cmds do not get execute
sometimes (seems more often just after restarting ctp_proxy),
when invoked from SMI service routine -i.e. 
from inside SMI_handle_command() in main_ctp.c.
Fix: one possobility (Franco suggested):
move ctpproxy call to endless while loop,
- set EXECUTING_FOR in SMI_handle
- set it back to 'RUNNING' in main endless loop after excution
  of the corresponding ctpproxy action
*/
int rc,secs=0;
struct stat buf;
//DIR *dp; //struct dirent *ep;
char *slp;
char dirpath[80];
strcpy(dirpath, name); slp= strrchr(dirpath, '/');
if(slp==NULL) {strcpy(dirpath,"./"); }
else {*(slp++)= '\0';};
while(1) {
  //FILE *opf;
  //dp = opendir(dirpath); (void) closedir (dp);
  rc= stat(name, &buf);
  if(rc==0) {
    printf("detectfile stat rc:%d from stat(%s) msecs:%d\n", rc, name, secs);
    rc= buf.st_size;
    if(rc==0) {
      printf("detectfile size:0, msecs:%d\n", secs);
    } else {
      break;
    };
  };
  if((secs%1000)==0) {
    printf("waiting %d secs rc:%d\n", secs, rc); fflush(stdout);
  };
  rc=-1;
  if(secs>=maxsecs*1000) break; //if(secs>=maxsecs) break;
  usleep(1000); secs=secs+1; //sleep(1); secs++;
  /* fopen does not refresh.
  opf= fopen(name,"r"); 
  if(opf!=NULL) {
    fclose(opf);
  } else {
    printf("%s -cannot be opened\n", name);
  }; */
};
return(rc);
}
/*-----------------------*/ int readfile(char *fname, char *mem, int maxlen) {
FILE *f;
int sp;
f=fopen(fname,"r"); if(f==NULL) return(-1);
//sp=fgets(mem, maxlen, f);  // bad idead (only till NL)
sp=fread((void *)mem, 1, maxlen-1, f); 
mem[sp]='\0';
fclose(f); return(sp);
}
