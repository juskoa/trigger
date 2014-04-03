#include "test1.h"
test1::test1()
{
 cout << "test1 constructior" << endl;
 A=new tt[3];
 A[0].a=0;
 A[1].a=1;
 A[2].a=2; 
}
void test1::Printt()
{
 for(int i=0;i<3;i++)cout << A[i].a << ' ';
 cout << endl;
}
int main()
{
 test1 x;
}
