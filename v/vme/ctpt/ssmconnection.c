#include "ssmconnection.h"
//----------------------------------------------------------------------
//               MAIN PROGRAM
//----------------------------------------------------------------------
/* ---------------------------------------------------------------A2BCD()
 * testing connection between boards
 * input:
 *        n - number of boards to be tested, 
 *        1st boards assumed to be generating
 *        2-n boards to be receiving
*/
int A2BCD(int n,...){	
 int i,ret=0;
 int boards[NSSMBOARDS];
 int submode;
 w32 modecodes[NSSMBOARDS];
 char *bomode; 
 char bomodes[NSSMBOARDS][FILENAMESIZE];
 char modes[NSSMBOARDS][NAMESIZE];
 va_list argp;
 initA2B(bomodes,modes,boards,modecodes);
 // Read parameters 
 va_start(argp,n);
 // read board numbers
 n=n-1;
 printf("Number of boards: %i \n",n);
 for(i=0; i<n; i++){                    
    bomode = va_arg(argp,char *);
    strcpy(bomodes[i],bomode);
    //printf("%s \n",bomodes[i]);
 }
 submode = va_arg(argp,int);
 printf("submode= %i \n",submode);
 va_end(argp);
 // End of read parameters
 if((ret=ParseNames(n,bomodes,modes,boards))) return ret;        
 if((ret=GetSignals(n,bomodes,boards,modecodes))) return ret;
 FindOrbitChannel(n,boards);
 HardWare.loop=1;
 if(n>1){
   //PrintConnections(n,boards,0);
   if((ret=WriteBoards(n,boards,modecodes[0],submode)))return ret;
 }
 if((ret=InitMode(n,boards,modecodes[0],submode))) return ret;
 while(HardWare.loop)
 {
  if((ret=SetBoardHW(n,boards,modecodes[0],submode)))return ret;   
  if((ret=StartBoards(n,bomodes,boards,modecodes,submode)))break; 
  if((ret=ReadBoards(n,boards))) break; 
  // sync is necessary only when > 1 receiving boards 
  if((ret=syncSSM(n,boards))) break;
  if((ret=CheckMode(n,boards,modecodes[0],submode)))continue;
  //PrintConnections(n,boards,0);
  //setoffsetsBR(n,boards);
 }
 freeSignals(n,boards); 
 return ret;  
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
   sms[i].signal=NULL;
   sms[i].orbit=33;  // code for no orbit
   //sms[i].orbit=0;
   sms[i].offset=0;
 }
 //setseeds(3,5);
 setseeds(7,5);
 //setseeds(13,7);
 setlinksl0();
 setlinksl1();
 initNamesCon();
 return 0;
}
//-----------------------------------------------------------------
//                INPUT/OUTPUT
//-----------------------------------------------------------------
//---------------------------------------------------------initNamesCon()
/*
  Reads backplane and front panel names from file 
  "CFG/ctp/ssmsigs/backplanefp.names";
  and store them in ConnectionNames[].
  Only these names can be used in *.sig files.
*/
void initNamesCon() {
int inames=0;
int nofbckpl;
FILE *con;
char line[MAXLINE];
char *fcon="CFG/ctp/ssmsigs/backplanefp.names";
//Read connection names from file
if((con=fopen(fcon,"r")) == NULL){
  printf("initNamesCon error: Cannot read file: backplanefp.names. Exiting. \n");
  exit(1);
}
while((fgets(line,MAXLINE,con)!=NULL) && nonemptyline(line)){
 ConnectionNames[inames]=(char *)malloc(NAMESIZE*sizeof(char));
 removespace(line); 
 strcpy(ConnectionNames[inames],line);
 //printf("%s \n",ConnectionNames[inames]);
 inames++;
} 
nofnames=inames;
inames=0;
while((inames<nofnames) && (ConnectionNames[inames][0] !='F'))inames++;
nofbckpl=inames;
printf("initNamesCon: # signal names=%i  # of backplane names=%i \n",nofnames,nofbckpl);
}
/*-----------------------------------------------------*/
/* Check for empty line - support routine for initNames()
*/
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
/*------------------------------------------------------------
 * Remove spaces
 * a little recursion
 * support routine for initNames(); 
 * */
