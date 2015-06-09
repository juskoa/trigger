/* 
aj@tp:~/CNTRRD$ $VMECFDIR/CNTRRD/linux/analyse '20.05.2011 23:56:01' '21.05.2011 00:02:01' 11 out.file 1 2 4
aj@tp:~/CNTRRD$ $VMECFDIR/CNTRRD/linux/analyse
*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h> 
#include <string.h> 
#include "common.h"
#include "ctpcounters.h"
#define MAXLINE 20000
#define MAXWORD 30
#define MAXCOUNTERS NCOUNTERS+2 
/*------------------------------------------------------------char2i()
 Purpose: Character to integer converter
 Parameters: input char a
             output: w32 *num
 Returns: error code: 0=ok
*/
int char2i(char a,w32 *num){
 *num=0;
 if(a >= 0x30 && a <= 0x39) *num = (a-0x30);
 else if (a >= 0x61 && a <= 0x66) *num = (a-0x57);
 else {
       printf("char2i: wrong pattern character:%c:\n",a);
       return 1;       
 }
 return 0; 
}
/*----------------------------------------------------------string2int()
 Purpose: convert string (hex eg:ab or int eg: 234) to int 
 Parameters:
   Input: - c:pointer to char: least significant digit of number
          - length: number of digits
          - base b = h : string represents hexa number
                     d : string represents decimal number
   Output: w32 *num 
 Return: 1 if error
         0 if ok
 Output: *num - converted number
*/
int string2int(char *cstr,w32 length,w32 *num,char b){
 int i;
 w32 digit,base,power;
//printf("string2int: cstr%s len:%d base:%c\n", cstr, length, b);
 if(length <=0){
  printf("string2int error: string length=%i <=0 \n",length);
  return 1;
 }
 if(cstr == NULL){
  printf("string2int error: cstr=NULL \n");
  return 1;
 }
 //printf("string2int start:%s strlength=%i \n",cstr,length);

 if(b == 'h') base=16 ;
 else if( b == 'd') base =10;
 else{
  printf("string2int error: unknown base %cstr \n",b);
  return 1;
 }
 if(length>(2*sizeof(w32)) && b =='h'){
  printf("string2int error: length=%i >2*sizeof(w32) \n",length);
  return 1;
 }
 if(length>9 && b == 'd'){
  printf("string2int error: length=%i >2*sizeof(w32) \n",length);
  return 1;
 }
*num=0; i=0; power=1; cstr= cstr + length-1;
while(i<(int)length){
  //printf("ss= %c \n",*cstr);
  if(char2i(*cstr,&digit) == 0) *num=*num+digit*power; else return 1;
  power=power*base;
  cstr--;
  i++;
  if(*cstr==' ') break;   // ignore leading spaces
};
//if((b =='h'))printf("num= 0x%x \n",*num);else printf("num= %i \n",*num);
return 0;
}
//-----------------------------------------------------------------------
int parseline(FILE *file,char counters[][MAXWORD]){
 int k,c,nc;
 k=0;
 nc=0;
 while(1) {
   //while(fgets(line,MAXLINE,con)!=NULL) {
   c=fgetc(file);
   if(c== EOF) return 1;
   if(nc>=(NCOUNTERS+2)) {   // date time 970counters
     printf("Error: nc:%d >= NCOUNTERS:%d\n", nc, NCOUNTERS);
     return 2;
   };
    //printf("%c \n",c);
   if(c ==' ' || c=='\n'){
     counters[nc][k]='\0'; k=0; nc++;
   }else{
     counters[nc][k]=c; k++;
   }
   if(c== '\n') break;
 }
 return 0;
}
//-----------------------------------------------------------------------
int converth2i(w32 *numm,int ic,char counters[][MAXWORD]){
char number[20];
w32 num;
int j,len;
len=(int)strlen(counters[ic]);
for(j=0;j<len;j++){
   //number[len-j-1]=counters[ic][j];
   number[j]=counters[ic][j];
}
number[len]='\0';
len=strlen(number);
if(len>0) {
  if(string2int(number,len,&num,'h')) {
    printf("DEBUG num=%s %x %u \n",counters[ic],num,num);
    return(1);
  };
}
*numm=num;
return 0; 
}
w32 h2w32(char *abcdef) {
int rcerr; w32 num;
rcerr= string2int(abcdef, strlen(abcdef), &num, 'h');
if(rcerr!=0) {
  printf("Error in string2int(%s,...)\n", abcdef); num=0xffffffff;
};
return(num);
}
#define NBO 10
#define NCLUST 7
#define NL0 152+2
#define NL1 300+2
#define NL2 446+2
//-----------------------------------------------------------------------
/*int getBUSYclust(w32 clusters[][NBO],char counters[][MAXWORD]){
 int i;
 for(i=0;i<NL0;i++){
  w32 num;
  //L0
  converth2i(&num,i+NL0,counters);
  clusters[i][1]=num;
  //L1
  converth2i(&num,i+NL1,counters);
  clusters[i][2]=num;
  //L2
  converth2i(&num,i+NL2,counters);
  clusters[i][3]=num;
 }
 return 0;
}*/
void printHelp() {
printf("\n\
analyse.exe 'date1 time1' 'date2 time2' opt runn outfile 0 23 45 ...\n\
\n\
opt:  hexa -give readings in hexa format, default. (e.g. 0fab)\n\
      dec  -give readings in decimal (unsigned long)\n\
      gnu  -give readings in decimal and '-' separator between date and time\n\
runn: 0: no run given (just take all readings date1..date2)\n\
     >0: run number -find SOR after 'date1 time1' and EOR from CSTART_RUNX[].\n\
                     Do not search after 'date2 time2'.\n\
outfile: the output file name. \n\
         NOT DONE YET:anypath.rrd  -create .rrd (.txt has to exist\n\
                      in curr. directory describing rrd)\n\
rel. numbers: in cnames.sorted2 file, i.e. 0 corresponds to l0byclstT,\n\
    1486..1491: spare1486runx... -6 runx numbers (see cnames1.sorted2)\n\
    1513: l2orbit\n\
    or\n\
    dimall -put all counters in binary file (like DIM service MONCOUNTERS)\n\
\n\
Example: extracting 2 counters, epochsecs and l0inp1, during run 216241:\n\
         into a text file r216241.dat\n\
\n\
cd .../rawcnts\n\
$VMECFDIR/CNTRRD/linux/analyse '12.03.2015 00:00:00' '12.03.2015 23:59:59' hexa 216241 $HOME/r216241.dat 1511 119\n\
\n\
Note: for run1, use cnames1.sorted2 + analyse should be modified (or recompiled with \n\
      ctpcounters_run1.h)\n\
");
};
void date_plus1(char *date) {
int y,m,d; char dc[20];
int dim[]={31,0,31,30,31,30,31,31,30,31,30,31};
strncpy(dc,date,8); dc[2]='\0'; dc[5]='\0'; dc[10]='\0';
y= atoi(&date[6]); m= atoi(&date[3]); d= atoi(&date[0]);
d=d+1;
if( (y%4)==0 ) { dim[1]=29;} else {dim[1]=28;};
if(d>dim[m]) {
  d=1; m=m+1; 
  if(m>12) {m=1; y= y+1; };
};
sprintf(dc,"%2.2d.%2.2d.%4.4d", d,m,y);
strncpy(date,dc,10);
}
FILE *open_rawcnts(char *date) {
FILE *file;
char filename[200];
sprintf(filename,"%10.10s.rawcnts",date);
printf("open_rawcnts: %s\n",filename);
file=fopen(filename,"r");
return(file);
}
//-----------------------------------------------------------------------
int main(int argc, char **argv){
int i;
FILE *file=NULL, *ofile=NULL;
//char line[MAXLINE];
char counters[MAXCOUNTERS][MAXWORD];
if(argc==1) {
  char date[32]="21.05.2011";
  char oline[1000];
  //w32 clusters[NCLUST][NBO];
  printHelp(); return 8;
  file= open_rawcnts(date);
  if(file==NULL) {printf("Error: %s.rawcnts not found\n",date); return 2;};
  while(1) {
    int rc;
    rc= parseline(file,counters); if(rc==1) break;
    if(rc==2) { printf("Error: short array\n"); break; };
    //printf("%s %s\n",counters[0],counters[1]);
    //oline[0]='\0';
    sprintf(oline, "%s %s ", counters[0], counters[1]);
    for(i= CSTART_RUNX+2;i< CSTART_RUNX+2+6;i++){   // 898..893
      //w32 number; 
      char cnumber[MAXWORD];
      //converth2i(&number,i,counters);
      //printf("n:%i \n",number);
      //sprintf(oline, "%s%i ",oline, number);
      strcpy(cnumber, counters[i]);
      sprintf(oline, "%s%s ",oline, cnumber);
      //printf("%i %s %i\n",i,counters[i],number);
    };
    oline[strlen(oline)-1]='\0'; printf("%s\n", oline); 
  };
  //getBUSYclust(clusters,counters);
} else {
#define MAXNOC 100
  int noc;   // number of counters
  int runxpos;   //  pisition in RUNX array
  w32 runi;   // atoi(runc)
  int decdata=0;   // 0: hexa   1: decimal
  int cpos[MAXNOC];
  char date[20], dati1[20],dati2[20];   // "21.05.2011 23:40:01"
  char runc[20]; char outfilepath[80];
  char oline[MAXNOC*15];
  int olines=0; int binarydata=0;
  char fmtsep[20]="%s %s ";
  if(argc < (6+1)) {printHelp(); return 8; };
  /* dati1 dati2 option run datafile.out n1 n2 ...
     1     2            3   4            5     old (before 31.5.2013)
     1     2     3      4   5            6
  */
  strcpy(dati1, argv[1]); strcpy(dati2, argv[2]);
  strcpy(runc, argv[4]); runi=atoi(runc); strcpy(outfilepath, argv[5]);
  printf("%s %s %d %s. Counters' rel. addresses:\n", dati1, dati2, runi, outfilepath);
  runxpos=-1;
  if((argc>= 7) && (strcmp("dimall",argv[6])==0)) {
    binarydata=1;
    printf("dimall ");
  } else {
    if(strcmp(argv[3],"dec")==0) {
      decdata=1;
      printf("decimal ");
    } else if(strcmp(argv[3],"gnu")==0) {
      decdata=1;
      strcpy(fmtsep, "%s-%s ");
      printf("gnu decimal ");
    } else {
      decdata=0;
      strcpy(fmtsep, "%s %s ");
      printf("hexa ");
    }
    for(noc=0; noc<=MAXNOC; noc++) {
      if(noc>=MAXNOC) {
        printf("Error: too many counters. max:%d\n",MAXNOC); return 3;
      };
      if( noc>= (argc-6)) break;
      cpos[noc]= atoi(argv[noc+6]);
      printf("%d ", cpos[noc]);
    }; 
  };
  printf("\n");
  strcpy(date,dati1);   //1st file
  file= open_rawcnts(date);
  if(file==NULL) {printf("Error: %s.rawcnts not found\n",dati1); return 2;};
  while(1){
    int rc; 
    rc= parseline(file,counters); 
    //printf("dbg1: %s %s\n", counters[0], counters[1]);
    if(rc==1) {
      if(strncmp(date,dati2,10)==0) break;
      date_plus1(date);
      file= open_rawcnts(date);
      if(file==NULL) {printf("Error: %s.rawcnts not found\n",date); return 2;};
      continue;
    };
    if(strncmp(&dati1[0], &date[0], 10)==0) {  // check lower time only in 1st file 
      if(strncmp(counters[1], &dati1[11], 8)<0) { continue; };
    };
    //printf("dbg2: %s %s\n", counters[0], counters[1]);
    if(strcmp(runc,"0")!=0) { // check run number if != 0
      if(runxpos == -1) {
        int irx;
        for(irx= CSTART_RUNX+2;irx< CSTART_RUNX+2+6;irx++){
          //printf("%d ",h2w32(counters[irx]));
          if(h2w32(counters[irx])==runi) {
            // run just started:
            runxpos= irx;
            break;
          };
        }; //printf("\n");
        if(runxpos==-1) continue;  // runc not found yet
      } else {
        if(h2w32(counters[runxpos])!=runi) {
          // run finished, stop reading:
          break;
        };
      }
    };
    if(strncmp(&dati2[0], &date[0], 10)==0) {  // check upper time only in last file 
      if(strncmp(counters[1], &dati2[11], 8)>0) { break; };
    };
    if(ofile==NULL) {
      ofile= fopen(outfilepath,"w");
      if(ofile==NULL) {
        printf("cant open %s\n", outfilepath);
        return(3);
      };
      if(binarydata!=1) {
        int ix;
        strcpy(oline, "# date     time    ");
        for(ix=0; ix<noc; ix++) {
          sprintf(oline,"%s %s", oline, argv[ix+6]);
        };
        fprintf(ofile, "%s\n", oline); 
      };
    };
    if(binarydata==1) {
      //printf("Bindata: ");
      for(i=2; i<NCOUNTERS+2; i++) {
        w32 cnt;
        converth2i(&cnt, i, counters);
        //if(i<7) printf("%d:%x ", i-2, cnt);
        fwrite(&cnt, 4, 1, ofile);
      };  //printf("\n");
    } else {
      //oline[0]='\0'; sprintf(oline, "%s %s ", counters[0], counters[1]);
      oline[0]='\0'; sprintf(oline, fmtsep, counters[0], counters[1]);
      for(i=0; i<noc; i++) {
        //w32 number; 
        int ix; char cnumber[MAXWORD];
        ix= cpos[i]+2 ;
        //converth2i(&number,i,counters);
        //printf("n:%i \n",number);
        //sprintf(oline, "%s%i ",oline, number);
        if(decdata!=0) {
          w32 cnumberdec;
          converth2i(&cnumberdec,ix,counters);
          sprintf(oline, "%s%u " ,oline, cnumberdec);
        } else {
          strcpy(cnumber, counters[ix]);
          sprintf(oline, "%s%s ",oline, cnumber);
        };
      }; 
      oline[strlen(oline)-1]='\0'; 
    };
    if(binarydata!=1) {
      fprintf(ofile, "%s\n", oline); 
    };
    olines++;
  };
  printf("%s: %d lines(or blocks in case of dimall) written\n", 
    outfilepath, olines);
};
if(file!=NULL) fclose(file);
if(ofile!=NULL) fclose(ofile);
}
