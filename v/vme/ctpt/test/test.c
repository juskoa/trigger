int main(){
int a[]={1,2,3};
int b[]={0,0,0};
int i;
i=0;
while(i<3)b[i]=a[i++];
i=0;
while(i<3){printf("%i %i ",i,b[i]);i++;}
printf("\n");
}
