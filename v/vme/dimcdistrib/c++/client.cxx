#include <cstdio>
#include <dic.hxx>
struct ScopeData{
 double time;
 double val1,val2,val3;
};
// ----------------------------------------------
// Client
// ------------------------------------------------
class ScopeSigma : public DimInfo
{
    void infoHandler()
    {
        a = (ScopeData*) getData();
        printf("Sigma: Size of the service %i time= %f %f %f %f\n",getSize(),a->time,a->val1,a->val2,a->val3);
    }
    ScopeData *a;
    public :
        ScopeSigma() : DimInfo("ScopeServer/SIGMAS",-1) {};
}; 

int main()
{
 ScopeSigma scope1;
 while(1)
   pause();
 return 0;
}


