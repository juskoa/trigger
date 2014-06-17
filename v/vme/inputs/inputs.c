/*BOARD SYNCctp 0x820000 0xd000 A24
*/
/* ADCI.c
5.7.2004
from version >ac.rbf works only with front panel cabel connected with
BC comming through L1 data wire. With version <=ac.rbf should work
autonomously
27.10.2005
rndtest(): now PLL_RESET done after each change of BC_DELAY_ADD and
           average wait for end of PLL_RESET is printed
24.8.2008:
SMAQDATA -environment variable. Name of directory in alidcscom027:SMAQ/datadir
          default: "last"
          see bin/smaq script
28.8.2008
two new features:
- option with interface board is printing BC number as found on interface board
- checkInputs2: calcultes correlation between inpnum (= trigger) and other selected
  inputs to get alignment even faster.
11.9.
send 'bobr' DIM command to ACR07/BOBR server running on aldaqacr07 machine
*/
#include <stdio.h>
#include <unistd.h>    /* usleep */
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>

#ifdef CPLUSPLUS
#include <dic.hxx>
#else
#include <dic.h>
#endif

#include "vmewrap.h"
#include "../ctp/ctplib/ctplib.h"
#include "../ctp/ctp.h"
#include "shmaccess.h"
#define DBMAIN
#include "../ctp_proxy/Tpartition.h"
#include "ssmctp.h"

#define BC_STATUSpll 0x2
#define nread 300
#define bit8 256
#define bit0_7 (1+2+4+8+16+32+64+128)

char dirname[40]="last";
int siteflag=0;
void getdatetime(char *);
int dumpssm(int board,char *filename);
//double rnlx();
//void setseeds(long,int);
int align(w32 *sm1,int *chans1,int *sm2,int *chans2,int offset,int cordist,int delta);
int syncSSM(int n, int *boards);
w32 orbitstatus;
// DIM 
char DETSET_COM[256]="NONE";
char DETSTAT_COM[256]="NONE";
#define MAXCTPINPUTS 10 
char StatusString[MAXCTPINPUTS];
char StatusFailed[MAXCTPINPUTS];

/*HIDDEN Common */
/*HIDDEN L012 */
/*HIDDEN DebCon */
/*HIDDEN DbgScopeCalls */
/* HIDDEN DebugSSMcalls */
/*HIDDEN DbgSSMBROWSERcalls */

