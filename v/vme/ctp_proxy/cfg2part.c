/* history
22.06.2016
instead of stack allocation (>2M) in readDatabase2Tpartition, line by line read used in ParseFile
8.7.2016 2 attempts to read file in ParseFile in case of 'short BCMASKS line'
5.8.2016 did not help, seems fgets is not reliable for lines longer  >4k
  https://www.gnu.org/software/libc/manual/html_node/Line-Input.html
Standard C has functions to do this, but they aren’t very safe: null characters and even (for gets) long lines can confuse them. So the GNU C Library provides the nonstandard getline function that makes it easy to read lines reliably.

  fgets changed to: malloc + getline

April/May 2017 
- reread the file (i.e. new fopen), with ignoring RBIF line (preceeding BCMASKS)
- malloc repeated in case of 'short BCMASKS' error -did not help
28.5.2017 run 270933 -appeared again
- own (mygetline) using fgetc used instead of getline()
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "vmewrap.h"
#include "vmeblib.h"
#include "daqlogbook.h"
#include "infolog.h"
#include "ctp.h"
#include "ctplib.h"
#include "Tpartition.h"
#include "lexan.h"

//int inplm=-1;   // -1: not connected 1..24: l0inp number of 0HCO input
//int inplm_swn;   // -1: not connected 1..24: l0inp number of 0HCO input
/*extern char //TRD_TECH[];   // partition name in case it is TRD technical partition */

//ctplib.h:
int l0AB();

/*------------------------------------------------------------findchar()
Input:
c    -any char from this string to be found
line -string where to look for it
rc   - pointer to int where return code (0:ok) is returned
Operation:
- return -1 in case char not found (i.e. EOL or '\0' found before)
- return index to found char
*/
int findchar(char *c, char *line, int *rc) {
int ixli=0;
*rc=1;
while(1) {
  w32 i;
  for(i=0; i<strlen(c); i++)
    if(line[ixli]==c[i]) {
      *rc=0; return(ixli);
    };
  if(line[ixli]=='\n') return(-1);  //was ixli
  if(line[ixli]=='\0') return(-1);  //was ixli
  //if(line[ixli]==' ') goto NXT;
  ixli++; 
};
}
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
 Called by: RBIF2Partition(),INTSEL2Partition(),BCMASK2Partition(),
            CLA2Partition() 
