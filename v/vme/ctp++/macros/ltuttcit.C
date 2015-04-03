#include "TTCITBOARD.h"
#include "LTUBOARD.h"
/*
 * TTC analysis from begining of sequence
 * Prepare LTU: load slm sequence
 *              set  rate
 *              DO NOT start slm !
 */
int main(){
 int vmesp=-1;
 string boardname("ttcit");
 TTCITBOARD *ttc= new TTCITBOARD(boardname.c_str(),0x8a0000,vmesp);
 vmesp=-1;
 string ltuname("ltu");
 LTUBOARD *ltu= new LTUBOARD(ltuname.c_str(),0x812000,vmesp);

 printf("vsp= %i \n",ttc->getvsp());
 w32 ver= ttc->getFPGAversion();
 printf("Version: 0x%x %i\n",ver,ver);
 ltu->ClearFIFOs();
 if(ttc->start_stopSSM(ltu)) return 1;
 ttc->DumptxtSSM();
 ttc->Dump2quSSM();
 ttc->AnalyseSSM();
 printf("FIFO MAX: 0x%x \n",ltu->GetFIFOMAX());
 return 0;
}
