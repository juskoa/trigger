#include <iostream>
#include <cstdio>
#include <cmath>
#include <stdlib.h>
using namespace std;
int main(){
 long long int k;
 cout << sizeof(k) << endl;
 cout << "Rand max " << log2(RAND_MAX) << endl;
 unsigned int  bc=0;
 bc=(bc-1)%3564u;
 //printf("bc=0x%x \n",bc);
 int a=-1;
 unsigned int b=-1;
 int c=b;
 printf("%i %x %i %i %i\n",a,b,a<0,b<0,c<0);
 return 0;
}