*/
int string2int(char *cstr,int length,w32 *num,char b){
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
 if((length>8) && (b =='h')){
  char cstr20[20];
  strncpy(cstr20, cstr, 20); cstr[19]='\0';
  printf("string2int error: length=%i >8 cstr[0..20]:%s:\n",length,cstr20);
  return 1;
 }
 if(length>9 && b == 'd'){
  char cstr20[20];
  strncpy(cstr20, cstr, 20); cstr[19]='\0';
  printf("string2int error: length=%i >9 cstr[0..20]:%s:\n",length, cstr20);
  return 1;
 }
 *num=0;
 i=0;
 power=1;
 while(i<length){
  if(char2i(*cstr,&digit) == 0) { *num=*num+digit*power; 
  } else {
    /* seems there is memory leak in ctp_proxy, see logs from 21.8.2009 pm*/
    printf("string2int error: cstr:%s length=%i base:%c\n",cstr,length,b);
    return 1;
  };
  power=power*base;
  cstr--;
  i++;
  if(*cstr==' ') break;   // ignore leading spaces
 }
 //if((b =='h'))printf("num= 0x%x \n",*num);else printf("num= %i \n",*num);
 return 0;
}
/*----------------------------------------------------PFL2Partition()
  Purpose: to convert PFL line in cfg to klas structure
  Parameters:
        Input: - string containig PFL line from cfg file 
               - pointer to partition where to save result
        Output: pf
  Return: error code
  Output: writes to part->Past
  Called by: ParseFile()
*/
int PFL2Partition(char *line,TPastFut *pf){
 return 0;
}
/*----------------------------------------------------BUSY2Partition()
  Purpose: to convert BUSY line in cfg to klas structure
  Input: - string containig BUSY line from cfg file 
         - pointer to partition where to save result
  Return: error code
  Output: writes to part->busy
  Called by: ParseFile()
*/
int BUSY2Partition(char *line,TBUSY *busy){
 // Same as FO
 // ---------------- probably has no sense:
 // - BUSY (as given in VALID.LTUS (or validLTUs table) is VALID
 // - BUSY from .pcfg file -there is no such line (should not be)
 return 0;
}
//
// INPS to partition
// INRND1 0x20 0x200
// RND1_EN_FOR_INPUTS
int INPCTP2Partition(char *line,Tpartition *part)
{
 char hex[10];
 int i=7; // for INRND1
 while(line[i] != ' ' && (i<MAXLINECFG)){
  hex[i-7]=line[i];
  i++;
 }
 hex[i-7]='\n';
 w32 word1= hex2int(&hex[2]); 
 w32 word2= hex2int(&line[i+3]);
 printf("INPSCTP2Part: RND1 0x%x 0x%x \n",word1,word2);
 part->inpsctp = new TINPSCTP;
 part->inpsctp->rnd1enabled1=word1;
 part->inpsctp->rnd1enabled2=word2;
 return 0;
}
/*----------------------------------------------------FO2Partition()
  Purpose: to get clusters definitions from FO lines in cfg file
  Input: 
    line: string containig FO line from cfg file 
         FO.X 0xA 
           X: 1..4
           A: 4x 8bits for clusters feeding connectors 4..1
           Example of detectors to cluster association: 
           FO.1: 0x109 detectors 0 belongs to clusters 1,4
                       detector  1 belongs to clusters 1
           FO.2: 0x030000: detector 6 belong to cluster 1,2 

    part: pointer to partition where to save result
  Return: error code: 0=ok
  Output: writes to part->Cluster[]
  Called by: ParseFile()
  Comment: here the table between detectors and FO connectors (FO <->Det)
           should be used later; For the moment it is assumed that
           - detectors names are numbers according to ECS table
           - detector i is connected to FO connector i
             i(DET) = FO number (from 0 to 5) + order of connector (0 to 3)
*/
int FO2Partition(char *line,Tpartition *part){
int i; //j, rc;
w32 word,ifo,maskdet,idet,clustercode;
enum Ttokentype tok; char hexw[20];
 i=0;
 while(line[i] != '.' && (i<MAXLINECFG))i++;
 i++;
 if(char2i(line[i],&ifo)) return 1;
i=i+2;   // 0x...
//if(string2int(&line[i-1],i-j-3,&word,'h')) {
tok= nxtoken(line, hexw, &i);
if(tok != tHEXNUM) {
  printf("FO2Partition error:line:%s:\n",line);
  return 1; 
};
word= hex2int(&hexw[2]);
 // Find clusters
 maskdet=0x3f;
 for(i=0;i<4;i++){      // loop over connectors
    clustercode=(word&maskdet)>>(8*i);
    //printf("FO2Partition: i=%i clustcode = 0x%x\n",i,clustercode);    
    if(clustercode){
      idet=(ifo-1)*NCON + i ;    //Detector code 0..23
      //printf("FO2Partition: idet=%i clustcode = 0x%x\n",idet,clustercode);
      part->Detector2Clust[idet] = clustercode;
     }
     maskdet=maskdet<<8;   
 }
if(DBGcfgin) {
  printf("FO2Partition: %s ifo=%i word=0x%x Detector2Clust:\n",
    part->name, ifo,word);
  for(i=0;i<NDETEC;i++) {
    if(i==10 || i==20) printf("\n");
    printf(" 0x%x",part->Detector2Clust[i]);
  }; printf("\n");
};
return 0;
}
/*--------------------------------------------------- checkRBIFown
*/
char *genNamesab[6]={"L0f1","L0f2","RND1", "RND2", "BC1", "BC2"};
char *genNamesac[8]={"L0f1","L0f2","L0f3", "L0f4", "RND1", "RND2", "BC1", "BC2"};
int checkRBIFown(TRBIF *rbif, TKlas *klas) {
int retrc=0,rabc, upto, ix;
char **genNames;
char emsg[ERRMSGL];
if(DBGcfgin) printf("checkRBIFown: klas:0x%x\n", klas->l0inputs);
if(l0AB()==0) {   //firmAC
  upto= L0CONBITLac; genNames= genNamesac;
} else {
  upto= L0CONBITL; genNames= genNamesab;
};
for(rabc=L0CONBIT0; rabc<=upto; rabc++) {  // rnd1,2, bc1,2
  if(((1<<rabc) & (~klas->l0inputs))) {   // resource used
    l0condition2rbif(rabc,&ix);
    if(rbif==NULL) {
      sprintf(emsg, "checkRBIFown: resource %s used but RBIF is NULL",
        genNames[rabc-L0CONBIT0]);
      infolog_trgboth(LOG_FATAL, emsg); retrc=1; break;
    };
    if(rbif->rbifuse[ix]==nothwal) continue;   // OK, defined too
    sprintf(emsg, "checkRBIFown: resource %s used but not defined",
        genNames[rabc-L0CONBIT0]);
    infolog_trgboth(LOG_FATAL, emsg); retrc=1;
  };
}; return(retrc);
}
/*------------------------------------------------- rmLMrnd1
remove RND1 if defined for class, where 0HCO used
I: inplm (global): 1..24 -number of 0HCO
   inplm_swn: corresponding switch input number (1..48)
rc: modified l0inputs

w32 rmLMrnd1(w32 l0inputs) {
#define RND1MASK 0x10000000
w32 inps= l0inputs; int ix;
if((inps & RND1MASK) == 0) {   // RND1 used in this class
  for(ix=0; ix<24; ix++) {
    w32 msk;
    msk= 1<<ix;     // ix: l0inp-1 (i.e. 0..23)
    if((inps & msk) == 0) {
      if( (ix+1) == inplm ) {
        w32 rndw1, rndw2;
        inps= inps & (~RND1MASK);  // do not use it at L0 level
        // programming RND1_EN_FOR_INPUTS
        rndw1= vmer32(RND1_EN_FOR_INPUTS);
        rndw2= vmer32(RND1_EN_FOR_INPUTS+4);
        printf("rmLMrnd1: l0inp:%d l0swin:%d RND1_EN_FOR_INPUTS:0x%x 0x%x \n", 
          inplm, inplm_swn, rndw1, rndw2);
      };
    };
  };
};
printf("rmLMrnd1: 0x%x -> 0x%x\n", l0inputs, inps);
return inps;
}
*/
/*----------------------------------------------------CLA2Partition()
  Purpose: to convert CLA line in cfg to klas structure
  Parameters:
        Input: - string containig CLA line from cfg file 
        Output: pointer to error: 
                                 1 & return NULL = error
                                 0 & return NULL = class is masked , not taken
                                 return != NULL  = class taken
  Return: NULL if not success
          pointer to klas if ok
  Output: 
  Called by: ParseFile()
*/
TKlas *CLA2Partition(char *line,int *error, char *pname){
 TKlas *klas;
 int i,j; w32 group=0; w32 mskCLAMASK;
 w32 l0inputs,l0inverted,l0vetos,scaler;
 w32 l1definition,l1inverted,l2definition;
int sdgix=-1;   // -1: not SDG class
char emsg[300];
char clsnum[3];
 *error=1;
 i=0;
if(l0AB()==0) {   //firmAC
  mskCLAMASK=getCLAMASK();
} else {
  mskCLAMASK=0x10000;
}
strncpy(clsnum,&line[4],2); clsnum[2]='\0';
while(line[i] != ' ' && (i<MAXLINECFG))i++;
j=i++;    // i.e. j=i; i=i+1
/* now:
       l0inputs       l0vetos    presc    l1def      l1i l2def      cg
CLA.01 0xffffffff 0x0 0x7feffdf1 0x1fffea 0x1fffffff 0x0 0x1f000fff 1
       i
      j 
alco valid for:
CLA.001 0x
       ji
*/
 while(line[i] != ' ' && (i<MAXLINECFG))i++;
/* 
      0123456789.1
CLA.01 0xffffffff 0x0 0x7feffdf1 0x1fffea 0x1fffffff 0x0 0x1f000fff 1
                 i
      j 
                1   -here 1st parameter is pointing. 2nd = 11-3=8
*/
 if(string2int(&line[i-1],i-j-3,&l0inputs,'h'))return NULL; 
 //if(inplm>0) l0inputs= rmLMrnd1(l0inputs);
 j=i++;
 while(line[i] != ' ' && (i<MAXLINECFG))i++;
 if(string2int(&line[i-1],i-j-3,&l0inverted,'h'))return NULL; 
 j=i++;
 while(line[i] != ' ' && (i<MAXLINECFG))i++;
 if(string2int(&line[i-1],i-j-3,&l0vetos,'h'))return NULL; 
 j=i++;
 while(line[i] != ' ' && (i<MAXLINECFG))i++;
 /* printf("cfg2part:%s:chars:%d, i=%d j=%d\n",&line[i-1],i-j-3,i,j);
CLA.01 0xbfffffff 0x0 0x7fffffb1 0x1fae13 0x1bffffff 0x0 0x1f000fff
    i-j-3=6                     j        i
 line[j+1]: start of prescaler: symname or 0x23
            length of symname or hexnumber is: i-j-1
 */
 if(strncmp(&line[j+1],"0x",2)!=0) {   // SDG symname
   char symname[12];
   strncpy(symname, &line[j+1], i-j-1); symname[i-j-1]='\0';
   sdgix= SDGfind(symname, pname);
   if(sdgix==-1) {   // unknown SDG name in class definition
     sprintf(emsg, "unknown SDG name in class definition: %s for part:%s",
       symname, pname);
     infolog_trgboth(LOG_ERROR, emsg); return NULL;
   };
   scaler= SDGS[sdgix].l0pr;
 } else {                            // 0xvalue
   if(string2int(&line[i-1],i-j-3,&scaler,'h')) return NULL; 
   //printf("cfg2part scaler:%x\n",scaler);
 };
 j=i++;
 while(line[i] != ' ' && (i<MAXLINECFG))i++;
 if(string2int(&line[i-1],i-j-3,&l1definition,'h'))return NULL; 
 j=i++;
 while(line[i] != ' ' && (i<MAXLINECFG))i++;
 if(string2int(&line[i-1],i-j-3,&l1inverted,'h'))return NULL; 
 j=i++;
 while((line[i] != '\n') && (line[i] != ' ') && (i<MAXLINECFG))i++;
/* 
CLA.01 0xffffffff 0x0 0x7feffdf1 0x1fffea 0x1fffffff 0x0 0x1f000fff 1
                                                                   i
                                                        j
                1   -here 1st parameter is pointing. 2nd = 11-3=8
*/
if(string2int(&line[i-1],i-j-3,&l2definition,'h'))return NULL;
// i: points to the space|\n after l2definition
j=i++;
/* now: (X == new line character)
group defined:
CLA.01 0xffffffff 0x0 0x7feffdf1 0x1fffea 0x1fffffff 0x0 0x1f000fff 1X
group not defined:
CLA.01 0xffffffff 0x0 0x7feffdf1 0x1fffea 0x1fffffff 0x0 0x1f000fffX
                                                                    i
                                                                   j
*/
// j points to space|\n and i points to:
// ?  -1 char after \n (group definition is missing) 
// 'x' -start of group definition (x is digit)
if( (line[j] != '\n') && (line[i] != '\n') ) {
  while((line[i] != '\n') && (line[i] != ' ') && (i<MAXLINECFG))i++;
  // i: points to the space|\n after group
  if(string2int(&line[i-1],i-j-1,&group,'d')) {
    sprintf(emsg,"classgroup def. error i:%d line[i]:%c:%c:\n", 
      i, line[i], line[i-1]);
    infolog_trgboth(LOG_ERROR, emsg); return NULL;
  } else {
    ; //printf("classgroup:CLAfrom1.%s cg=%d\n", clsnum,group);
  };
};
 // class enable
 if(~(l0vetos) & mskCLAMASK){ 
  klas = (TKlas *) malloc(sizeof(TKlas));
  if(klas == NULL){
   printf("CLA2Partition: not enough memory \n");
   return NULL;
  }
  cleanTKlas(klas); 
 }else{
  printf("disabled class:line:%s:\n", line);
  *error=0;
  return NULL;
 }
 klas->l0inputs=l0inputs;
 klas->l0inverted=l0inverted;
 klas->l0vetos=l0vetos;
 // Warning if using 'not defined' resources:
klas->sdg= sdgix; klas->scaler=0;
klas->lmscaler=scaler;
 klas->l1definition=l1definition;
 klas->l1inverted=l1inverted;
 klas->l2definition=l2definition;
 klas->classgroup= group;
 klas->pf=(klas->l0vetos&0xf0)>>4;  // PF assignement
 return klas;
}
/*----------------------------------------------------findBCMasks()
Operation: find masks used by pcfgdef array
Mask used, if at least 1 bit is ON!
I: pcfgdef: pointer to rbif BCMASK array defining all 12 (4) masks
O: set bcmuse[0..11] to 1..12 for used bc masks
*/
void findBCMasks(w16 *pcfgdef, w8 *bcmuse) {
int i,bcmaskn; w8 bcmu[BCMASKN];
if(l0AB()==0) {bcmaskn=BCMASKN;} else {bcmaskn=4;};
for(i=0;i<bcmaskn;i++){ bcmuse[i]=0; bcmu[i]=0; };
//if(pcfgdef[0]=='\0') { return; };
/* if(strlen(pcfgdef) != ORBITLENGTH) {
 infolog_trgboth(LOG_FATAL, "findBCMasks: long bcmask");
 return;
};*/
for(i=0;i<ORBITLENGTH;i++) {
  w16 msk; int im;
  msk= pcfgdef[i];
  if(msk>0xfff) {
    char emsg[100];
    sprintf(emsg,"findBCMasks: 0x%x >0xfff found in BCMASK in .pcfg (at %d)",msk,i);
    infolog_trgboth(LOG_FATAL, emsg);
    return;
  };
  for(im=0; im<bcmaskn; im++) {
    if( (msk & (1<<im))!=0 ) bcmu[im]= im+1;
  };
};

printf("findBCMasks: ");
for(i=0;i<bcmaskn;i++){ 
  bcmuse[i]= bcmu[i]; printf("%d:%d ",i,bcmuse[i]);
}; printf("\n");
}
/*----------------------------------------------------PF2Partition()
line:
PF.x name bcmask int Tbef Tafter Nbef Nafter Offbef Offafter
parted checking syntax and number of pfs per partition
rc: 0: ok
*/
int PF2Partition(char *line,TRBIF *rbif){ 
int ixdef,ixx;
//enum Ttokentype tok; 
w32 ixpf=0; char hexw[50];
ixx=0;
int rc1=0;
for(ixdef=0; ixdef< 12; ixdef++) {
  nxtoken(line, hexw, &ixx);
  printf("PFDEF ixx=%i ixdef=%i hexw: %s \n",ixx,ixdef,hexw);
  switch(ixdef){
    case 0: continue; // PF
    case 1: continue; //.
    case 2:
    {
     if((hexw[0]<'1') || (hexw[0]>'4')) goto BADLINE;
     char2i(hexw[0],&ixpf); 
     ixpf--;
     continue; 
    }
    case 3: // Name
    {
     strcpy(rbif->pf[ixpf].name,hexw);
     continue;
    }
    case 4: // BCM
    {
     w32 dig1;
     if(strcmp(hexw, "NONE") == 0){
       rbif->pf[ixpf].bcmask=0;
     } else {
       char2i(hexw[3],&dig1);
       if(hexw[4]=='\0'){
         rbif->pf[ixpf].bcmask=dig1;
       } else {
         w32 dig2;
         char2i(hexw[4],&dig2);
         rbif->pf[ixpf].bcmask=dig1*10+dig2;       
       }
     };
     continue;
    }
    case 5: // INT: INTL01,INTL02,INTLM1,INTLM2
    {
     w32 inter;
     if (strcmp(hexw,"INT1") == 0 ) inter=1;
     else if (strcmp(hexw,"INT2") == 0 ) inter=2;
     else if (strcmp(hexw,"INT12") == 0 ) inter=3;
     else if (strcmp(hexw,"NONE") == 0 ) inter=0;
     //else if (strcmp(hexw,"INTLM1") == 0 ) inter=1;
     //else if (strcmp(hexw,"INTLM2") == 0 ) inter=2;
     else{
      printf("Wrong INT, should be one of INTL01,INTL02,INTLM1,INTLM2\n");
      goto BADLINE;
     }
     rbif->pf[ixpf].inter=inter;
     continue;
    }
    case 6: // PeriodBefore
    {
     w32 dec;
     rc1=gethexdec(hexw,&dec);
     if(rc1 != 0) goto BADLINE;
     rbif->pf[ixpf].PeriodBefore=dec;
     continue;
    }
    case 7: // PeriodAfter
    {
     w32 dec;
     rc1=gethexdec(hexw,&dec);
     if(rc1 != 0) goto BADLINE;
     rbif->pf[ixpf].PeriodAfter=dec;
     continue;
    }
    case 8: // NintBefore
    {
     w32 dec;
     rc1=gethexdec(hexw,&dec);
     if(rc1 != 0) goto BADLINE;
     rbif->pf[ixpf].NintBefore=dec;
     continue;
    }
    case 9: // Nintafter
    {
     w32 dec;
     rc1=gethexdec(hexw,&dec);
     if(rc1 != 0) goto BADLINE;
     rbif->pf[ixpf].NintAfter=dec;
     continue;
    }
    case 10: // Offsetbefore
    {
     w32 dec;
     rc1=gethexdec(hexw,&dec);
     if(rc1 != 0) goto BADLINE;
     rbif->pf[ixpf].OffBefore=dec;
     continue;
    }
    case 11: //OffsetAfter
    {
     w32 dec;
     rc1=gethexdec(hexw,&dec);
     if(rc1 != 0) goto BADLINE;
     rbif->pf[ixpf].OffAfter=dec;
     continue;
    }
    default : goto BADLINE;
  }
};
rbif->PFuse[ixpf]= ixpf+1;
printf("PF2Partition: \n");
printTPastFut(&rbif->pf[ixpf]);
return 0;
BADLINE:
{ char emsg[300];
  sprintf(emsg,"PF2Partition: rc=%i bad line in .pcfg:%s",rc1,line);
  infolog_trgboth(LOG_FATAL, emsg);
  return 1;
};
}
/*----------------------------------------------------BCMASK2Partition()
Purpose: to convert BCMASK line in cfg to TRBIF structure
Input: - string containig BCMASK line from pcfg file 
       - pointer to TRBIF struct where to save result (TRBIF malloc if NULL)
Return: pointer to rbif, NULL = error
*/
TRBIF *BCMASK2Partition(char *line,TRBIF *rbif){
int ix,il; unsigned int lenline,bcmaskn;
if(rbif == NULL){
  rbif = allocTRBIF();
  if(rbif == NULL){
   infolog_trgboth(LOG_FATAL, "BCMASK2Partition: not enough memory");
   return NULL;
  };
};
if(l0AB()==0) {bcmaskn=BCMASKN;} else {bcmaskn=4;};
lenline= strlen(line);
if(lenline < ((bcmaskn/4)*ORBITLENGTH+8)) { // 8:"BCMASKS "
  char errm[300];
  sprintf(errm, "BCMASK2Partition: short BCMASKS line (len:%d bcmaskn:%d) in .pcfg", lenline,bcmaskn);
  prtError(errm);
  infolog_trgboth(LOG_FATAL, "BCMASK2Partition: short BCMASKS line in .pcfg");
  return NULL;
};
il=8;   // "BCMASKS 0f0..."
if(l0AB()==0) { // firmAC
  for(ix=0; ix<ORBITLENGTH; ix++) {
    w16 hx; char hx3[4];
    strncpy(hx3, &line[il], 3); hx3[3]='\0'; hx=hex2int(hx3);
    //printf("%s:%x ",hx3,hx);
    rbif->BCMASK[ix]= hx; il=il+3;
  };
} else {
  for(ix=0; ix<ORBITLENGTH; ix++) {
    w16 hx; char hx3[4];
    hx3[0]= line[il]; hx3[1]='\0'; hx=hex2int(hx3);
    rbif->BCMASK[ix]= hx; il=il+1;
  };
};
findBCMasks(rbif->BCMASK, rbif->BCMASKuse);
//printTRBIF(rbif);
return rbif;
}
/*----------------------------------------------------INTSEL2Partition()
never used -commneted out 11.1.2012
  Purpose: to convert INTSEL line in pcfg to TRBIF structure
  Parameters:
         Input: - string containig INTSEL line from cfg file 
         Output: - pointer to partition where to save result
  Return: NULL of not succes
          pointer to rbif if ok
  Called by: ParseFile()

TRBIF *INTSEL2Partition(char *line,TRBIF *rbif){
 int i,j;
 w32 intsel,rare;
 i=0;
 while(line[i] != 'x' && (i<MAXLINECFG))i++;
 j=i;
 while(line[i] != ' ' && (i<MAXLINECFG))i++;
 if(string2int(&line[i-1],i-j-1,&intsel,'h')) return NULL;
 j=i;
 while(line[i] != '\n' && (i<MAXLINECFG))i++;
 if(string2int(&line[i-1],i-j-3,&rare,'h'))return NULL;
 if(rbif == NULL){
  rbif = allocTRBIF();
  if(rbif == NULL){
   printf("INTSEL2Partition: not enough memory \n");
   return NULL;
  }
 }
 rbif->intsel=intsel;
 //rbif->rare=rare;
 return rbif;
} */
/*----------------------------------------------------RBIF2Partition()
  Purpose: to convert RBIF line in cfg to TRBIF structure
  Parameters:
       Input: - string containig RBIF line from cfg file 
       Output - pointer to partition where to save result
  Return: NULL if not succes
          pointer to rbif if success
  Called by: ParseFile()
*/
TRBIF *RBIF2Partition(char *line,TRBIF *grbif){
 int rc,ixline,ixrbf;
w32 rbif[ixrbifdim];
w32 rbifu[ixrbifdim];
char lut8[4*LUT8_LEN];
char l0intfs[7*L0INTFSMAX];  // l0f1/2 int1/2/t as a text string

if(grbif == NULL){ // create rbif if not existed 
  grbif= allocTRBIF();
  if(grbif == NULL){ return NULL; }
};
for(ixrbf=0; ixrbf<ixrbifdim; ixrbf++) {
  rbif[ixrbf]=0; rbifu[ixrbf]=notused;   //not used
};
ixrbf=0; ixline=0;   //ix: index to the first 'not present' (from rbif line)
//ixline=findchar(":", &line[0], &rc); doea not work for 1st item rnd1
ixline=findchar("F", &line[0], &rc);
/*RBIF 0x1:0x2:0x3:0x4:0x5:0x6:0x7
 ixrbf 0    1    2   3   4    5    6    7
       rnd1:rnd2:bc1:bc2:l0f1:l0f2:int1:int2:intint:
*/
if(DBGcfgin){
 printf("RBIF2PArtition. LINE:'%s'\n",line);
 printf("ixline=%i %i\n",ixline,rc);
}
//ixline--;
ixline++;
if(rc==-1) ixrbf=ixrbifdim;   // shared resources not used
while(ixrbf<ixrbifdim){
  int vallen, vallen2, rc;
  char base; char emsg[ERRMSGL];
  ixline++;
  if(DBGrbif) {
    printf("RBIFF:%s ixline:%d ixrbf:%d\n",line,ixline,ixrbf);
  };
  if((line[ixline]=='\0') || (line[ixline]=='\0') ) break;
  if(strncmp(&line[ixline],"0x",2)==0) {
    base='h';
    ixline=ixline+2;
  } else {
    base='d';
  };
  vallen2=findchar(":", &line[ixline], &rc);
  if(rc==-1) break;
  /* RBIF ::::0x8888 0OB3 & 0SH1::
                |               |
                ixline          ixline+vallen2
  */
  if(DBGrbif) {
    printf("RBIF2Partition: ixline vallen2:%d %d\n",ixline,vallen2);
  };
  if(vallen2==0) { goto NEXT; };   // : found
  if(vallen2==-1) { break; };       // : not found, no more definitions
  if( (ixrbf >=ixl0fun1) && (ixrbf<=ixintfunt)) {   // l0f1..4/int1/int2/intt
    /* :0x1234 0OVB & 0SMB | 0VGC:
              |                  vallen2
              *-- vallen
    i.e. 1st space after lookuptable is delimiter 
    After 1st space, the function definition follows till : 
    Note about lut8: 
      - l0f3/4 inserted after l0f2
      - int1/2/t not used anyhow (at least not in .partition), but let's keep them after l0f1..4  */

    vallen=findchar(" :", &line[ixline], &rc);
    if(vallen==0) {
      // ': :' or ': 0VGA&0SMB:'
      sprintf(emsg,"Lookup table (0x... digits) missing in RBIF");
      prtError(emsg); goto RETNULL;
    };
    if(vallen==64) {
      int lut8ix;
      // l0f lut (66 chars 0x...):
      rbif[ixrbf]= 0xffffffff;
      lut8ix= LUT8_LEN*(ixrbf-ixl0fun1);
      strncpy( &lut8[lut8ix], &line[ixline-2], 66);
      lut8[lut8ix+66]= '\0';
    } else {
      // l0f lut (16 bits):
      if(string2int(&line[ixline+vallen-1], vallen,&rbif[ixrbf], base)) {
        sprintf(emsg,"RBIF2Partition l0/int fun:%s (base:%c) is not int neither hexa", 
          &line[ixline+vallen-1], base);
        prtError(emsg); goto RETNULL;
      };
    };
    // l0f definition human readable:
    strncpy(&l0intfs[(ixrbf-ixl0fun1)*L0INTFSMAX], 
      &line[ixline+vallen], vallen2-vallen);
    l0intfs[(ixrbf-ixl0fun1)*L0INTFSMAX + vallen2-vallen]='\0';
    printf("cfg2Part:l0intfs:len:%d %s:\n", 
      vallen2-vallen,&l0intfs[(ixrbf-ixl0fun1)*L0INTFSMAX]);
  } else {
    if(string2int(&line[ixline+vallen2-1], vallen2,&rbif[ixrbf], base)) {
      sprintf(emsg,"RBIF2Partition:%s base%c: is not int neither hexa", 
        &line[ixline+vallen2-1], base);
      prtError(emsg); goto RETNULL;
    };
  };
  if(DBGcfgin)printf("%i 0x%x \n",ixrbf,rbif[ixrbf]);
  ixline= ixline+vallen2;
  rbifu[ixrbf]=nothwal;
  NEXT: ixrbf++;
  //printf("RBIF2Partition: %d was:0x%x len:%d ixline:%d\n", 
  //  ixrbf-1, rbif[ixrbf-1], vallen2, ixline);
};
for(ixrbf=0; ixrbf<ixrbifdim; ixrbf++) {
  grbif->rbif[ixrbf]=rbif[ixrbf]; grbif->rbifuse[ixrbf]=rbifu[ixrbf];   //not used
  if(DBGrbif) {
    printf("RBIF2Partitiin: [%i 0x%x %i]",ixrbf,rbif[ixrbf],rbifu[ixrbf]);
  };
  if( (ixrbf >=ixl0fun1) && (ixrbf<=ixintfunt)) {   // l0f1/l0f2/in1/int2/intt
    if(rbifu[ixrbf]!= notused) {
      strncpy(&(grbif->l0intfs)[(ixrbf-ixl0fun1)*L0INTFSMAX], 
        &l0intfs[(ixrbf-ixl0fun1)*L0INTFSMAX],L0INTFSMAX);
      (grbif->l0intfs)[(ixrbf-ixl0fun1)*L0INTFSMAX+L0INTFSMAX-1]='\0'; 
      strncpy(&(grbif->lut8)[(ixrbf-ixl0fun1)*LUT8_LEN], 
        &lut8[(ixrbf-ixl0fun1)*LUT8_LEN],LUT8_LEN);
      (grbif->l0intfs)[(ixrbf-ixl0fun1)*L0INTFSMAX+L0INTFSMAX-1]='\0'; 
      (grbif->lut8)[(ixrbf-ixl0fun1)*LUT8_LEN+LUT8_LEN-2]='\0'; 
      if(DBGrbif) {
        printf(" %s", &grbif->l0intfs[(ixrbf-ixl0fun1)*L0INTFSMAX]);
      };
    };
  }; 
  if(DBGrbif) printf("\n");
};
//printTRBIF(grbif);

return grbif;
RETNULL: return(NULL);
}
/*
I: line -pointer to memory of ngetline length bytes (i.e. max. (ngetline-1) chars)
rc: length of the line or -1 in case of EOF
*/
int  mygetline( char *line, size_t ngetline, FILE *cfgfile) {
unsigned int lng=0;
line[0]= '\0';
while (1) {
  char ch;
  /*if(feof(cfgfile)) {
    if (lng==0) {
      return(-1);
    } else {
      return(lng);
    };
  };*/
  if(lng >= (ngetline-1) ) {
    break;
  };
  ch = fgetc(cfgfile);
  if(feof(cfgfile)) {
    if (lng==0) {
      return(-1);
    } else {
      return(lng);
    };
  };
  line[lng]= ch;
  lng++;
  line[lng]='\0';
  //printf("%c", ch);
  if(ch == '\n') break;
};
//printf("mygetline len:%d line:%s:",lng,line);
return(lng);
} 
//int clg_defaults[MAXCLASSGROUPS]= {0,1,1,1,1,1,6,7,119,9};  see Tpartition.h
/*----------------------------------------------------ParseFile()
  Purpose: to transform the partition written in the format of cfg file
           to structure Partition
  Parameters:            
       Input: cfg file stored in array of characters
              empty Tpartition structure
       Output: part
  Return: error code (0=ok)
  Called by: readDatabase2Tpartition()
*/
//int ParseFile(char lines[][MAXLINECFG],Tpartition *part){
int ParseFile(char *fnpath,Tpartition *part){
FILE *cfgfile;
int i,ix,retcode=0;
int iklas=0,ipf=0,ifo=0;
TRBIF *grbif,*rcgrbif;
int allclgrps=0; int clg,nreads;
int clgrps[MAXCLASSGROUPS]={0,0,0,0,0,0,0,0,0,0};
char *newlinesip;   //allocate before 1st one freed
char *linesip=NULL;
size_t ngetline;
char errmsg[300]="";
part->nclassgroups= 0; nreads=0;   // >1: ignore RBIF (done already when nreads was 0)
ngetline= MAXLINECFG+1; 
REREAD1:
newlinesip= (char *)malloc(ngetline); 
if(linesip!=NULL) free(linesip);
linesip= newlinesip;
if(linesip==NULL) {
  sprintf(errmsg, "BCMASK2Partition: malloc(%d) failed", int(ngetline));
  retcode= 1; goto RETERR;
};
cfgfile=fopen(fnpath,"r");
if(cfgfile == NULL){
  sprintf(errmsg, "ParseFile: cannot open fnpath is:%s:\n", fnpath);
  retcode=1; goto RETERR;
};
/*
inplm= findInputName("0HCO");
if(inplm>=0) {
  inplm_swn= validCTPINPUTs[inplm].switchn;
  inplm= validCTPINPUTs[inplm].inputnum;
};
printf("inplm:%d\n", inplm); fflush(stdout);
*/
for(i=0;i<MAXNLINES;i++){
  int linlen;
  char linesi[MAXLINECFG];
  if((i>=MAXNLINES-1)) {
     sprintf(errmsg, "ParseFile: too long .pcfg file (> %d lines)", MAXNLINES);
     retcode= 1; goto RETERR;
  };
  /*if(fgets(linesi, MAXLINECFG,cfgfile) == NULL) break;   // end of cfg file
  linlen= strlen(linesi); instead use getline: */
  //linlen= getline( &linesip, &ngetline, cfgfile); if(linlen==-1) break;  replaced 28.5.2017
  linlen= mygetline( linesip, ngetline, cfgfile); if(linlen<=0) break;
  if( linlen >= (MAXLINECFG-1) ) {
    sprintf(errmsg, "ParseFile: too long line (%d chars) in .pcfg file ", linlen);
    retcode= 1; goto RETERR;
  };
  strcpy(linesi, linesip);
  //printf("ParseFile linesi:%s:",linesi);
  if(linesi[0]=='\0') {
    sprintf(errmsg, "ParseFile: empty line in .pcfg file");
    retcode= 1; goto RETERR;
  };
  if( linesi[0]=='#') continue;
  //printf("%s line",linesi);
  if(strncmp("RBIF",linesi,4) == 0){
   if(nreads>0) {
     printf("RBIF line %d ignored (nreads:%d)\n",i,nreads);
     continue;
   };
   grbif=RBIF2Partition(linesi,part->rbif);
   if(grbif == NULL) {
     sprintf(errmsg,"ParseFile: RBIF2Partition error. line:%s",linesi);
     retcode= 1; break;
   };
   part->rbif= grbif;  // MUST be here (parameter value passing  for part->rbif!)
   //printf("RBIF2Partition finished:\n"); printTRBIF(part->rbif);
  } else if(strncmp("BCMASKS",linesi,7) == 0) {
    printf("BCMASKS line len (fgetc):%d:%.20s...%s. &linesip:%p\n",linlen, linesi, &linesi[linlen-20],
      (void *)linesip);
    /* possible memory leak with BCMASKS line: 
       if happens even after update from 22.6.2016, look into RBIF -the only line
       present before BCMASKS line...
       It happened again: 08.07.2016 07:28:32
       Modification: close cfgfile, open it again, and skip all lines till BCMASK line.
    */
    if(linlen < ((BCMASKN/4)*ORBITLENGTH+8)) {
      if( nreads<=1 ) { 
        printf("BCMASKS line error str len/start after: %d/%.20s... read once more...\n", 
          int(strlen(&linesi[linlen])), &linesi[linlen]);
        nreads++;
        fclose(cfgfile); goto REREAD1;
      } else {
        sprintf(errmsg, "BCMASK2Partition: short BCMASKS line in .pcfg (%d reads)", nreads);
        retcode= 1; goto RETERR;
      };
    } else {
      rcgrbif=BCMASK2Partition(linesi,part->rbif);
      if(rcgrbif == NULL) {
        sprintf(errmsg,"ParseFile: BCMASK2Partition error"); retcode= 1;
        goto RETERR;
      };
      printf("BCMASK2Partition finished:\n"); printTRBIF(part->rbif);
    };
  } else if(strncmp("SDG ",linesi,4) == 0){
   if(SDGadd(linesi, part->name)) {
     SDGclean(part->name);
     sprintf(errmsg,"ParseFile: SDGadd error"); retcode= 1;
     goto RETERR;
   };
  } else  if(strncmp("PF.",linesi,3) == 0){
   if(PF2Partition(linesi,part->rbif)) {
     sprintf(errmsg,"ParseFile: PF2Partition error"); retcode= 1;
     goto RETERR;
   };
   ipf++;
  } else if(strncmp("CLA",linesi,3) == 0){
   int error;
   if(iklas >= NCLASS) {
    sprintf(errmsg,"ParseFile error: more than %d CLA lines in .pcfg",
      NCLASS); retcode=2;
    goto RETERR;
   };
   if(part->klas[iklas] != NULL) {
    sprintf(errmsg,"ParseFile error: NULL pointer expected for klas %d",
      iklas); retcode=2;
    goto RETERR;
   };
   part->klas[iklas]=CLA2Partition(linesi,&error, part->name);
   if((part->klas[iklas] == NULL) && error) {
     sprintf(errmsg, "ParseFile: CLA2Partition() rc:%d line:%s:", 
       error, linesi);
     retcode= 1; goto RETERR;
   };
   if(part->klas[iklas] == NULL) {
     printf("masked out class?\n");
     continue;
   };
   clg= part->klas[iklas]->classgroup;
   if(clg >9 ) {
      goto BADGROUP;
   } else {
     //part->nclassgroups++;
     clgrps[clg]++;
   };
   error= checkRBIFown(part->rbif, part->klas[iklas]);
   if(error) {
     sprintf(errmsg,"ParseFile: checkRBIFown() rc:%d line:%s", error, linesi);
     retcode= 3; goto RETERR;
   };
   if(part->klas[iklas] != NULL)iklas++;
  } else  if(strncmp("FO.",linesi,3) == 0){
   //if(ifo==0)printf("FO found at %i line\n",i);
   if(FO2Partition(linesi,part)) {
     sprintf(errmsg,"ParseFile: FO2Partition error"); retcode= 1;
   };
   ifo++;
  } else  if(strncmp("BUSY",linesi,4) == 0){
   //printf("BUSY found at %i line\n",i);
  } else if(strncmp("PFL.",linesi,4) == 0){
   printf("PFL found at %i line\n",i);
  } else if(strncmp("INRND1",linesi,6) == 0){
   printf("INRND1 found at %i line \n",i);
   if(INPCTP2Partition(linesi,part)){
     sprintf(errmsg,"ParseFile: INPCTP2Partition error"); 
     retcode= 5; goto RETERR;
   }
  } else {
    sprintf(errmsg,"Unknown line %d in .pcfg file:%s", i, linesi);
    retcode= 4; goto RETERR;
  };
};

//printTpartition("ParseFile", part);
// all CLA lines processed, check classgroups -find which ones are used:
allclgrps=0;
for(i=0; i<MAXCLASSGROUPS; i++) {
  if((i>0) && (clgrps[i] >0)) allclgrps++;
  if(clgrps[i]>0) printf("ParseFile: clgrps[%d]:%d\n", i, clgrps[i]);
};
if(allclgrps == 1) {
   char msg[200];
   sprintf(msg,"Only 1 time sharing group defined, ignored");
   infolog_trgboth(LOG_INFO, msg);
   part->nclassgroups= 0;
} else {
  part->nclassgroups= allclgrps;
  for(ix=0; ix<NCLASS; ix++) {
    if(part->klas[ix] == NULL) continue;
    clg= part->klas[ix]->classgroup;
    if((clg>0) && (clg<=9)) {
      part->classgroups[clg]= clg_defaults[clg];  
    } else {
      if(clg>9) {
        goto BADGROUP;
      };
    };
  };
};
if(DBGcfgin) {
  printf("ParseFile: # classes=%i \n",iklas);
  printf("           # FO     =%i \n",ifo);
  printf("           # pf     =%i \n",ipf);
  //printTpartition(part);
};
RETERR:
if(errmsg[0]!='\0') {
  //prtError(errmsg);
  infolog_trgboth(LOG_ERROR, errmsg);
};
if(fclose(cfgfile)) {
  sprintf(errmsg, "ParseFile: cannot close %s", fnpath);
  infolog_trgboth(LOG_ERROR, errmsg);
};
free(linesip);
return retcode;
BADGROUP:
sprintf(errmsg,"Bad class group:%d for partition:%s",clg,part->name);
retcode=5; goto RETERR;
}
/*----------------------------------------------------ReadPartitionErrors()
  Purpose: print error occured in routine readPartitionErrors()
  Input: - error code 
         - filename of read file
  Return: void
  Called by: readDatabase2Tpartition()
*/
void readPartitionErrors(int error,char *filename){
 char emsg[ERRMSGL];
 switch(error){
   case 1:
        sprintf(emsg,"readDatabase2Tpartition error: cannot open file: %s \n",filename);
        prtError(emsg);
        break;
   case 2:
        prtError("readDatabase2Tpartition error: not enough memory"); 
        break;
   case 3:
        sprintf(emsg,"readDatabase2Tpartition error: number of lines in file %s is 0.\n",filename);
        prtError(emsg);
        break;
   case 4:
        sprintf(emsg,"readDatabase2Tpartition error: number of lines in file %s larger than MAXNLINES=%i\n",filename,MAXNLINES);
        prtError(emsg);
        break;
   case 5:
        sprintf(emsg,"readDatabase2Tpartition warning: cannot close file %s \n",filename);
        prtLog(emsg);
        break;
   case 6:
        sprintf(emsg,"readDatabase2Tpartition error: error in parsing %s \n",filename);
        prtError(emsg);
        break;
   default: printf("readPartitionErrors: wrong error code.");
 } 
}
/*---------------------------------------------------readDatabase2Tpartition()
  Purpose: to read partiton cfg file from disk and transform it to 
           structure Tpartition
  Parameters:
          Input: filename
  Return: NULL if no success
          pointer to the new partition structure if success
  Called by: ctp_StartPartition()
*/
Tpartition *readDatabase2Tpartition(char *filename_par){
// char *environ;
char fnpath[MAXNAMELENGTH+40];
Tpartition *newpart=NULL;
// bad idea (2764800 bytes on stack) char lines[MAXNLINES][MAXLINECFG];
char filename[MAXNAMELENGTH];
int i;
//printf("readDatabase2Tpartition:filename_par:%s:\n",filename_par);
if( partmode[0] == '\0') {
  strcpy(filename, filename_par);
} else {
  strcpy(filename, partmode);
};
//printf("readDatabase2Tpartition:partmode:%s:\n",partmode);
/* Open file (valid till 6.8.2009):
environ= getenv("VMECFDIR"); strcpy(fnpath, environ);
strcat(fnpath,"/"); strcat(fnpath,PARTDBDIR); 
strcat(fnpath, filename); strcat(fnpath, ".pcfg");*/
strcpy(fnpath,"/tmp/");
strcat(fnpath, filename); strcat(fnpath, ".pcfg");
//printf("readDatabase2Tpartition:partmode:%s:reading file %s...\n",partmode,fnpath);
// Create new partition in memory
newpart = (Tpartition *) malloc(sizeof(Tpartition));
if(newpart == NULL){ 
  readPartitionErrors(2,filename);
  return NULL;
}
initTpartition(newpart,filename_par);
//printTpartition("After init",newpart);
//--------------------------------- Parse and copy
if(ParseFile(fnpath,newpart)){
  readPartitionErrors(6,filename);
  goto ERROR;
}
if(DBGcfgin) printTpartition("After read from db", newpart);
fflush(stdout);
 if(checkClustV0Tpartition(newpart)) goto ERROR;
 if(checkClustVl012Tpartition(newpart)) goto ERROR;
 if(checkTClustVDClustTpartition(newpart))goto ERROR; 
 partmode[0]='\0';
 for(i=0;i<NCLASS;i++){
  if(newpart->klas[i] != NULL){
   newpart->klas[i]->partname=newpart->name;
  }
 }
 // assign PF name to class - this is not ncessary
 //if(pf2class(newpart))goto ERROR;
 return newpart;

ERROR:
 partmode[0]='\0';
 deleteTpartition(newpart);
 return NULL; 
}
/*------------------------------------------------------ checkPCFG()
rc: msg:"" pcfg ok
    msg: Error message to be logged
*/
void checkPCFG(char *pname, char *msg, int maxmsg) {
#define MAXPCFGLEN 800
int ret;
char name2[120];
char pcfgmem[MAXPCFGLEN];
msg[0]='\0';
sprintf(name2,"/tmp/%s.pcfg", pname);
ret= readfile(name2, pcfgmem, MAXPCFGLEN); 
if(ret>10) {
  if(strncmp(pcfgmem, "Errors:",7)==0) {
    // .pcfg file contains error message after 1st line "Errors:"
    strncpy(msg, &pcfgmem[8], maxmsg); msg[maxmsg-1]='\0';
  };
} else {
  sprintf(msg,"%s not found", name2);
};
}
/*------------------------------------------------------ getInputDets()
Go through HW.klas[hwclass]->l0inputs/l1definition/l2definition  
( it should be the same as: partit->klas[iclass]->...  -to be checked!!!)
and find corresponding Input detectors from VALID.CTPINPUTS
Input: klpo: mask of L0/L1/L2 inputs
       rbifs: we need pointer to rbif structure due to l0f names check
RC: <0 in case of Error
   >=0 mask of input detectors. 
       From 21.11.2009: including detectors partiticpating
       in l0f (i.e. if l0f defined in klpo)
*/
int getInputDets(TKlas *klpo, TRBIF *rbifs, w32 *l0finputs) {
int ix, ixlevel, maxbitallowed;
int allinpdets=0, inpdets, ins012;
// L0
*l0finputs=0;
if(l0C0()>=0xc606) {
  maxbitallowed=28;
} else {
  maxbitallowed=26;
};
/*if(DBGgetInputDets) 
  printf("getInputDets: %d clgr:%d 0x%x 0x%x 0x%x 0x%x\n",
    klpo->hwclass, klpo->classgroup,
    klpo->lmcondition, klpo->l0inputs, klpo->l1definition, klpo->l2definition);
*/
for(ixlevel=0; ixlevel<3; ixlevel++) {
  inpdets=0;
  if( ixlevel==2) { ins012= klpo->l2definition; };
  if( ixlevel==1) { ins012= klpo->l1definition; };
  if( ixlevel==0) { ins012= klpo->l0inputs; };
  for(ix=0; ix<maxbitallowed; ix++) {   // 0..23 L0 inputs, 24,25 L0functions
    w32 inpdet; w32 bit;
    if( (ixlevel!=0) && (ix>23) ) break;
    if( ixlevel==2) { if(ix>11) break; };
    bit=(1<<ix);
    if( (ins012 & bit) == 0) {  //1:input not used 0:input is used
      if(( ix>23) && (ixlevel==0) ) {   // L0fun: done for LM also, see below
        /*sprintf(emsg,"getInputDets:l0Fun %d used but not implemented", ix+1);
        infolog_trg(LOG_INFO, emsg);*/
        int purelm;
        inpdets= getIDl0f(rbifs, ix-24, l0finputs, &purelm);
        if(inpdets==-1) { allinpdets=-1; goto RTRN; };
      } else {   // 0/1/2 input
        inpdet= findINPdaqdet(ixlevel, ix+1);
        //inpdets= addinpdet(inpdets, inpdet);
        if(inpdet==0xffffffff) {
          char emsg[200];
          sprintf(emsg,"getInputDets:l%dinput %d used but not connected", 
            ixlevel, ix+1);
          infolog_trgboth(LOG_FATAL, emsg); allinpdets=-1; goto RTRN;
        } else {
          inpdets= inpdets | (1<<inpdet);
        };
      };   // endof: 0/1/2 input
    };   // input is used
    if((ixlevel==0) && (ix==23)) {    // l0inp24 was just checked, let's do LMs now:
      // after L0inp24, check all LM inputs if available:
      w32 lminps;
      lminps= (~klpo->lmcondition) & 0xfff;   // i.e. 1:used
      if((klpo->lmvetos & 0x800000)==0) {    // LM clmask enabled
        int ixlm;
        for(ixlm=0; ixlm<12; ixlm++) {
          if((lminps & (1<<ixlm))) {   // ixlm: LM input
            int inplmdet;
            printf("getInputDets: looking for LM: %d...\n", ixlm+1); fflush(stdout);
            inplmdet= findLMINPdaqdet(ixlm+1);
            printf("getInputDets: LM: %d active det:%d\n", ixlm+1,inplmdet); fflush(stdout);
            if(inplmdet==-1) {
              char emsg[200];
              sprintf(emsg,"getInputDets:lMinput %d used but not connected", 
                ix+1);
              infolog_trgboth(LOG_FATAL, emsg); allinpdets=-1; goto RTRN;
            } else {
              inpdets= inpdets | (1<<inplmdet);
            };
          }; 
        };         // for all LM inputs
      };       // endof: LM enabled
    };      // endof: l0inp24 + all LMs checked
  };   // for all inputs + L0F1/2
  if(DBGgetInputDets) 
    if((ixlevel==0) && (*l0finputs!=0)) {   // only relevant cases
    printf("getInputDets: L%d input detectors:0x%x l0finputs:0x%x\n", 
      ixlevel, inpdets, *l0finputs);
    };
  allinpdets= allinpdets|inpdets;
};   // for all 0/1/2 levels
RTRN:
return(allinpdets);
}
/*------------------------------------------------------- getDAQClusterInfo()
*/
int getDAQClusterInfo(Tpartition *partit, TDAQInfo *daqi) {
unsigned long long ULL1=1,classmasks_l[NCLUST],classmasks_u[NCLUST];
int idet, iclu, iclass, rcdaqlog=0;
w32 l0finputs=0;// L0 inputs referenced by l0functions 
w32 l0finputs1; // L0 inputs referenced by l0functions for 1 class (filled in getInputDets)
//if(partit->run_number<10) { return(0); };
// only in run1msg: daqi->MaskedDetectors= partit->MaskedDetectors;
for(iclu=0;iclu<NCLUST;iclu++){
  daqi->masks[iclu]=0;
  daqi->inpmasks[iclu]=0;
  //daqi->classmasks01_32[iclu]=0; daqi->classmasks33_64[iclu]=0;
  classmasks_l[iclu]=0;
  classmasks_u[iclu]=0;
};
//--------------------- masks:
for(idet=0;idet<NDETEC;idet++){
  int pclu, pclust, hwclust;
  pclust= partit->Detector2Clust[idet]; // det. can belong to more clusters!
  if(pclust ==0) continue;
  // idet is in pclust, find HWclust:
  for(pclu=0; pclu<NCLUST; pclu++) {
    if(pclust & (1<<pclu)) {
      //printf("updateDAQClusters:findHWc:%s pclust:%x pclu:%x\n", partit->name, pclust, pclu);
      hwclust= findHWCluster(partit, pclu+1);
      if(hwclust>0) {
        daqi->masks[hwclust-1]= daqi->masks[hwclust-1] | (1<<idet);
      };
    };
  };
};
if(DBGlogbook) {
  int pclu;
  printf("getDAQClusterInfo:masks[0-%d]:0x:",NCLUST);
  for(pclu=0; pclu<NCLUST; pclu++) {
    printf("%x ", daqi->masks[pclu]); 
  }; printf(" hwallocated:%d\n", partit->hwallocated);
};
//--------------------- classmasks and inpmasks:
for(iclass=0; iclass<NCLASS; iclass++) {
  int hwclass; int indets; TKlas *klas;
  if((klas=partit->klas[iclass]) == NULL) continue;
  hwclass= partit->klas[iclass]->hwclass;  // 0..99
  //if(hwclass>49) 
  if(hwclass>99) {
    intError("getDAQClusterInfo: hwclass>99"); rcdaqlog=10;
  };
  iclu= (HW.klas[hwclass]->l0vetos & 0x7)-1;
  //daqi->classmasks[iclu]= daqi->classmasks[iclu] | (ULL1<<hwclass);
  // 100 classes: see DAQlogbook.h
  if(hwclass<64){
    classmasks_l[iclu]= classmasks_l[iclu] | (ULL1<<hwclass);
  }else{
    classmasks_u[iclu]= classmasks_u[iclu] | (ULL1<<(hwclass-64));
  }
  //
  //von indets= getInputDets(HW.klas[hwclass], partit, &l0finputs1);
  indets= getInputDets(HW.klas[hwclass], HW.rbif, &l0finputs1);
  /*
  if(strcmp(TRD_TECH, partit->name)==0) { // LM correction for techn. run:
    int newindets;
    // actually, 0HCO is used with RND generator, i.e. we
    // do not want to tell DAQECS TRD is readout detector in technical run:
    newindets= indets & 0xffffef;   // e=1110 -exclude TRD
    printf("getDAQClusterInfo:correction (no TRD) for technical %s 0x%x -> 0x%x\n",
      TRD_TECH, indets, newindets);
    indets= newindets;
  };
  */
  l0finputs= l0finputs|l0finputs1;
  // l0finputs will be usd later when ctp_alignment called
  if(indets<0) rcdaqlog=2;   
  if(DBGlogbook) printf("getDAQClusterInfo:hwallocated:%d iclu:%d iclass:%i indets:0x%x\n",
    partit->hwallocated, iclu, iclass, indets);
  daqi->inpmasks[iclu]= daqi->inpmasks[iclu] | indets;
};
//daqi->inpmasks[iclu]= 0x82020;
//printf("DBGlogbook: indets forced to: 0x%x\n",daqi->inpmasks[iclu]);
for(iclu=0;iclu<NCLUST;iclu++){
  //daqi->classmasks01_32[iclu]= classmasks_l[iclu];
  //daqi->classmasks33_64[iclu]= classmasks_l[iclu]>>32;
  daqi->classmasks00_063[iclu]=classmasks_l[iclu];
  daqi->classmasks64_100[iclu]=classmasks_u[iclu];
  // If INRND1 doen send any inputs to daq
  if(partit->inpsctp)daqi->inpmasks[iclu]=0;
};
return(rcdaqlog);
}

void setglobalflags(int argc, char **argv) {
setglobalflag(argc, argv, "NODAQLOGBOOK", FLGignoreDAQLOGBOOK);
setglobalflag(argc, argv, "NODAQRO", FLGignoreDAQRO);
}
