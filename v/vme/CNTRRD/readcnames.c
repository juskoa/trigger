#include "common.h"
int findAddresses(
int ix;
FILE *cnames;
char *cfdir; char cnamesname[100];
Tsorted pl;

char line[MAXLINELE];
for(ix=0; ix<N24; ix++) {
  busy[ix].reladdr=-1;      // not found in cnames.sorted2
  l0s[ix].reladdr=-1;      // not found in cnames.sorted2
};
cfdir= getenv("VMECFDIR");
strcpy(cnamesname, cfdir); strcat(cnamesname, "/dimcdistrib/cnames.sorted2");
cnames= fopen(cnamesname, "r");
while(1) {
  int rc,isdet; char *rcp;
  rcp=fgets(line, MAXLINELE, cnames);
  if(rcp==NULL) break;
  rc=parseLine(line, &pl);
  /*printf("%s %d %s %s %c rc:%d\n",pl.cname,pl.addr,pl.board,pl.ltuname,
    pl.type, rc);*/:q
  if(rc!=0) {
    printf("parseLine rc: %d line:%s\n", rc, line);
    break;
  };
  //if(pl.addr>3) break;
  isdet= isDetector(pl.ltuname);
  if((strncmp(pl.cname,"byin",4)==0) && 
     ( (pl.cname[4]>='1') &&(pl.cname[4]<='9') ) &&
     (isdet!=-1)) {
     busy[isdet].reladdr= pl.addr;
     continue;
  };
};
fclose(cnames);
}

