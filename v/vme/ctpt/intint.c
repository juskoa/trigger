#include intint.h
typedef unsigned long long int lint;
int bit(w32 num,int channel){
 return ((num &(1<<channel))==(1<<channel));
}
//--------------------------------------------------------------------
//--------------------------------------------------------------------
// CTPReadout structure
//
typedef struct CTPR{
 int l2clusters;
 lint l2classes;
 int bcid;
 int orbit;
 int eob;   // used as eob flag, other items shoyld be zero
 int esr;
 int clt;    // calibration trigger
 int swc;    // software class
 int issm;   // position of the first word in ssm
}CTPR;
// Copy CTPR
void copyCTPR(CTPR *cpto,CTPR *cpfrom){
 cpto->l2clusters=cpfrom->l2clusters;
 cpto->l2classes =cpfrom->l2classes;
 cpto->bcid      =cpfrom->bcid;
 cpto->orbit     =cpfrom->orbit;
 cpto->esr       =cpfrom->esr;
 cpto->clt       =cpfrom->clt;
 cpto->swc       =cpfrom->swc;
 cpto->eob       =cpfrom->eob;
 cpto->issm      =cpfrom->issm;
}
// Write 0 to CTPR
void clearCTPR(CTPR *ctpr){
 ctpr->l2clusters=0;
 ctpr->l2classes =0;
 ctpr->bcid      =0;
 ctpr->orbit     =0;
 ctpr->esr       =0;
 ctpr->clt       =0;
 ctpr->swc       =0;
 ctpr->eob       =0;
 ctpr->issm      =0;
}
// Print header
void printHeader(FILE *file){
 fprintf(file,"      SSMBC  BCID BCID   ORBIT  L2Clusters  L2Classes ESR ClT SwC \n");
}
//Print CTPR
void printCTPR(CTPR *item,FILE *file){
 if(item->eob)fprintf(file,"EOB: %7i\n",item->issm);
 else if(!item->l2classes) fprintf(file,"ORBIT: %7i \n",item->issm);
 else fprintf(file,"CTP: %7i %4i 0x%3x %8i 0x%2x 0x%13llx  %1i  %1i  %1i\n",item->issm,item->bcid,item->bcid,item->orbit,item->l2clusters,item->l2classes,item->esr,item->clt,item->swc);
}
//Interaction Record
typedef struct IRDda{
 int error;
 int orbit;
 int Inter[251];
 int bc[251];
 int issm;
}IRDa;
// Copy IRDa
void copyIRDa(IRDa *cpto,IRDa *cpfrom){
 int i;
 cpto->error=cpfrom->error;
 cpto->orbit=cpfrom->orbit;
 cpto->issm=cpfrom->issm;
 for(i=0;i<251;i++){
  cpto->Inter[i]=cpfrom->Inter[i];
  cpto->bc[i]=cpfrom->bc[i];
 }
}
// Write 0 to IRDa
void clearIRDa(IRDa *irda){
 int j;
 irda->error=0;
 irda->orbit=0;
 irda->issm=0;
 for(j=0;j<251;j++){irda->Inter[j]=0;irda->bc[j]=0;}
}       
// Print irda
void printIRDa(IRDa *item,FILE *file){
 int i=0;
 fprintf(file,"IRD: %7i %1i %8i \n",item->issm,item->error,item->orbit);
 while(item->Inter[i]){
      fprintf(file," %i  %i  %4i 0x%3x \n",i,item->Inter[i],item->bc[i],item->bc[i]);
      //printf("  %i  0x%x \n",i,item->Inter[i]);
      i++;
 }
}
//
// List of CTPRIRDaeadout
// - list of CTP readouts and Int records as they come in ssm
//
typedef struct CTPRIRDList{
 struct CTPRIRDList *next;
 struct CTPRIRDList *first;
 CTPR *ctpr;
 IRDa *irda;
}CTPRIRDList;
//
// add CTPR to CTPRIRDList
//
CTPRIRDList *addCTPR(CTPRIRDList *last,CTPR *new){
 CTPRIRDList *p;
 p = (CTPRIRDList *) malloc(sizeof(CTPRIRDList));
 if(!p) goto ERR;
 p->ctpr = (CTPR *) malloc(sizeof(CTPR));
 if(!(p->ctpr)) goto ERR;
 copyCTPR(p->ctpr,new);
 p->irda=NULL;
 p->next=NULL;
 if(last){
  last->next=p;
  p->first=last->first;
 }else p->first = p;
 return p;
 ERR:
  printf("addCTPR error: not enough memory.\n");
  return p; 
}
//
// add IRDa to CTPRIRDList
//
CTPRIRDList *addIRDa(CTPRIRDList *last,IRDa *new){
 CTPRIRDList *p;
 p = (CTPRIRDList *) malloc(sizeof(CTPRIRDList));
 if(!p) goto ERR;
 p->irda = (IRDa *) malloc(sizeof(IRDa));
 if(!(p->irda)) goto ERR;
 copyIRDa(p->irda,new);
 p->ctpr=NULL;
 p->next=NULL;
 if(last){
  last->next=p;
  p->first=last->first;
 }else p->first = p;
 return p;
 ERR:
  printf("addIRDa error: not enough memory.\n");
  return p; 
}
//PrintCTPRIRDlist
void printlistN(CTPRIRDList *list,FILE *file){
 int ictpr=0,iirda=0;
 if(list){
  printHeader(file);
  list=list->first;
  while(list){
   if(list->ctpr){
      //fprintf(file, "#CTP %i \n",ictpr);
      printCTPR(list->ctpr,file);
      ictpr++;
   }
   if(list->irda){ 
     //fprintf(file, "#IRD %i \n",iirda);
     printIRDa(list->irda,file);
     iirda++;
   }
   list=list->next;
  }
  fprintf(file,"#CTPR=%i #IRDa=%i \n",ictpr,iirda);
 }else fprintf(file,"Empty list \n"); 
}
//Print CTPRIRD list for debug
void printlistN2(CTPRIRDList *list){
 int i=0;
 list=list->first;
 while(list){
  //printf("%i %x %x %x %x \n",i,list->next,list->first,list->ctpr,list->irda);
  list=list->next;
  i++;
 }
}
// free CTPRIRDList
CTPRIRDList *freenumsN(CTPRIRDList *list){
 if(list){
  CTPRIRDList *p;
  p=list->first;
  while(p){
   list=p->next;
   if(p->ctpr)free(p->ctpr);
   if(p->irda)free(p->irda);
   free(p);
   p=list;
  }
 }
 return NULL;
}
/*--------------------------------------------------getL2alist()
 * extract L2a data from backplane from L2 board and make a CTP list
 * add also orbit to record
*/
CTPRIRDList *getL2alist(int boardl2,CTPRIRDList *L2alist){
 int i=0,j,iorbit=0;
 int l2clusters,bcid,orbit,esr;
 lint l2class;
 Signal *sl2strobe,*sdata1,*sdata2;
 int sl2strobech,sdata1ch,sdata2ch;
 CTPR L2a,ORBIT;
 sl2strobe=findSignalS(boardl2,0,"l2strobe");
 sdata1=findSignalS(boardl2,0,"l2data1");
 sdata2=findSignalS(boardl2,0,"l2data2");
 if(sl2strobe && sdata1 && sdata2){
   sl2strobech=sl2strobe->channel;
   sdata1ch=sdata1->channel;
   sdata2ch=sdata2->channel;
 }else{
   sl2strobech=1;
   sdata1ch=2;
   sdata2ch=3;
 }
 printf("compareL2aCTPreadout: l2strobe l2data1 l2data2 channels= %i %i %i\n",sl2strobech,sdata1ch,sdata2ch);
 clearCTPR(&L2a);
 clearCTPR(&ORBIT);
 while(i<Mega){
  // ORBIT pulse
  if(bit(sms[boardl2].sm[i],0)){
   if(!iorbit){
    ORBIT.issm=i;
    L2alist=addCTPR(L2alist,&ORBIT);
    iorbit=1;
   }
  }else iorbit=0;
  if(bit(sms[boardl2].sm[i],sl2strobech)){
   L2a.issm=i;
   // L2 clusters -> one integer
   j=0;
   l2clusters=0;
   while((j<7) && (i+j)<Mega){
    l2clusters=l2clusters+bit(sms[boardl2].sm[i+j],sdata1ch)*(1<<(6-j));
    j++;
   }
   i=i+7;
   // BCID
   j=0;
   bcid=0;
   while((j<12) && (i+j)<Mega){
    bcid=bcid+bit(sms[boardl2].sm[i+j],sdata1ch)*(1<<(11-j));
    j++;
   }
   i=i+12;
   //ORBIT
   j=0;
   orbit=0;
   while((j<24) && (i+j)<Mega){
    orbit=orbit+bit(sms[boardl2].sm[i+j],sdata1ch)*(1<<(23-j));
    j++;
   }
   i=i+24;
   esr=bit(sms[boardl2].sm[i],sdata1ch);
   i=i+11;
   // L2class
   j=0;
   l2class=0;
   while((j<50) && (i+j)<Mega){
    if(bit(sms[boardl2].sm[i+j],sdata2ch))l2class=l2class+(1ll<<(49-j));
    j++;
   }
   L2a.l2clusters=l2clusters;
   L2a.l2classes=l2class;
   L2a.bcid=bcid;
   L2a.orbit=orbit; 
   L2a.esr=esr;
   L2a.clt=0;  // to be read from hw
   L2a.swc=0;  // to be read from hw
   L2alist=addCTPR(L2alist,&L2a);
   clearCTPR(&L2a);
   //printf("comparel2aCTPreadout: l2 clusters: 0x%x BCID: %i ORBIT: %i L2class: 0x%llx \n",l2clusters,bcid,orbit,l2class);
  }else i++;
 }
 //printlistN(L2alist);
 return L2alist; 
}
/*------------------------------------------------------getCTPRIRDList
 * Get CTP Readout list from INT board
 */
