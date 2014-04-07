#include "FOBOARD.h"
FOBOARD::FOBOARD(int vsp,w32 bashex)
:
	BOARD("fo",bashex,vsp,6)
{
 this->AddSSMmode("inmonl0",0); 
 this->AddSSMmode("inmonl1",1); 
 this->AddSSMmode("inmonl2",2); 
 this->AddSSMmode("igl0l1",3); 
 this->AddSSMmode("igl2",4); 
 this->AddSSMmode("outgen",5); 
}

