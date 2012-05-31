#include <stdio.h>
int main(){
 char test[]={' ','a',' ','b',' ','1','\0'};
 printf("%s \n",test);
 removespace(test);
 printf("%s \n",test);
}
int removespace(char *line){
 char *a;
 printf("rs: %s \n",line);
 a=line;
 while( (*a != ' ') && (*a != '\0'))a++;
 if( *a == '\0')return 1;
 while( *a != '\0')*a=*(a++ +1);
 removespace(line);
 return -1;
}
int findspace(char *line){
 char *a;
 a=line;
 while(*a != '\0'){
   if(*a ==  ' '){
    char *b;
    b=a+1;
    while(*a != '\0' )*a++=*b++;	  
    return 1;
   }
   a++; 
 }
 return 0;
}

