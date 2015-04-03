#include <cstdio>
#include <dis.hxx>
#include <sstream>
#include <string>
struct ScopeData{
 double time;
 double val1,val2,val3;
};
int main()
{
 //char *format={"D:4"};
 char format[4];
 char sco[256];
 strcpy(format,"D:4");
 printf("format= %s \n",format);
 ScopeData* a = new ScopeData;
 int asize=  sizeof(*a);
 //DimService scope("ScopeServer/SIGMAS",format,(void*)a,asize);
 DimService scope("ScopeTest/SIGMAS",sco);
 DimServer::start("ScopeTest"); 
 while(1){
   //a->time=time(0);
   int ttt=time(0);
   double tt=(ttt % 10000);
   std::ostringstream ss;
   ss << tt;
   float a=1./3.;
   ss << " " << a;
   std::string sss(ss.str());
   strncpy(sco,sss.c_str(),256);
   printf("time=%s \n",sco);  
   scope.updateService();
   usleep(3000000);
 };
 return 0;
}


