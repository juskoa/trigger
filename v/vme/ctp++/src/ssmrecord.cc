#include "ssmrecord.h"
ssmrecord::ssmrecord(w32 issm,w32 data)
:issm(issm),data(data),sdata(0),
ttcode(0),e(0),address(0),tdata(0),chck(0){}
ssmrecord::ssmrecord(w32 issm,w8 ttcode,w8 e,w16 address, w16 tdata, w8 chck)
:issm(issm),data(0),sdata(0),
ttcode(ttcode),e(e),address(address),tdata(tdata),chck(chck){}
ssmrecord::ssmrecord(w32 issm,w16* data,w32 Ndata)
:issm(issm),data(Ndata),sdata(data),
ttcode(0),e(0),address(0),tdata(0),chck(0){}
ssmrecord::~ssmrecord()
{
 if(sdata) delete sdata;
}
ssmrecord::ssmrecord(const ssmrecord &obj)
{
 issm=obj.issm;
 data=obj.data;
 sdata=obj.sdata;
 ttcode=obj.ttcode;
 e=obj.e;
 address=obj.address;
 tdata=obj.tdata;
 chck=obj.chck;
}
ssmrecord& ssmrecord::operator=(const ssmrecord& rec)
{
 if(this != &rec){
  issm=rec.issm;
  data=rec.data;
  sdata=rec.sdata;
  ttcode=rec.ttcode;
  e=rec.e;
  address=rec.address;
  tdata=rec.tdata;
  chck=rec.chck;
 }
 return *this;
}

