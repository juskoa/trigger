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
 strcpy(format,"D:4");
 ScopeData* a = new ScopeData;
 int asize=  sizeof(*a);
 DimService scope("ScopeServer/SIGMAS",format,(void*)a,asize);
 DimServer::start("ScopeTest");
 int nn=0; 
 while(1){
   int ttt=time(0);
   double tt=(ttt % 10000);
   a->time=tt;
   a->val1=nn;
   a->val2=2*nn;
   a->val3=3*nn;
   printf("time=%f %f %f %f \n",a->time,a->val1,a->val2,a->val3);  
   scope.updateService();
   usleep(3000000);
   nn++;
 };
 return 0;
}


