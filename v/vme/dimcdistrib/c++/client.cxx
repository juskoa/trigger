#include <cstdio>
#include <dic.hxx>
struct ScopeData{
 double time;
 double val1,val2,val3;
};
struct ScopeDimWave{
 float time;
 float t[1024];
 float w[1024];
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
        //double a = getDouble();
        //printf("Data a= %f \n",a);
    }
    ScopeData *a;
    public :
        ScopeSigma() : DimInfo("ScopeServer/SIGMAS",-1) {};
}; 
class ScopeEdge : public DimInfo
{
    void infoHandler()
    {
        a = (ScopeData*) getData();
        printf("Edge: Size of the service %i time= %f %f %f %f\n",getSize(),a->time,a->val1,a->val2,a->val3);
        //double a = getDouble();
        //printf("Data a= %f \n",a);
    }
    ScopeData *a;
    public :
        ScopeEdge() : DimInfo("ScopeServer/EDGES",-1) {};
}; 
class ScopeWave1 : public DimInfo
{
    void infoHandler()
    {
        a = (ScopeDimWave*) getData();
        printf("Edge: Size of the service %i time= %f %f %f %f\n",getSize(),a->time,a->t[0],a->t[1],a->t[2]);
        printf("Edge: Size of the service %i time= %f %f %f %f\n",getSize(),a->time,a->w[0],a->w[1],a->w[2]);
        //double a = getDouble();
        //printf("Data a= %f \n",a);
    }
    ScopeDimWave *a;
    public :
        ScopeWave1() : DimInfo("ScopeServer/WAVE1",-1) {};
}; 

int main()
{
 ScopeSigma scope1;
 ScopeEdge scope2;
 ScopeWave1 scope3;
 while(1)
   pause();
 return 0;
}


