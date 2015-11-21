#include "BOARDBASIC.h"
BOARDBASIC::BOARDBASIC(string const name,w32 const boardbase,int vsp)
:
 debug(0),
 d_name(name),
 d_boardbase(boardbase),
 CODE_ADD(0x4),
 SERIAL_NUMBER(0x8),
 VERSION_ADD(0xc),
 FPGAVERSION_ADD(0x80),
 BC_STATUS(0xc4)
{
 cout << showbase;
 if((!name.compare(0,3,"ltu")) || (!name.compare(0,3,"LTU")) ){
   d_vmebasehex= 0x810000; 
   if(vsp != -1){
     d_vsp=vsp;
     vmeopenflag=0;
   }else{
     char base[]="0x810000";
     char length[]="0xffff";
     d_vsp=OpenVME(base,length);
     vmeopenflag=1;
   }
 }else if(!name.compare(0,3,"ttc")){
   d_vmebasehex=0x8a0000;
   if(vsp != -1){
     d_vsp=vsp;
     vmeopenflag=0;
   }else{
     char base[]="0x8a0000";
     char length[]="0xffff";
     d_vsp=OpenVME(base,length);
     vmeopenflag=1;
   }  
 }else {
   d_vmebasehex=0x820000;
   if(vsp != -1){ 
     d_vsp=vsp;
     vmeopenflag=0;
   }else{
     char base[]="0x820000";
     char length[]="0xd000";
     d_vsp=OpenVME(base,length);
     vmeopenflag=1;
   }
 }
 d_codeaddress=vmer(CODE_ADD)&0xff;
 d_serialnumber=vmer(SERIAL_NUMBER)&0xff;
 d_vmeversion=vmer(VERSION_ADD)&0xff;
 //d_fpgaversion=vmer(FPGAVERSION_ADD)&0xff;
 d_fpgaversion=vmer(FPGAVERSION_ADD);
}
//----------------------------------------------------------------------------
BOARDBASIC::~BOARDBASIC(){
 if(vmeopenflag){
   cout << "Closing VME: " << d_vsp << " ----------------------VMEclosed" << endl;
   CloseVME();
 }
}
//----------------------------------------------------------------------------
void BOARDBASIC::printboardinfo(string const &option) const
{
 cout << d_name << ":";
 if(option == "with text"){
   cout << "code add:" << hex  << d_codeaddress ;
   cout << " ser num:" << hex  << d_serialnumber;
   cout << " vme base:" << hex << d_boardbase;
   cout << " vme ver:" << hex  << d_vmeversion;
   cout << " fpga ver: " << hex << d_fpgaversion;
   cout << " BCstatus:" << hex << getBCstatus() << endl;
 }else{
   cout << hex << d_codeaddress ;
   cout << " " << hex << d_serialnumber;
   cout << " " << hex << d_boardbase;
   cout << " " << hex << d_vmeversion;
   cout << " " << hex << d_fpgaversion;
   cout << " " << hex << getBCstatus() << endl;
 }
}
//---------------------------------------------------------------------------
// For documentation see vmewrap.c
int BOARDBASIC::OpenVME(char *vmebase,char *size) const
{
 int vsp=-1;
 int rc=vmxopen(&vsp,vmebase,size);
 if(rc == 0){ 
   cout << "BOARDBASIC: board " << d_name; 
   cout << " OpenVME success, vsp= " << vsp << " ";
   cout << vmebase << " " << size << endl;
   return vsp;
 }
 else {
  cout << "BOARDBASIC: cannot open vme (see vmewrap for error code): "<< rc << endl;
 exit(1); 
 }
}
//----------------------------------------------------------------------------	
void BOARDBASIC::CloseVME() const
{
 vmxclose(d_vsp);
}
//----------------------------------------------------------------------------
void BOARDBASIC::vmew(w32 const address,w32 const word) const
{
 vmxw32(d_vsp,d_boardbase-d_vmebasehex+address,word);	
}
//----------------------------------------------------------------------------
w32 BOARDBASIC::vmer(w32 const address) const
{
 //cout << hex << d_boardbase << " " << hex << d_vmebasehex << endl;
 return vmxr32(d_vsp,d_boardbase-d_vmebasehex+address);
}
//----------------------------------------------------------------------------
int BOARDBASIC::getvsp() const
{
 return d_vsp;
}
//-----------------------------------------------------------------------------
w32 BOARDBASIC::getBCstatus() const
{
 return vmer(BC_STATUS);
}
//----------------------------------------------------------------------------
w32 BOARDBASIC::getboardbase() const
{
 return d_boardbase;
}
