#include <cstdio>
#include <dis.hxx>
#include <sstream>
#include <string>
int main()
{
 double var;
 DimService omb("TEST/TEST",var);
 DimServer::start("TEST"); 
 while(1){
   int ttt=time(0);
   var=(ttt % 10000);
   omb.updateService();
   printf("TEST/TST publishes: %f \n",var);
   usleep(3000000);
 };
 return 0;
}


