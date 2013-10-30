#ifndef _BOARDBASIC_h
#define _BOARDBASIC_h_
#include "vmewrap.h"
#include "libctp++.h"
#include <cstdlib>
class BOARDBASIC 
{
 public: 
         BOARDBASIC();
	 BOARDBASIC(string const name,w32 const boardbase,int vsp);
	 ~BOARDBASIC();
	 void vmew(w32 const address,w32 const word) const;
	 w32 vmer(w32 const address) const;
	 int getvsp() const;
         w32 getboardbase() const;
         w32 getBCstatus() const;
	 string const d_name;        // l0,l1,l2,fo,busy,ltu
         string error_status;    // "ok" = no error
         void printboardinfo(string const &option) const;
 private:
         w32 d_codeaddress;
         w32 d_serialnumber;
	 w32 d_vmeversion;
	 w32 d_fpgaversion;   
         // VME stuff
	 int d_vsp;                  // vme space number
	 w32 const d_boardbase;            // l0: 0x829000,l1:0x02a000
         w32 d_vmebasehex;           // CTP: 0x820000, ltu:0x810000
         int vmeopenflag;
	 int OpenVME(char *vmebase,char *size) const;
	 void CloseVME() const; 
         // VME addresses common forl all boards
         w32 const CODE_ADD;
         w32 const SERIAL_NUMBER;
         w32 const VERSION_ADD;
	 w32 const FPGAVERSION_ADD;
         w32 const BC_STATUS;
};

#endif
