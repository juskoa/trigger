/*REGSTART32 */
#define REGA 0x20
/*REGEND */

extern int quit;
#include <string.h>
#include <stdio.h>
#include <unistd.h>   //usleep
#include "vmewrap.h"
//#include "vmeblib.h"

/*FGROUP
int example(int n, char *string, char c)
int: only >=0
string: \"abc\"
char: 'c'
float: not supported as parameter
*/
int example(int n, char *strg, char c) {
//printf("number:%d fpn:%f strg:%s c:%c\n", n,fpn, strg, c);
printf("number:%d strg:%s c:%c\n", n, strg, c);
return(n);
}
/*FGROUP
float not suported neither in function result
*/
float fexa(int ifpn) {
float rcf;
printf("fexa.fpn:%d\n",ifpn); rcf=ifpn;
return(rcf);
}

void initmain() {
printf("initmain called...\n");
}
void boardInit() {
printf("boardInit called...\n");
}
void endmain() {
printf("endmain called...\n");
}

