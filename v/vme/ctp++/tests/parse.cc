#include <fstream>
#include <iostream>
#include <string>
using namespace std;
//-----------------------------------------------------------------------
int parse(string fff,int ipos,int level){
 int 
 switch(fff[ipos]){
    case '(':
       level++; ipos++;
       return parse(fff,ipos,level);
    case ')':
       level--,ipos++;
       return 0;
    case  
    default:
     printf("Unexpected char \n");
     return 3;
 }
 return 0;
}
int main(){
 parse("a",0,0);
}
