/* SSM routines used in v/vme/ctp package (i.e. all the
ctp boards accessed through 1 vme space */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include "vmewrap.h"
#include "ctp.h"
#include "ssmctp.h"

/* Signal is basic structure.
 * Signals for each board are saved as list:
 * sig1->sig2->sig3->null
 * looping over signals:
 *  sig=BoardSignals[i]->first;
 *  while(sig)sig->next;
*/ 
typedef struct Signal{
 struct Signal *next;	
 //struct Signal *prev;
 struct Signal *first;
 int channel;            //channel
 int signamenum;         // numerical name according ConnectionNames
 char signame[NAMESIZE]; // char name
}Signal;

Signal *BoardSignals[NSSMBOARDS]; //list of signals for every board
Signal *addSignal(Signal *last,int channel,int namenum, char *namechar);
char *ConnectionNames[MAXNAMES]; /* used to store names of backplane 
				and Front Pannel connections. 
				The order number in array is used as
		                numerical name of the connection. */
int nofnames;
int A2BCD(int ,...);
int initA2B(char bomodes[][FILENAMESIZE],char modes[][NAMESIZE],
		int *boards,w32 *modecodes);
//Hardware
int readSSM(int); 
int writeSSM(int);
int setSSM(int board,char *mode,w32 omiocs);    // rc=0 -> OK
int WriteBoards(int n,int *boards);
int StartBoards(int n,char bomodes[][FILENAMESIZE],
		int *boards,w32 *modecodes);
int ReadBoards(int n, int *boards);

// Signal manipulation
int GetSignals(int n, char bomodes[][FILENAMESIZE],int *boards,w32 *modecodes);
int startSSM(int n, int *boards);    // Start from 0->n ; rc=0->OK
int syncSSM(int n, int *boards);
int syncSIG2(int board,int channel,char *p);
int compSIG(int board1,int chan1,int board2,int chan2,int offset2);
int compSIG2(int *board1,int chan1,int board2,int chan2,int offset2);

int ParseNames(int n,char bomodes[][FILENAMESIZE],
		char modes[][NAMESIZE],int *boards);
int FileRead(int board,char *filename,w32 *modecode);
int FindOrbitChannel(int n,int *boards);
int PrintConnections(int n, int *boards, int mode);
int scan(int board, int *offset);
int nonemptyline(char *line);
int parsemode(char *s,w32 *modecode);
int setoffsetsBR(int n,int *boards);
//  logic Modes
int FOconnect(int n, int *boards);
int FOL0L1(int n, int *boards);
void warnmess(char *mode,int board,char *signame);
void getCluster(int board,int clust[][7]);
// Pattern generation
int GenSeq(int board,int Period,int Start);
int writeSPn(int board,int Period,int Start,w32 n);
int writeSPb(int board,int Period,int Start,int Channel);
int writeSPP(int board,int Start,int Channel,char *Pattern);
int char2i(char a);
char i2char(int i);
char *getPatfromF(w32 *file,int channel,int length);