/*FGROUP TOP GUI SSMbrowser
Browse CTP snapshot memories 
*/
/*FGROUP TOP GUI INPUTS 
The main window allowing 
- to edit inputs:
    Delete,Modify,Add,Print
- Check activity: count all 1 in snapshot memory
- Synchronise chosen inputs
- Autocorrelate chosem inputs
- Align chosen input
---------------------------------------------
The full procedure is:
1.) Choose the inputs
2.) Measure the phases
3.) Synchronise inputs
4.) Align inputs:
    - first each level sepratelly
    - the between levels

*/
void callback(void *tag, int *retcode){
 char command[100];
 printf("callback: %li %i ",*(long *)tag,*retcode);
 if(*retcode)printf("succesful.");
 else{ 
   printf(" failed: Wrong detector name or detector server not running.\n");
   return;
 }
 switch(*(long *)tag){
   case(333):
        strcpy(command,"STARTRUNCOUNT");
        break;
   case(18):
        printf(" Command %s executed.\n",DETSET_COM);
        break;
   case(33):
        printf(" Command bobr executed.\n");
        break;
   case(4567):
        printf("Service %s,\n CTP inputs status:<%s>\n",DETSTAT_COM,StatusString);
        break;
   default:
        printf("Unknown tag %li \n",*(long *)tag);
        return;
 }
}
/*FGROUP ADCtools 
Reads adc 300 times as quickly as possible and print it.
*/
void adcitest(int board)
{
 struct timeval tval;
 struct timezone tz;
 long tsec0,tusec0;
 int i;
 int busy,value;
 w32 buffer[nread];
 long tt,time[nread]; 
 
 gettimeofday(&tval,&tz);
 tsec0=tval.tv_sec;
 tusec0=tval.tv_usec;
 
 for(i=0;i<nread;i++){
   if( (i % 10000) == 0) vmew32(BSP*ctpboards[board].dial+ADC_START,0x0);
   buffer[i]=vmer32(BSP*ctpboards[board].dial+ADC_DATA);
   gettimeofday(&tval,&tz);
   tt=(tval.tv_sec-tsec0)*1000000+(tval.tv_usec-tusec0);
   time[i]=tt;
 }
 for(i=0;i<nread;i++){
  busy= ((buffer[i]&bit8) == bit8);  /* bit 8 of buffer[i] */
  value=buffer[i]&bit0_7;
  if( (i % 20) == 0 ) printf(" SEQ TIME BUSY VALUE (Time in microsec) \n");
  printf("%4i %4d   %d   %i \n",i,(int)time[i],busy,value);
 }
}
/*-----------------------------------------------------------------------------*/
/*FGROUP ADCtools
Scan of BC delay with going from 0 to 31 
(waiting 10000milsecs between measurements).
*/
void scanDel(int micseconds,int board)
{
 int i,buf[32];
 int imax=0,imin=0;
 int val;
 for(i=0;i<32;i++){
   vmew32(BUSY_DELAY_ADD,i);
   vmew32(BSP*ctpboards[board].dial+PLLreset, DUMMYVAL);
   //usleep(10000);
   usleep(micseconds);
  while(1) {
    w32 bcs;
    bcs= vmer32(BSP*ctpboards[board].dial+BC_STATUS); bcs=0x2;
    if(bcs & BC_STATUSpll) break;
  };
   val=readadc(board);
   buf[i]=val;
   if(val>buf[imax]) imax=i;
   if(val<buf[imin]) imin=i;
   printf("i=%i, val=%i \n",i,buf[i]);
 } 
 printf("max=%i %i min=%i %i \n",imax,buf[imax],imin,buf[imin]); 
}
/*----------------------------------------------------------------------*/
/*FGROUP ADCtools
Demonstrates time constant parameters of RL element in delay line.
*/
void adctimeconst(w32 delay0,w32 delay1,int board)
{
 int i,val;
 vmew32(BUSY_DELAY_ADD,delay0);
 usleep(100000);
 vmew32(BUSY_DELAY_ADD,delay1);
 for(i=0;i<10;i++){
  val=readadc(board);
  printf(" %i adc=%i \n",i,val);
  usleep(10000);
 }
 return ;
}
/*FGROUP ADCtools
Generates random delays and measure adc for each of them.
*/
void rndtest(int board)
{
#define NMEASUREMENTS 32 
 int i,value, all0mics,allmics;
 w32 delay=0,boardoffset;
 /* FILE *prn=fopen("WORK/bc.txt","w"); */
 w8 delays[NMEASUREMENTS];
 w8 vals[NMEASUREMENTS];
 allmics=0; all0mics=0;
 boardoffset=BSP*ctpboards[board].dial;
 orbitstatus=vmer32(BUSY_ORBIT_SELECT);
 vmew32(BUSY_ORBIT_SELECT,(orbitstatus&0xfcfff)+0x3000); 
 //printf("rndtest boardoffset=0x%x \n",boardoffset);
 for(i=0;i<NMEASUREMENTS;i++){
  w32 seconds1,micseconds1, seconds2,micseconds2,diff;
  //delay=32*rnlx();
  delay=(delay+7)%32;
  vmew32(BUSY_DELAY_ADD,delay);
  /* PLL_RESET always after dealy change then wait for PLL_LOCK: */
  GetMicSec(&seconds1, &micseconds1);
  //vmew32(BSP*ctpboards[0].dial+PLLreset, DUMMYVAL);
  //vmew32(BSP*ctpboards[board].dial+PLLreset, DUMMYVAL);
  /* first wait for 'unlocked' at least 300 milsecs 
  while(1) {
    w32 bcs;
    bcs= vmer32(BC_STATUS);
    GetMicSec(&seconds2, &micseconds2);
    diff=DiffSecUsec(seconds2, micseconds2, seconds1, micseconds1);
    if( diff > 200000) break;
    if((bcs & BC_STATUSpll)==0) break;
  };
  all0mics=all0mics+diff; */ 
  while(1) {
    w32 bcs;
    bcs= vmer32(BSP*ctpboards[board].dial+BC_STATUS); bcs=0x2;
    if(bcs & BC_STATUSpll) break;
  };
  GetMicSec(&seconds2, &micseconds2);
  diff=DiffSecUsec(seconds2, micseconds2, seconds1, micseconds1);
  allmics=allmics+diff;
  value=readadc_s(board);
  /*  fprintf(prn,"%i %i \n",delay,value); */
  /*if(value < 0)  printf("%i \n",delay);
   if( (i % 10) == 0)printf("%i\n",i);*/
  delays[i]= delay; vals[i]= value;
  /*  printf("<%i> <%i> \n",delay,value); */
 };
 /* fclose(prn); */
 for(i=0;i<NMEASUREMENTS;i++){
   printf("<%i> <%i> \n",delays[i],vals[i]);
 };
 /* rndtest finished. 
    Average wait for PLL unlocked
    Average wait for PLL */
 //printf("<%f> <%f>\n", 1.0*all0mics/NMEASUREMENTS,
 printf("%f %f\n", 1.0*all0mics/NMEASUREMENTS,
   1.0*allmics/NMEASUREMENTS);
  vmew32(BUSY_ORBIT_SELECT,orbitstatus);
}
/**-------------------------------------------------------*/
/*FGROUP ADCtools
*/
void setbcdelay(w32 delay) {
 vmew32(BUSY_DELAY_ADD,delay);
}
/*FGROUP ADCtools
rc: 2 BC_STATUS low bits: [BC_STATUSpll, BC_STATUSerr] */
w32 getbcstatus(int board) {
 w32 boardoffset;
 boardoffset=BSP*ctpboards[board].dial;
 return 0x2; //return 3&vmer32(boardoffset + BC_STATUS);
}
/*
 * run2 is same as run1 at the moment one has to think
 * what to do
 */
