#include "ssmconnection.h"
/*--------------------------------------------------------------------
*/
typedef struct Data{
 w32 bcid;
 w32 orbit;
 w32 swtr;
 w32 esr;
 w32 clas[NCLASS];
 w32 clust[7];   // 0=test class
 w32 roc;
 w32 clt;
 w32 l2arf;
}Data;
void cleanData(Data *data){
 int i;
 data->bcid=0;
 data->orbit=0;
 data->swtr=0;
 data->esr=0;
 data->roc=0;
 data->clt=0;
 data->l2arf=0;
 for(i=0;i<NCLASS;i++)data->clas[i]=0;
 for(i=0;i<7;i++)data->clust[i]=0;
}

/*GROUP DATA
  Get l0 backplane data
  board: board number
  ndata: # of data you want to analyse
*/
Data *getL0data(int board,int ndata){
 int i,j,idata=0;
 int l0datachan,l0strobechan;
 w32 *sm;
 if(sms[board].sm == NULL){
  printf("getL0data: board %i memory not read (==NULL)\n",board);
  return NULL;
 }
 sm=sms[board].sm;
 // Find channels : l0data
 if(board == 1){
   l0strobechan=8;
   l0datachan=9;
 }else if(board == 2){
    l0strobechan=3;
    l0datachan=4;
 }else{
  printf("getL0data: board %i does not have chanbel l1 data\n",board);
  return NULL;
 }
 //Create Data
 Data *l0data= (Data *) malloc(sizeof(Data));
 cleanData(l0data);
 i=0;
 while(i<Mega){
  if(idata>=ndata)break;
  if(bit(sm[i],l0strobechan)){
    // L0SwC
    l0data->swtr=l0data->swtr+bit(sm[i],l0datachan);
    // L0 classes
    i++;
    j=0;
    while((j<NCLASS) && ((i+j)<Mega)){
      l0data->clas[49-j]=l0data->clas[49-j]+bit(sm[i+j],l0datachan);
      j++;
    }
    idata++;
    i=i+50;
  }else i++;
 }
 return l0data;
}
/*---------------------------------------------------------------------------
 Get l1 backplane data
*/
Data *getL1data(int board,int ndata){
 int i,j,idata=0;
 int l1datachan,l1strobechan;
 w32 *sm;
 if(sms[board].sm == NULL){
  printf("getL1data: board %i memory not read (==NULL)\n",board);
  return NULL;
 }
 sm=sms[board].sm;
 // Chaneels
 if((board == 2) || (board == 5)){
   l1strobechan=8;
   l1datachan=9;
 }else if(board == 3){
    l1strobechan=3;
    l1datachan=4;
 }else{
  printf("getL0data: board %i does not have chanbel l1 data\n",board);
  return NULL;
 }
 Data *l1data = (Data *) malloc(sizeof(Data));
 cleanData(l1data);
 i=0;
 while(i<Mega){
   if(idata>=ndata)break;
   if(bit(sm[i],l1strobechan)){
   // ESR
   l1data->esr=l1data->esr+bit(sm[i],l1datachan);
   // L0SwC
   l1data->swtr=l1data->swtr+bit(sm[i+1],l1datachan);
   // L1 classes
   i=i+2;
   j=0;
   while((j<NCLASS) && ((i+j)<Mega)){
     l1data->clas[49-j]=l1data->clas[49-j]+bit(sm[i+j],l1datachan);
     j++;
   }
   idata++;
   i=i+50;
  }else i++;
 }
 return l1data;
}
/*---------------------------------------------------------------------------
 Get l2 backplane data (between L2 and FO)
 All boards use data channels 2,3 so dont need to play with channels
*/
Data *getL2data(int board,int ndata){
 int i,j,idata=0;
 w32 bcid,orbit;
 w32 *sm;
 int l2strobechan,l2data1chan,l2data2chan;
 if(sms[board].sm == NULL){
  printf("getL2data: board %i memory not read (==NULL)\n",board);
  return NULL;
 }
 sm=sms[board].sm;
 // Find channels
 l2strobechan=1;
 l2data1chan=2;
 l2data2chan=3;
 // Data
 Data *l2data= (Data *) malloc(sizeof(Data));
 cleanData(l2data); 
 i=0;
 while(i<Mega){
  if(idata>=ndata)break;
  if(bit(sm[i],l2strobechan)){
   // L2 test cluster position 1
   l2data->clust[0]=l2data->clust[0]+bit(sm[i],l2data1chan);
   i++;
   // L2 clusters  position 2
   j=0;
   while((j<6) && ((i+j)<Mega)){
    l2data->clust[6-j]=l2data->clust[6-j]+bit(sm[i+j],l2data1chan);
    j++;
   }
   i=i+6;
   // BCID   position 8 
   bcid=0;
   j=0;
   while((j<12) && ((i+j)<Mega)){
    bcid=bcid+bit(sm[i+j],l2data1chan)*(1<<(11-j));
    j++;
  }
  l2data->bcid=bcid;
  i=i+12;
  //Orbit position 20
  orbit=0;
  j=0;
  while((j<24) && ((i+j)<Mega)){
      orbit=orbit+bit(sm[i+j],l2data1chan)*(1<<(23-j));
      j++;
  }
  l2data->orbit=orbit;
  i=i+24;
  //ESR position 44
  l2data->esr=l2data->esr+bit(sm[i],l2data1chan);
  // gap 
  i=i+11;
  // Classes position 55
   j=0;
   while((j<NCLASS) && ((i+j)<Mega)){
    l2data->clas[49-j]=l2data->clas[49-j]+bit(sm[i+j],l2data2chan);
    j++;
   }
  i=i+50;
  idata++;
  printf("L2data :bcid=%i orbit=%i\n",bcid,orbit);
  }else i++;
 }
 return l2data;
}
/*---------------------------------------------------------------------------
*/
void printdata(Data *l0,Data *l1,Data *l2,Data *l1ser,Data *l2ser){
 int i;
 printf("BOARD:     L0   L1   L2   FO  LTU\n");
 for(i=0;i<NCLASS;i++){
 printf("CLASS[%2i]: %2i   %2i   %2i   %2i   %2i\n",i+1,l0->clas[i],l1->clas[i],l2->clas[i],l1ser->clas[i],l2ser->clas[i]);
 }
 printf("SWTRG:     %2i   %2i    -   %2i   %2i\n",l0->swtr,l1->swtr,l1ser->swtr,l2ser->swtr);
 for(i=0;i<7;i++){
  printf("CLUST[%i]:   -    -   %2i   -    %2i\n",i,l2->clust[i],l2ser->clust[i]);
 }
 printf("ESR  :      -   %2i   %2i   %2i   %2i\n",l1->esr,l2->esr,l1ser->esr,l2ser->esr);
 printf("\n");
}
/*----------------------------------------------------------------
*/
Data *getL1serialdata(int board,int ndata){
 int i,j,idata=0;
 int l1strobechan,l1datachan;
 w32 *sm;
 if(sms[board].sm == NULL){
  printf("L1serialdata: board %i not read \n",board);
  return NULL;
 }
 sm=sms[board].sm;
 if(board >= 11){
   l1strobechan=3;
   l1datachan=4;
 }else if(board == 5){ 
   l1strobechan=10;   //assuming first connector
   l1datachan=14;
 }else{
  printf("getL1serialdata: board %i has noe L1 serial data\n",board);
  return NULL;
 }
 Data *l1data = (Data *)malloc(sizeof(Data));
 cleanData(l1data);
 i=0;
 while(i<Mega){
  if(idata>=ndata)break;
  if(bit(sm[i],l1strobechan)){
   //Spare  1
   i++;
   //Clt    2
   l1data->clt=l1data->clt+sm[i];
   i++;
   // Roc    3
   l1data->roc=sm[i]*8+sm[i+1]*4+sm[i+2]*3+sm[i+3]*1;
   i=i+4;
   // esr    7
   l1data->esr=l1data->esr+sm[i];
   i++;
   //Swc     8
   l1data->swtr=l1data->swtr+sm[i];
   i++;
   //Classes  9
   j=0;
   while((j<50) && ((i+j)<Mega)){
    l1data->clas[49-j]=l1data->clas[49-j]+bit(sm[i+j],l1datachan);
    j++;
   }
   idata++;
   i=i+50;
  }else i++;
 }
 return l1data;
}
/*GROUP 
 Get l2 serial data (between FO and LTU)
*/
Data *getL2serialdata(int board,int ndata){
 int i,j,idata=0;
 w32 bcid,orbit;
 w32 *sm;
 int l2strobechan,l2datachan;
 if(sms[board].sm == NULL){
  printf("L2message: board %i not read \n",board);
  return NULL;
 }
 sm=sms[board].sm;
 if(board >= 11){
   l2strobechan=5;
   l2datachan=6;
 }else if(board == 5){
   l2strobechan=10;  // first connector
   l2datachan=14; 
 }else{
   printf("getL2serialdata: board %i does not have l2 serial data\n",board);
   return NULL;
 }
 Data *l2data = (Data *)malloc(sizeof(Data));
 cleanData(l2data);
 i=0;
 while(i<Mega){
  if(idata>=ndata) break;
  if(bit(sm[i],l2strobechan)){
    //L2arF 1
    l2data->l2arf=bit(sm[i],l2datachan);
    i++;
    // BCID 2;
    bcid=0;
    j=0;
    while((j<12) && ((i+j)<Mega)){
     bcid=bcid+bit(sm[i+j],l2datachan)*(1<<(11-j));
     j++;
    }
    i=i+12;
    //ORBIT  14
    j=0;
    orbit=0;
    while((j<24) && (i+j)<Mega){
      orbit=orbit+bit(sm[i+j],l2datachan)*(1<<(23-j));
      j++;
    }
    i=i+24;
    // Spare 38
    i++;
    // esr   39
    l2data->esr=l2data->esr+bit(sm[i],l2datachan);
    i++;
    // clt  40
    l2data->clt=l2data->clt+bit(sm[i],l2datachan);
    i++;
    //L2swc 41
    l2data->swtr=l2data->swtr+bit(sm[i],l2datachan);
    i++;
    // L2clust 42
    j=0;
    while((j<6) && ((i+j)<Mega)){
     l2data->clust[6-j]=l2data->clust[6-j]+bit(sm[i+j],l2datachan);
     j++;
    }
    i=i+6;
    // L2classes 48
    j=0;
    while((j<NCLASS) && ((i+j)<Mega)){
      l2data->clas[49-j]=l2data->clas[49-j]+bit(sm[i+j],l2datachan);
      j++;
    } 
    printf("L2serial data: bcid=%i Orbit=%i \n",bcid,orbit);
    i=i+50;
    idata++;
  }else i++;
 }
 return l2data; 
}
/*--------------------------------------------------------------alignStrobes()
 Align offsets for l0,l1,l2,fo,ltu ssms in order to see all
 messages (l0data,l1data,l2data,data on fo nad l1,l2 messages)
*/
int alignStrobes(int n, int *boards){
 int i;
 w32 offset;
 Signal *sig;
 for(i=0;i<n;i++){
  if(boards[i] == 1){
     sig=findSignalS(boards[i],0,"l0strobe");
     offset=syncSIG1(sms[boards[i]].sm,sig->channel);
     if(offset < 0) continue;
     sms[boards[i]].offset=offset-1;
  }else if(boards[i] == 2){
     sig=findSignalS(boards[i],0,"l1strobe");
     offset=syncSIG1(sms[boards[i]].sm,sig->channel);
     sms[boards[i]].offset=offset-1;  
  }else if(boards[i] == 3){
     sig=findSignalS(boards[i],0,"l2strobe");
     offset=syncSIG1(sms[boards[i]].sm,sig->channel);
     sms[boards[i]].offset=offset-1;
  }else if(boards[i] == 5){
     sig=findSignalS(boards[i],0,"l1strobe");
     offset=syncSIG1(sms[boards[i]].sm,sig->channel);
     sms[boards[i]].offset=offset-1;
  }else if(boards[i] == 11){
     sig=findSignalS(boards[i],0,"FPL1");
     offset=syncSIG1(sms[boards[i]].sm,sig->channel);
     sms[boards[i]].offset=offset;
  }
 }
 return 0;
}
/*---------------------------------------------------------printALL()
*/
int printALL(){
 Data *l0data,*l1data,*l2data,*l1serialdata,*l2serialdata;
 l0data=getL0data(1,10);
 l1data=getL1data(2,10);
 l2data=getL2data(3,10);
 l1serialdata=getL1serialdata(5,10);
 l2serialdata=getL2serialdata(11,10);
 printdata(l0data,l1data,l2data,l1serialdata,l2serialdata);
 return 0; 
}

