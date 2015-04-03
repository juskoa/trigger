#include <list>
class cl{
};
using namespace std;
int main(){
 list<cl*> test;
 int a=2;
 int *p;
 p=&a;
 cl *aa;
 aa=new(cl);
 test.push_back(aa);
}