int setinputrun2(int board,w32 input){
  w32 boardoffset;
 int ret=0;
 boardoffset=BSP*ctpboards[board].dial;
 if(input > 53 || input < 0) {
   ret=2;
   goto RET;
 }
 if(board == 1 || board == 2){
 }else if(board == 3){
  if(input == 27)      input = 15;
  else if(input == 26) input = 14;
  else if(input == 25) input = 13;
  else if(input > 12){
   ret=1;
   goto RET;
  }
 }else{
  ret=3;
  goto RET;
 }
 vmew32(boardoffset+ADC_SELECT,input);
 RET:
 printf("board=%i input =%i ret=%i\n",board,input,ret);
 return ret;
}
int setinputrun1(int board,w32 input){
 w32 boardoffset;
 int ret=0;
 boardoffset=BSP*ctpboards[board].dial;
 if(input > 33 || input < 0) {
   ret=2;
   goto RET;
 }
 if(board == 1 || board == 2){
 }else if(board == 3){
  if(input == 27)      input = 15;
  else if(input == 26) input = 14;
  else if(input == 25) input = 13;
  else if(input > 12){
   ret=1;
   goto RET;
  }
 }else{
  ret=3;
  goto RET;
 }
 vmew32(boardoffset+ADC_SELECT,input);
 RET:
 printf("board=%i input =%i ret=%i\n",board,input,ret);
 return ret;
}
/*FGROUP ADCtools
  Inputs are counted from 1 to 24(12) as in hardware.
*/
int setinput(int board,w32 input){
 int ret=0;
 if(l0C0()==0){
   ret=setinputrun1(board,input);
 }else{
   ret=setinputrun2(board,input);
 }
 return ret;
}
/*FGROUP EDGEtools
Measure phase of ORBIT wrt to BC, by the edge mechanism on BUSY board.
*/
void measureedge(){
 int i,j;
 w32 delay,edge;
 for(i=0;i<32;i++){
  //delay=32*rnlx();
  delay = i%32;
  edge=0;
  vmew32(BUSY_DELAY_ADD,delay);
  for(j=0;j<20;j++){
   usleep(100);
   edge=edge+((vmer32(BUSY_ORBIT_SELECT)&(1<<14))==(1<<14));
  }
  printf("<%i> <%i> \n",delay,5*edge);
 }
}
/*FGROUP ALItools
*/
void setDelay(int board,w32 input,w32 delay){
 w32 word;
 if(delay>15){
   printf("Too big delay %i \n",delay);
   return;
 }
 word=vmer32(BSP*ctpboards[board].dial+SYNCH_ADD+4*(input-1));
 word=(word&0xfffffff0)+delay;
 vmew32(BSP*ctpboards[board].dial+SYNCH_ADD+4*(input-1),word); 
}
/* FGROUP EDGEtools 
moved to ctp/ctplib/inputsTools.c
*/

