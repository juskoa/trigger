#include <stdio.h>
int main()
{
 int j;
 unsigned long long int i;
 double d;
 printf("size of int %i\n",sizeof(int));
 printf("size of long int %i\n",sizeof(long int));
 printf("size of long long int %i\n",sizeof(long long int));
 printf("size of double %i\n",sizeof(double));
 printf("size of unsigned int %i\n",sizeof(unsigned int));
 printf("d=%f \n",d);
 i=1;
 for(j=0;j<50;j++){
  printf("%i %llx \n" ,j,i);
  i=i*2;
 }
 /*
 i=1<<31;
 d=1.;
 for(i=0;i<20;i++){
  d=d*(256.);
  printf("%i %f \n",i,d);
 }
 for(i=0;i<50;i++){
  d=d/(256.);
  printf("%i %f \n",i,d);
 }
 */
}
