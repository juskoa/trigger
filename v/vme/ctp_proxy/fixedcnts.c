/* fixedcnts.c
15.9.2010 - values (rel. position >=3) are given in hz as floats
            notdefined counter: -1.0
            see: dimservices.c
*/
#include <stdio.h>   //malloc
#include <stdlib.h>   //malloc
#include <string.h>
#include "lexan.h"
#include "ctpcounters.h"

typedef struct TFixedCnts {
int fixedadd; // 0,1,... (adress in output array provided by DIM)
int type;  /* 
  0: last item (not meaningfull)
  1: fixed position in array970
  11,12,13:raw L0/1/2input  
  101,102,103:L0/1/2 class_before_vetos
  104,105,106:L0/1/2 class_after_vetos
*/ 
int position;    /* 
  for type:1: direct position in array970
  type:11:  first L0 input position  (l0inp1)
  type:12:  first L1 input position  (l1inp1)
  type:101: first L0before class position  (l0classB1) CSTART_L0+16
  type:102: first L1before class position  (l1classB1) CSTART_L1+40
  type:104: first L0after class position  (l0classA1) CSTART_L0+100
*/
char name[40];   // input/class name without cluster (0TSC,CMUS1-AC-NOPF-)
} TFixedCnts;

/* after the change of this table, recompile/relink/restart 
   ctp_proxy (enough, unless more than 50 counters reached).
Commented: was valid before (May VDM). Last change: 28.01.2013
DCS test:
check CTPDIM/MONLUMCNTS service
.../v/vme/dimcdistrib > linux/dimccounters CTPDIM
or start the same client on gfoxtrot@alidcscom707 through alias counters
 */
