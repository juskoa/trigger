#include "ssmconnection.h"

static int clust[4][7];  
static int roc[4][4],calflag;
/*------------------------------------------------------------CheckModeFO()
 *  Steering routine for FO modes.
 */ 
int checkModeFO(int n,int *boards, w32 modecode,w32 submode){
  int ret=0;
  getCluster(boards[0]);
  switch(modecode){
       case 0x104:  //FO Connections
	       ret=Connect(n,boards);
	       HardWare.loop=0;
	       break;
       case 0x20c:  //FO L0L1 genertor mode
	       ret=FOL0L1(n,boards);
	       HardWare.loop=0;
	       break;
       case 0x21c:  // FO L2 generator mode	 
	       ret=FOL2(n,boards);
	       HardWare.loop=0;
	       break;      
       default:
         printf("A2B: FO modecode %i not found.\n",modecode);
         return 1;	 
   }
  return ret;
}
/*-------------------------------------------------------------setBoardFO()
 */
int setBoardFO(int n,int *boards,w32 modecode,w32 submode){return 0;}
/*-------------------------------------------------------------Connect()
 *
 * Testing the connection between the boards and other boards.
 * Alhorithm: generate simple pattern like 100010001...
 * On received board find first non zero bit and try to compare.
 * Fails often due to the wrong bits either at the beginning of ssm
 * or start of generation
*/
int Connect(int n,int *boards){
 int i,offset,error=0;
 Signal *si,*sg;
 int ii=0;
 // loop over receiving boards
 for(i=1;i<n;i++){                      
  si=sms[boards[i]].signal->first;
  //loop over signals of receiving board
  while(si && ii++<33){
    // Find if si signal exist in generating board
    sg=findSignal(boards[0],si->signamenum,si->signame);
    if(sg){     	     
     //pat=getPatfromF(sms[boards[0]].sm,sg->channel,sg->patlen);
     //if(pat)offset=syncSIG2(boards[i],si->channel,pat);
     //free(pat);

     // Assumes : test pattern starts with 1 !!!
     offset=syncSIG1(sms[boards[i]].sm,si->channel);
     if(offset<0){error++;si=si->next;continue;}
     //error=error+compSIG(boards[0],sg->channel,boards[i],si->channel,offset,0);    
     error=error+compSIG1(boards[0],sg->channel,boards[i],si->channel,offset,0);
    }else {
      //warnmess("Connect",boards[i],si->signame," not in table");  
    }
    si=si->next;	  
  }  
 }
 return error;
}
/*------------------------------------------------------ConnectPat()
 * Look for pattern
 * 
*/
int ConnectPat(int n,int *boards){
 int i,offset,error=0;
 Signal *si,*sg;
 char *pat; 
 int ii=0;
 // loop over receiving boards
 for(i=1;i<n;i++){                      
  si=sms[boards[i]].signal->first;
  //loop over signals of receiving board
  while(si && ii++<33){
    // Find if si signal exist in generating board
    sg=findSignal(boards[0],si->signamenum,si->signame);
    if(sg){     	     
     pat=getPatfromF(sms[boards[0]].sm,sg->channel,sg->patlen);
     if(pat)offset=syncSIG2(boards[i],si->channel,pat);
     free(pat);

     // Assumes : test pattern starts with 1 !!!
     //offset=syncSIG1(sms[boards[i]].sm,si->channel);
     if(offset<0){error++;si=si->next;continue;}
     error=error+compSIG(boards[0],sg->channel,boards[i],si->channel,offset,0);    
     //error=error+compSIG1(boards[0],sg->channel,boards[i],si->channel,offset,0);
    }else {
      //warnmess("Connect",boards[i],si->signame," not in table");  
    }
    si=si->next;	  
  }  
 }	 
 return error;
}
//------------------------------------------------------------------
//                 FO L0L1 mode
// Direct comparison between sms on fo and ltu, no sms[test]                
//------------------------------------------------------------------
int l0l1FPP(int i,int *boards,Signal *si){
 Signal *sg;
 char *pat;
 int offset,connector;
 connector=boards[i]-11;  // connector = detector	
 // calflag + test cluster	
 if(calflag && clust[connector][0]){ 
   //PP signal
   sg=findSignal(boards[0],31,"pp");
   if(sg && sg->patlen){
     pat=getPatfromF(sms[boards[0]].sm,sg->channel,sg->patlen);	  
     if(pat)offset=syncSIG2(boards[i],si->channel,pat);
     free(pat);
     if(offset<0) return 1;
     si->offset=offset;  // toto neskor mozno skovat
     compSIG(boards[0],sg->channel,boards[i],si->channel,offset,0);
   }else{
     warnmess("FOL0L1",boards[0],"FPPP"," not in table or patlen=0");
   }
 }else{
   warnmess("FOL0L1",boards[i],"FPPP"," No calflag or test cluster");	
 }	
 return 0;
}
//--------------------------------------------------------------------
//L0 signal
int l0l1FPL0(int i,int *boards,Signal *si){
 Signal *sg;
 int j,inclust,patlen,offset,connector;
 char *pat;
 w32 *sm;
 if(!sms[TEST].sm) sms[TEST].sm=TestSSM;
 sm=TestSSM;
 if(!sm){
   printf("l0l1FPL) error: not enough memory !");
   exit(1);
 }
 connector=boards[i]-11;  // connector = detector	
 for(j=0;j<Mega;j++)sm[j]=0;
 patlen=0;  
 for(j=0;j<7;j++){
  sg=findSignal(boards[0],24+j,"l0clstt-l0clst6");
  if(clust[connector][j] && sg && sg->patlen){
    inclust++;	  
    patlen=sg->patlen;
    for(j=0;j<Mega;j++)sm[j]=sm[j]+bit(sms[boards[0]].sm[j],sg->channel); 
  }
 }
 if(patlen){ // at least one cluster was generated	
   // 1/0 signal
   for(j=0;j<Mega;j++)if(sm[j])sm[j]=1;
   pat=getPatfromF(sm,0,patlen);
   if(pat)offset=syncSIG2(boards[i],si->channel,pat);
   free(pat);
   if(offset<0){
     //free(sm);
     return 1;
    }
    si->offset=offset;
    compSIG2(sm,0,boards[i],si->channel,offset,0);
 }else{
    warnmess("FOL0L1",boards[i],si->signame," patlen=0 or no cluster");  	
 }
 //free(sm);	
 return 0;
}
//-------------------------------------------------------------------
// L1, L1data signals
int l0l1FPL1(int i,int *boards,Signal *si){  
 Signal *sg,*sd;      
 int j,k,inclust,patlen,offset,intest,connector;
 char *pat;
 w32 *smd=NULL;
 w32 *sm = (w32 *) malloc(Mega*sizeof(w32));
 if(!sm){
  printf("l0l1FOL1 error: not enough memory! \n");
  exit(1);  
 } 
 connector=boards[i]-11;     // connector = detector	
 for(j=0;j<Mega;j++)sm[j]=0;
 patlen=0;
 //test cluster 
 sg=findSignal(boards[0],17,"CLuL11 Test Cluster");
 //intest =  clust[connector][0] && sg && sg->patlen;
 intest =  sg && sg->patlen;
 if(intest && clust[connector][0]){
  inclust=1;	 
  patlen=sg->patlen;
  for(j=0;j<Mega;j++)sm[j]=sm[j]+bit(sms[boards[0]].sm[j],sg->channel);  
 }
 // Other clusters
 for(j=1;j<7;j++){
  sg=findSignal(boards[0],17+j,"CluL12-CluL17");
  if(clust[connector][j] && sg && sg->patlen){
    inclust++;	  
    patlen=sg->patlen;
    for(j=0;j<Mega;j++)sm[j]=sm[j]+bit(sms[boards[0]].sm[j],sg->channel); 
  }
 }
 if(patlen){ //at least one cluster generated 
   // 1/0 signalu	
   for(j=0;j<Mega;j++)if(sm[j])sm[j]=1;
   pat=getPatfromF(sm,0,patlen);
   if(pat)offset=syncSIG2(boards[i],si->channel,pat);
   free(pat);
   if(offset<0){
     free(sm);
     return 1;
   }
   // Write to sms[TEST] channel 0 for debugging
   for(j=0;j<Mega;j++)sms[TEST].sm[j]=wbit(sms[TEST].sm[j],sm[j],0);
   // Compare
   si->offset=offset;
   compSIG2(sm,0,boards[i],si->channel,offset,0);	
   // FPL1D ---------------------------------------------------------
   sg=findSignal(boards[0],7,"L1D");
   if(!sg){
     warnmess("FOL0L1",boards[0],"L1D"," not in table");
     return 1;  
    }
    sd=findSignal(boards[i],40,"FPL1D");
    if(!sd){
      warnmess("FOL0L1",boards[i],"FPL1D", " not in table");
      return 1; 	  
    }	  
    smd = (w32 *)  malloc(Mega*sizeof(w32));
    if(!smd){
     printf("l0l1FPL1D error: not enough memory !\n");
     exit(1);     
    } 
    // Create transformed L1 data in sm[]
    //printf("intest= %i \n",intest);
    j=0;k=0;
    while(k<Mega){
      if(sm[k]){	  
	int k0,j0;
	j=k;
	//sms[boards[0]].sm[j] & mask
	if(k<Mega)smd[k++]=0;                 //Spare 
        if(k<Mega)smd[k++]=calflag*intest;	 // Calibration flag   
	k0=k;
	//Roc copy (if testcluster roc is copied)
        while(((k-k0)<4) && k<Mega){
	     smd[k]=roc[connector][3-(k-k0)]*intest;
             k++;
        }
	//ESR, L1SwC 
	if(k<Mega)smd[k++]=bit(sms[boards[0]].sm[j++],sg->channel);
	if(k<Mega)smd[k++]=bit(sms[boards[0]].sm[j++],sg->channel);
	//L1 classes
	j0=j;
	while(((j-j0)<50) && k<Mega)smd[k++]=bit(sms[boards[0]].sm[j++],sg->channel);	  
      }else smd[k++]=0;
    }
    //for(j=0;j<Mega;j++)if(sm[j])sm[j]=1;
    // sync and compare
    offset=syncSIG1(sms[boards[i]].sm,sd->channel);
    if(offset<0){
	free(sm);free(smd);
	return 1;
    }
    // Write to sms[TEST].sm for debugging
    for(j=0;j<Mega;j++)sms[TEST].sm[j]=wbit(sms[TEST].sm[j],smd[j],1);
    j=syncSIG1(smd,0);
    compSIG3(smd,0,boards[i],sd->channel,offset-j,0);	
 }else{
   warnmess("FOL0L1",boards[i],si->signame," patlen=0 or no cluster"); 
   warnmess("FOL0L1",boards[i],"FPL1D"," patlen=0"); 
 }
 //printf("%i %x \n",i,smd);
 free(smd);
 free(sm);
 return 0;	
}
//------------------------------------------------------------------
int FOL0L1(int n,int *boards){
 int i;
 Signal *si; 
 for(i=1;i<n;i++){                       // loop over receiving boards 
  si=sms[boards[i]].signal->first;
  while(si){                            //loop over signals of rec board [i]	
    switch(si->signamenum){             
      case 37 ://FPPP---------------------------------------------- 
	l0l1FPP(i,boards,si);break;      
      case 38 ://FPL0----------------------------------------------
        l0l1FPL0(i,boards,si);break;
      case 39 ://FPL1--------------page 3 of fo_l1.tdf-----------------------
        l0l1FPL1(i,boards,si);break;      
      /*case 40: //FPL1D
        FPL1D can follow only if FPL1 is generated,(if so ?)
        so this goes after FPL1	        
        break;*/	
      default:
        //printf("FOL0L1 error: %s not found.\n",si->signame); 
      break; 
    }
    si=si->next;
  }	
 } 	 
 return 0;
}
/*-------------------------------------------------------------------------
 
*/ 
int FOL2(int n, int *boards){
 int i,j,offset,connector;
 Signal *si,*sgS,*sgD1,*sgD2;
 Signal *sl1clst[7];
 w32 *sm=NULL,*smd=NULL;

 // Find L2Strobe in generating board      
 sgS = findSignal(boards[0],3,"L2Strobe");
 if(!sgS || !sgS->patlen){
    warnmess("FOL2",boards[0],"StrL2D"," "); goto ERR;
 } 
 // Find L2D1 in generating board         
 sgD1=findSignal(boards[0],4,"L2Data1");
 if(!sgD1 || !sgD1->patlen){
   warnmess("FOL2",boards[0],"L2D1"," "); goto ERR;
 } 
 // Find L2D2 in generating board         
 sgD2=findSignal(boards[0],5,"L2Data2");
 if(!sgD2 || !sgD2->patlen){
   warnmess("FOL2",boards[0],"L2D2"," "); //goto ERR;
 }
 // Find l1clstt - l1clst6
 for(i=0;i<7;i++){
	 sl1clst[i]=findSignal(boards[0],17+i,"l1clstx");
	 if(!sl1clst[i])warnmess("FOL2",boards[0],"l1clst","x ");
 }

 sm = (w32 *) malloc(Mega*sizeof(w32));
 if(!sm) goto ERR2;
 smd = (w32 *) malloc(Mega*sizeof(w32));
 if(!smd) goto ERR2;
 
 if(!sms[TEST].sm)sms[TEST].sm=TestSSM;
 
 for(i=1;i<n;i++){
   connector=boards[i]-11;            // connector = detector	
  //FPStrL2D-------------------------------------------------- 	 
  si=findSignal(boards[i],41,"FPStrL2Data");  
  if(!si){
   warnmess("FOL2",boards[i],"FPStrL2D"," "); 	  
   continue;
  }
  for(j=0;j<Mega;j++){
    int j0;	  
    int l1clusters=0;	  
    if(bit(sms[boards[0]].sm[j],sgS->channel)){	
      l1clusters=                  // Test cluster
	      bit(sms[boards[0]].sm[j0],sgD1->channel)*clust[connector][0];
      for(j0=j-6;j0<j;j0++)l1clusters=l1clusters +  // Clusters
	     bit(sms[boards[0]].sm[j0],sgD1->channel)*clust[connector][j-j0];   
    }
    //if(l1clusters)printf("%i\n",j);
    if(l1clusters)sm[j]=1; else sm[j]=0;
    //ak je strobe pojdu aj data    
  } 
  offset=syncSIG1(sms[boards[i]].sm,si->channel);
  if(offset<0){
   printf("FOL2: board %i signal FPStrL2D not found. \n",boards[i]);	  
   continue;
  }
  sms[boards[i]].offset=offset;
  // Write to sms[test] for debugging
  for(j=0;j<Mega;j++)sms[TEST].sm[j]=wbit(sms[TEST].sm[j],sm[j],0);
  // Compare
  j=syncSIG1(sm,0);
  compSIG3(sm,0,boards[i],si->channel,offset-j,0);
  sms[TEST].offset=j;
  // L2Data------------------------------------------------
  // j - pointer to output serial bit
  // k - pointer to Data1/Data2 bit 
  for(j=0;j<Mega;j++){	  
   if(sm[j]){ 
    int k,k0,j0;	   
    int l2arf=0;
    int l2clstt;
    l2clstt=bit(sms[boards[0]].sm[j],sgD1->channel);
    // OR of L2 clusters
    j0=j;    
    while(j0<j+7 && j0<Mega)l2arf=l2arf+bit(sms[boards[0]].sm[j0++],sgD1->channel);
    //printf("l2arf=%i\n",l2arf);
    if(j<Mega && l2arf)smd[j++]=1; 
    // BCID and Orbit
    j0=j;
    k=j+6; //
    while(j<Mega && (j-j0)<36)smd[j++]=bit(sms[boards[0]].sm[k++],sgD1->channel);
    //Spare
    if(j<Mega)smd[j++]=0;
    if(j<Mega)smd[j++]=0;
    // Clt
    if(j<Mega)smd[j++]=calflag*l2clstt;
    if(j<Mega)smd[j++]=l2clstt;   // L2SwC
    //L2 clusters
    j0=j;
    k0=k-36-6;
    while(j<Mega && (j-j0)<6)smd[j++]=bit(sms[boards[0]].sm[k0++],sgD1->channel); 
    // 11 BC data gap
    k=k+11;
    // L2 classes
    j0=j; 
    while(j<Mega && (j-j0)<50){
      smd[j++]=bit(sms[boards[0]].sm[k++],sgD2->channel);
    }    
   }else sm[j]=0;
  }
  //L2Data comparison
  si=findSignal(boards[i],42,"FPL2Data");
  if(!si){
   warnmess("FOL2",boards[i],"FPL2D"," "); 	  
   continue;
  }
  offset=syncSIG1(sms[boards[i]].sm,si->channel); // tu je divna 1 na zaciatku

  if(offset<0){
   printf("FOL2: board %i L2data not found.\n",boards[i]);	  
   continue;
  }
  //Write to sms[test] channel 1 for debugging
  for(j=0;j<Mega;j++)sms[TEST].sm[j]=wbit(sms[TEST].sm[j],smd[j],1);
  // Compare
  j=syncSIG1(smd,0);
  compSIG3(smd,0,boards[i],si->channel,offset-j,0);
 }
 free(sm);
 free(smd);
 return 0;
ERR:
 return 1;
ERR2:
 printf("FOL2: not enough memory \n");
 return 2; 
}
void warnmess(char *mode,int board,char *signame,char *message){
 printf("%s warning: board %i signal %s not checked: %s\n",mode,board,signame,message);
}
/*----------------------------------------------------getCluster()
 * get FO settings and output them as table.
*/ 
void getCluster(int board){
 //roc[detecor][bit]	
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
 calflag = (word & (1<<20)) == (1<<20);
 printf("Calflag=%i \n",calflag);
 for(i=0;i<4;i++){
  mask=1<<(16+i);
  clust[i][0] = (word & mask) == mask; 
  for(j=0;j<4;j++){
   mask=1<<(4*i+j);	  
   roc[i][j] = (word & mask) == mask;	  
  } 
 }
 for(i=0;i<4;i++){
   printf("Detector %i CLUSTERS:",i);	 
   for(j=0;j<7;j++)printf(" %i ",clust[i][j]);
   printf("\n");
   printf("Detector %i      Roc:",i);
   for(j=0;j<4;j++)printf(" %i",roc[i][j]);
   printf("\n");
 }
 return;
}
/*---------------------------------------------------------WriteBoardsFO()
 */ 
