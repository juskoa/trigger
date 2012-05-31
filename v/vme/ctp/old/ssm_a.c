/* SSM routines used in v/vme/ctp package (i.e. all the
ctp boards accessed through 1 vme space */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/time.h>
#include "vmewrap.h"
#include "ctp.h"
#define SSMCTP
#include "ssmctp.h"

typedef struct{           //Mode (just to allow variable string length)
 int ModeNameSize;
 char *ModeName; 
}Mode;

typedef struct{           //Receiving channel on board
 struct RChannel *start;	
 struct RChannel *next;	
 int board;            
 int channel;             
}RChannel;

RChannel BoardChannels[NSSMBOARDS][32];

char *ConnectionNames[MAXNAMES]; /* used to store names of backplane 
				and Front Pannel connections. 
				The order number in array is used as
		                numerical name of the connection. */	
int nofnames;
int nofbckpl;
int A2BCD(int ,...);

void initSSM() {
int ix,jx,inames=0;
FILE *con;
char line[MAXLINE];
char *fcon="CFG/ctp/ssmsigs/backplanefp.names";

// Memory,offset,syncronisation flag
for(ix=0; ix<NSSMBOARDS; ix++) {
  sms[ix].sm= NULL;                
  sms[ix].modeSW[0]='\0';        
  sms[ix].modeHW[0]='\0'; 
  sms[ix].offset= 0;
  sms[ix].syncflag= 0;
  for(jx=0;jx<32;jx++)BoardChannels[ix][jx].start=NULL;  
};

// Assign board names
for(ix=0; ix<NCTPBOARDS; ix++) {
 strcpy(sms[ix].name,ctpboards[ix].name);	
}
strcpy(sms[NCTPBOARDS].name,"ltu1");	
strcpy(sms[NCTPBOARDS+1].name,"ltu2");
strcpy(sms[NCTPBOARDS+2].name,"ltu3");
strcpy(sms[NCTPBOARDS+3].name,"ltu4");
printf("The number of boards type is %i : \n",NSSMBOARDS);
for(ix=0; ix<NSSMBOARDS; ix++) printf(" %s",sms[ix].name);printf("\n");

//Read connection names from file
if((con=fopen(fcon,"r")) == NULL){
  printf("Cannot read initialisation file: backplanefp.names. Exiting. \n");
  exit(1);
}
while((fgets(line,MAXLINE,con)!=NULL) && nonemptyline(line)){
 ConnectionNames[inames]=(char *)malloc(NAMESIZE*sizeof(char));
 strcpy(ConnectionNames[inames],line);
 printf("%s \n",ConnectionNames[inames]);
 inames++;
} 
nofnames=inames;
inames=0;
while((inames<nofnames) && (ConnectionNames[inames][0] !='F'))inames++;
nofbckpl=inames;
printf("# signal names=%i  # of backplane names=%i \n",nofnames,nofbckpl);
}
/*-----------------------------------------------------*/
/* Check for empty line */
int nonemptyline(char *line){
 char *a,*b;	
 a = strchr(line,'\n');
 //printf("*a=%c\n",*a);
 b=line;
 while(b<a){
  //printf("*b=%i\n",*b);	 
  if(*b != ' '){
    *a='\0';   // remove '\n'	  
    return 1;
  }	  
  b++;  
 } 
 return 0; 
}
/*-------------------------------------------------------------- setomSSM()
Set operation & mode. If SSM is BUSY, an attempt is made to stop
the recording.
opmo values (see as defined in ctp.h):
0x0 -VME access, read
0x1 -VME access, write
0x2 -RECORDING, After  (cca 26 milsec)
0x3 -RECORDING, Before (should be stopped by SSMstoprec)
RC:  0->ok, mode set
     1->mode not set, possible errors (printed to stdout):
        -BC signal not connected
        -Cannot stop recording operation
*/
int setomSSM(int board, w32 opmo) {
w32 status;
if((opmo==SSMomreca || opmo==SSMomrecb)&&((vmer32(BC_STATUS)&0x3)!=0x2)) {
  printf("ERROR: BC signal not connected\n");/* BC not necessary for vme R/W*/
  return(1);
};
status=vmer32(SSMstatus+BSP*ctpboards[board].dial);
if( status & 0x4) {
  printf("SSM busy, stopping recording before setting new/op. mode...\n");
  vmew32(SSMstop+BSP*ctpboards[board].dial,DUMMYVAL);
  status=vmer32(SSMstatus+BSP*ctpboards[board].dial);
  if( status & 0x4) {
    printf("ERROR: Cannot stop recording, operation/mode\n");
    printf("       not set!\n");
    return(1);
  };
};
vmew32(SSMcommand+BSP*ctpboards[board].dial, opmo);
return(0);
}

