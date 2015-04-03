#include <iostream>
#define Mega 111
typedef int w32;
int main(){
 char filename[200];
 char *environ;
 char fnpath[1024];
 FILE *dump;
 sprintf(filename,"%s","test");
  // Open file
 environ= getenv("VMEWORKDIR"); strcpy(fnpath, environ);
 strcat(fnpath,"/"); strcat(fnpath,"WORK/"); 
 strcat(fnpath, filename); strcat(fnpath, ".dump");
 dump=fopen(fnpath,"w");
 if(dump == NULL){
  printf("Cannot open file: fnpath: %s\n", fnpath);
  exit(1);
 }else{
  printf("File: fnpath %s opened.\n", fnpath);
 }
 for(int i=0; i<Mega; i++) {
    w32 d=1;
    fwrite(&d, sizeof(w32), 1, dump);
  };
 fclose(dump); 
 return 0;
}

