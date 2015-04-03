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
    }
    ScopeData *a;
    public :
        Scope() : DimInfo("SCOPE/SIGMAS",-1) {};
}; 
int main()
{
 Scope scope;
 while(1)
   pause();
 return 0;
 char format[4];
 strcpy(format,"D:4");
 printf("format= %s \n",format);
 ScopeData* a = new ScopeData;
 int asize=  sizeof(a);
 DimInfo scope2("SCOPE/SIGMAS",(void*)a,asize);
 printf("ScopeData: %f %f %f %f \n",a->time,a->val1,a->val2,a->val3);
 return 0;
}