CTPRIRDList *getCTPRIRDList(int boardint,CTPRIRDList *list){
 int i=0,first=1,firstirda=1;
 int iCTPR=0,iIRDa=0;
 // Total Block counters
 int nCTPR=0,nIRDa=0;
 int blockid,eob;
 lint l2cl;
 IRDa irda;
 CTPR ctpr;
 Signal *sctrl,*sten,*sblockid;
 sctrl=findSignalS(boardint,0,"ddl.fb_ctrl");
 sten=findSignalS(boardint,0,"ddl.fb_ten");
 sblockid=findSignalS(boardint,0,"ddl.fb_d[15]");
 list=NULL;
 clearCTPR(&ctpr);
 clearIRDa(&irda);
 // Start to analyse data
 while(i<Mega){     
   // Find  end of block
   if(bit(sms[boardint].sm[i],sctrl->channel)){
     eob=sms[boardint].sm[i]&0xffff;
     if(eob != 0x64){
       printf("getCTPRIRDList: Incorrect EOB in data: 0x%x \n",eob);
       return list;
     }
     //eob found and added to list
     if(iIRDa && iCTPR){
      printf("getCTPRIRDList: ERROR iIRDa=%i iCTPR=%i \n",iIRDa,iCTPR);
      return list;
     }
     if(!iIRDa && !iCTPR ){
      if(first) first=0; else{
      printf("getCTPRIRDList: ERROR %i iIRDa=%i iCTPR=%i \n",i,iIRDa,iCTPR);
      return list;
      }
     }
     if(iIRDa){
      list=addIRDa(list,&irda);
      clearIRDa(&irda);
      iIRDa=0;
      nIRDa++;
      firstirda=1;
     }
     if(iCTPR){
      if(iCTPR == (8+5) ){   // Inputs in CTP readout
       list=addCTPR(list,&ctpr);
       clearCTPR(&ctpr);
       iCTPR=0;
       nCTPR++;
      }else{
       printf("getCTPRIRDList: ERROR iCTPR != 8: %i \n",iCTPR);
       return list;
      }
     }
     ctpr.eob=1;
     ctpr.issm=i;
     list=addCTPR(list,&ctpr);
     clearCTPR(&ctpr);
     i++;
   }else{
    if(bit(sms[boardint].sm[i],sten->channel)){
      blockid=bit(sms[boardint].sm[i],sblockid->channel);
      //printf("i=%i blockid=%i iCTPR=%i \n",i,blockid,iCTPR);
      if(blockid){
        // Interaction record
        // Check if beginning of orbit
        if(!(sms[boardint].sm[i]&0x3000)){
          if(!firstirda){
           list=addIRDa(list,&irda);
           clearIRDa(&irda);
           iIRDa=0;
           nIRDa++;
          }else firstirda=0;
        }
        if(iIRDa == 0){
          irda.orbit=(sms[boardint].sm[i]&0xfff)<<12;
          irda.issm=i;
        }else if(iIRDa == 1){
          if(!(sms[boardint].sm[i]&0x3000)){
           printf("getCTPRIRDList: IR ERROR flag != 1 1\n");
           return list;
          }
          irda.orbit=irda.orbit+(sms[boardint].sm[i]&0xfff);
        }else if(iIRDa < 253){
         irda.Inter[iIRDa-2] = (sms[boardint].sm[i]&0x3000)>>12;
         //irda.Inter[iIRDa-2] = (sms[boardint].sm[i]);
         irda.bc[iIRDa-2] = (sms[boardint].sm[i]&0xfff);
         //printf("%i %i %x\n",i,iIRDa-2,sms[boardint].sm[i]);
        }else{
         printf("getCTPRIRD: error in getting IRDA, iIRDA=%i issm=%i \n",iIRDa,i);
         return list;
        }
        iIRDa++;
        //printf("iIRDA=%i \n",iIRDa);
      }else{
      // CTP readout
       if(iCTPR == 0){
         ctpr.bcid=(sms[boardint].sm[i]&0xfff);
         ctpr.issm=i;
       }else if(iCTPR == 1){
         ctpr.orbit=(sms[boardint].sm[i]&0xfff)<<12;
       }else if(iCTPR == 2){
         ctpr.orbit=ctpr.orbit + (sms[boardint].sm[i]&0xfff);
       }else if(iCTPR == 3){
         ctpr.l2clusters=(sms[boardint].sm[i]&0xfc)>>2;
         l2cl=sms[boardint].sm[i]&0x3;
         ctpr.l2classes=l2cl<<48;
       }else if(iCTPR < 8){
         l2cl=sms[boardint].sm[i]&0xfff;
         ctpr.l2classes=ctpr.l2classes+(l2cl<<(36-(iCTPR-4)*12));
       }else if(iCTPR < (5+8)){
         // L0,L1,L2 INPUTS
       }else{
         printf("getCTPRIRD: error in getting CTPR iCTPR=%i issm=%i \n",iCTPR,i);
         return list;
       }
       iCTPR++;    
       //printf("iCTPR=%i \n",iCTPR);
      }
    } 
   i++;
   }
 }
 return list;
}
/*------------------------------------------------------getL1IRList()
 *  Get list of interactions from L1 board
 */ 
