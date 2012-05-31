#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "vmewrap.h"
#include "ctp.h"
#define DBMAIN
/* only 1 of TRGDBmysql TRGDBfiles can be defined */
//#define TRGDBmysql
#define TRGDBfiles
#include "Tpartition.h"
#include "lexan.h"

#ifdef TRGDBmysql
#include "mysql.h"
int msOpen();
int msClose();
MYSQL_RES *msSelect(char *fields, char *table, char *whereexp, int *nfields);
MYSQL_ROW msFetch(MYSQL_RES *result, unsigned long **lengths);
int msSelect1(char *fields, char *table, char *whereexp, char *outstr);
#endif

/*------------------------------------------------------------readTables()
read files: VALID.LTUS into validLTUs[]
            VALID.CTPINPUTS into validCTPINPUTs[]
*/
void readTables() {
enum Ttokentype token;
int ix;
char emsg[ERRMSGL];
char em1[ERRMSGL];
char detname[MAXDETNAME];    // Name
int ixtab;
int a3[6];          // id, FO, FOcon, BUSYinput, i2cchan, i2cbran
char ltubase[12];   // LTUbase
#ifdef TRGDBfiles
char *environ;
FILE *cfgfile;
char fnpath[MAXNAMELENGTH+40];
char ctpinputspath[MAXNAMELENGTH+40];
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
environ= getenv("VMECFDIR"); strcpy(fnpath, environ);
strcat(fnpath,"/");
strcat(fnpath,"CFG/ctp/DB/"); strcpy(ctpinputspath,fnpath); 
strcat(fnpath, "VALID.LTUS");
strcat(ctpinputspath, "VALID.CTPINPUTS");
/*Format:  detname=DAQdet fo focon bsyinp ltubase i2cchan i2cbran
See description in VALID.LTUS file
Example:
TRD=4
TOF=5 1 3
MUON_TRK=9 1 2 0 0x812000 1 1
MUON_TRG=9 1 2 0 muon_trg 1 1
*/
cfgfile=fopen(fnpath,"r");
if(cfgfile == NULL){
  printf("fnpath:%s:\n", fnpath);
  perror(strerror(errno));
  return;
};
#endif
#ifdef TRGDBmysql
msOpen();
resultLTUS=msSelect("*", "TRG_LTUS","", &fields);
if(fields!=5) {   // error: "required # of fields not received"
  printf("Error: 5 fields expected, got:%d\n", fields);
};
#endif
for(ix=0; ix<NDETEC; ix++) {
  validLTUs[ix].name[0]='\0';
  validLTUs[ix].fo=0;
  validLTUs[ix].foc=0;
  validLTUs[ix].busyinp=0;
  validLTUs[ix].i2cchan=-1;
};
#ifdef TRGDBfiles
while(fgets(line, MAXLINELENGTH, cfgfile)){
  //printf("Decoding line:%s ",line);
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
    printf("mSelect1 error\n"); continue;
  };
  ixtab=atoi(outstr);
  if(rc==0) {   // error: "required # of fields not received"
    printf("id:%d. %s missing in DETECTOR_CODES\n", ixtab, detname); 
    continue;
  };
  if((ixtab<0) || (ixtab>23)) {
    printf("Error: id:%d for det. %s in DETECTOR_CODES\n", ixtab, detname); 
    continue;
  };
  if(validLTUs[ixtab].name[0]!='\0') {
    printf("Error: id:%d for detectors %s %s\n", ixtab, detname,
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
  printf("readTables:%s %d %d %d %s\n", validLTUs[ixtab].name,
    validLTUs[ixtab].fo, validLTUs[ixtab].foc, validLTUs[ixtab].busyinp,
    validLTUs[ixtab].ltubasea); 
}; 
goto RTRN;
#ifdef TRGDBfiles
ERR:
sprintf(emsg, "readTables: bad line:%s in VALID.LTUS. %s\n",line,em1); 
prtError(emsg);
RTRN: 
fclose(cfgfile);
/*----------------------------------------------- VALID.CTPINPUTS */
cfgfile=fopen(ctpinputspath,"r");
if(cfgfile == NULL){
  printf("fnpath:%s:\n", ctpinputspath);
  perror(strerror(errno));
  return;
};
for(ixtab=0; ixtab<NCTPINPUTS; ixtab++) {
  validCTPINPUTs[ixtab].name[0]='\0';
  validCTPINPUTs[ixtab].detector=-1;
  validCTPINPUTs[ixtab].level=-1;
  validCTPINPUTs[ixtab].signature=-1;
  validCTPINPUTs[ixtab].inputnum=-1;
  validCTPINPUTs[ixtab].dimnum=-1;
  validCTPINPUTs[ixtab].configured=-1;
};
ixtab=0;
while(fgets(line, MAXLINELENGTH, cfgfile)){
  int ixx, detnum;
  char inpname[MAXCTPINPUTLENGTH];
  //printf("Decoding line:%s ",line);
  if(line[0]=='#') continue;
  if(line[0]=='\n') continue;
  ix=0; token= nxtoken(line, value, &ix);
  if(token!=tSYMNAME) {
    strcpy(em1,"Input name expected first"); goto ERRctpignore; };
  value[MAXCTPINPUTLENGTH-1]='\0'; strcpy(inpname, value); 
  token=nxtoken(line, value, &ix);
  if(token!=tASSIGN) {strcpy(em1,"= expected"); goto ERRctpignore; };
  if(strncmp(inpname,"l0f",3)==0) {
    // ignore l0f* definitions (there can be any number of these definitions):
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
  for(ixx=0; ixx<5; ixx++) a3[ixx]=0;
  for(ixx=0; ixx<5; ixx++) {
    token=nxtoken(line, value, &ix);
    if(token==tINTNUM) { // level, signature, InpNum Dimnum Configured
      a3[ixx]= str2int(value);
    } else {
      strcpy(em1,"Int expected as level, signature, InpNum Dimnum or Conf"); 
      goto ERRctp;
    };
  };
  strcpy(validCTPINPUTs[ixtab].name, inpname);
  validCTPINPUTs[ixtab].detector= detnum;
  validCTPINPUTs[ixtab].level= a3[0];
  validCTPINPUTs[ixtab].signature= a3[1];
  validCTPINPUTs[ixtab].inputnum= a3[2];
  validCTPINPUTs[ixtab].dimnum= a3[3];
  validCTPINPUTs[ixtab].configured= a3[4];
  printf("readTables:%s %d L%d %d in:%d %d configured:%d\n", 
    validCTPINPUTs[ixtab].name,
    validCTPINPUTs[ixtab].detector,
    validCTPINPUTs[ixtab].level,
    validCTPINPUTs[ixtab].signature,
    validCTPINPUTs[ixtab].inputnum,
    validCTPINPUTs[ixtab].dimnum,
    validCTPINPUTs[ixtab].configured);
  ixtab++;
  if(ixtab>NCTPINPUTS) {
    strcpy(em1,"Too many inputs in VALID.CTPINPUTS"); 
    goto ERRctp;
  };
  continue;
  ERRctpignore:
  sprintf(emsg, "VALID.CTPINPUTS line ignored:%s\n %s",line,em1); 
  prtWarning(emsg);
};
goto RTRNctp;
ERRctp:
sprintf(emsg, "readTables: bad line:%s in VALID.CTPINPUTS. %s",line,em1); 
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
/*------------------------------------------------------------findINPdaqdet()
Input: level (0-2) and input number(1-24)
Out:   ECA/DAQ detector number connected to this input (0..23) or
       -1 -not connected
*/
int findINPdaqdet(int level, int input) {
int ix, rc=-1;
for(ix=0; ix<NCTPINPUTS; ix++) {
  //printf("%s %d\n", validCTPINPUTs[ix].name, validCTPINPUTs[ix].detector);
  if((validCTPINPUTs[ix].level==level) &&
     (validCTPINPUTs[ix].inputnum==input)  &&
     (validCTPINPUTs[ix].configured==1) 
    ) return(validCTPINPUTs[ix].detector);
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
int ix;
for(ix=0; ix<NDETEC; ix++) {
  //printf("%s %d\n", validLTUs[ix].name, validLTUs[ix].fo);
  if(strcmp(validLTUs[ix].name, ltuname)==0 ) return(validLTUs[ix].detnum);
}; //printf("\n");
return(-1);
}
/*------------------------------------------------------------findBUSYINP()
fo:1-6, foc:1-4
Ret: 1-24, 0: not connected
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
ltunames: string (length:200) containing LTU names according 
          to given busy inputs pattern
*/
void findLTUNAMESby(w32 busypat, char *names) {
int bix;
Tdetector *detp;
names[0]='\0';
for(bix=0; bix<NDETEC; bix++) {
  int byin;
  detp= findLTUdaqdet(bix); 
  if(detp==NULL) continue;    // not connected
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
    if(validLTUs[ix].foc==foc) {
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
/*----------------------------------------------------------------------Connector2Detector()
Purpose: to get detector number from fo and connector.
0 = ok
1 = det not found
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


