#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv) {
char *line; int ixl, countL, countH;
int reps, repslen;
if(argc!=2) {
  printf("ERROR: Usage: linux/bcmcheck BCMstring    (e.g. 2L58H) \n"); return(8);
};
line= argv[1]; ixl=0; countL=0; countH=0;
reps=1; repslen=0;
while(1) {
  char creps[16]; char cc;
  cc= line[ixl];
  if((cc>='0') && (cc<='9')) {
    creps[repslen]= cc; repslen++;
  } else if((cc=='H') or (cc=='L') or (cc=='h') or (cc=='l')) {
    if(repslen>0) {
      creps[repslen]='\0';
      reps=atoi(creps);
    };
    if((cc=='H') or (cc=='h')) {
      countH = countH + reps;
    } else {
      countL = countL + reps;
    };
    reps=1; repslen=0;
  } else if(cc=='\0') {
    if(repslen>0) {
      printf("missing H or L at the end of the BCM string\n");
    } else {
      printf("Counts: H:%d L:%d\n", countH, countL);
    };
    break;
  } else {
    printf("error:%c -unknown char\n");
    break;
  };
  ixl++;
};
}

