#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define READ 0
#define WRITE 1

/* working directory: $VMEWORKDIR */
char ltupgm[80]="";
char VMEWORKDIR[80]="";

pid_t popen2(const char *baseaddr, int *infp, int *outfp) {
    int p_stdin[2], p_stdout[2];
    pid_t pid;
    char *cfdir, *ltupgmdir;
    ltupgmdir= getenv("VMECFDIR");
    strcpy(ltupgm, ltupgmdir); strcat(ltupgm, "/ltu/ltu.exe");
    cfdir= getenv("VMEWORKDIR");
    if(cfdir==NULL) {
      printf("VMEWORKDIR env. variable not defined\n");
      return(-1);
    }else {
      strcpy(VMEWORKDIR, cfdir);
    };
    if (pipe(p_stdin) != 0 || pipe(p_stdout) != 0)
        return -1;
    pid = fork();
    if (pid < 0)
        return pid;
    else if (pid == 0) {
      int rc;
        rc= chdir(VMEWORKDIR);
        if(rc!=0) {perror("chdir() error"); exit(1); };
        close(p_stdin[WRITE]); close(READ);
        dup2(p_stdin[READ], READ);
        close(p_stdout[READ]); close(WRITE);
        dup2(p_stdout[WRITE], WRITE);
        //execl("/bin/sh", "sh", "-c", baseaddr, NULL);
        execl(ltupgm, ltupgm,"-noboardInit", baseaddr, NULL);
        //printf("popen2:ltupgm:%s\n",ltupgm);
        perror("execl");
        exit(1);
    }
    if (infp == NULL)
        close(p_stdin[WRITE]);
    else
        *infp = p_stdin[WRITE];
    if (outfp == NULL)
        close(p_stdout[READ]);
    else
        *outfp = p_stdout[READ];
    return pid;
}

//#include    "ourhdr.h"
int prtfdfs(int fide) {
int accmode,val;
val = fcntl(fide, F_GETFL, 0);
if(val < 0)
  printf("ERROR:fcntl error for fd %d", fide);
accmode = val & O_ACCMODE;
if      (accmode == O_RDONLY)   printf("fd %d:read only",fide);
else if (accmode == O_WRONLY)   printf("fd %d:write only",fide);
else if (accmode == O_RDWR)     printf("fd %d:read write",fide);
else printf("ERROR: unknown access mode");

if (val & O_APPEND)         printf(", append");
if (val & O_NONBLOCK)       printf(", nonblocking");
#if !defined(_POSIX_SOURCE) && defined(O_SYNC)
    if (val & O_SYNC)           printf(", synchronous writes");
#endif
putchar('\n');
return(val);
}

