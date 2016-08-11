#ifndef _Dim_h_
#define _Dim_h
#include <dic.hxx>
#include "Log.h"
#include "MonScal.h"

typedef unsigned int w32;
//
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

