#ifndef _Dim_h_
#define _Dim_h
#include <dic.hxx>
#include "Log.h"
#include "MonScal.h"

typedef unsigned int w32;
/*
  DIMDNSUNDEF	DIM_FATAL	DIM_DNS_NODE undefined
DIMDNSREFUS	DIM_FATAL	DIM_DNS refuses connection
DIMDNSDUPLC	DIM_FATAL	Service already exists in DNS
DIMDNSEXIT	DIM_FATAL	DNS requests server to EXIT
DIMDNSTMOUT	DIM_WARNING	Server failed sending Watchdog
 	 	 
DIMDNSCNERR	DIM_ERROR	Connection to DNS failed
DIMDNSCNEST	DIM_INFO	Connection to DNS established
 	 	 
DIMSVCDUPLC	DIM_ERROR	Service already exists in Server
DIMSVCFORMT	DIM_ERROR	Bat format string for service
DIMSVCINVAL	DIM_ERROR	Invalid Service ID
 	 	 
DIMTCPRDERR	DIM_ERROR	TCP/IP read error
DIMTCPWRRTY	DIM_WARNING	TCP/IP write error - Retrying
DIMTCPWRTMO	DIM_ERROR	TCP/IP write error - Disconnected
DIMTCPLNERR	DIM_ERROR	TCP/IP listen error
DIMTCPOPERR	DIM_ERROR	TCP/IP open server error
DIMTCPCNERR	DIM_ERROR	TCP/IP connection error
DIMTCPCNEST	DIM_INFO	TCP/IP connection established
*/

class ErrorHandler : public Log, public DimErrorHandler 
{
private:
  void errorHandler(int severity, int code, char *msg);
public:
 ErrorHandler();
};
class OpenDim : public MonScal, public DimInfo
{
 private:
          int count;
 public:
   OpenDim(w32 kPrint,bool copy2dcs);
   void infoHandler();
};
#endif

