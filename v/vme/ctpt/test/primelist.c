#include <math.h>
int main(){
 int a,p,i,flag,count=0,flag2=1;
 //p=(1<<21)-1;
 //p=3;
 p=0xffffffff;
 p=0x55555555;
 p=0x11111111;
 p=0x1010101;
 p=0x1001;
 //p=257;
 while(flag2){
  flag2=0;
  a=sqrt((double) p)+1;
  //printf("%i %x \n",p,p);
  //printf("a=%i\n",a);
  flag=1;
  for(i=3;i<a;i=i+2){if(p == (p/i)*i){
    flag=0;
    break;
   }
  //else printf("%i %i\n",i,p- (p/i)*i);
  }
  if(!flag){
   printf("%i %i %i %x\n",p,i,p/i,p/i);
   count++;
  }
  else printf("%i is prime\n",p);
  if((count+1) == 10){
    printf("\n");
    count=0;
  }
  p=p+2;
 }
}