int writeBoardsFO(int n,int *boards, w32 modecode,w32 submode){
 int i;
 writeSPn(boards[0],0,1,0);         // Write all 0 to generating board board[0]
 switch(modecode){
	 case 0x104:   // FO connections
            writeSPP(boards[0],0,1,"100000");
 	    writeSPP(boards[0],0,2,"100000");
            writeSPP(boards[0],0,3,"100000");
            writeSPP(boards[0],0,4,"100000");
            writeSPP(boards[0],0,5,"100000");
            writeSPP(boards[0],0,6,"100000");
	 break;
	 case 0x20c:    // FO L0L1
	    // l0clstt  
            writeSPP(boards[0],0,1,"100000");
            // l0clst1
            //writeSPP(boards[0],0,2,"100000");
            // l0clst2
            //writeSPP(boards[0],0,3,"000000");
            // l0clst3
            //writeSPP(boards[0],0,4,"000000");
            // l0clst4
            //writeSPP(boards[0],0,5,"000000");
            // l0clst5
            //writeSPP(boards[0],0,6,"000000");
            // l0clst6
            //writeSPP(boards[0],0,7,"000000");
            // pp
            writeSPP(boards[0],0,8,"100000");
            // l1clstt  
            //writeSPP(boards[0],0,9,"10000000000000000000");
            // l1clst1  
            writeSPP(boards[0],0,10,"10000000000000000000");
            // l1clst2  
            writeSPP(boards[0],0,11,"10000000000000000000");
            // l1clst3  
            writeSPP(boards[0],0,12,"10000000000000000000");
            // l1clst4  
            writeSPP(boards[0],0,13,"10000000000000000000");
            // l1clst5  
            writeSPP(boards[0],0,14,"10000000000000000000");
            // l1clst6  
            writeSPP(boards[0],0,15,"10000000000000000000");
 
            // StrL1D recently redundant, so not tested
            //writeSPP(boards[0],0,16,"faa00");
            // l1data
            writeSPP(boards[0],0,17,"11111111111111000000");
	 break;
	 case 0x21c:    //L2 gen mode
	   // There is a minimum distance between signal which is ?? bc
           // StrL2D L2 mode
           writeSPP(boards[0],0,1,"0800000000000000000000000000");
           //writeSPP(boards[0],0,1,"0000000000000000000000000000");
           // L2D1
	   // T654 321T 6543 21BC 
	   // 0111 1110 0101 0101 0101
	   //   e   7    a    a     a
           //writeSPP(boards[0],0,2,"e7aa0aaaaaaa3000000000000000");
           writeSPP(boards[0],0,2,"00000aaaaaaa3000000000000000");
           // L2D2
           writeSPP(boards[0],0,3,"000000000000000bbbbbbbbbbbb3");
         break;
         default:
	    printf("WriteBoards error: mode %i not found.\n",modecode);
	 return 1;	 
 } 
 writeSSM(boards[0]);                 // Write sms[board[0]].sm to hardware
 
 for(i=1;i<n;i++)writeSPn(boards[i],0,1,0);   //Write all 0 to receiving boards
 for(i=1;i<n;i++)writeSSM(boards[i]);          // Write ssm[board[i]].sm to hardware
 return 0;
}

