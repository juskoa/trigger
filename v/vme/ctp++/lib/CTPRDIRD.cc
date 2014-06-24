#include "libctp++.h"
void clearIRDda(IRDa &i)
{
 i.error1=0;
 i.error2=0;
 i.incomplete=0;
 i.orbit=0;
 i.issm=0;
 for(int j=0;j<251;j++){
  i.Inter[j]=0;
  i.bc[j]=0;
 }
}
void printIRDda(IRDa &i)
{
 printf("%7i:IR ORBIT:0x%6x\n",i.issm,i.orbit);
 for(int j=0;j<251;j++){
    if(i.Inter[j])printf("       %03i BCID:0x%3x 0x%1x Errs: 0x%1x 0x%1x\n",j,i.bc[j],i.Inter[j],i.error1,i.error2);
 }
}
void printL2Data(L2Data& c)
{
 printf("%7i:CLST:0x%02x CLS:0x%010llx%015llx ID:0x%3x 0x%6x eob/esr/clt/sec:%1i %1i %1i %1i\n",c.issm,c.l2clusters,c.l2classes1,c.l2classes2,c.bcid,c.orbit,c.eob,c.esr,c.clt,c.swc);
}
void clearL2Data(L2Data &c)
{
 c.l2clusters=0;
 c.l2classes1=0; c.l2classes2=0;
 c.bcid=0; c.orbit=0;
 c.eob=0;c.esr=0;c.clt=0;c.swc=0;
 c.issm=0;
}

