#include <cstdio>
#include <dic.hxx>
struct ScopeData{
 double time;
 double val1,val2,val3;
};
// ----------------------------------------------
// Client
// ------------------------------------------------
class Scope : public DimInfo
{
    void infoHandler()
    {
        a = getString();
        printf("Time: %s \n",a);
    }
    char *a;
    public :
        Scope() : DimInfo("ScopeTest/SIGMAS",-1) {a = new char[256];};
}; 
int main()
{
 Scope scope;
 while(1)
   pause();
 return 0;
}


