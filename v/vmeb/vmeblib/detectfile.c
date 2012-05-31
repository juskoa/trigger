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
   rc:   -1: error or file length
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
    printf("detectfile rc:%d from stat(%s) secs:%d\n", rc, name, secs);
    rc= buf.st_size;
    if(rc==0) {
      printf("detectfile size:0, secs:%d\n", secs);
    } else {
      break;
    };
  };
  rc=-1;
  if(secs>=maxsecs) break;
  //printf("waiting %d secs\n", secs);
  sleep(1); secs++;
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
