int main(){
 int i;	
 int bitsize=3;
 int number=0x4c;
 int lutsize;
 int k,l;
 lutsize=1<<bitsize;
 k=1<<(lutsize-1);
 printf("k=%i \n\n",k);
 for(i=0;i<lutsize;i++){
  if(l=number/k){
   printf("%i \n",lutsize-i-1);
   number=number-k;   
  } 
  k=k/2; 
 }
}