TFixedCnts FixedCnts[]= {
{0, 1, CSTART_SPEC, "epochsecs"},     // 921
{1, 1, CSTART_SPEC+1, "epochmics"},   
{2, 1, CSTART_L0+13, "l0time"},       //13
/* pb2011:
{3, 102, CSTART_L1+40, "C1ZED_R1-B-NOPF-"},
{4, 102, CSTART_L1+40, "C1ZED_R2-AC-NOPF-"},
{5, 102, CSTART_L1+40, "C1ZED_R2-E-NOPF-"},
{6, 101, CSTART_L0+16, "CPBI2-B-NOPF-"},
{7, 101, CSTART_L0+16, "CPBI2-AC-NOPF-"},
{8, 101, CSTART_L0+16, "CPBI2-E-NOPF-"},
{9, 101, CSTART_L0+16, "CVHN-B-NOPF-"},
{10, 101, CSTART_L0+16, "CVHN-AC-NOPF-"},
{11, 101, CSTART_L0+16, "CVHN-E-NOPF-"},
{12, 101, CSTART_L0+16, "CVLN-B-NOPF-"},
{13, 101, CSTART_L0+16, "CVLN-AC-NOPF-"},
{14, 101, CSTART_L0+16, "CVLN-E-NOPF-"},
{15, 101, CSTART_L0+16, "CPBI1MSL-B-NOPF-"},
{16, 101, CSTART_L0+16, "CPBI1MSL-AC-NOPF-"},
{17, 101, CSTART_L0+16, "CPBI1MSL-E-NOPF-"},
{18, 104, CSTART_L0+100, "C1ZAC_R1-B-NOPF-"},
{19, 102, CSTART_L1+40,  "C1ZAC_R1-B-NOPF-"},
{20, 104, CSTART_L0+100, "C1ZAC_R2-AC-NOPF-"},
{21, 102, CSTART_L1+40,  "C1ZAC_R2-AC-NOPF-"},
{22, 104, CSTART_L0+100, "C1ZAC_R2-E-NOPF-"},
{23, 102, CSTART_L1+40,  "C1ZAC_R2-E-NOPF-"},
{24, 101, CSTART_L0+16, "CPBI1-R-NOPF-"},
{25, 101, CSTART_L0+16, "CCUP4-B-NOPF-"},
{26, 101, CSTART_L0+16, "CCUP4-AC-NOPF-"},
{27, 101, CSTART_L0+16, "CCUP4-E-NOPF-"},
{28, 12, CSTART_L1+6, "1ZED"},
{29, 101, CSTART_L0+16, "C0TVX-B-NOPF-"},
{30, 101, CSTART_L0+16, "C0TVX-AC-NOPF-"},
{31, 101, CSTART_L0+16, "C0TVX-E-NOPF-"},
{32, 12, CSTART_L1+6, "1ZAC"},
{33, 12, CSTART_L1+6, "1ZMB"},
{34, 12, CSTART_L1+6, "1ZMD"}, */
/* pp12 April:
{3, 101, CSTART_L0+16, "CMUS7-B-NOPF-"},
{4, 101, CSTART_L0+16, "CMUS7-AC-NOPF-"},
{5, 101, CSTART_L0+16, "CMUS7-E-NOPF-"},
{6, 101, CSTART_L0+16, "CVBAND-ABCE-NOPF-"},
{7, 101, CSTART_L0+16, "CMUS7-ABCE-NOPF-"},
{8, 101, CSTART_L0+16, "CINT1-B-NOPF-"},
{9, 101, CSTART_L0+16, "CINT1-AC-NOPF-"},
{10, 101, CSTART_L0+16, "CINT1-E-NOPF-"},
{11, 101, CSTART_L0+16, "CVBAND-B-NOPF-"},
{12, 101, CSTART_L0+16, "CVBAND-AC-NOPF-"},
{13, 101, CSTART_L0+16, "CVBAND-E-NOPF-"},
{14, 101, CSTART_L0+16, "CVBANOTC-B-NOPF-"},
{15, 101, CSTART_L0+16, "CVBANOTC-AC-NOPF-"},
{16, 101, CSTART_L0+16, "CVBANOTC-E-NOPF-"},
{17, 101, CSTART_L0+16, "CVBCNOTA-B-NOPF-"},
{18, 101, CSTART_L0+16, "CVBCNOTA-AC-NOPF-"},
{19, 101, CSTART_L0+16, "CVBCNOTA-E-NOPF-"},
{20, 101, CSTART_L0+16, "CEMC7-B-NOPF-"},
{21, 101, CSTART_L0+16, "CEMC7-AC-NOPF-"},
{22, 101, CSTART_L0+16, "CEMC7-E-NOPF-"},
{23, 101, CSTART_L0+16, "CPHI7-B-NOPF-"},
{24, 101, CSTART_L0+16, "CPHI7-AC-NOPF-"},
{25, 101, CSTART_L0+16, "CPHI7-E-NOPF-"},
{26, 101, CSTART_L0+16, "C0TSC-B-NOPF-"},
{27, 101, CSTART_L0+16, "C0TSC-AC-NOPF-"},
{28, 101, CSTART_L0+16, "C0TSC-E-NOPF-"},
{29, 101, CSTART_L0+16, "C0TVX-B-NOPF-"},
{30, 101, CSTART_L0+16, "C0TVX-AC-NOPF-"},
{31, 101, CSTART_L0+16, "C0TVX-E-NOPF-"},
{32, 101, CSTART_L0+16, "CDG5-B-NOPF-"},
{33, 101, CSTART_L0+16, "CDG5-AC-NOPF-"},
{34, 101, CSTART_L0+16, "CDG5-E-NOPF-"}, */
/* pp12 April 16th
{3, 101, CSTART_L0+16, "CMSL7-B-NOPF-"},
{4, 101, CSTART_L0+16, "CMSL7-AC-NOPF-"},
{5, 101, CSTART_L0+16, "CMSL7-E-NOPF-"},
{6, 101, CSTART_L0+16, "CVBAND-ABCE-NOPF-"},
{7, 101, CSTART_L0+16, "CMSL7-ABCE-NOPF-"},
{8, 101, CSTART_L0+16, "CINT1-B-NOPF-"},
{9, 101, CSTART_L0+16, "CINT1-AC-NOPF-"},
{10, 101, CSTART_L0+16, "CINT1-E-NOPF-"},
{11, 101, CSTART_L0+16, "CVBAND-B-NOPF-"},
{12, 101, CSTART_L0+16, "CVBAND-AC-NOPF-"},
{13, 101, CSTART_L0+16, "CVBAND-E-NOPF-"},
{14, 101, CSTART_L0+16, "CVBANOTC-B-NOPF-"},
{15, 101, CSTART_L0+16, "CVBANOTC-AC-NOPF-"},
{16, 101, CSTART_L0+16, "CVBANOTC-E-NOPF-"},
{17, 101, CSTART_L0+16, "CVBCNOTA-B-NOPF-"},
{18, 101, CSTART_L0+16, "CVBCNOTA-AC-NOPF-"},
{19, 101, CSTART_L0+16, "CVBCNOTA-E-NOPF-"},
{20, 101, CSTART_L0+16, "CEMC7-B-NOPF-"},
{21, 101, CSTART_L0+16, "CEMC7-AC-NOPF-"},
{22, 101, CSTART_L0+16, "CEMC7-E-NOPF-"},
{23, 101, CSTART_L0+16, "CPHI7-B-NOPF-"},
{24, 101, CSTART_L0+16, "CPHI7-AC-NOPF-"},
{25, 101, CSTART_L0+16, "CPHI7-E-NOPF-"},
{26, 101, CSTART_L0+16, "CT0OR-B-NOPF-"},
{27, 101, CSTART_L0+16, "CT0OR-AC-NOPF-"},
{28, 101, CSTART_L0+16, "CT0OR-E-NOPF-"},
{29, 101, CSTART_L0+16, "C0TVX-B-NOPF-"},
{30, 101, CSTART_L0+16, "C0TVX-AC-NOPF-"},
{31, 101, CSTART_L0+16, "C0TVX-E-NOPF-"},
{32, 101, CSTART_L0+16, "CDG-B-NOPF-"},
{33, 101, CSTART_L0+16, "CDG-AC-NOPF-"},
{34, 101, CSTART_L0+16, "CDG-E-NOPF-"},
{0,0,0,""} */
/* pp12 July 17th 
{3, 101, CSTART_L0+16, "C0TVX-B-NOPF-"},
{4, 101, CSTART_L0+16, "C0TVX-A-NOPF-"},
{5, 101, CSTART_L0+16, "C0TVX-C-NOPF-"},
{6, 101, CSTART_L0+16, "C0TVX-E-NOPF-"},
{7, 101, CSTART_L0+16, "CT0OR-B-NOPF-"},
{8, 101, CSTART_L0+16, "CT0OR-A-NOPF-"},
{9, 101, CSTART_L0+16, "CT0OR-C-NOPF-"},
{10, 101, CSTART_L0+16, "CT0OR-E-NOPF-"},
{11, 101, CSTART_L0+16, "CVBAND-B-NOPF-"},
{12, 101, CSTART_L0+16, "CVBAND-A-NOPF-"},
{13, 101, CSTART_L0+16, "CVBAND-C-NOPF-"},
{14, 101, CSTART_L0+16, "CVBAND-E-NOPF-"},
{15, 101, CSTART_L0+16, "CVBAND-ABCE-NOPF-"},
{16, 101, CSTART_L0+16, "CVBANOTC-B-NOPF-"},
{17, 101, CSTART_L0+16, "CVBANOTC-A-NOPF-"},
{18, 101, CSTART_L0+16, "CVBANOTC-C-NOPF-"},
{19, 101, CSTART_L0+16, "CVBANOTC-E-NOPF-"},
{20, 101, CSTART_L0+16, "CVBCNOTA-B-NOPF-"},
{21, 101, CSTART_L0+16, "CVBCNOTA-A-NOPF-"},
{22, 101, CSTART_L0+16, "CVBCNOTA-C-NOPF-"},
{23, 101, CSTART_L0+16, "CVBCNOTA-E-NOPF-"},
{24, 101, CSTART_L0+16, "CINT1-B-NOPF-"},
{25, 101, CSTART_L0+16, "CINT1-A-NOPF-"},
{26, 101, CSTART_L0+16, "CINT1-C-NOPF-"},
{27, 101, CSTART_L0+16, "CINT1-E-NOPF-"},
{28, 101, CSTART_L0+16, "CSPI7-B-NOPF-"},
{29, 101, CSTART_L0+16, "CSPI7-A-NOPF-"},
{30, 101, CSTART_L0+16, "CSPI7-C-NOPF-"},
{31, 101, CSTART_L0+16, "CSPI7-E-NOPF-"},
{0,0,0,""} */
/* Jan28 2013 */
{3, 101, CSTART_L0+16, "C0TVX-B-NOPF-"},
{4, 101, CSTART_L0+16, "C0TVX-A-NOPF-"},
{5, 101, CSTART_L0+16, "C0TVX-C-NOPF-"},
{6, 101, CSTART_L0+16, "CT0OR-B-NOPF-"},
{7, 101, CSTART_L0+16, "CT0OR-A-NOPF-"},
{8, 101, CSTART_L0+16, "CT0OR-C-NOPF-"},
{9, 101, CSTART_L0+16, "CVBAND-B-NOPF-"},
{10, 101, CSTART_L0+16, "CVBAND-A-NOPF-"},
{11, 101, CSTART_L0+16, "CVBAND-C-NOPF-"},
{12, 101, CSTART_L0+16, "CVBANOTC-B-NOPF-"},
{13, 101, CSTART_L0+16, "CVBANOTC-A-NOPF-"},
{14, 101, CSTART_L0+16, "CVBANOTC-C-NOPF-"},
{15, 101, CSTART_L0+16, "CVBCNOTA-B-NOPF-"},
{16, 101, CSTART_L0+16, "CVBCNOTA-A-NOPF-"},
{17, 101, CSTART_L0+16, "CVBCNOTA-C-NOPF-"},
{18, 12, CSTART_L1+6, "1ZED"},
{19, 101, CSTART_L0+16, "C0TVX-E-NOPF-"},
{20, 101, CSTART_L0+16, "CT0OR-E-NOPF-"},
{21, 101, CSTART_L0+16, "CVBAND-E-NOPF-"},
{22, 101, CSTART_L0+16, "CVBANOTC-E-NOPF-"},
{23, 101, CSTART_L0+16, "CVBCNOTA-E-NOPF-"},
{24, 101, CSTART_L0+16, "C0VGO-ABCE-NOPF-"},
{0,0,0,""} 
/* abort gap measurements
{3, 101, CSTART_L0+16, "C0TVX-GA-NOPF-"},
{4, 101, CSTART_L0+16, "C0TVX-GC-NOPF-"},
{0,0,0,""} */
};