CTPRIRDList *getL1IRList(int offset,int board,CTPRIRDList *list){
 int i,iIRDa,bc=0,first=1;
 int int1,int2;
 IRDa irda;
 Signal *sint1,*sint2;
 sint1=findSignalS(board,0,"int1");
 sint2=findSignalS(board,0,"int2");
 clearIRDa(&irda);
 iIRDa=0;
 i=offset;
 while(i<Mega){
  // Start of orbit
  if(bit(sms[board].sm[i],0) && (bc>50)){
   bc=0;
   if(first) first=0; else{
     list=addIRDa(list,&irda);
     clearIRDa(&irda);
     irda.issm=i-offset;
     iIRDa=0;
   }
  }
  int1=bit(sms[board].sm[i],sint1->channel); 
  int2=bit(sms[board].sm[i],sint2->channel); 
  if(int1 || int2){
   irda.Inter[iIRDa]=int1+2*int2;
   irda.bc[iIRDa]=bc;
   if(iIRDa < 250) iIRDa++;
  }
  bc++;
  i++;
 }
 return list;
}
/*------------------------------------------------------countList()
 * Counting interactions in CTPRIRDList
 */
int countList(CTPRIRDList *list,FILE *file){
   int ibc=0,ic=0;
   CTPRIRDList *loop;
   //printf("CountList started. \n");
   if(!list) return 0;
   loop=list->first;
   while(loop){
    if(loop->irda){
     ibc=0;
     while((loop->irda->Inter[ibc])){
      //printf(" int %i \n",(loop->irda->Inter[ic]));
      ibc++;
     }
     if(!(ic%30))fprintf(file,"\n");
     fprintf(file,"%i ",ibc);
     ic++;
    }
    loop=loop->next;
   }
   fprintf(file,"\n");
 return 0;
}
/*FGROUP IntBoard------------------------------------------------L2a2Interface()
 *  analyze interface board data
 *  L2alist - list of CTP readout from L2 board
 *  INTlist - list of CTP readout and IR data from Interface board
 */
