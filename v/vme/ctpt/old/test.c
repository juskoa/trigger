#include <stdio.h>
int main(){
	char bb[]={};
	char a;
	int i;
	i=0;
	printf("1<<2=%i \n",1<<2);
	return;
	printf("bb[0] %c \n",bb[0]);
	while(bb[i] != '\0') {
		printf("%i %c\n",i,bb[i]);
		i++;
	}
	printf("strlen= %i \n",strlen(bb));

	//a='f';
	a=0x66;
	printf("%c %x %i\n",a,a,a);
}