/*
In: rcfg:     .rcfg string
    startpos: position of the line from where to search for cname
    cname:    string to be found: input or part of the class name
              e.g.: '0HWU', CVBAND-AC-NOPF-

Out: rc: -1 not found
       > -1 position in array970
*/
int findClassInput(char *rcfg, int startpos, char *cname) {
int clen,ix=0; int rc=-1;
clen= strlen(cname);
//printf("\nfindClassInput:%s:%d\n",cname,clen);
while(1) {
  int nxl;
  if(strncmp(&rcfg[ix], cname, clen)==0) {
    // take first occurence
    enum Ttokentype tok; char tokstr[120];
    //printf("line:%d:%25.25s:\n", clen,&rcfg[ix]);
    ix=ix+clen;
    if((cname[0]=='C') && (cname[clen-1]=='-')) {   
      // classname part: 'CVBAND-B-NOPF-'
      tok= nxtoken(rcfg, tokstr, &ix);   // remaining part (cluster name)
      tok= nxtoken(rcfg, tokstr, &ix);   // class number
      if(tok==tINTNUM) {
        rc= startpos + str2int(tokstr) -1;
        break;
      } else {
        printf("ERROR: got:%s, but class number expected after class name %s\n",
          tokstr, cname); 
      };
    //} else if(cname[0]=='0')
    } else {
      tok= nxtoken(rcfg, tokstr, &ix);
      tok= nxtoken(rcfg, tokstr, &ix);
      tok= nxtoken(rcfg, tokstr, &ix);
      tok= nxtoken(rcfg, tokstr, &ix);
      if(tok==tINTNUM) {
        rc= startpos + str2int(tokstr) -1;
        break;
      } else {
        printf("ERROR: got:%s, but inp. number expected as last number for line starting with %s\n",
          tokstr, cname); 
      };
    };
  };
  nxl= getNextLine(&rcfg[ix]);
  if(nxl==0) { break; };
  ix= ix + nxl;
  if((strncmp(&rcfg[ix], "INTERACTIONS:", 13)==0) && (cname[0]!='C')) {
    //printf("finishing search for raw input:%s\n",cname);
    break;
  };
};
return(rc);
}
/*return pointer to array of ints. The array is allocated by malloc()
in this routine.
Input: rcfg: .rcfg config
       fixpos: pointer to int[] array to be returned
Output: rc: length of the array fixpos (including final -2)
         0: all 0 (i.e. no sense to provide service)
        >1: fixpos finished with -2
       fixpos: array of values pointing to cnames.sorted2
               -1: not allocated
*/
int get_fixed(char *rcfg, int *fixpos) {
int ix,rc=0; int *fixposloc;
int classesFrom=0, inputsFrom=0, sectionFrom=0, fixposln=0;
// find the length of fixedpos:
while(1) {
  fixposln++;
  if(FixedCnts[fixposln].type==0) break;
}; fixposln++;
//fixposloc= (int *)malloc(fixposln*sizeof(int)); *fixpos= fixposloc;
fixposloc= fixpos;
// find INPUTS: and CLASSES: sections:
while(1) {
  int le;
  le= getNextLine(&rcfg[sectionFrom]);
  if(le==0) { // fatal error (no CLASSES section found)
    break;
  }
  sectionFrom= sectionFrom + le;
  //printf("isectionFrom:%d fixposln:%d\n",sectionFrom, fixposln);
  if(strncmp(&rcfg[sectionFrom], "INPUTS:", 7)==0) {
    le= getNextLine(&rcfg[sectionFrom]);
    inputsFrom= sectionFrom + le;
    continue;
  };
  if(strncmp(&rcfg[sectionFrom], "CLASSES:", 8)==0) {
    le= getNextLine(&rcfg[sectionFrom]);
    classesFrom= sectionFrom + le;
    break;
  };
}
printf("inputsFrom:%d classesFrom:%d fixposln:%d\n",inputsFrom, classesFrom, fixposln);
for(ix=0; ix<fixposln; ix++) {
  char icname[80]; // max. class/input name length 
  switch(FixedCnts[ix].type){
  case 0:
    fixposloc[ix]= -2; break;
  case 1:
    fixposloc[ix]= FixedCnts[ix].position;
    break;
  case 11: case 12: case 13:
    // add ' ' to be sure we search for right name (ie: 0HWU 0HWUX)
    strcpy(icname, FixedCnts[ix].name); strcat(icname, " ");
    fixposloc[ix]= findClassInput(&rcfg[inputsFrom], 
      FixedCnts[ix].position, icname);
    break;
  case 101: case 102: case 103: case 104: case 105: case 106:
    // before vetos,i.e. do not consider cluster name -already given in table:
    fixposloc[ix]= findClassInput(&rcfg[classesFrom], 
      FixedCnts[ix].position, FixedCnts[ix].name);
    break;
  //case 0: rc= ix; goto RET;
  default:
    fixposloc[ix]= -1;
  };
  printf("%3d:%3d %s\n", ix, fixposloc[ix], FixedCnts[ix].name);
}; rc= fixposln;
/*RET:*/ return(rc);
}