// Codes---------------------------------------------------------initSSM()
void initSSM() {
int ix,inames=0;
int nofbckpl;
FILE *con;
char line[MAXLINE];
char *fcon="CFG/ctp/ssmsigs/backplanefp.names";

// Memory,offset,syncronisation flag
for(ix=0; ix<NSSMBOARDS; ix++) {
  sms[ix].sm= NULL;                
  sms[ix].mode[0]='\0';        
  sms[ix].syncmode=0; 
  sms[ix].offset= 0;
  sms[ix].syncflag= 0;
  //for(jx=0;jx<32;jx++)BoardChannels[ix][jx].start=NULL;  
};

// Assign board names
for(ix=0; ix<NCTPBOARDS; ix++) {
 strcpy(sms[ix].name,ctpboards[ix].name);	
}
strcpy(sms[NCTPBOARDS].name,"ltu1");	
strcpy(sms[NCTPBOARDS+1].name,"ltu2");
strcpy(sms[NCTPBOARDS+2].name,"ltu3");
strcpy(sms[NCTPBOARDS+3].name,"ltu4");
printf("initSSM: The number of boards type is %i : \n",NSSMBOARDS);
for(ix=0; ix<NSSMBOARDS; ix++) printf(" %s",sms[ix].name);printf("\n");

//Read connection names from file
if((con=fopen(fcon,"r")) == NULL){
  printf("initSSM error: Cannot read initialisation file: backplanefp.names. Exiting. \n");
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
printf("initSSM: # signal names=%i  # of backplane names=%i \n",nofnames,nofbckpl);
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

/*------------------------------------------------------ syncSSM()
Synchronise SSMs (pointers to their contents are in sms[i].sm)
Input:
n - number of SMS to be synced (minimal 1)
... -numbers of SSMs to be synced.
ORDER is IMPORTANT !
First SSM in list -> first SSM started -> largest offset.
Assumption: all SSM start during one orbit.
(Solution without this assumption may exists if SSM multiple of Orbit).
*/
int syncSSM(int n, int *boards) {
 int i,j,ret,offset,offsets[NSSMBOARDS];
 
 for(i=1; i<n; i++){                       // loop over ssms
    if((ret=scan(boards[i],&offset))){
      printf("syncSSM error: Wrong sequence board=%i ret=%i \n",boards[i],ret);
      ret=1;
      goto RET;
    }else{
      printf("syncSSM: Board %i sequence correct, offset= %i\n",boards[i],offset);
      offsets[i]=offset;
      for(j=0;j<i;j++){
         if(offsets[j] < offset) offsets[j] = offsets[j]+Orbit;
      }
    }
 }
 sms[boards[0]].offset=0;
 sms[boards[0]].syncflag=SYNCFLAG;
 for(i=1;i<n;i++){
    sms[boards[i]].offset=offsets[i];
    sms[boards[i]].syncflag=SYNCFLAG;    
 }
 SYNCFLAG++;
 //for(i=0;i<n;i++)printf("%i ",offsets[i]);printf("\n");
 ret=0;
 RET:
 return ret; 
}
/*
    Finds first 1 and then checks if they follow Orbit.
    If fails on the first orbit, try next one (only next one)
    since first orbit can be inetrupted.
*/
int scan(int board, int *offset)
{
 int bit,i=0,i0,flag=0;
 int OrbitMask;
 OrbitMask=sms[board].orbit;
CONT:
 while( ((sms[board].sm[i]&OrbitMask) == 0) && (i<Mega))i++;      // change !!      // find first 1
 //printf("B of seq1 %i \n",i);
 *offset=i;
 if(i == Mega){
   printf("scan error: Board: %i No Orbit found. \n",board);
   return 1;   
 }
 while(i<Mega){
      i0=i;
      while((i-i0)<40 && i<Mega){
        bit = (sms[board].sm[i]&OrbitMask) == OrbitMask; 
        if(!bit){
	 if(flag) goto ERR1;
	 // one more chance since first orbit can be broken
	 flag=1;
         goto CONT;	 
	}
        i++;	
      }      
      while((i-i0)<Orbit && i<Mega){
        bit = (sms[board].sm[i]&OrbitMask) == OrbitMask;
        if(bit) goto ERR2;
        i++;	
      }
 }
 //printf("%i \n",flag);
 return 0;
ERR1:
 printf("scan: 0 found in first 40 bits of ORBIT.\n");
 return 1;
ERR2:
 printf("scan: 1 found outside ORBIT.\n");
 return 2;
}
/*--------------------------------------------------------syncSIG2()  
 *  Synchronize the beginning of test pattern
 *  input: board,channel,pattern
 *   *  -checking the pattern
 * algoritm: valec
 *       - try to check length=strlen(pat)*4 bit of channel from offset+i
 *         i goes from 0 to length  
*/
int syncSIG2(int board,int channel, char *pat){
 int j,k,j0,len,pp;
 int length; 
 int *pattern;
 int offset; 
 w32 ChanMask;
 len=strlen(pat);
 length=4*len; 
 pattern= (int *) malloc(length*sizeof(int));
 for(j=0;j<len;j++){
    pp=char2i(pat[j]);	 
    for(k=0;k<4;k++){
       pattern[4*j+k]=((1<<k) & pp) == (1<<k);    
    }
 } 
 offset=sms[board].offset;
 ChanMask=1<<channel;
 j0=offset;
 while((j0-offset)<length){
   j=j0;   
   while( (j<Mega)  &&  ((j-j0)<length) && 
       (pattern[j-j0] == ((sms[board].sm[j]&ChanMask)==ChanMask)))j++;
   if((j-j0) == length) break;
   j0++; 
 }
 if ((j-j0) == length){
    printf("syncSIG2: pattern found for board,channel %i %i offset=%i\n",board,channel,j0-offset);          
    free(pattern);
    return j0-offset;
 }else{
   printf("syncSIG2 error: pattern for board,channel %i %i cannot be found\n",board,channel);
   free(pattern);
   return -1;
 }
}
/*----------------------------------------------------------compSIG() 
 * Compare signals of board1 and board2 
 * input:
 *       board1 - number of generating board
 *       chan1  - channel of generating board 
 *       board2 - number of receiving board
 *       chan3 - channel of receiving board
 *       offset2 - ofset of receiving board 
*/
int compSIG(int board1,int chan1,int board2,int chan2,int offset2){
 int j,ret=0;
 int offset=0; 
 w32 ChanMaskG,ChanMaskR;
 offset=sms[board2].offset;
 offset=offset+offset2;   
 ChanMaskG=1<<chan1;   	
 ChanMaskR=1<<chan2;
 j=offset; 
 while((j<Mega) &&
      ((sms[board1].sm[j-offset]&ChanMaskG) == ChanMaskG) ==
      ((sms[board2].sm[j]&ChanMaskR) == ChanMaskR) )j++;     
    if((j != Mega) || (offset == Mega)){
	printf("compSIG: ERROR on BOARD %s CHANNEL %i %i %i\n",
			sms[board2].name,chan2,j,offset);
        ret=1;	
       }
 if(!ret) printf("compSIG: CONNECTION CHECKED ! board,channel %i %i OK !\n",board2,chan2);
 return ret;
}
/*----------------------------------------------------------compSIG2() 
 * Compare signals of file *sm (channel=0) to board2 
 * input:
 *       *sm - signal file
 *       chan1 - not used
 *       board2 - number of receiving board
 *       chan2 - channel of receiving board
 *       offset2 - offset of receiving board
 *        
*/
int compSIG2(int *sm,int chan1,int board2,int chan2,int offset2){
 int j,ret=0;
 int offset=0; 
 w32 ChanMaskG,ChanMaskR;
 offset=sms[board2].offset;
 offset=offset+offset2;   
 ChanMaskG=1<<chan1;   	
 ChanMaskR=1<<chan2;
 j=offset; 
 while((j<Mega) &&
      ((sm[j-offset]&ChanMaskG) == ChanMaskG) ==
      ((sms[board2].sm[j]&ChanMaskR) == ChanMaskR) )j++;     
    if((j != Mega) || (offset == Mega)){
	printf("compSIG2: ERROR on BOARD %s CHANNEL %i %i %i\n",
			sms[board2].name,chan2,j,offset);
        ret=1;	
       }
 if(!ret) printf("compSIG2: CONNECTION CHECKED ! board,channel %i %i OK !\n",board2,chan2);
 return ret;
}

/*FGROUP DebCon
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
 return 1;
}
/*FGROUP DebCon
 * ---------------------------------------------------------------GenSeq()
   Generates sequence of length Length and period Period.
   E.g. it enables to generate array which correspond to orbit channel
*/
int GenSeq(int board,int Period,int Start)
{
 int i;
 if(sms[board].sm == NULL)sms[board].sm = (w32 *)malloc(Mega*sizeof(w32));
 if(!sms[board].sm){
  printf("GenSeq error: not enough memory.");
  return 1;  
 }
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
 return 0;
}
/*FGROUP DebCon
 * --------------------------------------------------------------WriteSPn()
 Write int n to ssm structure with period Period from Start. 
 Not to hardware !
*/
int writeSPn(int board,int Start, int Period,w32 n)
{
 int i;	
 if(sms[board].sm == NULL)sms[board].sm = (w32 *)malloc(Mega*sizeof(w32));
 if(!sms[board].sm){
  printf("writeSPn error: not enough memory.");
  return 1;  
 }
 for(i=Start;i<Mega;i++)if(((i-Start) % Period) == 0)sms[board].sm[i]=n;
 sms[board].syncflag=0;
 printf("writeSPn: Board %i %s : Start:%i Period:%i n=%i\n",
		 board,sms[board].name,Start,Period,n);
 return 0;
}
/*FGROUP DebCon
 * --------------------------------------------------------------WriteSPb()
 Write 1 to bit channel with Period from Start.
 Not overwriting other bits.
 */
int writeSPb(int board,int Start,int Period,int Channel){
 int i;
 w32 mask;
 if(sms[board].sm == NULL)sms[board].sm = (w32 *)malloc(Mega*sizeof(w32)); 
 if(!sms[board].sm){
  printf("writeSPb error: not enough memory.");
  return 1;  
 }
 mask=1<<Channel;
 for(i=Start;i<Mega;i++)if(((i-Start) % Period) == 0){
      sms[board].sm[i]=sms[board].sm[i] | mask;
 }
 sms[board].syncflag=0;
 printf("writeSPb: Board %i %s : Start:%i Period:%i Channel=%i\n",board,sms[board].name,Start,Period,Channel);
 return 0;
}
/*FGROUP DebCon
-----------------------------------------------------------------WriteSPP()
Write bit pattern Pattern to channel n from Start. 
Pattern is string of (0-9),(a-f) which is interpreted as hexadecimal number.
Least significant bits on the left.
*/
int writeSPP(int board,int Start,int Channel,char *Pattern){
 int i,j,i0,length;
 w32 bit,mask0,mask1;
 int pat;
 if(Pattern)length=strlen(Pattern);
 else return 3;
 //printf("writeSPP: strlen= %i \n",length);
 if(!length) return 1;     
 if(sms[board].sm == NULL)sms[board].sm = (w32 *)malloc(Mega*sizeof(w32));
 if(!sms[board].sm){
  printf("writeSPP error: not enough memory.");
  return 2;  
 }
 mask1=1<<Channel;
 mask0= ~(0xffffffff & mask1);
 i=Start;
 j=0;         // char count
 while(i<Mega){
   pat = char2i(Pattern[j]);
   if(pat<0) return 4;
   i0=i;
   while(((i-i0)<4) && i<Mega){  
     bit=(1<<(i-i0)) & pat;	   
     if(bit)sms[board].sm[i]=sms[board].sm[i] | mask1;
     else sms[board].sm[i]=sms[board].sm[i] & mask0;
     i++;
   }
   j= (j+1) % length;
 }
 return 0;
}
int char2i(char a){
 if(a >= 0x30 && a <= 0x39) return (a-0x30);
 else if (a >= 0x61 && a <= 0x66) return (a-0x57);
 else {
       printf("char2i: wrong pattern character %c \n",a);
       return -1;       
 } 
}
char i2char(int i){
 if(i>=0 && i<=9) return (i+0x30);
 else if(i>=10 && i<=15) return (i+0x57);
 else{
      printf("i2char: wrong number %i \n",i);
      return -1;
 } 
}
/*--------------------------------------------------------------getPatfromF()
 * - reads first 4*n numbers from channel of *file
 * - translate them to string with i2char
 * input *file,channel,n
 * output : pointer to created string 
*/  
char *getPatfromF(w32 *file,int channel,int n){
 int i,j,num;
 w32 mask,bit;
 char *pat;
 if(4*n>Mega){
  printf("getPatfromF error: 4*n=%i > Mega ! \n",4*n);
  return NULL;
 }
 mask=1<<channel;
 pat = (char *) malloc((n+1)*sizeof(char));
 if(!pat){
  printf("getPatfromF error: not enough memory \n");
  return pat;  
 }
 for(i=0;i<n;i++){
  num=0;	 
  for(j=0;j<4;j++){
   bit= (file[4*i+j] & mask) == mask;
   num= num + (bit<<j);   
  }
  pat[i]=i2char(num);  
 }
 pat[n]='\0';
 printf("getPatfromF: pattern %s %u \n",pat,*pat);
 return pat;
}

/*FGROUP DebCon
 * ---------------------------------------------------------------FO2LTU()
 * Debbuging A2BCD
 * FO2LTU() -- ltu1 = top connector -> first in parameters !!!
*/
int FO2LTU(){	
 //return A2BCD(3,"fo4_outgen","ltu1_i1","ltu2_i1"); 
 //return A2BCD(2,"fo4_outgen","ltu2_i1"); 
 return A2BCD(2,"fo4_igl0l1","ltu1_ipp"); 
 //return A2BCD(3,"fo4_igl0l1","ltu1_ipp","ltu2_ipp"); 
}	
/*
 * ---------------------------------------------------------------A2BCD()
 * testing connection between boards
 * input:
 *        n - number of boards to be tested, 
 *        1st boards assumed to be generating
 *        2-n boards to be receiving
*/
int A2BCD(int n,...){	
 int i;
 int boards[NSSMBOARDS];
 w32 modecodes[NSSMBOARDS];
 char *bomode; 
 char bomodes[NSSMBOARDS][FILENAMESIZE];
 char modes[NSSMBOARDS][NAMESIZE];
 va_list argp;

 initA2B(bomodes,modes,boards,modecodes); 
 va_start(argp,n);
 // read board numbers
 for(i=0; i<n; i++){                    
    bomode = va_arg(argp,char *);
    strcpy(bomodes[i],bomode);
    //printf("%s \n",bomodes[i]);
 }
 va_end(argp);
 if(ParseNames(n,bomodes,modes,boards)) return 1;        
 if(GetSignals(n,bomodes,boards,modecodes))return 3;
 PrintConnections(n,boards,0);
 FindOrbitChannel(n,boards);
 if(WriteBoards(n,boards)) return 2;
 if(StartBoards(n,bomodes,boards,modecodes))return 4; 
 if(ReadBoards(n,boards)) return 5;   
 if(syncSSM(n,boards)) return 6;
 switch(modecodes[0]){
       case 0x1004:  //Connections
	       FOconnect(n,boards);break;
       case 0x2014:  //L0L1 genertor mode
	       FOL0L1(n,boards);break;
       default:
         printf("A2B: modecode %i not found.\n",modecodes[0]);
         return 7;	 
 }  
 //setoffsetsBR(n,boards); 
 //freeSignals();
 return 0;  
}
/*-------------------------------------------------------------FOconnect()
 *
 * Now generaly si=fun(sg,params) where params are settings of board
*/
int FOconnect(int n,int *boards){
 int i,nsi,offset;
 Signal *si,*sg;
 char *pat; 
 // loop over boards
 for(i=1;i<n;i++){                      
  si=BoardSignals[i]->first;
  //loop over signals of receiving board
  while(si){
    nsi=si->signamenum;	  
    sg=BoardSignals[0]->first;
    // Find if si signal exist in generating board
    while(sg && (nsi != (sg->signamenum)))sg=sg->next;
    if(sg){	     
     pat=getPatfromF(sms[boards[0]].sm,sg->channel,5);
     if(pat)offset=syncSIG2(boards[i],si->channel,pat);
     free(pat);
     if(offset<0)goto ERR;
     compSIG(boards[0],sg->channel,boards[i],si->channel,offset);     
    }else {
      warnmess("FOconnect",boards[i],si->signame);  
    }
      si=si->next;	  
  }  
 }	 
 return 0;
ERR:
 return 1;
}

int FOL0L1(int n,int *boards){
 int i,offset;
 Signal *si,*sg; 
 char *pat;
 w32 sm[Mega];
 int clust[4][7];   // These are params !
 getCluster(boards[0],clust);
 for(i=1;i<n;i++){
  si=BoardSignals[i]->first;
  while(si){
    switch(si->signamenum){             
      case 37 ://FPPP---------------------------------------------- 
	{
	// tu treba dorobit nastavenia dosky	
	sg=BoardSignals[0]->first;
        while(sg && (si->signamenum != (sg->signamenum)))sg=sg->next;
        if(sg){
  	  pat=getPatfromF(sms[boards[0]].sm,sg->channel,5);
	  if(pat)offset=syncSIG2(boards[i],si->channel,pat);
	  free(pat);
          if(offset<0)goto ERR;
          compSIG(boards[0],sg->channel,boards[i],si->channel,offset);
	}else{
	  warnmess("FOL0L1",boards[i],si->signame);  
	  }
      break;
	}
      case 38 ://FPL0----------------------------------------------
        {
	int j,inclust;
        for(j=0;j<Mega;j++)sm[j]=0;
        sg=BoardSignals[0]->first;
	while(sg){
	// Find which clusters are in this connector
	// "CluL01" = 24 
         j=0;
	 while(j<7 && (sg->signamenum != (24+j)))j++;
	 inclust=1;
	 if( j==7 ) inclust=0; else if(!clust[i-1][j])inclust=0;
	 // OR of clusters
	 if(inclust){
           w32 mask;
	   mask=1<<(sg->channel);
	   for(j=0;j<Mega;j++){
	     sm[j]=sm[j]+((sms[boards[0]].sm[j] & mask)==mask);
	   }
	 }
        sg=sg->next;
	}
        // negacia signalu	
	for(j=0;j<Mega;j++)if(sm[j])sm[j]=0;else sm[j]=1;
	pat=getPatfromF(sm,0,5);
	if(pat)offset=syncSIG2(boards[i],si->channel,pat);
	free(pat);
        compSIG2(sm,0,boards[i],si->channel,offset);	
      break;
	}
      case 39 ://FPL1-----------------------------------------------------
        {
	int j,inclust;
        for(j=0;j<Mega;j++)sm[j]=0;
        sg=BoardSignals[0]->first;
	while(sg){
	// Find which clusters are in this connector
	// "CluL11" = 17 
         j=0;
	 while(j<7 && (sg->signamenum != (17+j)))j++;
	 inclust=1;
	 if( j==7 ) inclust=0; else if(!clust[i-1][j])inclust=0;
	 // OR of clusters
	 if(inclust){
           w32 mask;
	   mask=1<<(sg->channel);
	   for(j=0;j<Mega;j++){
	     sm[j]=sm[j]+((sms[boards[0]].sm[j] & mask)==mask);
	   }
	 }
        sg=sg->next;
	}
        // negacia signalu	
	for(j=0;j<Mega;j++)if(sm[j])sm[j]=0;else sm[j]=1;
	pat=getPatfromF(sm,0,5);
	if(pat)offset=syncSIG2(boards[i],si->channel,pat);
	free(pat);
        compSIG2(sm,0,boards[i],si->channel,offset);	
      break;	      
        }     
      default:
        //printf("FOL0L1 error: %s not found.\n",si->signame); 
      break; 
    }
    si=si->next;
  }	
 } 	 
 return 0;
ERR:
 return 1;
}
void warnmess(char *mode,int board,char *signame){
 printf("%s warning: board %i signal %s not in FO sigs.\n",mode,board,signame);
}
/*----------------------------------------------------getCluster()
 * get FO settings and output them as table.
*/ 
void getCluster(int board,int clust[][7]){
 w32 word,mask;
 int i,j,k=0;
 word= vmer32(FO_CLUSTER+BSP*ctpboards[board].dial);
 printf("CLUSTER: %x \n",word);
 for(i=0;i<4;i++){
   for(j=1;j<7;j++){
    mask=1<<(j+k-1);
    clust[i][j]= (word & mask) == mask;    
   }	  
   k=k+8; 
 }
 word=vmer32(FO_TESTCLUSTER+BSP*ctpboards[board].dial);
 printf("TEST_CLUSTER: %x \n",word);
 for(i=0;i<4;i++){
  mask=1<<(16+i);
  clust[i][0] = (word & mask) == mask;  
 }
 for(i=0;i<4;i++){
   for(j=0;j<7;j++)printf(" %i ",clust[i][j]);
   printf("\n");
 }
 return;
}
/*------------------------------------------------------ -initA2B()
 Set everything to 0;
 */
int initA2B(char bomodes[][FILENAMESIZE],char modes[][NAMESIZE],
		int *boards,w32 *modecodes){
 	
 int i;
 for(i=0;i<NSSMBOARDS;i++){
   boards[i]=-1;
   modecodes[i]=0;
   strcpy(bomodes[i],"EMPTY");
   strcpy(modes[i],"EMPTY");
   BoardSignals[i]=NULL;
 }
 return 0;
}
/*-----------------------------------------------------------setoffsetsBR
   Set offsets for browser. Not necessary for A2BS
*/ 
int setoffsetsBR(int n,int *boards){
/*
*/
 return 0;
}
/*-------------------------------------------------------ReadBoards()
 * Read from hardware to sms
*/
int ReadBoards(int n,int *boards){
 int i;	
 for(i=1;i<n;i++)readSSM(boards[i]);               //read SSM
 /*
 writeSPb(boards[1],2*200,3600,0);    // debug
 writeSPb(boards[2],200,3600,0);
 writeSPb(boards[1],400+200,200,1);
 writeSPb(boards[1],400+200,200,2); 
 writeSPb(boards[1],400+200,200,3); 
 writeSPb(boards[1],400+200,200,4);
 writeSPb(boards[1],400+200,200,5); 
 writeSPb(boards[1],400+200,200,6); 
 */
 return 0;	
}
/*--------------------------------------------------------WriteBoards()
 * write to ssms test patterns
 * write sms to hardware 
 */
int WriteBoards(int n,int *boards){
 int i;
 writeSPn(boards[0],0,1,0);         // Write all 0 to generating board board[0]

 //writeSPP(boards[0],0,1,"f0a00");
 //writeSPP(boards[0],0,2,"f00a0");
 //writeSPP(boards[0],0,3,"f0c00");
 //writeSPP(boards[0],0,4,"f0d80");
 //writeSPP(boards[0],0,5,"f0e00");
 //writeSPP(boards[0],0,6,"f0f00");
 //writeSPP(boards[0],0,7,"f8100");
 //writeSPP(boards[0],0,8,"f8800");
 writeSPP(boards[0],0,9,"f8800");
 
 
 writeSSM(boards[0]);                 // Write sms[board[0]].sm to hardware
 
 for(i=1;i<n;i++)writeSPn(boards[i],0,1,0);   //Write all 0 to receiving boards
 for(i=1;i<n;i++)writeSSM(boards[i]);          // Write ssm[board[i]].sm to hardware
 return 0;
}
/*--------------------------------------------------------StartBoards()
  Prepare all hardware (set modes) and start boards.
  Check if time< 80.
  input: *boards - board numbers
          bomodes[][] - modes
  
*/
int StartBoards(int n,char bomodes[][FILENAMESIZE],
		int *boards,w32 *modecodes){
 int i;
 for(i=0;i<n;i++){
   if(setSSM(boards[i],&bomodes[i][0],modecodes[i])){    //set modes
     printf("StartBoards error: cannot set mode %s \n",bomodes[i]);
     return 1;     
   }
 }
 for(i=0;i<n;i++)printf("StartBoards: Board %i Mode: %s \n",boards[i],sms[boards[i]].mode);
 //startSSM(n,boards);         //Start generation
 if(startSSM(n,boards)){
  printf("StartBoards error: SSMs have not started inside one ORBIT. Sync not possible \n");
  return 1;
 }
 usleep(30000);
 return 0;
}
/*--------------------------------------------------------parsenames()
   From bomodes[] (e.g. "fo1_omode1")assign:
           board numbers in *boards ("fo1" -> 5) 
           mode file names in *bomodes ("fo1_omode1"->"fo_omode1")    
*/
int ParseNames(int n,char bomodes[][FILENAMESIZE],char modes[][NAMESIZE],int *boards){
 int i,j;
 char *name;
 char name0[FILENAMESIZE],mode0[NAMESIZE];
 char *bomode,*mode;
 for(i=0;i<n;i++){     
   name=name0;
   bomode=bomodes[i];
   //printf(" bomode %s \n",bomode);
   j=0;
   while(((*name = *bomode++) != '_') && (j<NAMESIZE) && *name){name++;j++;}
   if(!(*name) || (j==NAMESIZE)){
    printf("ParseNames error: Wrong name syntax: no _  or too long name %i for board! Exiting. \n",j);
    return 1;
   }
   *name='\0';   
   //printf("name= %s",name0);
   j=0;
   while(strcmp(sms[j].name,name0) && j<NSSMBOARDS)j++;
   if(j == NSSMBOARDS){
    printf("ParseNames error: Board name %s not in list !\n",name0);
    return 2;    
   }
   //printf(" %i ",j);
   boards[i]=j;
   *(--name)='_';
   name++;
   mode=mode0;
   *mode++=*bomode;   
   while((*name++ = *bomode++))*mode++=*bomode;      // create mode file name 
   *mode='\0';
   //printf("%s \n",mode0);
   strcpy(bomodes[i],name0);
   strcpy(modes[i],mode0);
 }
 printf("BOARDS: ");for(i=0; i<n; i++)printf(" %s %i,",sms[boards[i]].name,boards[i]);printf("\n");
 printf("FILEMODES: ");for(i=0; i<n; i++)printf(" %s",bomodes[i]);printf("\n");
 printf("MODES: ");for(i=0; i<n; i++)printf(" %s",modes[i]);printf("\n");  
 return 0;
}
/*-----------------------------------------------------------FindOrbitChannel()
 * Find orbit channels for all receiving boards
 * necessary for syncSSM
*/
int FindOrbitChannel(int n,int *boards){
 int i,j;
 w32 mask;
 Signal *s;
 int orbit[]={34,36,43,50,57};
 printf("FindOrbitChannel ");
 for(i=1;i<n;i++){
  printf("board %i: ",boards[i]);	 
  s=BoardSignals[i]->first;
  while(s){
   for(j=0;j<5;j++)if(s->signamenum == orbit[j]){
     sms[boards[i]].orbit=s->channel;
     printf("%i",s->channel);
   }
   s=s->next;   
  } 
  printf("\n"); 
 }
 for(i=1;i<n;i++){
   //printf(" %s %i ",sms[boards[i]].name,sms[boards[i]].orbit);printf("\n");
   mask=1<<sms[boards[i]].orbit;
   sms[boards[i]].orbit=mask;
 }

 return 0; 
}
/*-----------------------------------------------------------GetSignals()
*/
int GetSignals(int n, char bomodes[][FILENAMESIZE],int *boards,w32 *modecodes){
 int i;
 // Read receiving mode files and assign signals to channels
 for(i=0;i<n;i++)if(FileRead(i,bomodes[i],&modecodes[i])) return 1;
 return 0;
}
/*-----------------------------------------------------------addSignal() 
 * Add signal to list*/
Signal *addSignal(Signal *last,int channel,int namenum, char *namechar){
 Signal *p;
 p = (Signal *) malloc(sizeof(Signal));
 if(!p) goto MEM;
 p->channel=channel;
 p->signamenum=namenum; 
 strcpy(p->signame,ConnectionNames[namenum]);
 p->next=NULL;
 if(last){
   last->next=p;	 
   //p->prev=last;
   p->first=last->first;
 }else{
   //p->prev= NULL;
   p->first=p;
 }
 return p; 
MEM:
 printf("addSignal error: not enough memory.\n");
 return p;
}
/* Print connections-------------------------------------------------------
 * mode : 0 from 1st to last
 *        1 from last to first 
 * */

int PrintConnections(int n, int *boards,int mode){
 int i;
 Signal *s;
 for(i=0;i<NSSMBOARDS;i++){
   if(BoardSignals[i]){	 
    printf("CONNECTIONS BOARD %i %i 1->last: \n",i,boards[i]);
    s=BoardSignals[i]->first;
    while(s){
     printf("[%i %s %i] ",s->channel,s->signame,s->signamenum);
     s=s->next;
    }
   printf("\n");
   }
 }
 return 1; 
}
/*--------------------------------------------------------------FileRead()   
 *   Reads mode files and fills signals
 *   - no protection against wrong syntax in input file
 *   - chan=32 interpreted as modecode (omiocs)
 *   - if chan>31 warning issued, but can be used as comment 
 *   */
int FileRead(int board ,char *filename,w32 *modecode){	
 char dfilename[FILENAMESIZE+30]; 
 FILE *ff;
 int i,j,ret;
 char signame[NAMESIZE];
 printf("FileRead: filename= %s \n",filename);
 sprintf(dfilename,"CFG/ctp/ssmsigs/%s.sig",filename);
 if(!(ff=fopen(dfilename,"r"))){
  printf("FileRead error: Cannot open mode file %s \n",dfilename);
  ret=1; goto RET;  
 }
 printf("FileRead: File %s successfully opened. \n",dfilename);
 while(1) {
  char line[100], comment[100];
  if( fgets(line, 100, ff)==NULL) break;
  sscanf(line,"%i %s %s\n",&i,signame, comment);
  //printf("%s:%i %s ",filename,i,signame);	  
  if(i>32 || i<0){
    printf("FileRead warning: File:%s Input channel: %i %s\n",
       filename,i,signame);
    //ret=3;goto RET; 
  }else if(i == 32){   //mode number
   printf("FileRead: Mode number: %s \n",signame);
   parsemode(signame,modecode);
  }else{
   j=0;
   while((j<nofnames) && (ret=strcmp(ConnectionNames[j],signame))){
	  //printf("%s %i\n",signame,strlen(signame));
	  //printf("%s %i\n",ConnectionNames[j],strlen(ConnectionNames[j]));
	  j++;
   }
   if(j == nofnames){
    printf("FileRead error: Cannot find Connection Name %s \n",signame);
    ret=2; goto RET;
   }
   //printf("connection # %i \n",j); 
   //signal[i]=j;
    BoardSignals[board]=addSignal(BoardSignals[board],i,j,ConnectionNames[j]);
  } 
 }
 ret=0; 
 RET:
 printf("FileRead: finished \n"); 
 if(ff)fclose(ff); 
 return ret;
  
}
int parsemode(char *s,w32 *modecode){
 int i,ret=0;
 printf("parsemode: ");
 *modecode=0;
 for(i=0;i<8;i++){ 
   printf("%c",s[i]);
   switch(s[i]){
     case '0': break;
     case '1': if(i<3)*modecode=*modecode+(1<<i);
	       else if(i==3)*modecode=*modecode+(1<<(4));
	       else if(i<6)*modecode=*modecode+(1<<(8+i-4));
	       else *modecode=*modecode+(1<<(12+i-6));
	       break;
     default:printf("parsemode: unexpected chracter in mode %c \n",s[i]);
	     ret=1;break;
   }
 }
 printf("\n");
 printf("parsemode: finished %x\n",*modecode);
 return ret;
}

