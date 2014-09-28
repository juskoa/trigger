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
        a = (ScopeData*) getData();
        printf("Size of the service %i time= %f %f %f %f\n",getSize(),a->time,a->val1,a->val2,a->val3);
        //double a = getDouble();
        //printf("Data a= %f \n",a);
    }
    ScopeData *a;
    public :
        Scope() : DimInfo("ScopeServer/SIGMAS",-1) {};
}; 
int main()
{
 Scope scope;
 while(1)
   pause();
 return 0;
}


