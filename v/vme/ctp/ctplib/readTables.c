#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "vmewrapdefs.h"
#include "infolog.h"
#include "ctp.h"
#include "ctplib.h"
/* only 1 of TRGDBmysql TRGDBfiles can be defined */
//#define TRGDBmysql
#define TRGDBfiles
#include "Tpartition.h"
#include "lexan.h"
#include "vmeblib.h"

#ifdef TRGDBmysql
#include "mysql.h"
int msOpen();
int msClose();
MYSQL_RES *msSelect(char *fields, char *table, char *whereexp, int *nfields);
MYSQL_ROW msFetch(MYSQL_RES *result, unsigned long **lengths);
int msSelect1(char *fields, char *table, char *whereexp, char *outstr);
#endif

#define MaxIntItems 9
int findSwitchInput(int swinput);

/*-------------------------------------------------------copyDetector2Clust()
*/
void copyDetector2Clust(w32 *dest, w32 *src) {
int i;
for(i=0;i<NDETEC;i++){
  dest[i]= src[i];
};
}
/*-------------------------------------------------------printDetector2Clust()
*/
void printDetector2Clust(w32 *src) {
int i; Tdetector *det; char line[500]="";
for(i=0;i<NDETEC;i++){
  if(src[i]!=0) {
    det= findLTUdaqdet(i);
    if(det==NULL) {
      char emsg[200];
      sprintf(emsg,"printDetector2Clust: det:%d src:0x%x", i, src[i]);
      intError(emsg); return;
    };
    sprintf(line,"%s%s:0x%x ", line, det->name, src[i]);
  };
};
printf("%s\n",line);
}
/*------------------------------------------------------------isTrigDet(char *name)
see ACT: /CTP/trgInput_DETECTOR
*/
int isTrigDet(char *name) {
int rc;
if((strcmp(name,"SDD")==0) ||
   (strcmp(name,"SSD")==0) ||
   (strcmp(name,"HMPID")==0) ||
   (strcmp(name,"MUON_TRK")==0) ||
   (strcmp(name,"FMD")==0) ||
   (strcmp(name,"PMD")==0) ||
   (strcmp(name,"CPV")==0) ||
   (strcmp(name,"DAQ")==0)) {
  rc=0;
} else {
  rc=1;   // triggering detector
}; return(rc);
}
/*------------------------------------------------------------update_dimnum()
checkchangeonly: 0: update validCTPINPUTs   1: just check if filter changed
rc: 0: filter file did not change
    1: filter line changed, validCTPINPUTsi[].dimnum updated
    2: filter line changed, validCTPINPUTsi[].dimnum updated, ERRORS in filter line!
    3: filter not available
*/
int update_dimnum(int checkchangeonly) {
FILE *filfile; int rc=0;  // 0: filter did not change  1: changed
int filterdone=0; w32 flgbit;
static char lastfilterline[200]="";  // last filter line (checked at SOR)
char line[MAXLINELENGTH], value[MAXCTPINPUTLENGTH];
flgbit= FLG_FILTEREDOUT<<24;
filfile=openFile("filter","r");
if(filfile == NULL) { return(3); };
while(fgets(line, MAXLINELENGTH, filfile)){
  int ix,tkns,ixtab;
  char em1[ERRMSGL];
  //printf("Decoding line:%s ",line);
  if(line[0]=='#') continue;
  if(filterdone==1) break;
  if(line[0]=='\n') continue;
  ix=0; tkns=0; 
  if(strcmp(line, lastfilterline)==0) {
    sprintf(em1, "no change in filtered inputs: %s", line);
    infolog_trgboth(LOG_INFO, em1);
    break;
  } else {
    sprintf(em1, "New filter: %s", line);
    infolog_trgboth(LOG_INFO, em1);
  }; 
  filterdone=1; rc= 1; 
  if(checkchangeonly==1) break;
  sprintf(em1, "CTP inputs filter:%s", line); infolog_trgboth(LOG_INFO, em1);
  strncpy(lastfilterline, line, 200); lastfilterline[199]='\0';
  // before going over all dets, reset all FLG_FILTEREDOUT flags:
  for(ixtab=0; ixtab<NCTPINPUTS; ixtab++) {
    if( validCTPINPUTs[ixtab].name[0] != '\0') {      // valid entry
      validCTPINPUTs[ixtab].dimnum= validCTPINPUTs[ixtab].dimnum & (~flgbit);
    };
  };
  while(1) {   // ZDC ACORDE MUON_TRG ...
    //Tdetector *ltuitem;
    enum Ttokentype token;
    int ltuecsn;
    char em1[ERRMSGL];
    token= nxtoken1(line, value, &ix);
    if(token==tEOCMD) break;
    if(token!=tSYMNAME) {
      sprintf(em1,"%s is not a valid detector name in filter file", value); prtError(em1);
      rc= 2;
      continue;
    };
    //ltuitem= *findLTU(value);
    ltuecsn= findLTUdetnum(value);
    //if(ltuitem==NULL) {
    if(ltuecsn==-1) {
      sprintf(em1,"%s is not a detector name in filter file", value); prtError(em1);
      rc= 2;
      continue;
    };
    for(ixtab=0; ixtab<NCTPINPUTS; ixtab++) {
      if( validCTPINPUTs[ixtab].name[0] != '\0') {      // valid entry
        if( validCTPINPUTs[ixtab].detector==ltuecsn) {  // detector filtered out
          validCTPINPUTs[ixtab].dimnum= validCTPINPUTs[ixtab].dimnum | flgbit;
          if(((validCTPINPUTs[ixtab].switchn==0) && (validCTPINPUTs[ixtab].level==0)) ||
             (validCTPINPUTs[ixtab].inputnum==0)
            ) {
            sprintf(em1,"Filtering out a not connected input %s", validCTPINPUTs[ixtab].name);
            prtLog(em1);
          };
        };
      };
    };
  };
};
fclose(filfile);
return(rc);
};
/*------------------------------------------------------------readTables()
read files: VALID.LTUS into validLTUs[]
            ctpinputs.cfg into validCTPINPUTs[]
*/
void readTables() {
enum Ttokentype token;
int ix;
char emsg[ERRMSGL];
char em1[ERRMSGL];
char detname[MAXDETNAME];    // Name
int ixtab;
int a3[MaxIntItems]; // id, FO, FOcon, BUSYinput, i2cchan, i2cbran or
                     // inpName Det Level Signature InpNumber DimNum Config Edge Delay deltamin deltamax
char ltubase[12];   // LTUbase
#ifdef TRGDBfiles
FILE *cfgfile;
char line[MAXLINELENGTH], value[MAXCTPINPUTLENGTH];
#endif
#ifdef TRGDBmysql
MYSQL mysql;
MYSQL_RES *resultLTUS;
MYSQL_ROW rowLTUS;
int fields;   // # of columns in TRG_LTUS
unsigned long *lengths;
#endif
#ifdef TRGDBfiles
/*Format:  detname=DAQdet fo focon bsyinp ltubase i2cchan i2cbran
See description in VALID.LTUS file
Example:
TRD=4
TOF=5 1 3
MUON_TRK=9 1 2 0 0x812000 1 1
MUON_TRG=9 1 2 0 muon_trg 1 1
*/
cfgfile=openFile("VALID.LTUS","r");
if(cfgfile == NULL) return;
//printf("INFO cfgfile ok\n");
#endif
#ifdef TRGDBmysql
msOpen();
resultLTUS=msSelect("*", "TRG_LTUS","", &fields);
if(fields!=5) {   // error: "required # of fields not received"
  printf("ERROR 5 fields expected in VALID.LTUS, got:%d\n", fields);
};
#endif
for(ix=0; ix<NDETEC; ix++) {
  validLTUs[ix].name[0]='\0';
  validLTUs[ix].fo=0;
  validLTUs[ix].foc=0;
  validLTUs[ix].busyinp=0;
  validLTUs[ix].i2cchan=-1;
};
//printf("INFO going to read ltus...\n");
#ifdef TRGDBfiles
while(fgets(line, MAXLINELENGTH, cfgfile)){
  //printf("INFO Decoding line:%s\n",line); fflush(stdout);
  if(line[0]=='#') continue;
  if(line[0]=='\n') continue;
  ix=0; token= nxtoken(line, value, &ix);
  if(token==tSYMNAME) {
    int ixx;
    value[MAXDETNAME-1]='\0';
    strcpy(detname, value); 
    token=nxtoken(line, value, &ix);
    if(token!=tASSIGN) {strcpy(em1,"= expected"); goto ERR; };
    a3[0]=-1;   // DAQdet
    for(ixx=1; ixx<4; ixx++) a3[ixx]=0;   // fo, focon, bsyinp
    a3[4]=-1; a3[5]=-1;                   // i2cchan i2cbranch
    ltubase[0]='\0';
    for(ixx=0; ixx<4; ixx++) {
      token=nxtoken(line, value, &ix);
      if(token==tINTNUM) {         //DAQdet: det. number (DAQ/ECS)
        a3[ixx]= str2int(value);
        /*token=nxtoken(line, value, &ix);
        if(token==tDOT) {
          continue;
        } else*/ 
        if(token==tEOCMD) goto FIN;
      } else if(token==tEOCMD) goto FIN;
      else {
        sprintf(em1,"Decimal number expected, got:%s", value); goto ERR; };
    };
    token=nxtoken(line, value, &ix);   //ltubase
    if(token == tHEXNUM) {
      if(strlen(value)!=8) {
        sprintf(em1,"Bad LTU base address:%s",value); goto ERR;
      } else {
        strcpy(ltubase,value);
        /* token=nxtoken(line, value, &ix);
        if(token != tEOCMD) {
          strcpy(em1,"end of line expected"); goto ERR; 
        }; */
      };
    } else if(token == tSYMNAME) {  //LTUDIM name
        strcpy(ltubase,value);
    } else if(token == tINTNUM) {
      if( str2int(value)!=0) {
        sprintf(em1, "0, 0xHEXA or DIM name expected but got:%s",value);
        goto ERR;
      };
    } else if(token != tEOCMD) {
      strcpy(em1,"0, 0xHEXA, DIM name or EOL expected"); goto ERR; 
    };
    token=nxtoken(line, value, &ix);   // i2chan
    //printf("readTables: value:%s:\n", value);
    if( (token == tSYMNAME) && (strcmp(value,"N")==0)) {
      goto FIN;
    } else if(token == tINTNUM) {
      int i2cc;
      i2cc= str2int(value);
      if( (i2cc>=0) && (i2cc<=7) ) {
        a3[4]= i2cc;
      } else {
        sprintf(em1, "Instead %s 0-7 or N expected for i2cchan",value);goto ERR;
      }; 
      token=nxtoken(line, value, &ix);
      if(token == tINTNUM) {        // i2cbranch
        int i2cb;
        i2cb= str2int(value);
        if( (i2cc>=0) && (i2cc<=7) ) {
          a3[5]= i2cb;
        } else {
          sprintf(em1, "Instead %s 0-7 expected for i2cbran",value);goto ERR;
        };
      } else {
        sprintf(em1, "Instead %s 0-7 expected for i2cbran",value);goto ERR;
      };
    } else if(token != tEOCMD) {
      strcpy(em1,"N or i2cchannel expected"); goto ERR; };
  } else {strcpy(em1,"symbolic name expected"); goto ERR; };
  FIN:
  if((ixtab=a3[0])==-1) {strcpy(em1,"at least detnum has to be given");
    goto ERR;};
#endif
#ifdef TRGDBmysql
while ((rowLTUS = msFetch(resultLTUS, &lengths))!=NULL) { 
  int i;
  int ixtab;   // detector Id
  int rc;
  char where[16];
  char outstr[80];
  sprintf(where,"name=%d",ixtab);
  /*for(i = 0; i < fields; i++) { 
    printf("[%.*s] ", (int) lengths[i], row[i] ? row[i] : "NULL"); 
  }; printf("\n"); */
  //strncpy(detname, rowLTUS[0], lengths[0]);
  strcpy(detname, rowLTUS[0]);
  rc=msSelect1("id", "DETECTOR_CODES",where, outstr);
  if(rc<0) {
    printf("ERROR mSelect1 error\n"); continue;
  };
  ixtab=atoi(outstr);
  if(rc==0) {   // error: "required # of fields not received"
    printf("ERROR id:%d. %s missing in DETECTOR_CODES\n", ixtab, detname); 
    continue;
  };
  if((ixtab<0) || (ixtab>23)) {
    printf("ERROR id:%d for det. %s in DETECTOR_CODES\n", ixtab, detname); 
    continue;
  };
  if(validLTUs[ixtab].name[0]!='\0') {
    printf("ERROR id:%d for detectors %s %s\n", ixtab, detname,
      validLTUs[ixtab].name); 
    continue;
  };
  a3[0]= ixtab;     // ! MySQL order in TRG_LTUS is important !
  for(i=1; i<=3; i++) {
    if(rowLTUS[i]!=NULL) {
      if(rowLTUS[i][0]!='\0') {
        a3[i]=atoi(rowLTUS[i]);
      };
    };
  };
  strcpy(ltubase, rowLTUS[4]);
#endif
  strcpy(validLTUs[ixtab].name, detname);
  validLTUs[ixtab].detnum=a3[0];   // the same as index in validLTUs
  validLTUs[ixtab].fo=a3[1];
  validLTUs[ixtab].foc=a3[2];
  validLTUs[ixtab].busyinp=a3[3];
  validLTUs[ixtab].i2cchan=a3[4];
  validLTUs[ixtab].i2cbran=a3[5];
  strcpy(validLTUs[ixtab].ltubasea, ltubase);
  validLTUs[ixtab].ltuvsp=-2;
  /*printf("readTables:%s %d %d %d %s\n", validLTUs[ixtab].name,
    validLTUs[ixtab].fo, validLTUs[ixtab].foc, validLTUs[ixtab].busyinp,
    validLTUs[ixtab].ltubasea); */
}; 
goto RTRN;
#ifdef TRGDBfiles
ERR:
sprintf(emsg, "readTables: bad line:%s in VALID.LTUS. %s\n",line,em1); 
prtError(emsg);
RTRN: 
fclose(cfgfile);
/*----------------------------------------------- ctpinputs.cfg */
cfgfile=openFile("ctpinputs.cfg","r");
if(cfgfile == NULL) { return; };
for(ixtab=0; ixtab<NCTPINPUTS; ixtab++) {
  validCTPINPUTs[ixtab].name[0]='\0';
  validCTPINPUTs[ixtab].detector=-1;
  validCTPINPUTs[ixtab].level=-2;
  validCTPINPUTs[ixtab].signature=0xffffffff;
  validCTPINPUTs[ixtab].inputnum=-1;  // 0..24 (0..12 for L2)
  validCTPINPUTs[ixtab].lminputnum=-1;  // 0..12
  validCTPINPUTs[ixtab].dimnum=-1;      // -1: not set yet
  validCTPINPUTs[ixtab].switchn=-1;   // 1..48 for L0, 0: for others
  validCTPINPUTs[ixtab].edge=-1;
  validCTPINPUTs[ixtab].delay=-1;
  validCTPINPUTs[ixtab].lmdelay=-1;
  validCTPINPUTs[ixtab].deltamin=-1;
  validCTPINPUTs[ixtab].deltamax=-1;
};
ixtab=0;
while(fgets(line, MAXLINELENGTH, cfgfile)){
  int ixx, detnum;
  char inpname[MAXCTPINPUTLENGTH];
  //printf("Decoding line:%s ",line);
  if(line[0]=='#') continue;
  if(line[0]=='\n') continue;
  ix=0; token= nxtoken1(line, value, &ix);
  if(token!=tSYMNAME) {
    strcpy(em1,"Input name expected first"); goto ERRctpignore; };
  value[MAXCTPINPUTLENGTH-1]='\0'; strcpy(inpname, value); 
  token=nxtoken(line, value, &ix);
  if(token!=tASSIGN) {strcpy(em1,"= expected"); goto ERRctpignore; };
  if((strncmp(inpname,"l0f",3)==0) ||
     (strncmp(inpname,"l0A",3)==0) ||
     (strncmp(inpname,"l0B",3)==0)) {
    // ignore l0f/ l0f34 definitions (there can be any number of these definitions):
    continue;
  };
  token=nxtoken(line, value, &ix);
  if(token!=tSYMNAME) {
     strcpy(em1,"Det. name expected after ="); goto ERRctpignore; };
  value[MAXCTPINPUTLENGTH-1]='\0'; strcpy(detname, value); 
  if(strcmp(detname, "CTP")==0) continue;
  detnum= findLTUdetnum(detname);
  if(detnum==-1) {
    sprintf(em1,"unknown detector: %s",detname); 
    goto ERRctpignore;
  };
  for(ixx=0; ixx<MaxIntItems; ixx++) a3[ixx]=0;
    a3[5]=-1; a3[6]=-1;  // edge, delay not defined
    a3[7]=1000; a3[8]=1000;  // delta min/max not defined
  // inpname, detname
  for(ixx=0; ixx<MaxIntItems; ixx++) {
    int negv;
    negv=1;
    token=nxtoken(line, value, &ix);
    //printf("INFO inpname:%s %s :%s:\n", inpname, detname, value);
    if(token==tINTNUM) { 
      // level, signature, InpN DimN SwitchN(was Configured) Edge Delays DeltaMin DeltaMax
      // 0      1          2    3    4                       5    6      7        8
      a3[ixx]= str2int(value);
    } else if(token==tSYMNAME) {
      // possible M-line or error
      if(strcmp(value, "M")==0) {
        int ixtb;
        a3[0]=3;   // M level, we need to fill a3[2] a3[4]  a3[6]
        token=nxtoken(line, value, &ix);
        token=nxtoken(line, value, &ix);   // lminput 1..12
        if(token==tINTNUM) { 
          a3[2]= str2int(value);
          if(a3[2] > 12) {
            sprintf(em1, "M-line: bad LM input (number 0..12 expected) ");
            goto ERRctp;
          };
        } else {
          sprintf(em1, "M-line: bad LM input after LM switch (0..12 expected) ");
          goto ERRctp;
        };
        token=nxtoken(line, value, &ix);
        token=nxtoken(line, value, &ix);   // switch input 1..12
        if(token==tINTNUM) { 
          a3[4]= str2int(value);
          if(a3[4] > 12) {
            sprintf(em1, "M-line: bad switch input (number 1..12 expected) ");
            goto ERRctp;
          };
        } else {
          sprintf(em1, "M-line: bad switch input (1..12 expected) ");
          goto ERRctp;
        };
        ixtb= findSwitchInput(a3[4]);
        if(ixtb<0) {
          sprintf(em1, "M-line not preceeded by L0 line definition)");
          goto ERRctp;
        };
        token=nxtoken(line, value, &ix);
        token=nxtoken(line, value, &ix);   // lmdelay 0..7
        if(token==tINTNUM) { 
          a3[6]= str2int(value);
          if(a3[6] > 7) {
            sprintf(em1, "M-line: bad delay (number 0..7 expected) ");
            goto ERRctp;
          };
        } else {
          sprintf(em1, "M-line: bad delay (0..7 expected) ");
          goto ERRctp;
        };
        validCTPINPUTs[ixtb].lminputnum= a3[2];
        validCTPINPUTs[ixtb].lmdelay= a3[6];
        break;
      } else {
        sprintf(em1, "Incorrect 4th item (0, 1, 2 or M expected got:%s)",value);
        goto ERRctp;
      };
    } else {
      if( (ixx==5) || (ixx==6) ) {
        sprintf(emsg, "Missing edge or delay"); prtWarning(emsg);
      } else if( (ixx==7) || (ixx==8) ) {
        // negative values allowed for deltamin/max:
        if(token==tMINUS) {
          negv=-1;
          token=nxtoken(line, value, &ix);
          if(token==tINTNUM) { // level, signature, InpNum Dimnum swin edge del
            a3[ixx]= negv*str2int(value);
          } else {
            sprintf(emsg, "Bad deltamin or deltamax:int expected after - sign"); prtWarning(emsg);
          };
        } else {
          sprintf(emsg, "Missing deltamin or deltamax"); prtWarning(emsg);
        };
      } else {
        strcpy(em1,"Int expected as level, signature, InpNum, Dimnum, SwitchN, Edge ,Delay or deltamin/max"); 
        goto ERRctp;
      };
    };
  };
  if( (a3[4]==0) and (a3[2]==0) ) continue;   // not configured
  if(a3[0]==3) continue;   // LM-line processed just now
  // todo: add here the check for duplicate names among L0/1/2 inputs, (no LM -see previous line)
  strcpy(validCTPINPUTs[ixtab].name, inpname);
  validCTPINPUTs[ixtab].detector= detnum;
  validCTPINPUTs[ixtab].level= a3[0];
  validCTPINPUTs[ixtab].signature= a3[1];
  validCTPINPUTs[ixtab].inputnum= a3[2];
  validCTPINPUTs[ixtab].dimnum= a3[3];
  validCTPINPUTs[ixtab].switchn= a3[4];
  validCTPINPUTs[ixtab].edge= a3[5];
  validCTPINPUTs[ixtab].delay= a3[6];
  validCTPINPUTs[ixtab].deltamin= a3[7];
  validCTPINPUTs[ixtab].deltamax= a3[8];
  printf("INFO readTables:%d:%s %d L%d %d in:%d %d switch:%d edge:%d del:%d deltas:%d %d\n", 
    ixtab,
    validCTPINPUTs[ixtab].name,
    validCTPINPUTs[ixtab].detector, validCTPINPUTs[ixtab].level,
    validCTPINPUTs[ixtab].signature, validCTPINPUTs[ixtab].inputnum,
    validCTPINPUTs[ixtab].dimnum, validCTPINPUTs[ixtab].switchn,
    validCTPINPUTs[ixtab].edge, validCTPINPUTs[ixtab].delay,
    validCTPINPUTs[ixtab].deltamin, validCTPINPUTs[ixtab].deltamax); 
  ixtab++;
  if(ixtab>NCTPINPUTS) {
    strcpy(em1,"Too many inputs in ctpinputs.cfg"); 
    goto ERRctp;
  };
  continue;
  ERRctpignore:
  sprintf(emsg, "ERROR ctpinputs.cfg line ignored:%s\n %s",line,em1); 
  prtWarning(emsg);
};
update_dimnum(0);
printf("INFO readTables LM inputs: ixtab name lmin lmdel\n");
for(ixtab=0; ixtab<NCTPINPUTS; ixtab++) {
  int lmin,flgs;
  if( (validCTPINPUTs[ixtab].name[0] == '\0') || (validCTPINPUTs[ixtab].dimnum == -1)) continue;
  lmin=validCTPINPUTs[ixtab].lminputnum;
  flgs=validCTPINPUTs[ixtab].dimnum>>24;
  if(lmin > 0) {
    printf("INFO %d:%s -> %d %d\n", ixtab,validCTPINPUTs[ixtab].name, lmin,
      validCTPINPUTs[ixtab].lmdelay);
  };
  if(flgs&FLG_FILTEREDOUT) {
    printf("INFO %d:%s filtered out\n", ixtab,validCTPINPUTs[ixtab].name);
  };
  if(flgs&0xfe) {   // only 'filtered out' flag (0x1) existing for now
    printf("INTERROR %d:%s flgs: 0x%x\n", ixtab,validCTPINPUTs[ixtab].name, flgs);
  };
};
goto RTRNctp;
ERRctp:
sprintf(emsg, "readTables: bad line:%s in ctpinputs.cfg. %s",line,em1); 
prtError(emsg);
RTRNctp: 
fclose(cfgfile);
#endif
#ifdef TRGDBmysql
ERR:
RTRN:
mysql_free_result(resultLTUS);
msClose();
#endif
return;
}
/*------------------------------------------------------------findInputName()
Input: name
Out:   index (0..) pointing to validCTPINPUTs[]. -1 in case of error
!Note: check validCTPINPUTs[ix].inpnum for L0 (it can be 0 i.e. not connected
in case of L0 input)
*/
int findInputName(char *name) {
int ix;
for(ix=0; ix<NCTPINPUTS; ix++) {
  //printf("%s %d\n", validCTPINPUTs[ix].name, validCTPINPUTs[ix].detector);
  if(strcmp(validCTPINPUTs[ix].name, name) == 0) return(ix);
}; //printf("\n");
return(-1);
}
/*------------------------------------------------------------findSwitchInput()
swinput: 1..48    rc: -1 not found    or index with 1st occurance of swinput
Note: swinput can feed more L0 (1..24) inputs, i.e. the line with
value of swinput in SwitchN column can appear more times.
*/
int findSwitchInput(int swinput) {
int ix;
for(ix=0; ix<NCTPINPUTS; ix++) {
  if((validCTPINPUTs[ix].level==0) &&
     (validCTPINPUTs[ix].switchn==swinput)
    ) return(ix);
}; //printf("\n");
return(-1);
}
/*------------------------------------------------------------findInput()
Input: level (0-2) and input number(1-24) i.e. after L0 switch (if L0 input)
Out:   index (0..) pointing to validCTPINPUTs[]. -1 in case of error
Note: use findSwitchInput for switch input (i.e. input: 1..48)
*/
int findInput(int level, int input) {
int ix;
if(input>24) return(-1);
for(ix=0; ix<NCTPINPUTS; ix++) {
  //printf("%s %d\n", validCTPINPUTs[ix].name, validCTPINPUTs[ix].detector);
  if((validCTPINPUTs[ix].level==level) &&
     (validCTPINPUTs[ix].inputnum==input)  // && (validCTPINPUTs[ix].configured==1) 
    ) {
    if(level==0) {
      if(validCTPINPUTs[ix].switchn!=0) {   // connected
        return(ix);
      };
    } else {   // do not care about switchn
      return(ix);
    };
  };
}; //printf("\n");
return(-1);
}
/*------------------------------------------------------------getEdgeDelayDB()
Input: level (1-2) and input number(24 or 12)
       level:0  input: 1.48 (i.e. input before switch!)
Out:   rc: 0:ok, -1: error (not configured)
       edge delay
*/
int getEdgeDelayDB(int level, int input, int *edge, int *delay) {
int ix;
/*if((level==0) && l0C0()) {
we need to find similar func like l0C0() getting input from sh. memory? */
if((level==0) && (NCTPINPUTS==84) ) {
  ix= findSwitchInput(input);
} else {
  ix= findInput(level, input);
};
if(ix==-1) return(-1);
*edge= validCTPINPUTs[ix].edge;
*delay= validCTPINPUTs[ix].delay;
return(0);
}
/*------------------------------------------------------------getSwnDB(inp)()
input: CTP input (after L0 switch), i.e. 1..24
rc: Switch input (1..48)
*/
int getSwnDB(int input) {
int ix;
ix= findInput(0, input);
if(ix==-1) return(-1);
return(validCTPINPUTs[ix].switchn);
}
/*------------------------------------------------------------findINPdaqdet()
Input: level (0-2) and input number(1-24)
Out:   ECS/DAQ detector number providing this input (0..23) or
       -1 -not connected
*/
int findINPdaqdet(int level, int input) {
int ix, rc=-1;
for(ix=0; ix<NCTPINPUTS; ix++) {
  //printf("%s %d\n", validCTPINPUTs[ix].name, validCTPINPUTs[ix].detector);
  if((validCTPINPUTs[ix].level==level) &&
     (validCTPINPUTs[ix].inputnum==input)  // && (validCTPINPUTs[ix].configured==1) 
    ) {
    if(level==0) {
      if(validCTPINPUTs[ix].switchn!=0) {   // connected
        return(validCTPINPUTs[ix].detector);
      };
    } else {   // do not care about switchn
      return(validCTPINPUTs[ix].detector);
    };
  };
}; //printf("\n");
return(rc);
}
/*------------------------------------------------------------findLMINPdaqdet()
lminput: LM input number(1-12) i.e. after switch
Out:   ECS/DAQ detector number providing this input (0..23) or
       -1 -not connected
*/
int findLMINPdaqdet(int lminput) {
int ix, rc=-1;
for(ix=0; ix<NCTPINPUTS; ix++) {
  //printf("%s %d\n", validCTPINPUTs[ix].name, validCTPINPUTs[ix].detector);
  if((validCTPINPUTs[ix].level==0) &&
     (validCTPINPUTs[ix].lminputnum==lminput)  // also LM input
    ) {
    if(validCTPINPUTs[ix].switchn!=0) {   // conneted
      return(validCTPINPUTs[ix].detector);
    };
  };
}; //printf("\n");
return(rc);
}
/*------------------------------------------------------------findLTUdaqdet()
Input: DAQ/ECS detector number (0...)
rc: return pointer into validLTUs[] item table or NULL if not found */
Tdetector *findLTUdaqdet(int daqdetnum) {
int ix=daqdetnum;
if( (daqdetnum<NDETEC) && (validLTUs[daqdetnum].name[0]!='\0') ){
  return(&validLTUs[ix]);
} else {
  return(NULL);
};
}
/*------------------------------------------------------------findLTU()
rc: return pointer into validLTUs[] item table or NULL if not found */
Tdetector *findLTU(char *ltuname) {
int ix;
for(ix=0; ix<NDETEC; ix++) {
  //printf("%s %d\n", validLTUs[ix].name, validLTUs[ix].fo);
  if(strcmp(validLTUs[ix].name, ltuname)==0 ) return(&validLTUs[ix]);
}; //printf("\n");
return(NULL);
}
/*------------------------------------------------------------findLTUdetnum()
rc: ECS/DAQ det number (0...). rc== -1 if not found */
int findLTUdetnum(char *ltuname) {
int ix; char value[24];
if(strcmp(ltuname,"DAQ_TEST")==0) {
  strcpy(value, "DAQ");   // DAQ in VALID.LTUS
} else {
  strcpy(value, ltuname);
};
for(ix=0; ix<NDETEC; ix++) {
  //printf("%s %d\n", validLTUs[ix].name, validLTUs[ix].fo);
  if(strcmp(validLTUs[ix].name, value)==0 ) return(validLTUs[ix].detnum);
}; //printf("\n");
return(-1);
}
/*------------------------------------------------------------ bit2nameold() 
int bit2name(w32 ctprodets, char *detname){
int ix, det=-1;
Tdetector *ltuitem;
for(ix=0; ix<24; ix++) {
  if(ctprodets & (1<<ix)) {
    if(det==-1) {
      det= ix;
    } else {
      printf("bit2name: more dets, det. pattern:0x%x\n",ctprodets);
      detname[0]='\0'; return;   // more detectors
    };
  };
};   // attempt to pause a detector not included in the partition
if(det==-1) {
  printf("bit2name: empty det. pattern:0x%x\n",ctprodets);
  detname[0]='\0'; return;};   // no detector in bit pattern
ltuitem= findLTUdaqdet(det); 
if(ltuitem==NULL) {
  printf("bit2name: detector %d not found in VALID.LTUS, det. pattern:0x%x\n",
    det, ctprodets);
  detname[0]='\0'; return;};   // unknown detector
strcpy(detname, ltuitem->name);
return;
} */
/*------------------------------------------------------------ bit2name() 
ctprodets: bit pattern of readout detectors
out: rc: number of detectors in bit pattern
     detname: e.g.: "SPD,MUON_TRK,TOF"
*/
int bit2name(w32 ctprodets, char *detname){
int ix, det=-1, ndet=0;
Tdetector *ltuitem;
detname[0]='\0';
for(ix=0; ix<24; ix++) {
  if(ctprodets & (1<<ix)) {
    det= ix;
    ltuitem= findLTUdaqdet(det); 
    if(ltuitem==NULL) {
      printf("ERROR bit2name: detector %d not found in VALID.LTUS, det. pattern:0x%x\n",
        det, ctprodets);
    } else {
      ndet++;
      if(ndet==1) {
        sprintf(detname, "%s",ltuitem->name);
      } else {
        sprintf(detname, "%s,%s",detname,ltuitem->name);
      };
    };
  };
};
return(ndet);
}

/*------------------------------------------------------------detList2bitpat()
I: dlist: "SPD,MUON_TRK"
O: rc: detector pattern (bits 0..23 set for given detectors)
       -1 in case of bad dlist
12.4.2016: "CTP" detector ignored
*/
int detList2bitpat(char *dlist) {
enum Ttokentype token; int ix=0; int rc=0;
char value[MAXLINELENGTH];
while(1) {
  token= nxtoken(dlist, value, &ix); if(token==tEOCMD) break;
  if(token==tSYMNAME) {
    int bit;
    if(strcmp(value,"CTP")!=0) {
      bit= findLTUdetnum(value);
      if(bit==-1) {
        // sprintf(em1," unknown detector:%s",value);
        rc=-1; break;
      };
      rc= rc | (1 << bit);
    }
  } else {
    rc=-1; break;
  };
  token= nxtoken(dlist, value, &ix); if(token==tEOCMD) break;
  if(token!=tCOMMA) {
    //strcpy(em1,", expected");
    rc=-1; break;
  };
};
return(rc);
}
/*------------------------------------------------------------findBUSYINP()
This is used only fom ctpcfg.py.
fo:1-6, foc:1-4
Ret: 1-24, 0: not connected (
*/
int findBUSYINP(int fo, int foc) {
int ix;
for(ix=0; ix<NDETEC; ix++) {
  if(validLTUs[ix].fo==0) continue;
  if(validLTUs[ix].fo==fo) {
    if(validLTUs[ix].foc==foc) {
      return(validLTUs[ix].busyinp);
    };
  };
};
return(0);
}
/*------------------------------------------------------------ findBUSYinputs()
ctprodets: readout detectors pattern (bits 0..23)
rc: busy pattern (bits 0..23). 1: corresponds to used BUSY input
*/
w32 findBUSYinputs(w32 ctprodets) {
int idet; w32 busyclusterT=0;
for(idet=0;idet<NDETEC;idet++){
  int busyinp;
  if((ctprodets & (1<<idet))==0) continue;
  if((busyinp=validLTUs[idet].busyinp)) {  //BUSY line connected too:
    busyclusterT= busyclusterT | (1<<(busyinp-1));
  };
}; return(busyclusterT);
}
/*------------------------------------------------------------findLTUNAMESby()
I: busypat
   detpat: consider only detectors given in this pattern
O:
ltunames: string (length:200) containing LTU names according 
          to given busy inputs pattern
*/
void findLTUNAMESby(w32 busypat, w32 detpat, char *names) {
int bix;
Tdetector *detp;
//printf("findLTUNAMESby: 0x%x 0x%x\n", busypat, detpat);
names[0]='\0';
for(bix=0; bix<NDETEC; bix++) {
  int byin;
  detp= findLTUdaqdet(bix); 
  if(detp==NULL) continue;    // not connected
  if( (detpat & (1<<bix))==0 ) continue;  // not participating detector
  byin= detp->busyinp;
  if(byin==0) continue;    // not connected
  if( busypat & (1<<(byin-1)) ) {
    strcat(names, detp->name); strcat(names," ");
  };
};
}
/*------------------------------------------------------------printLTUname()
fo:1-6, foc:1-4
Ret: name\n printed or ""
*/
void printLTUname(int fo, int foc) {
int ix;
for(ix=0; ix<NDETEC; ix++) {
  if(validLTUs[ix].fo==0) continue;
  if(validLTUs[ix].fo==fo) {
    if(validLTUs[
ix].foc==foc) {
      printf("%s",validLTUs[ix].name);
      return;
    };
  };
};
}
/*----------------------------------------------Detector2Connector()
 Purpose: to get connector and FO number from detector number
 Parameters: 
       input: idet - DAQ detector number (index into validLTUs[])
       output: ifo - FO number-1: 0-5
               iconnector - connector number-1: 0-3
 Returns: 1: not connected
          0: OK, connected, ifo,iconnector valid
*/
int Detector2Connector(int idet,int *ifo,int *iconnector){
*ifo= validLTUs[idet].fo-1;
*iconnector= validLTUs[idet].foc-1;
/*von
if(idet == 8) {
  idetout=0;   // FO 0 det 0
} else if(idet == 9) {
  idetout=1;   // FO 0 det 1
} else {

*/
if(*ifo==-1) {
  char emsg[ERRMSGL];
  sprintf(emsg, "Detector2Connector: detector %i not connected to FO\n",idet); 
  prtError(emsg);
  return 1;
};
return 0;
}
/*-------------------------------------------------------Connector2Detector()
Purpose: to get detector number from fo and connector.
Input: fo: 1-6 con: 1-4
Output:
RC: 0 = ok 1 = det not found
det: index to validLTUs[] table
*/
int Connector2Detector(int fo,int con,int *det){
 int i;
 Tdetector *d;
 for(i=0;i<NDETEC;i++){
   d=&validLTUs[i];
   if((d->fo==fo) && (d->foc == con)) break;
 }
 if(i<NDETEC){ *det=i ;return 0;}else return 1;
}

