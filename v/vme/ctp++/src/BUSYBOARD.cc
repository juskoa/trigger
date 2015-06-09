#include "BUSYBOARD.h"
BUSYBOARD::BUSYBOARD(int vsp)
:
BOARD("busy",0x828000,vsp,4),
//DAQ_BUSY(0x21c)
DAQ_BUSY(0x61c),
SET_CLUSTER(0x600)
{
 this->AddSSMmode("inmon",0);
 this->AddSSMmode("outmon",1);
 this->AddSSMmode("ingen",2);
 this->AddSSMmode("outgen",3);
 this->SetNumofCounters(160);
}