/*FGROUP DebugSSMcalls
 * ------------------------------------------------------ readSSM()
read whole SSM into array of unsigned ints
Input:
board: board according to ctpboards global array
*/
int readSSM(int board) {
w32 ssmoffset,d; int i,rc;
w32 *array;
if(sms[board].sm==NULL) {
  array= (w32 *)malloc(Mega*sizeof(w32));
  sms[board].sm= array;
} else {
  array= sms[board].sm;   /* rewrite already allocated memory */
};
sms[board].syncflag= -1;   /* this SSM is not synchronised */
/* pattern for debugging: */
for(i=0; i<Mega; i++) {
  array[i]= i<<board;
};
return(0);
ssmoffset= BSP*ctpboards[board].dial;
/* Read Mega words from SSM into buf.*/
 rc= setomSSM(board, SSMomvmer);
 if(rc!=0) return(rc);
 vmew32(SSMaddress+ssmoffset,-1);
 d= vmer32(SSMdata+ssmoffset);
 d= vmer32(SSMdata+ssmoffset);
 for(i=0; i<Mega; i++) {
   array[i]= vmer32(SSMdata+ssmoffset);
 };
return(rc);
}
/*FGROUP DebugSSMcalls
 * ------------------------------------------------------ writeSSM()
write whole sms[].sm into hardware
Input:
board: board according to ctpboards global array
*/
int writeSSM(int board){
 printf("Writing sms[%i].sm to hardware \n",board);	
}
/*FGROUP DebugSSMcalls
print to stdout SSM board from word 'fromadr'
*/
void printSSM(int board, int fromadr) {
w32 *array; int adr;
array= sms[board].sm;   /* rewrite already allocated memory */
for(adr=fromadr; adr<fromadr+10; adr++) {
  printf("%7d: 0x%8.8x\n", adr, array[adr]);
};
}
/*------------------------------------------------------ syncSSM()
Synchronise SSMs (pointers to their contents are in sms[i].sm)
Input:
n - number of SMS to be synced (minimal 2)
... -numbers of SSMs to be synced.
ORDER is IMPORTANT !
First SSM in list -> first SSM started -> largest offset.
Assumption: all SSM start during one orbit.
(Solution without this assumption may exists if SSM multiple of Orbit).
*/
int syncSSM(int n, int *boards) {
 int i,j,ret,offset,offsets[NSSMBOARDS];
 for(i=0; i<n; i++){                       // loop over ssms
    if(ret=scan2(boards[i],&offset)){
      printf("Wrong sequence %i ret=%i \n",i,ret);
      ret=0;
      goto RET;
    }else{
      printf("Sequence %i correct, offset= %i\n",i,offset);
      offsets[i]=offset;
      for(j=0;j<i;j++){
         if(offsets[j] < offset) offsets[j] = offsets[j]+Orbit;
      }
    }
 }
 for(i=0;i<n;i++){
    sms[boards[i]].offset=offsets[i];
    sms[boards[i]].syncflag=SYNCFLAG;    
 }
 SYNCFLAG++;
 for(i=0;i<n;i++)printf("%i ",offsets[i]);printf("\n");
 ret=1;
 RET:
 return ret; 
}
/*
    Finds first 1 and then checks if they follow Orbit.
*/
int scan2(int board, int *offset)
{
 int bit,flag=0,i=0;
 while( ((sms[board].sm[i]&OrbitMask) == 0) && (i<Mega))i++;      // change !!      // find first 1
 //printf("B of seq1 %i \n",i);
 *offset=i;
 if(i == Mega){
   printf("No Orbit found. \n");
   return -1;   
 }
 while(i<Mega){
      bit = (sms[board].sm[i]&OrbitMask) == OrbitMask; 	 
      if( (i- *offset) % Orbit ){
	flag = flag + bit;            // check 0s
      }else{
	flag = flag + 1 - bit;        // check 1s
      }
      //if(flag)printf("i=%i %i %i\n",i,flag,seq1[i]);
      i++;
 }
 //printf("%i \n",flag);
 return flag;
}
/*FGROUP DebugSync
Debugging sync by generatinf ssm in memory.
 */ 
