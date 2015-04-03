#include <stdio.h>
#include <string.h>
#include "vmewrap.h"
//int readSSMDump_compress(int board,char *filename, int compress){

#define Mega 1024*1024
#define orbitbit 0x1
#define orbitlen 3564
char line[100];
w32 sm[Mega];

char mygetchar(){
char c; int i;
fgets(line, 99, stdin); c=line[0];
for(i=0; i<=99; i++) {
  if(line[i]=='\n') line[i]='\0';
};
line[99]='\0';
return c;
}

int bcdif(int bc1, int bc2) {
/* find bc2-bc1 distance */
if(bc2<bc1) {
  bc2= bc2+ Mega;
};
return(bc2-bc1);
}
int nextbc(int bc, int plus) {
bc= bc +plus; if(bc>=Mega) { bc=bc-Mega; };
return(bc);
}
int prevbc(int bc, int minus) {
bc= bc -minus; if(bc<0) { bc=bc+Mega; };
return(bc);
}
/*------------------------------------------*/ int find1storbit() {
/* find 1st orbit signal:
rc: -1: orbit not found
     n: pointing to start of the obit signal
*/
int i, rc=-1;
for(i=0;i<Mega;i++){
  if((sm[i] & orbitbit)==0) break;
};
if(i>50) return(rc); 
for(i=0;i<Mega;i++){
  if(( sm[i]& orbitbit) != 0) {rc=i; break;};
};
return(rc);
}
/*------------------------------------------*/ int find0(int bc, w32 bit) {
/* bc: has to point to '1' (start of the orbit) 
rc: length of the pulse or Mega if '0' bit not found
*/
int i;
i= bc;
while(1) {
  if((sm[i] & bit)!=0) {
    i= nextbc(i,1); 
    if( i== bc) return(Mega);
    continue;
  };
  return(bcdif(bc, i));
};
}
/*------------------------------------------*/ int find1(int bc, w32 bit) {
/* bc: has to point to '0' (start of the orbit) 
rc: length of the pulse or Mega if '1' bit not found
*/
int i;
i= bc;
while(1) {
  if((sm[i] & bit)==0) {
    i= nextbc(i,1); 
    if( i== bc) return(Mega);
    continue;
  };
  return(bcdif(bc, i));
};
}

/*-----------------------------------------*/ int main(int argc, char **argv) {
int i,rc=0, prevorbit, bc, bc1= -1;
int goodorbits=0;
w32 word;
FILE *dump;
char filename[120];
if(argc != 2){
  printf("Expected: one argument - SSM dump file name \n");
  return 1;
}
//inpnum = atoi(argv[1]);
strcpy(filename, argv[1]);
dump = fopen(filename,"rb");
if(dump==NULL) {
  printf("cannot open file %s\n",filename);
  return(1);
};
for(i=0;i<Mega;i++){
  int nwords;
  nwords=fread(&word,sizeof(w32),1,dump);
  //printf("%i 0x%x %i\n",i,word,nwords);
  if(nwords!=1) break;
  sm[i]= word;
}; fclose(dump);
if(i!=(Mega)) {
  printf(".dmp file ? words:%d\n",i);
  return(1);
};
// check orbit:
bc1= find1storbit();
if(bc1==-1) {
  printf("Orbit not found\n"); return(4);
} else {
  printf("1st orbit starts at:%d\n", bc1);
};
prevorbit= bc1; bc= bc1;
while(1) { //for(i=0;i<Mega;i++){
  int oe,os;
  oe= find0(bc, orbitbit); 
  bc= nextbc(bc, oe);
  os= find1(bc, orbitbit); 
  bc= nextbc(bc, os);
  //printf("%8d:1...   +%d %8d:0... +%d\n",prevorbit, oe, bc, os);
  if(bcdif(prevorbit,bc)== orbitlen) { 
    goodorbits++;
    /*printf("orbit: start(1st 1 after 0s): %d end(last 0 before 1):%d len:%d\n",
      prevorbit, prevbc(bc,1), bcdif(prevorbit,bc)); */
  } else {
    printf("Bad orbit: start(1st 1 after 0s): %d end(last 0 before 1):%d len:%d\n",
      prevorbit, prevbc(bc,1), bcdif(prevorbit,bc));
  };
  if(bc==bc1) break;
  //if(bc>20000) break;
  prevorbit=bc;
};
printf("First orbit:%d good orbits:%d\n", bc1, goodorbits);
while(1) {
  char c;
  printf("\n:"); c=mygetchar();
  if(c=='w') {
    int ix,ii;
    sscanf(&line[2],"%d", &ix);
    printf("sm[%d]: ", ix);
    for(ii=0; ii<10; ii++) {
      if(ii==4) printf("   ");
      printf(" 0x%x", sm[ix+ii]);
    }; printf("\n");
  } else if(c=='q') { break; };
};
return(rc);
}