int L2a2Interface(int boardl1,int boardl2,int boardint){
 int offset;
 FILE *file=fopen("IntList.txt","w");
 CTPRIRDList *L2alist=NULL,*INTlist=NULL,*L1list=NULL; 
 // L2 list
 L2alist=getL2alist(boardl2,L2alist);
 fprintf(file,"L2 BOARD: \n");
 printlistN(L2alist,file);
 // INT list
 fprintf(file,"\n INTERFACE BOARD: \n");
 INTlist=getCTPRIRDList(boardint,INTlist);
 printlistN(INTlist,file);
 offset=sms[boardl1].offset-sms[boardl2].offset;
 // L1 list
 fprintf(file,"\n L1 BOARD int1,in2 \n");
 printf("L2a2Interface: offset= %i \n",offset);
 L1list=getL1IRList(offset,boardl1,L1list);
 printlistN(L1list,file);
 // Count intercations for comparison between L1 amd INT boards
 fprintf(file,"L1list count:");
 countList(L1list,file);
 fprintf(file,"INTlist count:");
 countList(INTlist,file);
 // Free memory
 L1list=freenumsN(L1list);
 L2alist=freenumsN(L2alist);
 INTlist=freenumsN(INTlist);
 fclose(file);
 return 0;
}
/*FGROUP IntBoard---------------------------------------------------dumpINTSSM()
 * Dumps Interface board ssm. Word is in output if any bit nonzero.
 * SSM should be read before
 */
