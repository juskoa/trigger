#include "BUSYBOARD.h"
BUSYBOARD::BUSYBOARD(int vsp)
:
BOARD("busy",0x828000,vsp,4),
//DAQ_BUSY(0x21c)
DAQ_BUSY(0x61c),
SET_CLUSTER(0x600),
DELAY_ADD(0x4c8),
ORBIT_SELECT(0x0d4),
BUSYMAX_DATA(0x168),
BUSYMINI_DATA(0x16c),
MINIMAX_SELECT(0x570),
MINIMAX_CLEAR(0x174),
MINIMAX_LIMIT(0x578),
BUSYLAST_SELECT(0x57c),
BUSY_CLUSTER(0x600)
{
 this->AddSSMmode("inmon",0);
 this->AddSSMmode("outmon",1);
 this->AddSSMmode("ingen",2);
 this->AddSSMmode("outgen",3);
 this->SetNumofCounters(160);
}
//-------------------------------------------------
// Measure phase wrt to BC. See documentation on ORBIT_SELECT
void BUSYBOARD::measurephase()
{
 for(w32 i=0;i<32;i++){
   vmew(DELAY_ADD,i);
   printf("Delay: %i %x \n",vmer(DELAY_ADD),vmer(ORBIT_SELECT));
   usleep(500);
 }
}