int removespace(char *line){
 char *a;	
 //printf("rs: %s \n",line);
 a=line;
 while( (*a != ' ') && (*a != '\0'))a++;
 if( *a == '\0') return 1;
 //while( *a != '\0')*a=*(a++ +1);
 while( *a != '\0'){*a=*(a +1);a++;}
 removespace(line);
 printf("removespace error, exiting.");
 exit(1);
}
/*--------------------------------------------------------parsenames()
   input: bomodes[]
   output: bomodes[],modes[],boards[]
   From bomodes[] (e.g. "fo1_mode1")assign:
          - board numbers in *boards
               "busy -> 0"
               "l0 -> 1" ...
               "fo1" -> 5 ...
               "fo6" -> 10 
            The names should be l0,l1,l2,busy,fo1,..fo6,ltu1,ltu2
          - modes:
                "l0_mode2" -> "mode2"
                "fo1_mode1" -> "mode1"
          - mode file names in *bomodes
                "l0_mode2" -> "l0_mode2" 
                "fo1_mode1"->"fo_mode1"    
*/
int ParseNames(int n,char bomodes[][FILENAMESIZE],char modes[][NAMESIZE],int *boards){
 int i,j;
 char *name;
 char name0[FILENAMESIZE],mode0[NAMESIZE];
 char *bomode,*mode;
 for(i=0;i<n;i++){     
   name=name0;
   bomode=bomodes[i];
   printf(" bomode %s \n",bomode);
   j=0;
   while(((*name = *bomode++) != '_') && (j<NAMESIZE) && *name){name++;j++;}
   if(!(*name) || (j==NAMESIZE)){
    printf("ParseNames error: Wrong name syntax: no _  or too long name %i for board! Exiting. \n",j);
    return 1;
   }
   *name='\0';   
   //printf("name= %s \n",name0);
   j=0;
   while(strcmp(sms[j].name,name0) && j<NSSMBOARDS){
	   //printf("%i %s %s \n",j,sms[j].name,name0);
	   j++;
   }
   if(j == NSSMBOARDS){
    printf("ParseNames error: Board name %s not in list !\n",name0);
    return 2;    
   }
   //printf(" %i ",j);
   boards[i]=j;
   // Only ltu and fo boards are numbered (1..4) 
   if(j>4) *(--name)='_'; else *name='_';
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
/*--------------------------------------------------------------readMODE()
 *  input:
           board,filename
    output: modecode
            GLOBAL sms[board].signal
   
 *   Reads bo_mode.sig files and fills signals to sms[].signal
 *   - chan=32 interpreted as modecode (omiocs)
 *   - if chan>31 warning issued, but can be used as comment 
 *   - no protection against wrong syntax in input file
 *   */
int readMODE(int board ,char *filename,w32 *modecode){	
 char dfilename[FILENAMESIZE+30]; 
 FILE *ff;
 int i,j,ret;
 char signame[NAMESIZE];
 printf("readMODE: filename= %s \n",filename);
 sprintf(dfilename,"CFG/ctp/ssmsigs/%s.sig",filename);
 if(!(ff=fopen(dfilename,"r"))){
  printf("readMODE error: Cannot open mode file %s \n",dfilename);
  ret=1; goto RET;  
 }
 printf("readMODE: File %s successfully opened. \n",dfilename);
 while(1) {
  char line[100], comment[100];
  if( fgets(line, 100, ff)==NULL) break;
  sscanf(line,"%i %s %s\n",&i,signame, comment);
  removespace(signame);
  //printf("%s:%i %s ",filename,i,signame);	  
  if(i>32 || i<0){
    //printf("readMODE warning: File:%s Input channel: %i %s\n",filename,i,signame);
    //ret=3;goto RET; 
  }else if(i == 32){   //mode number
   printf("readMODE: Mode number: %s \n",signame);
   parsemode(signame,modecode);
  }else{
   j=0;
   while((j<nofnames) && (ret=strcmp(ConnectionNames[j],signame))){
	  //printf("%s %i\n",signame,strlen(signame));
	  //printf("%s %i\n",ConnectionNames[j],strlen(ConnectionNames[j]));
	  j++;
   }
   if(j == nofnames){
    printf("readMODE error: Cannot find Connection Name %s \n",signame);
    ret=2; goto RET;
   }
   //printf("connection # %i \n",j); 
   //signal[i]=j;
    sms[board].signal=addSignal(sms[board].signal,i,j,ConnectionNames[j]);
  } 
 }
 ret=0; 
 RET:
 printf("readMODE: finished \n"); 
 if(ff)fclose(ff); 
 return ret;
  
}
int parsemode(char *s,w32 *modecode){
 int i,ret=0;
 //printf("parsemode: %s\n",s);
 *modecode=0;
 for(i=0;i<6;i++){
   if(s[i] == '1') *modecode=*modecode + (1<<i); 	 
 } 
 if(s[6] == '1') *modecode=*modecode + (1<<8); 	 
 if(s[7] == '1') *modecode=*modecode + (1<<9); 	 
 //printf("parsemode: finished %x\n",*modecode);
 return ret;
}
/*-----------------------------------------------------------setoffsetsBR
   Set offsets for browser. Not necessary for A2BS
   No protection against different offsets for different signals.
   Not used.
*/ 
int setoffsetsBR(int n,int *boards){
 int i;	
 Signal *s;
 for(i=1;i<n;i++){
  s=sms[boards[i]].signal->first;
  while(s){
   if(s->offset>=0){	  
    sms[boards[i]].offset=sms[boards[i]].offset+s->offset;
    break;    
   }
  s=s->next;  
  }
  if(!s)printf("setoffsetsBR warning: board %s offset not changed.\n",sms[boards[i]].name);  
 }
 return 0;
}
//------------------------------------------------------------------
//               SIGNAL MANIPULATION
//------------------------------------------------------------------
/*FGROUP DebCon
 * Compares 2 channels in the snapshot memory
 */
int Comp2Channels(int board1,int chan,int start){
 return compSIG1(board1,chan,TEST,chan,0,start);	
}
/*FGROUP DebCon
 * Compares one by one all pf chanels (14-18)
 */
int CompallPF(){
 int i,j;
 printf("CompallPF: \n");
 for(i=14;i<19;i++){
  for(j=i+1;j<19;j++){
   printf("%i %i ",i,j);
   compSIG1(1,i,1,j,0,0);
  }
 }
 return 0; 
}
/*-----------------------------------------------------------FindOrbitChannel()
 * Find orbit channels for all receiving boards
 * necessary for syncSSM
 * If orbit channel is not find for given board, 
   the default value 33 assigned in initA2B is kept.
*/
int FindOrbitChannel(int n,int *boards){
 int i,i0=0;
 w32 mask;
 Signal *s;
 //int j,orbit[]={34,36};
 //if(n==1)i0=0;
 printf("FindOrbitChannel ");
 for(i=i0;i<n;i++){
  printf("board %i: ",boards[i]);	 
  if((sms[boards[i]].signal) == NULL)continue;
  s=sms[boards[i]].signal->first;
  while(s){
   //for(j=0;j<2;j++)if(s->signamenum == orbit[j]){  // looking through numerical name
   if(!strcmp(s->signame,"Orbit")){                  
     sms[boards[i]].orbit=s->channel;
     printf("%i",s->channel);
   }
   s=s->next;   
  } 
  printf("\n"); 
 }
 for(i=0;i<n;i++){
   printf(" %s %i ",sms[boards[i]].name,sms[boards[i]].orbit);printf("\n");
   if(sms[boards[i]].orbit != 33){
    mask=1<<sms[boards[i]].orbit;
    sms[boards[i]].orbit=mask;
   }
 }

 return 0; 
}
/*-----------------------------------------------------------GetSignals()
 * input: n,bomodes,boards
   output: modecodes
           sms[].signal (GLOBAL)
 * Read signals from bo_mode.sig file.
*/
int GetSignals(int n, char bomodes[][FILENAMESIZE],int *boards,w32 *modecodes){
 int i;
 // Read receiving mode files and assign signals to channels
 for(i=0;i<n;i++)if(readMODE(boards[i],bomodes[i],&modecodes[i])) return 1;
 //for(i=0;i<n;i++)sms[boards[i]].signal=BoardSignals[i];
 printf("Modecodes:");
 for(i=0;i<n;i++)printf("%x ",modecodes[i]);
 printf("\n");
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
syncSSM checks if orbit channel is found (!= 33).
If orbit channel is not found, syncssm does not synchronise that board.
Assuming first board is none.
*/
int syncSSM2(int n, int *boards) {
 int i,ret,offset;
 int sum,start=0;
 int offsets[NSSMBOARDS],offsetscor[NSSMBOARDS];
 printf("syncSSM: n=%i\n ",n);
 for(i=n-1; i>=0; i--){                       // loop over ssms
  offsets[i]=-1;
  //printf("i board, orbitchan %i %i %i\n", i,boards[i],sms[boards[i]].orbit);
  if(sms[boards[i]].orbit != 33){
    if((ret=scan2(boards[i],&offset,start))){
    //ret=scan(boards[i],&offset);
    //if(0){    do not care about errors
      printf("syncSSM error: WRONG ORBIT board=%i ret=%i \n",boards[i],ret);
      ret=1;
      goto RET;
    }else{
      printf("syncSSM: Board %i ORBIT correct, offset= %u\n",boards[i],offset);
      offsets[i]=offset;
      start=offset;
    }
   }
 }
 printf("Before \n");
 for(i=0;i<n;i++)printf("%i ",offsets[i]);printf("\n");
/*
 offsetscor[n-1]=0;
 sum=0;
 for(i=n-2;i>0;i--){
  if(offsets[i+1]>offsets[i]){
   offsetscor[i]=sum+ORBITLENGTH-offsets[i+1]+offsets[i];
   sum=offsetscor[i];
  }else{
   offsetscor[i]=sum+offsets[i]-offsets[i+1];
   sum=offsetscor[i];
  }
 }
*/
 for(i=n-1;i>0;i--)offsetscor[i]=offsets[i]-offsets[n-1];
 printf("After 1 \n");
 for(i=0;i<n;i++)printf("%i ",offsetscor[i]);printf("\n");
 sms[boards[1]].syncflag=SYNCFLAG;
 for(i=1;i<n-1;i++){
    if(offsets[i] != -1){
      sms[boards[i]].syncflag=SYNCFLAG;
      sms[boards[i]].offset=offsetscor[i];
    }    
 }
 SYNCFLAG++;
 printf("After \n");
 for(i=0;i<n;i++)printf("%i ",sms[boards[i]].offset);printf("\n");
 ret=0;
 RET:
 return ret; 
}

/*------------------------------------------------------ syncSSMold()
Synchronise wrt to biginning od SSM
Synchronise SSMs (pointers to their contents are in sms[i].sm)
Input:
n - number of SMS to be synced (minimal 1)
... -numbers of SSMs to be synced.
ORDER is IMPORTANT !
First SSM in list -> first SSM started -> largest offset.
Assumption: all SSM start during one orbit.
(Solution without this assumption may exists if SSM multiple of Orbit).
syncSSM checks if orbit channel is found (!= 33).
If orbit channel is not found, syncssm does not synchronise that board.
*/
int syncSSM(int n, int *boards) {
 int i,j,ret,offset,offsets[NSSMBOARDS];
 printf("syncSSM: n=%i\n ",n);
 for(i=0; i<n; i++){                       // loop over ssms
  offsets[i]=-1;
  //printf("i board, orbitchan %i %i %i\n", i,boards[i],sms[boards[i]].orbit);
  if(sms[boards[i]].orbit != 33){
    if((ret=scan(boards[i],&offset))){
    //ret=scan(boards[i],&offset);
    //if(0){    do not care about errors
      printf("syncSSM error: WRONG ORBIT board=%i ret=%i \n",boards[i],ret);
      ret=1;
      goto RET;
    }else{
      printf("syncSSM: Board %i ORBIT correct, offset= %u\n",boards[i],offset);
      offsets[i]=offset;
      for(j=0;j<i;j++){
         if(offsets[j] < offset) offsets[j] = offsets[j]+Orbit;
      }
    }
   }
 }
 sms[boards[0]].offset=0;
 sms[boards[0]].syncflag=SYNCFLAG;
 for(i=1;i<n;i++){
    if(offsets[i] != -1){
      sms[boards[i]].syncflag=SYNCFLAG;
      sms[boards[i]].offset=offsets[i];
    }    
 }
 SYNCFLAG++;
 //for(i=0;i<n;i++)printf("%i ",offsets[i]);printf("\n");
 ret=0;
 RET:
 return ret; 
}
/*FGROUP SimpleTests
*/
void scanOrbit(int board){
 int offset;
 int boards[1];
 boards[0]=board;
 FindOrbitChannel(1,boards);
 scan(board,&offset);
 printf("board=%i offset=%u \n",board,offset);
}
/*----------------------------------------------------------------scan()
    Finds first 1 and then checks if they follow Orbit.
    If fails on the first orbit, try next one (only next one)
    since first orbit can be inetrupted.
*/
int scan2(int board, int *offset,int start)
{
 int bit,i=0,i0,flag=0;
 //int start=3;  // This is necessary due to wrong ssm at beginning
 w32 OrbitMask;
 if(!sms[board].sm){
  printf("scan: Board %i sm = NULL\n",board);
  return 3;
 }
 //OrbitMask=sms[board].orbit;
 OrbitMask=1;
 printf("board=%i OrbitMask=%i \n",board,OrbitMask);
i=start;
CONT:
 //Find first 1
 while( ((i<Mega) && (sms[board].sm[i]&OrbitMask) == 0))i++;
 if(i == Mega){
   printf("scan error: Board: %i No Orbit found. \n",board);
   return 1;   
 }
 *offset=i;
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
 printf("scan: 0 found in first 40 bits of ORBIT %i.\n",i);
 return 1;
ERR2:
 printf("scan: 1 found outside ORBIT %i.\n",i);
 return 2;
}
int scan(int board, int *offset){
 return scan2(board,offset,0);
}
/*--------------------------------------------------------syncSIG1()
 *  Find first nonzero bit in channel from the beginning of ssm
 */
int syncSIG1(int *sm,int channel){
 int i=1; // This is not 0 due to sporious noice at word 0 
 w32 mask;
 mask=1<<channel;
 while(i<Mega && !(sm[i] & mask))i++;
 if(i == Mega){
   printf("syncSIG1: channel %i empty.\n",channel);	 
   return -1; 
 }else {
   printf("syncSIG1: channel %i offset= %i\n",channel,i); 
   return i;
 }
}
/*--------------------------------------------------------syncSIG1a()
 *  Find first nonzero bit in channel from start
 */
int syncSIG1a(int *sm,int channel,int start){
 int i=start; // This is not 0 due to sporious noice at word 0 
 w32 mask;
 mask=1<<channel;
 while(i<Mega && !(sm[i] & mask))i++;
 if(i == Mega){
   printf("syncSIG1a: channel %i empty.\n",channel);	 
   return Mega; 
 }else {
   printf("syncSIG1a: channel %i offset= %i\n",channel,i); 
   return i;
 }
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
 char *name=NULL;
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
 name=findChannel(board,channel);	 
 if ((j-j0) == length){
    printf("syncSIG2: pattern found for board,channel %i [%s %i] offset=%i\n",board,name,channel,j0-offset);          
    free(pattern);
    return j0-offset;
 }else{
   printf("syncSIG2 error: pattern for board,channel %i [%s %i] cannot be found\n",board,name,channel);
   free(pattern);
   return -1;
 }
}
/*----------------------------------------------------------CompSIGs
 * Note about compSIGs:
 * there are 5 compSIGs:
 * compSIG=compSIG2 but difference in parameters
 * compSIG1=compSIG3 but difference in parameters
 * compSIG4 - chi2 calculation , differs from others 
 * compSIG5 - compares more bits simultaneosly defined by mask
 */ 
/*----------------------------------------------------------compSIG() 
 * Compare signals of board1 and board2 
 * input:
 *       board1 - number of generating board
 *       chan1  - channel of generating board 
 *       board2 - number of receiving board
 *       chan3 - channel of receiving board
 *       offset2 - ofset of receiving board 
 *       difference wrt compsig2:
 *                board1 vs *sm;
*/
int compSIG(int board1,int chan1,int board2,int chan2,int offset2,int start){
 int j;
 int offset; 
 w32 ChanMaskG,ChanMaskR;
 offset=sms[board2].offset;
 offset=offset+offset2;   
 ChanMaskG=1<<chan1;   	
 ChanMaskR=1<<chan2;
 j=offset+start; 
 while((j<Mega) &&
      ((sms[board1].sm[j-offset]&ChanMaskG) == ChanMaskG) ==
      ((sms[board2].sm[j]&ChanMaskR) == ChanMaskR) )j++;     
 if((j != Mega) || (offset >= Mega)){
   compMessage("compSIG",board2,chan2,j);	 
   return 1;	
 }else{
   compMessage("compSIG",board2,chan2, -1);	    
   return 0;
 }
}
/*----------------------------------------------------------compSIG1() 
 * Compare signals of board1 and board2 
 * input:
 *       board1 - number of generating board
 *       chan1  - channel of generating board 
 *       board2 - number of receiving board
 *       chan3 - channel of receiving board
 *       offset2 - ofset of receiving board 
 *       difference wrt compsig:
 *                the board offset sms[].offset is not used
*/
int compSIG1(int board1,int chan1,int board2,int chan2,int offset2,int start){
 int j;
 w32 ChanMaskG,ChanMaskR;
 ChanMaskG=1<<chan1;   	
 ChanMaskR=1<<chan2;
 j=offset2+start; 
 while((j<Mega) &&
      ((sms[board1].sm[j-offset2]&ChanMaskG) == ChanMaskG) ==
      ((sms[board2].sm[j]&ChanMaskR) == ChanMaskR) )j++;     
 if((j != Mega) || (offset2 >= Mega)){
   compMessage("compSIG+1",board2,chan2,j);	 
   return 1;	
 }else{
   compMessage("compSIG",board2,chan2,-1);	    
   return 0;
 }
}

/*----------------------------------------------------------compSIG2() 
 * Compare signals of file *sm (channel=0) to board2 
 * input:
 *       *sm - signal file
 *       chan1 - not used
 *       board2 - number of receiving board
 *       chan2 - channel of receiving board
 *       offset2 - offset of receiving board
 *       Difference wrt compSIG1:
 *         first parameter *sm versus board number
 *       Difference wrt compSIG3: 
 *         starts to compare from sms[board2].offset+offset2
 *        
*/
int compSIG2(int *sm,int chan1,int board2,int chan2,int offset2,int start){
 int j;
 int offset; 
 w32 ChanMaskG,ChanMaskR;
 offset=sms[board2].offset;
 offset=offset+offset2;   
 ChanMaskG=1<<chan1;   	
 ChanMaskR=1<<chan2;
 j=offset+start; 
 while((j<Mega) &&
      ((sm[j-offset]&ChanMaskG) == ChanMaskG) ==
      ((sms[board2].sm[j]&ChanMaskR) == ChanMaskR) )j++;     
 if((j != Mega) || (offset >= Mega)){
   compMessage("compSIG2",board2,chan2,j);	 
   return 1;
 }else{
   compMessage("compSIG2",board2,chan2,-1);	 
  return 0;
 }	 
}
/*----------------------------------------------------------compSIG3() 
 * Compare signals of file *sm (channel=0) to board2 
 * input:
 *       *sm - signal file
 *       chan1 - not used
 *       board2 - number of receiving board
 *       chan2 - channel of receiving board
 *       offset2 - offset of receiving board
 *       Difference wrt compSIG2: starts to compare from offset2
 *        
*/
int compSIG3(int *sm,int chan1,int board2,int chan2,int offset,int start){
 int j; 
 w32 ChanMaskG,ChanMaskR;   
 ChanMaskG=1<<chan1;   	
 ChanMaskR=1<<chan2;
 j=offset+start; 
 while((j<Mega) &&
      ((sm[j-offset]&ChanMaskG) == ChanMaskG) ==
      ((sms[board2].sm[j]&ChanMaskR) == ChanMaskR) )j++;     
 if((j != Mega) || (offset == Mega)){
   compMessage("compSIG3",board2,chan2,j);	    
   return 1;	
 }else{
   compMessage("compSIG3",board2,chan2,-1);	 
   return 0;
 }
}
/*----------------------------------------------------------compSIG4() 
 * Compare signals of file *sm (channel=0) to board2 
 * input:
 *       *sm - signal file
 *       chan1 - not used
 *       board2 - number of receiving board
 *       chan2 - channel of receiving board
 *       offset2 - offset of receiving board
 *       Calculates chi2 difference
 *       
 *        
*/
int compSIG4(int *sm,int chan1,int board2,int chan2,int offset,int start,int flag){
 int j,dif,chi2=0; 
 w32 ChanMaskG,ChanMaskR; 
 FILE *ff;
 if(flag)ff=fopen("comsig4.log","w"); 
 ChanMaskG=1<<chan1;   	
 ChanMaskR=1<<chan2;
 j=offset+start; 
 while(j<Mega){
  dif=((sm[j-offset]&ChanMaskG) == ChanMaskG) -
      ((sms[board2].sm[j]&ChanMaskR) == ChanMaskR);
  if(dif && flag)fprintf(ff,"%i %i \n",j,dif);
  dif=dif*dif;
  chi2=chi2+dif; 
  j++;
 }
 compMessage("chi2",board2,chan2,chi2);
 if(flag)fclose(ff); 
 return chi2;
}
/*----------------------------------------------------------compSIG5() 
 * Compare signals of file *sm to board2
 * bits should be together 
 * input:
 *       *sm - signal file       
 *       board2 - number of board
 *       mask1 - mask of compare bits on *sm
 *       mask2 - mask of compare bits on board2
 *       offset - offset of receiving board
 *       
 *        
*/
int compSIG5(int *sm,int board2,w32 mask,int offset,int start){
 int j;
 w32 *a,*b;
 a=sm;
 b=sms[board2].sm;
 j=offset+start; 
 while((j<Mega) &&
      (a[j-offset]&mask) == (b[j]&mask) )j++;     
 if((j != Mega) || (offset == Mega)){
   compMessage("compSIG5",board2,-1,j);	    
   return 1;	
 }else{
   compMessage("compSIG5",board2,-1,-1);	 
   return 0;
 }
}

/*-----------------------------------------------------------compMessage()
 *  Print message from comparing programs
*/
void compMessage(char *name,int board,int channel,int j){
 if(channel >= 0){
   if(j != -1){
     printf("%8s: ---Board %4s channel%10s at %u --- SIGNAL ERROR !\n",name,sms[board].name,findChannel(board,channel),j);
   }else{
     printf("%8s: ---Board %4s channel%10s --- SIGNAL CHECKED OK !\n",name,sms[board].name,findChannel(board,channel));	 
 }
 }else{
 if(j != -1){
     printf("%8s: ---Board %4s channel at %u --- SIGNAL ERROR !\n",name,sms[board].name,j);
   }else{
     printf("%8s: ---Board %4s channel --- SIGNAL CHECKED OK !\n",name,sms[board].name); 
 }
 } 
}
/*----------------------------------------------------------bit()
 * Calculates bit at channel in num.
*/ 
int bit(w32 num,int channel){
 return ((num &(1<<channel))==(1<<channel));
}
/*----------------------------------------------------------wbit()
 * writes bit=(0 or 1) in word num at position channel.
 */ 
int wbit(w32 num, w32 bit,int channel){
 if(bit)return (1<<channel) | num;
 else return ~(1<<channel) & num;
}
/*FGROUP DebCon------------------------------------------------------------
Debugging sync by generating ssm in memory.
 */ 
int DebugSync()
{
 int i,boards[NSSMBOARDS];
 for(i=0;i<NSSMBOARDS;i++)boards[i]=0; 
 genSeq(1,3600,1);
 genSeq(2,3600,55);
 genSeq(3,3600,33);
 boards[0]=1;boards[1]=2;boards[2]=3;
 syncSSM(3,boards); 
 return 1;
}
/*FGROUP DebCon---------------------------------------------------------
*/
int checkToggle(int board, int channel){
 int i;
 w32 *sm;
 w32 bit0,bit,mask;
 sm=sms[board].sm;
 if(sm == NULL){
   printf("checkToggle: memory not read.");
   return 1;
 }
 mask=(1<<channel);
 bit0=(sm[0]&mask)==mask;
 for(i=1;i<Mega;i++){
  bit=(sm[i]&mask)==mask;
  if((bit+bit0) != 1){
    printf("checkToggle: error at %i \n",i);
  }
  bit0=bit;
 }
 return 0;
}
//-------------------------------------------------------------------------
//               PATTERN GENERATION
//-------------------------------------------------------------------------
/*FGROUP DebConPat
 * ---------------------------------------------------------------genSeq()
   Generates sequence of length Length and period Period.
   E.g. it enables to generate array which correspond to orbit channel
*/
int genSeq(int board,int Period,int Start)
{
 int i;
 if(sms[board].sm == NULL)sms[board].sm = (w32 *)malloc(Mega*sizeof(w32));
 if(!sms[board].sm){
  printf("genSeq error: not enough memory.");
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
/*FGROUP DebConPat
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
 printf("writeSPn: Board %i %s : Start:%i Period:%i n=%x\n",
		 board,sms[board].name,Start,Period,n);
 return 0;
}
/*FGROUP DebConPat
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
/*FGROUP DebConPat
 * --------------------------------------------------------------WriteSPb2()
 Write 11..1 of length length to bit channel with Period from Start.
 Not overwriting other bits.
 */
int writeSPb2(int board,int Start,int Period,int length,int Channel){
 int i,j;
 w32 mask;
 if(sms[board].sm == NULL)sms[board].sm = (w32 *)malloc(Mega*sizeof(w32)); 
 if(!sms[board].sm){
  printf("writeSPb2 error: not enough memory.");
  return 1;  
 }
 mask=1<<Channel;
 for(i=Start;i<Mega;i++)if(((i-Start) % Period) == 0){
      j=i;
      while((j-i < length) && (j < Mega) ){
             sms[board].sm[j]=sms[board].sm[j] | mask;
             j++;
      }
      i=i+length;
 }
 sms[board].syncflag=0;
 printf("writeSPb2: Board %i %s : Start:%i Period:%i Channel=%i\n",board,sms[board].name,Start,Period,Channel);
 return 0;
}
/*FGROUP DebConPat
-----------------------------------------------------------------WriteSPP()
Write bit pattern Pattern to channel n from Start. 
Pattern is string of (0-9),(a-f) which is interpreted as hexadecimal number.
Least significant bits on the left.
*/
int writeSPP(int board,int Start,int Channel,char *Pattern){
 int i,j,i0,length;
 w32 bit,mask0,mask1;
 int pat;
 Signal *sig;
 if(Pattern)length=strlen(Pattern);
 else return 3;
 //printf("Pattern=%x %s \n",Pattern,Pattern);
 printf("writeSPP: strlen= %i \n",length);
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
 // Write pattern length to signal - assuming generating board
 if(sms[board].signal){
  sig = sms[board].signal->first;
  while(sig && (sig->channel != Channel))sig=sig->next;
  if(!sig){
   printf("writeSPP warning: channel %i not found on board %i \n",Channel,board);	
   return 5; 
  }
  sig->patlen=length;
 }
 return 0;
}
/*FGROUP DebConPat
-----------------------------------------------------------------WriteSPF()
Write bit pattern Pattern to channel n from Start. 
Pattern is string with formated input, see form2i() help.
*/
int writeSPF(int board,int Start,int Channel,char *Pattern){
 int i,j,k,length,patout[20],nperiod;
 w32 bit,mask0,mask1;
 Signal *sig;
 if(form2i(Pattern,patout,&nperiod))return 3;     
 if(sms[board].sm == NULL)sms[board].sm = (w32 *)malloc(Mega*sizeof(w32));
 if(!sms[board].sm){
  printf("writeSPF error: not enough memory.");
  return 2;  
 }
 mask1=1<<Channel;
 mask0= ~(0xffffffff & mask1);
 i=Start;       
 while(i<Mega){
   j=0;                          // period count
   bit=1;
   while(j<nperiod && i<Mega){
     k=0;
     while(k<patout[j] && i<Mega){  	   
       if(bit)sms[board].sm[i]=sms[board].sm[i] | mask1;
         else sms[board].sm[i]=sms[board].sm[i] & mask0;
       i++;k++;
     }
     bit=!bit;
     j++;
   }
 }
 // Write pattern length to signal - assuming generating board
 if(sms[board].signal){
  sig = sms[board].signal->first;
  while(sig && (sig->channel != Channel))sig=sig->next;
  if(!sig){
   printf("writeSPF warning: channel %i not found on board %i \n",Channel,board);	
   return 5; 
  }
  length=0;
  for(i=0;i<nperiod;i++)length=length+patout[i];
  sig->patlen=length;
 }
 return 0;
}
/*FGROUP DebConPat
 *----------------------------------------------------------------form2i()
 * Aim: to generate binary pattern
 * Input: string of formatted input: number1anumber2anumber3a
                                       2a10a3
                                      110000000000111
   a - delimiter
   number1 = number of 1, number2- number of 0 ....
 * Output: array of integers [number1,number2,...]
 */
int form2i(char *patin,int *patout,int *nperiod){
 int i10,num;
 char *loop,*start,*fin;
 if(!patin) return 1;
 printf("patin=%s %i\n",patin,strlen(patin));
 *nperiod=0;
 loop=patin;
 start=patin;
 while( *loop != '\0'){
  if( *loop == 'a'){
   fin=loop;
   num=0;
   i10=1;
   while(loop != start){
    loop--;
    num=num+char2i(*loop)*i10;
    i10=i10*10;
   }
   //printf("num = %i \n",num);
   patout[*nperiod]=num;
   *nperiod=*nperiod+1;
   if(*nperiod>20) return 2;
   loop=fin;
   loop++;
   start=loop;
  }else loop++;
 }
 return 0;
}
/*------------------------------------------------------------------
 Character to integer converter
*/
int char2i(char a){
 if(a >= 0x30 && a <= 0x39) return (a-0x30);
 else if (a >= 0x61 && a <= 0x66) return (a-0x57);
 else {
       printf("char2i: wrong pattern character %c \n",a);
       return -1;       
 } 
}
/*
 integer to character converter
*/
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
 int i,j,num,start=0;
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
 for(i=start;i<n+start;i++){
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
//-------------------------------------------------------------------
//                    LOGIC MODES
//-------------------------------------------------------------------
/*--------------------------------------------------------CheckMode()
 * Steering routine for checking the mode results, 
 * i.e. compare HW with model calculation
 */
int CheckMode(int n,int *boards, w32 modecode,w32 submode){
 // FO boards	
 if((boards[0]>4) && (boards[0]<9))return checkModeFO(n,boards,modecode,submode);
 // BUSY board
 else if(boards[0] == 0)return checkModeBU(n,boards,modecode,submode);
 // L0 board 
 else if(boards[0] == 1)return checkModeL0(n,boards,modecode,submode);
 // L1 board 
 else if(boards[0] == 2)return checkModeL1(n,boards,modecode,submode);
 // L2 board  
 else if(boards[0] == 3)return checkModeL2(n,boards,modecode,submode);
 // None generates 
 else if(boards[0] == (NSSMBOARDS-1))return checkModeN(n,boards,modecode,submode); 
 else{printf("CheclMode: unknown board %i. \n",boards[0]);
 HardWare.loop=0;
 return 9;
 }
 return 0;
}
/*----------------------------------------------------------InitMode()
 * Steering routine for initialization of the mode.
 */
int InitMode(int n,int *boards, w32 modecode,w32 submode){
 // FO boards	
 if((boards[0]>4) && (boards[0]<9))return 0;
 // L0 board 
 else if(boards[0] == 0)return 0;
 else if(boards[0] == 1)return initModeL0(n,boards,modecode,submode);  
 else if(boards[0] == 2)return initModeL1(n,boards,modecode,submode);  
 else if(boards[0] == 3)return initModeL2(n,boards,modecode,submode);  
 else if(boards[0] == (NSSMBOARDS-1))return initModeN(n,boards,modecode,submode);  
 else{printf("InitMode: unknown board %i. \n",boards[0]);
 HardWare.loop=0;
 return 9;
 }
 return 0;
}
//--------------------------------------------------------------------
//               HARDWARE MANIPULATIONS
//--------------------------------------------------------------------
/*--------------------------------------------------------SetBoardHW()
 * Steering routine for setting the board HW. 
 */
int SetBoardHW(int n,int *boards, w32 modecode,w32 submode){
 // FO boards	
 if((boards[0]>4) && (boards[0]<9))return setBoardFO(n,boards,modecode,submode); 
 else if(boards[0] == 0)return setBoardBU(n,boards,modecode,submode);
 else if(boards[0] == 1)return setBoardL0(n,boards,modecode,submode);
 else if(boards[0] == 2)return setBoardL1(n,boards,modecode,submode);
 else if(boards[0] == 3)return setBoardL2(n,boards,modecode,submode);
 else if(boards[0] == (NSSMBOARDS-1))return setBoardN(n,boards,modecode,submode);
 else{
 printf("SetBoardHW: unknown board %i. \n",boards[0]);
 HardWare.loop=0;
 return 9;
 }
 return 0;
}
/*--------------------------------------------------------WriteBoards()
 * write to ssms test patterns
 * write sms to hardware 
 */
int WriteBoards(int n,int *boards,w32 modecode,w32 submode){
 if((boards[0]>4) && (boards[0]<9)) return writeBoardsFO(n,boards,modecode,submode); 
 else if(boards[0] == 0) return writeBoardsBU(n,boards,modecode,submode);
 else if(boards[0] == 1) return writeBoardsL0(n,boards,modecode,submode);
 else if(boards[0] == 2) return writeBoardsL1(n,boards,modecode,submode);
 else if(boards[0] == 3) return writeBoardsL2(n,boards,modecode,submode);
 else if(boards[0] == (NSSMBOARDS-1)) return writeBoardsN(n,boards,modecode,submode);
 else return 1;
}
/*---------------------------------------------------ReadBoards()
 * Read from hardware to sms
*/
int ReadBoards(int n,int *boards){
 int i,i0=0;
 //if(n==1)i0=0; 
 printf("ReadBoards: ");
 for(i=i0;i<n;i++){
      if(boards[i] == 16) continue; // none board
      readSSM(boards[i]); 
      printf(" %i ",boards[i]);
 }
 printf("\n");
 return 0;
}
/*--------------------------------------------------StartBoards()
  Prepare all hardware (set modes) and start boards.
  Check if time< 80.
  input: *boards - board numbers
          bomodes[][] - modes
  
*/
int StartBoards(int n,char bomodes[][FILENAMESIZE],
		int *boards,w32 *modecodes,w32 submode){
 int i;
 // for browser
 for(i=0;i<n;i++)strcpy(sms[boards[i]].mode,&bomodes[i][0]);
 printf("StartBoards: setomSSM ");
 // first board not set!!
 for(i=1;i<n;i++){
    if(setomSSM(boards[i],modecodes[i])){    //set modes
     printf("StartBoards error: cannot set mode %s \n",bomodes[i]);
     return 1;     
    } 
    printf("board %i mode: %s 0x%x ",boards[i],sms[boards[i]].mode,modecodes[i]);
 }
 printf("\n");
 //Start generation
 // L0 and none boards have special steering routines
 if(boards[0] == 1)return startBoardsL0(n,boards,modecodes[0],submode);
 else if(boards[0] == (NSSMBOARDS-1)) return startBoardsN(n,boards,modecodes[0],submode);
 else{
   //first board set
   if(setomSSM(boards[0],modecodes[0])) return 1;
   printf("board %i mode: %s 0x%x \n",boards[0],sms[boards[0]].mode,modecodes[0]);
   if(n>1){
     printf("StartBoards default n=%i \n",n);
     if(startSSM(n,boards)){    // first board start last
       printf("StartBoards error: SSMs have not started inside one ORBIT. Sync not possible \n");
      return 1;
     }
     usleep(30000);
     return 0;
  }else{
    printf("StartBoards default n=%i \n",n);
    if(startSSM1(boards[0])) return 1;;
    usleep(30000);
    return 0;
  }
 }
}
/*-----------------------------------------------------------StartBoards()
 * New version
 * Start routine can be different for every board and mode
int StartBoardsNEW(int n,char bomodes[][FILENAMESIZE],
		int *boards,w32 *modecodes, w32 submode){
int i,first;
// Load modes 
if(modecodes[0] == 0x33f)first=1; else first=0;
printf("StartBoards: setomSSM ");
for(i=first;i<n;i++){
   if(setomSSM(boards[i],modecodes[i])){    //set modes
     printf("StartBoards error: cannot set mode %s \n",bomodes[i]);
     return 1;     
   } 
   printf("board %i mode: %s 0x%x ",boards[i],sms[boards[i]].mode,modecodes[i]);
   strcpy(sms[boards[i]].mode,&bomodes[i][0]);
 }
 if((boards[0]>4) && (boards[0]<9)) return startBoardsFO(n,boards,modecodes[0],submode); 
 else if(boards[0] == 0) return startBoardsBU(n,boards,modecodes[0],submode);
 else if(boards[0] == 1) return startBoardsL0(n,boards,modecodes[0],submode);
 else if(boards[0] == 2) return startBoardsL1(n,boards,modecodes[0],submode);
 else if(boards[0] == 3) return startBoardsL2(n,boards,modecodes[0],submode);
 else if(boards[0] == (NSSMBOARDS-1)) return startBoardsN(n,boards,modecodes[0],submode);
 else return 1;
}
*/
w32 CountTime();
/*FGROUP BC_STATUS
 Check the stability of bc status by reading it n times.
 board - board number
 n - number of readings
*/
void bcstat(int board,int n){
 int i;
 w32 stat,stato;
 stato=vmer32(BC_STATUS+BSP*ctpboards[board].dial);
 CountTime();
 for(i=0;i<n;i++){
  stat=vmer32(BC_STATUS+BSP*ctpboards[board].dial);
  if(stat != stato){
   printf("%s change %i old=0x%x new=0x%x \n",ctpboards[board].name,i,stato,stat);
   stato=stat;
  }
 }
 printf("%s bc status finished: 0x%x Time=%i\n",ctpboards[board].name,stat,CountTime());
}
/*FGROUP BC_STATUS
 Check the stability of bc status by reading it n times.
 All boards.
 n - number of readings
*/
void bcstat2(int n){
 int status[NCTPBOARDS],statuso[NCTPBOARDS],stat[NCTPBOARDS];
 int i,j;
 for(i=0;i<NCTPBOARDS;i++){
  stat[i]=0;
  statuso[i]=vmer32(BC_STATUS+BSP*ctpboards[i].dial);
 }
 CountTime(); 
 for(i=0;i<n;i++){
  for(j=0;j<NCTPBOARDS;j++){
   status[j]=vmer32(BC_STATUS+BSP*ctpboards[j].dial);
   if(status[j] != statuso[j]){
    stat[j]=stat[j]+1;
    statuso[j]=status[j];
   }
  }
 }
 printf("Time=%i\n",CountTime());
 for(i=0;i<NCTPBOARDS;i++){
  printf("%s : # of changes %i \n",ctpboards[i].name,stat[i]);
 }
}
//--------------------------------------------------------------------
//      List
//--------------------------------------------------------------------
List *addnum(List *last,int num){
 List *p;
 p = (List *) malloc(sizeof(List));
 if(!p){
  printf("addnum error: not enough memory.\n");
  return p; 
 }
 p->intnum=num;
 p->next=NULL;
 if(last){
  last->next=p;
  p->first=last->first;
 }else p->first = p;
 return p;
}
List *freenums(List *list){
 if(list){
  List *p;
  p=list->first;
  while(p){
   list=p->next;
   free(p);
   p=list;
  }
 }
 return NULL;
}
void printlist(List *list){
 if(list){
  list=list->first;
  while(list){
   printf(" %i ",list->intnum);
   list=list->next;
  }
  printf("\n");
 }else printf("Empty list \n");
}
//--------------------------------------------------------------------
//             SIGNAL LIST METHODS
//--------------------------------------------------------------------
/*-----------------------------------------------------------addSignal() 
 * Add signal to list*/
Signal *addSignal(Signal *last,int channel,int namenum, char *namechar){
 Signal *p;
 p = (Signal *) malloc(sizeof(Signal));
 if(!p) goto MEM;
 p->channel=channel;
 p->signamenum=namenum; 
 strcpy(p->signame,ConnectionNames[namenum]);
 //strcpy(p->pattern,"");
 p->patlen=0;
 p->offset=-1;
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
/*----------------------------------------------------------findSignal()
 * Finds signal in board with numname.
 * char *name is dummy variable to have also chracter name
*/ 
Signal *findSignal(int board,int namenum,char *name){
 Signal *s=sms[board].signal;
 if(s){
  s=s->first;
  while(s && (s->signamenum != namenum))s=s->next;
 }
 //if(!s)printf("findSignal: board %s signal %s %i not found.\n",sms[board].name,name,namenum);
 return s; 
}
/*----------------------------------------------------------findSignalS()
 * Finds signal in board with numname.
 * int namenum is dummy variable
*/ 
Signal *findSignalS(int board,int namenum,char *name){
 Signal *s=sms[board].signal;
 if(s){
   s=s->first;
   while(s && strcmp(name,s->signame))s=s->next;
 }
 if(!s)
  printf("findSignalS: board %s signal %s %i not found.\n",sms[board].name,name,namenum);
 return s; 
}
/*--------------------------------------------------------findChannel()
 * Finds signal in board correponding to channel 
*/ 
char *findChannel(int board,int channel){
 Signal *s=sms[board].signal;
 if(s){
  s=s->first;
  while(s && (s->channel != channel))s=s->next;
 }
 if(!s){
  //printf("findChannel: board %s channel %i not found. \n",sms[board].name,channel);	 
  return NULL;
 }
 return s->signame;
}
/*--------------------------------------------------------freeSignals()
 * free memory allocated to signals
*/ 
int freeSignals(int n,int *boards){
 int i;
 Signal *s0,*s1;
 for(i=0;i<n;i++){
  if(sms[boards[i]].signal){
   s0=sms[boards[i]].signal->first;
   while(s0){
    s1=s0->next;
    free(s0);
    s0=s1;   
   }
   sms[boards[i]].signal->first=NULL;
  //printf("freeSignals: %i, \n",boards[i]); 
  } 
 } 
 return 0;
}
/* Print connections-------------------------------------------------------
 * mode : 0 from 1st to last
 *        1 from last to first 
 * */

int PrintConnections(int n, int *boards,int mode){
 int i;
 Signal *s;
 for(i=0;i<NSSMBOARDS;i++){
   if(sms[i].signal){	 
    printf("CONNECTIONS BOARD %i %i 1->last: \n",i,boards[i]);
    s=sms[i].signal->first;
    while(s){
     printf("[%i %s %i %i] ",s->channel,s->signame,s->signamenum,s->offset);
     s=s->next;
    }
   printf("\n");
   }
 }
 return 1; 
}