int DebugSync()
{
 int i,boards[NSSMBOARDS];
 for(i=0;i<NSSMBOARDS;i++)boards[i]=0; 
 GenSeq(1,3600,1);
 GenSeq(2,3600,55);
 GenSeq(3,3600,33);
 boards[0]=1;boards[1]=2;boards[2]=3;
 syncSSM(3,boards); 
}
/*
   Generates sequence of length Length and period Period.
   E.g. it enables to generate array which correspond to orbit channel
*/
int GenSeq(int board,int Period,int Start)
{
 int i;
 if(sms[board].sm == NULL)sms[board].sm = (w32 *)malloc(Mega*sizeof(w32));
 for(i=0;i<Start;i++) sms[board].sm[i]=0;
 for(i=Start;i<Mega;i++){
    sms[board].sm[i]=0;
    if( ((i-Start) % Period) == 0){
      sms[board].sm[i+0]=1;
    }
    //printf("%1i",seq[i]);
 }
 //printf("\n");
 sms[board].syncflag=0;
 return 1;
}
/*
 Write int n to ssm structure with period Period from Start. 
 Not to hardware !
*/
int writeSPn(int board,int Start, int Period,w32 n)
{
 int i;	
 if(sms[board].sm == NULL)sms[board].sm = (w32 *)malloc(Mega*sizeof(w32)); 
 for(i=Start;i<Mega;i++)if(((i-Start) % Period) == 0)sms[board].sm[i]=n;
 sms[board].syncflag=0;
 printf("Board %i %s : Start:%i Period:%i n=%i\n",board,sms[board].name,Start,Period,n);
}
/*FGROUP TESTFO->LTU
*/
int FOLTU(){
 A2BCD(3,"fo1_o1","ltu1_i1","ltu2_i2"); 
}	
int A2BCD(int n,...){	
 int i,board,boards[NSSMBOARDS];
 char *bomode; 
 Mode bomodes[NSSMBOARDS];
 va_list argp;

 va_start(argp,n);
 // read board numbers
 for(i=0; i<n; i++){                    
    bomode = va_arg(argp,char *);
    bomodes[i].ModeNameSize=strlen(bomode)+1;
    bomodes[i].ModeName=(char *)malloc((strlen(bomode)+1)*sizeof(char));
    strcpy(bomodes[i].ModeName,bomode);
    //printf("%s \n",bomodes[i]);
 }
 va_end(argp);
 if(setsms(n,bomodes,boards)) return 1;  // Prepare sms[] 
 printf("BOARDS: ");for(i=0; i<n; i++)printf(" %s",sms[boards[i]].name);printf("\n");
 printf("MODES: ");for(i=0; i<n; i++)printf(" %s",bomodes[i].ModeName);printf("\n");
 
 writeSPn(boards[0],0,1,0);           // Write all 0 to generating board board[0]
 writeSPn(boards[0],0,200,1);         // Write 1 with priod 200 to board[0]
 writeSSM(boards[0]);                 // Write sms[board[0]].sm to hardware
 
 for(i=1;i<n;i++)writeSPn(boards[i],0,1,0);   //Write all 0 to receiving boards
 for(i=1;i<n;i++)writeSSM(boards[i]);          // Write ssm[board[i]].sm to hardware
 if(StartBoards(n,bomodes)){ 
   if(GetConnections(n,bomodes))return 4; // -> backplane connection for generating board
   for(i=1;i<n;i++)GenSeq(boards[i],3600,40*i);      // readSSM
   syncSSM(n-1,boards+1);
   // synchronise signals
   // compare signals 
 }else{
   printf("SSMs have not started inside one ORBIT. Sync not possible \n");
  return 2;  
 }
 // free everything
 return 0;
}
/*
  Prepare all hardware and start boards.
  Check if time< 80.
*/
int StartBoards(int n,Mode *bomodes){
 struct timeval tval0,tval;
 struct timezone tz;
 double time;
 int i;
 gettimeofday(&tval0,&tz);         
 for(i=1;i<n;i++){
    // StartRecSSM(bomodes[i].Modename);       //Start SSMs
 }
 //  StartGenSSM(boards[0],mode[0]);         //Start generation
 gettimeofday(&tval,&tz);             
 time= (tval.tv_sec-tval0.tv_sec)*1000000.
      +(tval.tv_usec-tval0.tv_usec);
 printf("Time=%f \n",time);
 if(time < 80.) return 1;
 return 0;
}
/*
   From bomodes[] (e.g. "fo1_omode1")assign:
           board numbers in *boards ("fo1" -> 5) 
           mode file names in *bomodes ("fo1_omode1"->"fo_omode1")    
*/
int setsms(int n, Mode *bomodes,int *boards){
 int i,j;
 char *name0;
 char *name,*bomode;
 for(i=0;i<n;i++){     
   name=(char *) malloc(bomodes[i].ModeNameSize*sizeof(char));
   name0=name;
   bomode=bomodes[i].ModeName;
   while(((*name = *bomode++) != '_') && (*name))*name++;
   if(!(*name)){
    printf("Wrong name syntax: no _ ! Exiting. \n");
    return 1;
   }
   *name='\0';   
   //printf("name= %s",name0);
   j=0;
   while(strcmp(sms[j].name,name0) && j<NSSMBOARDS)j++;
   if(j == NSSMBOARDS){
    printf("Board name %s not in list !\n",name0);
    return 2;    
   }
   //printf(" %i ",j);
   boards[i]=j;
   *(--name)='_';
   *name++;   
   while(*name++ = *bomode++);             // create mode file name 
   //printf("%s \n",name0);
   strcpy(bomodes[i].ModeName,name0);
   free(name);
 }
 return 0;
}
/*
 Finds connections
*/
int GetConnections(int n, Mode *bomodes){
 int i,gsigs[32],rsigs[n-1][32];
 if(FileRead(&bomodes[0],gsigs))return 1;
 for(i=1;i<n;i++)if(FileRead(&bomodes[i],&rsigs[i-1])) return 1;
 //for(i=0;i<32;i++)printf("%i ",rsigs[1][i]);printf("\n");
 
 return 0;	 
}
/*   Reads mode files and fills signals */
int FileRead(Mode *bomodes,int *signal){
 char *filename; 
 FILE *ff;
 int i,j,ret;
 char signame[NAMESIZE];
 filename = (char *)malloc((bomodes[0].ModeNameSize+30)*sizeof(char));
 sprintf(filename,"CFG/ctp/ssmsigs/%s.sig",bomodes[0].ModeName);
 if(!(ff=fopen(filename,"r"))){
  printf("Cannot open mode file %s \n",filename);
  ret=1; goto RET;  
 }
 printf("File %s successfully opened. \n",filename);
 for(i=0;i<32;i++)signal[i]=-1;
 while( fscanf(ff,"%i %s ",&i,signame) != EOF ){
  printf("%i %s ",i,signame);
  if(i>31){
    printf("Input channel larger than 31: %i \n",i);
    ret=3;goto RET;
  }
  j=0;
  while((j<nofnames) && (ret=strcmp(ConnectionNames[j],signame))){
	  //printf("%s\n",signame);
	  //printf("%s\n",ConnectionNames[j]);
	  j++;
  }
  if(j == nofnames){
    printf("Cannot find Connection Name %s \n",signame);
    ret=2; goto RET;
  }
  printf("connection # %i \n",j); 
  signal[i]=j; 
 }
 ret=0; 
 RET: 
 free(filename);
 fclose(ff); 
 return ret;
  
}