int dumpIntSsm(int board){
 int i=0,ic=0;
 //count CTPR and IRDa
 int ictpr=0,iirda=0;
 // find when transmission os going up
 int tenbefore=0;
 w32 blockid=0x8000,error=0x4000,Inter=0x3000,orbbc=0xfff,ctrl=0x10000,ten=0x20000;
 FILE *file=fopen("IntDump.txt","w");
 w32 word;
   while(i<Mega){
     if((word=(sms[board].sm[i]&0x3ffff))){
      blockid=(word&0x8000)>>15;
      error=  (word&0x4000)>>14;
      Inter=  (word&0x3000)>>12;
      orbbc=   word&0xfff;
      ctrl=   (word&0x10000)>>16;
      ten=    (word&0x20000)>>17;
      if(!(ic%30))fprintf(file,"   ISSM  Ctrl Ten BlockID ERROR INT ORBBC ORBBC \n");
      fprintf(file,"%7i    %i   %i     %1i       %1i   %1i  %4i %4x ",i,ctrl,ten,blockid,error,Inter,orbbc,orbbc);
      if(ten) fprintf(file," TRANSMISSION");
      if(ten && blockid) fprintf(file," IRD %i",iirda);
      if(ten && !blockid && !ctrl)fprintf(file," CTPR %i",ictpr);
      if(ctrl) fprintf(file," EOB");
      if(error) fprintf(file," Error");
      if(ten){
       if(ten && !tenbefore){
        if(blockid) iirda++; else ictpr++;
       }
      }
      tenbefore=ten;
      fprintf(file,"\n");
      ic++;
     }
  i++;
 }
 fprintf(file,"#CTPR=%i #IRDa=%i \n",ictpr,iirda);
 fclose(file);
 return 0;
}
/*FGROUP IntBoard
 * get L2 message
 */
int dumpL2amesage(int board){
 FILE *file=fopen("L2amessageList.txt","w");
 CTPRIRDList *L2alist=NULL; 
 L2alist=getL2alist(board,L2alist); 
 fprintf(file,"L2 BOARD: \n");
 printlistN(L2alist,file);
 L2alist=freenumsN(L2alist);
 fclose(file);
 return 0;
}
/*--------------------------------------------------------bcoffset
  This routine is almost same as getL2alist
*/
int bcoffset(int boardl2,int boardint){
 w32 *sm;
 int i=0;
 if((sm=sms[boardl2].sm) == NULL){
   printf("bcoffset: L2 board ssm not read. \n");
   return 1;
 }
 //printf("bcoffset starts. \n");
 while(i<Mega){
  if(sm[i]&2){
   // BC
   i=i+6;
   int j=1,bc=0;
   while(j<13 && (i+j)<Mega){
    bc=bc+(1<<(12-j))*((sm[i+j]&4)==4);
    j++;
   }
   printf("i=%i, bc=%i ",i,bc);
   i=i+j-1;
   int orbit=0;
   j=1;
   while(j<25 && (i+j)<Mega){
    orbit=orbit+(1<<(24-j))*((sm[i+j]&4)==4);
    j++;
   } 
   printf(" orbit=%i ",orbit);
   i=i+j;
  }else i++;
 }
 return 0;
}
/*FFGROUP Generate
*/
void BCoffset2(int bc0,int ntimes){
 // clear  ssm
 int i;
 DAQonoff( 0xb);
 writeSPn(3,0,1,0);
 writeSSM(3);
 //outmon mode
 setsmssw(3,"l2_outmon");
 setsmssw(4,"int_ddldat");
 // start ssm
 for(i=bc0;i<bc0+ntimes;i++){
  setomSSM(3,0x2);
  setomSSM(4,0x2);
  startSSM1(3);
  startSSM1(4);
  // generate sw trigger
  GenSwtrg( 1, 's', 0, i, 2);
  usleep(21000);
  readSSM(3);
  bcoffset(3,4);
  printf("bcin= %i \n",i);
 }
}
/*FFGROUP Generate
   Generate software triggers
*/
void genSW(){
 DAQonoff( 0xb);
 while(1){
  GenSwtrg( 1, 's', 0, 333, 2);
 }
}
