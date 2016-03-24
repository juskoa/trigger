#include <stdlib.h>
#include <stdio.h>
#include <string.h>
void printenvironment() {
char *environ;
char msg[800];
environ= getenv("VMEDRIVER"); sprintf(msg, "VMEDRIVER:%s ", environ);
environ= getenv("VMESITE"); sprintf(msg, "%sVMESITE:%s\n", msg, environ);
environ= getenv("VMEBDIR"); sprintf(msg, "%sVMEBDIR:%s\n", msg, environ);
environ= getenv("VMECFDIR"); sprintf(msg, "%sVMECFDIR:%s\n", msg, environ);
environ= getenv("VMEWORKDIR"); sprintf(msg, "%sVMEWORKDIR:%s\n", msg, environ);
environ= getenv("dbctp"); sprintf(msg, "%sdbctp:%s\n", msg, environ);
environ= getenv("DIM_DNS_NODE"); sprintf(msg, "%sDIM_DNS_NODE:%s\n", msg, environ);
printf("%s", msg);
} 
/*------------------------
rc: 0: The content of evirnoment variable 'name' is equal to'value'
*/
int envcmp(char *name, char *value) {
char *environ;
//char msg[400];
environ= getenv(name); 
return(strcmp(environ, value));
//sprintf(msg, "%s=%s\n", name, environ);
}
/* Input: line: command to be executed
Out: result -result
     rc: EXIT_SUCCESS or EXIT_FAILURE
Usage:
#define cmdlen 20
char result[cmdlen], cmd[40];
int rc;
strcpy(cmd,"ls");
// $VMEBDIR//trigdb.py   2>/dev/null   -supress stderr
// $VMEBDIR//trigdb.py   2>&1          -redirect stderr to result
rc= popenread(cmd, result, cmdlen);
if(rc==EXIT_FAILURE) { 
  printf("error. rc:%d\n", rc, cmd);
} else {
  printf("result(len:%d):%s\n", strlen(result), result);
};  
*/
int popenread(char *cmd, char *output, int leng) {
int rc;
FILE *read_fp;
output[0]= '\0'; read_fp= popen(cmd,"r");
if(read_fp==NULL){
  return(EXIT_FAILURE);
};
rc= fread(output, sizeof(char), leng-1, read_fp);
output[rc]='\0';
pclose(read_fp);
return(EXIT_SUCCESS);
}
int isArg(int argc, char **argv, char *isin) {
int ix;
for(ix=0; ix<argc; ix++) {
  //printf("%d:%s\n", ix, argv[ix]);
  if(strcmp(argv[ix], isin)==0) {
    return(ix);
  };
}; return(0);  // program name or not found
}

