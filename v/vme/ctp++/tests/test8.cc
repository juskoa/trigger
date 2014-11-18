#include <cstdio>
class base
{
 public:
 base(){};
 virtual void met1()=0;
 void met2(){ met1();};
};
class child1:public base
{
 public:
 child1();
 void met1();
 int const i;
};
child1::child1():
base(),
i(66)
{
}
void child1::met1()
{
 printf("I am child1 i=%i \n",i);
}
//------------------------------------------------
class child2:public base
{
 public:
 void met1();//{printf("I am child2 \n");};
};
int main()
{
 base *aa = new child1;
 aa->met1();
 return 0;
}
 
