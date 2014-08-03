#ifndef _ssmrecord_h_
#define _ssmrecord_h_
#include "vmewrap.h"
class ssmrecord
{
  public:
    ssmrecord(w32 issm,w32 data);
    ssmrecord(w32 issm,w16* data,w32 Ndata);
    ssmrecord(w32 issm,w8 ttcode,w8 e,w16 address, w16 tdata, w8 chck);
    ssmrecord(const ssmrecord &obj);
    ssmrecord& operator=(const ssmrecord& rec);
    ~ssmrecord();
    w32 issm; // position in ssm
    w32 data; // orbit
    w16 *sdata;
    w8 ttcode,e;
    w16 address,tdata;
    w8 chck;
  //string name;
};
#endif
