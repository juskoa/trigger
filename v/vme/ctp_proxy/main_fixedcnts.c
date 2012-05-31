#include <stdio.h>
#include <stdlib.h>   //free
#include <string.h>

int get_fixed(char *rcfg, int *fixpos);

/*----------------------------------------*/ int main(int argc, char **argv) {
FILE *ifile; int rl, ix, rc;
int fixpos[100];
char name[80]; char rcfg[10000];
if(argc==1) {
  printf("Usage:\n\
main_fixedcnts name\n\
");
  return(8);
};
strcpy(name, argv[1]);
ifile= fopen(name,"r");
if(ifile==NULL) {
  printf("cannot read %s\n",name); return(8);
}
rl=fread((void *)rcfg, 1, 10000, ifile); rcfg[rl]='\0';
//printf("%s:bytes:%d\n%s:\n", name, rl, rcfg);
printf("%s:bytes:%d\n", name, rl);
rc= get_fixed(rcfg, &fixpos[0]);
printf("fixpos length:%d\n", rc);
for(ix=0; ix<rc; ix++) {
  printf("%d:%d ",ix, fixpos[ix]);
}; printf("\n");
//von free(fixpos);
return(0);
}