/*FGROUP EDGEtools
*/
void getEdge(int board,w32 input){
 w32 edge,delay;
 char status;
 edge=getedge(board,input,&delay);
 switch(edge){
    case 0: status='P';break;
    case 1: status='N';break;
    case 2: status='L';break;
    case 3: status='T';break;
    default: status='E';break;
 }
 printf("status=<%c> delay=<%i>\n",status,delay);
}
/*FGROUP ADCtools
Steering for phase measurement
Always set edge to 'P' before measurement and then set it back.
*/
int measurephase(int board,int input){
 int edge;
 w32 dum;
 if(setinput(board,input)) return 1;
 //edge=getedge(board,input,&dum);
 //if(edge>3) return 2;
 //setEdge(board,input,0);
 rndtest(board);  
 //setEdge(board,input,edge); 
 return 0;
}

//------------------SIGNATURE TOOLS---------------------------------
/*FGROUP SIGNATUREtools
Routine:
- read the detector name from the input window
- gets dns node
- get the status of detector inputs:
  0:N = normal
  1:T = toggling
  2:S = signature
  3:R = Random generator
    E = error
*/
void getDetInputStatus(char *Detector,int numofinputs){
 int ret,service_id;
 char node_name[256];
 strcpy(DETSET_COM,Detector);
 strcat(DETSET_COM,"/SET_OPTIONCODE");
 printf("Set command is %s \n",DETSET_COM);
 strcpy(DETSTAT_COM,Detector);
 strcat(DETSTAT_COM,"/STATUS_OPTIONCODE");
 printf("Stat command is %s \n",DETSTAT_COM);
 ret=dic_get_dns_node (node_name);
 if(ret != 1){
  printf("Cannot find Dim Name Server (DNS) ! \n");
 }else{
  printf("DIM_DNS_NODE=%s \n",node_name);
  //StatusString[0]='\0';
  strcpy(StatusString,"");
  for(int i=0;i<MAXCTPINPUTS;i++){
     StatusString[i]='?';
     StatusFailed[i]='X';
  }
  //StatusString[MAXCTPINPUTS]='\0';
  //StatusFailed[MAXCTPINPUTS]='\0';
  //service_id=dic_info_service(DETSTAT_COM,ONCE_ONLY,0,StatusString, numofinputs+1, 0 ,4567,&retcode,sizeof(int));
  service_id=dic_info_service(DETSTAT_COM,ONCE_ONLY,1,StatusString,MAXCTPINPUTS+1, 0 ,4567,StatusFailed,MAXCTPINPUTS+1);
  sleep(1);
  printf("Det status: <%s>  %i\n",StatusString,service_id);
  printf("Det status: %s  %i\n",StatusFailed,service_id);
  fflush(stdout);
  //dic_release_service (service_id);
 }
}
/*FGROUP SIGNATUREtools
N,T,S,R
Inputs are counted from 1. (Check in server.c)
*/
void setStatus(char *Detector,int input,char stat){
 int op,buffer[2];
 strcpy(DETSET_COM,Detector);
 strcat(DETSET_COM,"/SET_OPTIONCODE");
 //printf("Set command is %s \n",DETSET_COM);
 strcpy(DETSTAT_COM,Detector);
 strcat(DETSTAT_COM,"/STATUS_OPTIONCODE");
 printf("Stat command is %s \n",DETSTAT_COM);

 switch(stat){
  case 'N':op=0;break;
  case 'T':op=1;break;
  case 'S':op=2;break;
  case 'R':op=3;break;
  default:printf("setStatus: error: unknown stat %c\n",stat);return;
 }
  buffer[0]=op;
  buffer[1]=input;
  dic_cmnd_callback(DETSET_COM,buffer,2*sizeof(int),&callback,18);
}
/*FGROUP SIGNATUREtools
1=L0, 2=L1, 3=L2. 
Fast check of all input signals for choosen board.
The routine takes the snapshot of inputs and counts number of nonzero bits.
The approximate values are following:
- 524288 ~ TOGGLING (Mega/2),
- 11760  ~ ORBIT
- 11517(28)  ~ SIGNATURE 
You can check it with ssmbrowser.
*/
void checkInputsActivity(int board){
 char mode[32];
 int ChannelEmpty[24]; 
 int i,j,chan0,chanM;
 w32 word;
 for(i=0;i<24;i++)ChannelEmpty[i]=0;
 if(board==1){ 
   strcpy(mode,"l0_inmon");
   chan0=8;chanM=32;
 }
 else if(board==2){
   strcpy(mode,"l1_inmon");
   chan0=8;chanM=32;
 }
 else if(board==3){
   strcpy(mode,"l2_inmon");
   chan0=6;chanM=18;
 }
 else if(board==0){
   chan0=0;chanM=1;   
   strcpy(mode,"busy_inmon");
 }else{
  printf("Board %i has no inputs ! \n",board);
  return;
 }
 //printf("mode=%s \n",mode);
 setomSSM(board,0xa);   
 startSSM1(board);
 getswSSM(board);
 //setsmssw(board,mode);
 usleep(40000);
 //stopSSM(board);
 readSSM(board);
 getswSSM(board);
 for(i=0;i<Mega;i++){
  if((word=sms[board].sm[i])){
   for(j=chan0;j<chanM;j++)if((1<<j)&word)ChannelEmpty[j-chan0]++;
   }
 }
 //printf("Board %i Input occupancy: \n",board);
 if(board == 0)printf("<0> <%i> \n",ChannelEmpty[0]);
 else{
  printf(" BOARD L%i INP NONZERO \n",board-1);
  for(i=chan0;i<chanM;i++)if(ChannelEmpty[i-chan0])printf("<%i> <%i>\n",i-chan0+1,ChannelEmpty[i-chan0]);
 }
}
/*FGROUP SIGNATUREtools
  Check rnds and bcs for activity
*/
void checkInputsActivityRB(){
 char mode[32];
 int ChannelEmpty[32];
 int board=1,chan0,chanM,i,j;
 w32 word;
 for(i=0;i<32;i++)ChannelEmpty[i]=0;
 setomSSM(1,0x2);
 startSSM1(board);
 getswSSM(board);
 setsmssw(board,mode);
 usleep(40000);
 stopSSM(board);
 readSSM(board);
 //getswSSM(board);
 strcpy(mode,"l0_outmon");
 setsmssw(board,mode);
 chan0=0;chanM=31;
 for(i=0;i<Mega;i++){
  if((word=sms[board].sm[i])){
   for(j=chan0;j<chanM+1;j++)if((1<<j)&word)ChannelEmpty[j-chan0]++;
   }
 }
 printf(" BOARD L%i BC/RND NONZERO \n",board-1);
 for(i=chan0;i<chanM;i++)if(ChannelEmpty[i-chan0])printf("<%i> <%i>\n",i,ChannelEmpty[i-chan0]);
}
/*FGROUP SIGNATUREtools
Translates Signature number to LVDST SEQ_DATA word, i.e the least
2 significant bits are zero.
lvdst2 is hexa signature
*/
int SigNum2LVDSTNum(int SigNum){
 int lvdst,lvdst2;
 if(SigNum > 0x7f){
  printf("Signature > 0x7f \n");
  return 0;
 }
 lvdst=((~SigNum)& 0x7f)<<2;
 lvdst=lvdst+(SigNum<<9);
 lvdst=lvdst+(0xb1 << 16);  // HEADER
 lvdst2=((~SigNum)& 0x7f)+(SigNum<<7)+(0xb1<<14);
 printf("SEQ_DATA=0x%x 0x%x 0x%x\n",lvdst,lvdst2,lvdst2<<2);
 return (lvdst);
}
//-----------------
int checkSignature(w32 *sm,int *channels,int offset); // in signature.c
/*FGROUP SIGNATUREtools
  Signature= Header(8 bits)+Number(7 bits)+(~Number)
  Check signature at board in input. Input counted from 1
  Otput:
  Headers:    number of headers found
  FHeaders:   number of headers attemted but failed
  Signatures: number of ssm signatures equaled to input signature
  CmplCntsS:  number of cases when ssm signature == ~(compl ssm signature) 
  CmplCntsI:  number of cases when (compl ssm signature) = ~(input signature)
  CheckDist:  number of changes of distance between two subsequent headers

  Typical situations.
--------------------------------------------------------------------
Channel  Headers FHeaders Signatures CmplCntsS CmplCntsI CheckDist
    16        0        0         0         0         0         0 
No headers found, nothing in ssm channel.
---------------------------------------------------------------------
Channel  Headers FHeaders Signatures CmplCntsS CmplCntsI CheckDist
    16        0    11760         0         0         0         0 
No headers found, ssm channel has some nonzero bits.
-------------------------------------------------------------------
Channel  Headers FHeaders Signatures CmplCntsS CmplCntsI CheckDist
    19     1048        0         0         0         0         0 
Headers ok.
-------------------------------------------------------------------
Channel  Headers FHeaders Signatures CmplCntsS CmplCntsI CheckDist
    19     1048     1048      1048         0         0         0 
Complements are not ok, also some spurious signalbetween headers. 
--------------------------------------------------------------------
Channel  Headers FHeaders Signatures CmplCntsS CmplCntsI CheckDist
    19     1047        0      1047         0         0         0 
Headers and signatures ok, complements wrong
---------------------------------------------------------------------
Channel  Headers FHeaders Signatures CmplCntsS CmplCntsI CheckDist
    19     1047     1047      1047      1047      1047         0 
Looks like you have some spurious signal between signatures.
---------------------------------------------------------------------
Channel  Headers FHeaders Signatures CmplCntsS CmplCntsI CheckDist
    19     1048        0         0      1048         0         0 
You are almost there,
looks like signature in ssm is different from one you input
but otherwise everything ok.
---------------------------------------------------------------------
Channel  Headers FHeaders Signatures CmplCntsS CmplCntsI CheckDist
    19     1048        0      1048      1048      1048         0 
Everything OK!

  Check only one input
*/
void CheckSignature(int board,int signature,int input){
 int channels[32];
 int i,channel,offset;
 channel=input-1;
 for(i=0;i<32;i++)channels[i]=0;
 if(board==3)offset=6;else offset=8;
 channel=channel+offset;
 signature= SigNum2LVDSTNum(signature);
 channels[channel]=signature;
 checkInputsActivity(board);
 checkSignature(sms[board].sm,channels,offset);
 /*if(sms[board].sm) checkSignature(sms[board].sm,channels);
 else{
  printf("sms for board %i not read. Run checkInputs.\n",board);
 }*/
}
/*FGROUP SIGNATUREtools
  Check all inputs
*/
void FindSignatures(int board,char *inputs){
 int i,numinp=24,offset=8;
 int channels[32];
 if(board == 3){
   numinp=12;
   offset=6;
 } 
 for(i=0;i<32;i++)channels[i]=0;
 numinp=strlen(inputs);
 if(numinp>24)numinp=24;
 //for(i=0;i<strlen(inputs);i++)if(inputs[i]=='1')channels[i+offset]=1;
 for(i=0;i<numinp;i++)if(inputs[i]=='1')channels[i+offset]=1;
 printf("%s ",inputs);
 for(i=0;i<32;i++)printf("%1i",channels[i]);printf("\n");
 if(sms[board].sm) checkSignature(sms[board].sm,channels,offset);
 else{
  printf("sms for board %i not read. Run checkInputs.\n",board);
 } 
}
/*FGROUP ADCtools
*/
void getorbitstatus(){
 w32 orbitstatus;
 char status[7];
 orbitstatus=(vmer32(BUSY_ORBIT_SELECT)&(0x3000))>>12;
 printf("Orbit status= 0x%x \n",orbitstatus);
 switch(orbitstatus){
    case 0: strcpy(status,"Pos");break;
    case 1: strcpy(status,"Neg");break;
    case 2: strcpy(status,"Loc Orb");break;
    case 3: strcpy(status,"Toggle");break;
    default: strcpy(status,"Error");break;
 }
 printf("status=<%s> \n",status);  
}
/*----------------------------------------------------------------------
  Set ssms and browser
*/
void setL012mode(){
 char mode[32];
 strcpy(mode,"l0_inmon");
 setsmssw(1,mode);
 strcpy(mode,"l1_inmon");
 setsmssw(2,mode);
 strcpy(mode,"l2_inmon");
 setsmssw(3,mode);
}
/*********************************************************
*/
void initmain() {
 char *datadir;
 //setseeds(3,3); 
 cshmInit();
 checkCTP();
 initSSM();
 setL012mode();
 datadir= getenv("SMAQDATA");
 if(datadir !=NULL) {
   strcpy(dirname, datadir);
 };
}
void endmain() {
}
void boardInit(){
}
/* FGROUP Measure GUI Phase_Edge
Measurement of
- phase of the ORBIT wrt BC
- phase of the INPUT SIGNAL wrt to BC

To start CLICK 'Measure' and then  'Phase_Edge'
RIGHT mouse click on any button/label often helps you!

To meausure ORBIT wrt BC:
-------------------------
- Set Board=0 (BUSY)
- Input is not relevant, so ignored
- Click Measure
To measure INPUT SIGNAL wrt to BC:
----------------------------------
- Set Board = 1,2 or 3 (L0,L1 or L2)
- Choose input 
- Click Measure 

*/

