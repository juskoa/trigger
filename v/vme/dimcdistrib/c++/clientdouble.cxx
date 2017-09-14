#include <cstdio>
#include <dic.hxx>
// ----------------------------------------------
// Client
// ------------------------------------------------
class Test : public DimInfo
{
    void infoHandler()
    {
        *dd = getDouble();
        printf("Time: %f \n",*dd);
    }
    double *dd;
    public :
        Test() : DimInfo("TEST/TEST",-1) {dd = new double;};
}; 
int main()
{
 Test test;
 while(1)
   pause();
 return 0;
}


