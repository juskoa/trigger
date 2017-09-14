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
 char format[4];
 char sco[256];
 strcpy(format,"D:4");
 printf("format= %s \n",format);
 DimService scope("ScopeTest/SIGMAS",sco);
 DimServer::start("ScopeTest"); 
 while(1){
   //a->time=time(0);
   int ttt=time(0);
   double tt=(ttt % 10000);
   std::ostringstream ss;
   ss<<tt;
   std::string sss(ss.str());
   strncpy(sco,sss.c_str(),256);
   printf("time=%s \n",sco);  
   scope.updateService();
   usleep(3000000);
 };
 return 0;
}


