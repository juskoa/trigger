#include "INTBOARD.h"
//===========================================================================================================
INTBOARD::INTBOARD(int vsp)
:
	BOARD("int",0x82c000,vsp,4),
	INT_ORBIT_READ(0x140),
	TCSET(0x400),
	BCOFFSET(0x5a8)
{
  this->AddSSMmode("ddldat",0); 
  this->AddSSMmode("ddllog",1); 
  this->AddSSMmode("i2c",2); 
  this->AddSSMmode("inmon",3); 
  this->SetNumofCounters(19);
}
//=================================================================================
int INTBOARD::CheckCountersNoTriggers()
{
 int ret=0;
 for(int i=0;i<3;i++){
   if(countdiff[i+5] != 0){
      printf("Counter %s != 0 \n",CounterNames[i].c_str());
      ret=1;
    }
 }
 if(ret==0)printf("%s CheckCountersNoTriggers: NO ERROR detected. \n",getName().c_str());

 return ret;
}
//=================================================================================
void INTBOARD::printIRList()
{
 cout << "INTBOARD: Printing IR list, size: " << qirda.size() << endl;
 for(w32 i=0;i<qirda.size();i++)printIRDda(qirda[i]);
}
//================================================================================
void INTBOARD::printReadOutList()
{
 //printf("      SSMBC  BCID BCID   ORBIT  L2Clusters  L2Classes ESR ClT SwC \n");
 cout << "INTBOARD: Printing ReadOut List, size: " << qctpro.size() << endl; 
 for(w32 i=0;i<qctpro.size();i++){
   if(qctpro[i].eob)printf("%7i:EOB \n",qctpro[i].issm);
   else printL2Data(qctpro[i]);
 //else if(!qctpro[i].l2classes1 && !qctpro[i].l2classes2) printf("ORBIT: %7i \n",qctpro[i].issm);
 //else 
 //printf("CTP: %7i %4i 0x%3x %8i 0x%2x 0x%13llx  %1i  %1i  %1i\n",qctpro[i].issm,qctpro[i].bcid,qctpro[i].bcid,qctpro[i].orbit,qctpro[i].l2clusters,qctpro[i].l2classes1,qctpro[i].esr,qctpro[i].clt,qctpro[i].swc);
 }
}
/*
int INTBOARD::checkIR2L2a()
{
 w32 ir=0,l2a=0;
 for(w32 i=0;i<qctpro.size();i++){
    w32 flag=1;
    l2a++;
    for(w32 j=0;j<qirda.size();j++){
     if(qirda[j].orbit == qctpro[i].orbit){
       // assuming ir has to be first
       if(qirda[j].bc[0] == qctpro[i].bcid){
        ir++;
        flag=0;
        break;
       }
     }
    }
    if(flag){
     printf("Orbit/bcid  0x%x found in L2a but not in IR \n",qctpro[i].orbit);
    }
 }
 printf("check IRs versus L2 on INT board: l2a: %i; found in IR: %i \n",l2a,ir);
 return 0;
}
*/
int INTBOARD::checkIR2L2a()
{
 w32 ir=0,l2a=0;
 for(w32 i=0;i<qirda.size();i++){
    if(qirda[i].Inter[0] == 0) continue;
    ir++;
    w32 flag=1;
    for(w32 j=0;j<qctpro.size();j++){
     if(qirda[i].orbit == qctpro[j].orbit){
       // assuming ir has to be first
       if(qirda[i].bc[0] == qctpro[j].bcid){
        l2a++;
        flag=0;
        break;
       }
     }
    }
    if(flag){
     printf("Orbit/bcid  0x%x found in IR but not in L2a \n",qirda[i].orbit);
    }
 }
 printf("check IRs versus L2 on INT board: l2a: %i; found in IR: %i \n",l2a,ir);
 return 0;
}
void INTBOARD::getCTPReadOutList()
{
 int i=0,first=1,firstirda=1;
 int iCTPR=0,iIRDa=0;
 int rc=0;
 // Total Block counters
 int nCTPR=0,nIRDa=0;
 int blockid,eob;
 w64 l2cl;
 IRDda irda;
 clearIRDda(irda);
 L2Data ctpr;
 clearL2Data(ctpr);
 w32 sctrl,sten,sblockid;
 if((sctrl=getChannel("ddl.fb_ctrl"))>32) rc=1;
 if((sten=getChannel("ddl.fb_ten"))>32) rc=2;;
 if((sblockid=getChannel("ddl.fb_d[15]"))>32) rc=3;
 if(rc){
  printf("Error: getCTPReadoutList: rc=%i \n ",rc);
  return;
 }
 // Start to analyse data
 w32* sm=GetSSM();
 while(i<Mega){     
   // Find  end of block
   if(bit(sm[i],sctrl)){
     eob=sm[i]&0xffff;
     if(eob != 0x64){
       printf("getCTPRIRDList: Incorrect EOB in data: 0x%x \n",eob);
       return;
     }
     //eob found and added to list
     if(iIRDa && iCTPR){
      printf("getCTPRIRDList: ERROR iIRDa=%i iCTPR=%i \n",iIRDa,iCTPR);
      return;
     }
     if(!iIRDa && !iCTPR ){
      if(first) first=0; else{
      printf("getCTPRIRDList: ERROR %i iIRDa=%i iCTPR=%i \n",i,iIRDa,iCTPR);
      return;
      }
     }
     if(iIRDa){
      qirda.push_back(irda);
      clearIRDda(irda);
      iIRDa=0;
      nIRDa++;
      firstirda=1;
     }
     if(iCTPR){
      if(iCTPR == (13+5) ){   // Inputs in CTP readout
       qctpro.push_back(ctpr);
       clearL2Data(ctpr);
       iCTPR=0;
       nCTPR++;
      }else{
       printf("getCTPRIRDList: ERROR iCTPR != 13+5: %i \n",iCTPR);
       return ;
      }
     }
     ctpr.eob=1;
     ctpr.issm=i;
     qctpro.push_back(ctpr);
     clearL2Data(ctpr);
     i++;
   }else{
    if(bit(sm[i],sten)){
      blockid=bit(sm[i],sblockid);
      //printf("i=%i blockid=%i iCTPR=%i \n",i,blockid,iCTPR);
      if(blockid){
        // Interaction record
        // Check if beginning of orbit
        if(!(sm[i]&0x3000)){
          if(!firstirda){
	   qirda.push_back(irda);
	   clearIRDda(irda);
           iIRDa=0;
           nIRDa++;
          }else firstirda=0;
        }
        if(iIRDa == 0){
          irda.orbit=(sm[i]&0xfff)<<12;
          irda.error1=(sm[i]&0x3000)>>12;
          irda.issm=i;
        }else if(iIRDa == 1){
          if(!(sm[i]&0x3000)){
           printf("getCTPRIRDList: IR ERROR flag != 1 1\n");
           return;
          }
          irda.orbit=irda.orbit+(sm[i]&0xfff);
          irda.error2=(sm[i]&0x3000)>>12;
        }else if(iIRDa < 253){
         irda.Inter[iIRDa-2] = (sm[i]&0x3000)>>12;
         //irda.Inter[iIRDa-2] = (sm[i]);
         irda.bc[iIRDa-2] = (sm[i]&0xfff);
         //printf("%i %i %x\n",i,iIRDa-2,sm[i]);
        }else{
         printf("getCTPRIRD: error in getting IRDA, iIRDA=%i issm=%i \n",iIRDa,i);
         return;
        }
        iIRDa++;
        //printf("iIRDA=%i \n",iIRDa);
      }else{
      // CTP readout
       if(iCTPR == 0){
         ctpr.bcid=(sm[i]&0xfff);
         ctpr.issm=i;
       }else if(iCTPR == 1){
         ctpr.orbit=(sm[i]&0xfff)<<12;
       }else if(iCTPR == 2){
         ctpr.orbit=ctpr.orbit + (sm[i]&0xfff);
       }else if(iCTPR == 3){      // clusters
         ctpr.l2clusters=(sm[i]&0xff);
       }else if(iCTPR==4){    //  class word 4 [100..97]
         l2cl=(sm[i]&0xf);
         ctpr.l2classes1=l2cl<<36;
       }else if(iCTPR < 8){  // classes words 5,6,7 
         l2cl=sm[i]&0xfff;
         ctpr.l2classes1=ctpr.l2classes1+(l2cl<<(7-iCTPR)*12);
       }else if(iCTPR<13){   // classes words 8-12
         l2cl=sm[i]&0xfff;
         ctpr.l2classes2=ctpr.l2classes2+(l2cl<<(12-iCTPR)*12);
       }else if(iCTPR < (13+5)){
         // L0,L1,L2 INPUTS
       }else{
         printf("getCTPRIRD: error in getting CTPR iCTPR=%i issm=%i \n",iCTPR,i);
         return;
       }
       iCTPR++;    
       //printf("iCTPR=%i \n",iCTPR);
      }
    } 
   i++;
   }
 }
 return ;
}


