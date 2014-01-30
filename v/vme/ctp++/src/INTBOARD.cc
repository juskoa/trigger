#include "INTBOARD.h"
// CTPReadout structure
//
class CTPReadOut{
 public:
 	int l2clusters;
 	w64 l2classes1;
	w64 l2classes2;
 	int bcid;
 	int orbit;
	int eob;   // used as eob flag, other items shoyld be zero
 	int esr;
 	int clt;    // calibration trigger
 	int swc;    // software class
 	int issm;   // position of the first word in ssm
};
//Interaction Record
class IRDdata{
 public:
 	int error;
 	int orbit;
 	int Inter[251];
 	int bc[251];
 	int issm;
};

//===========================================================================================================
INTBOARD::INTBOARD(int vsp)
:
	BOARD("int",0x82c000,vsp,4)
{
  this->AddSSMmode("ddldat",0); 
  this->AddSSMmode("ddllog",1); 
  this->AddSSMmode("i2c",2); 
  this->AddSSMmode("inmon",3); 
}
void INTBOARD::getCTPReadOutList()
{
}

