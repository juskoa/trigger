#include <stdio.h>
#include <string.h>
#include "vmewrap.h"
#define CTPMAIN
#include "ctp.h"

/*---------------------------------------------------------- readBICfile
op:
- read ~/root/NOTES/boardsincrate file
- fill in ctpboards[ix].vmever (indicating the presence of the board)
*/
void readBICfile() {
FILE *con;
int nctp;
char line[MAXLINE];
if((con=fopen(BICfile,"r")) == NULL){
  printf("Cannot read %s file. \n", BICfile);
  return;
};
while(fgets(line,MAXLINE,con)!=NULL) {
  /* printf("readBICfiledbg: line:%s\n",line); */
  if(line[0]=='\n') continue;
  if(strncmp(line,"ltu",3)==0) {
    // for ltu do nothing (see initSSM())
    continue;
  } else if(strncmp(line,"fo",2)==0) {
    /* in BICfile, FO boards are placed in ascending order of their
     * base address. They have to have dials: 0-5 (for FO1-FO6).
       From 19.4.2007 we use dials 1-6 for FO1-FO6 */
    nctp= FO1BOARD + (line[7]-'0')-1;
    ctpboards[nctp].vmever=NICRATE;
  } else if(strncmp(line,"busy",4)==0) {
    ctpboards[0].vmever=NICRATE;       /* real one will be read later */
  } else if(strncmp(line,"l0",2)==0) {
    ctpboards[1].vmever=NICRATE;
  } else if(strncmp(line,"l1",2)==0) {
    ctpboards[2].vmever=NICRATE;
  } else if(strncmp(line,"l2",2)==0) {
    ctpboards[3].vmever=NICRATE;
  } else if(strncmp(line,"int",3)==0) {
    ctpboards[4].vmever=NICRATE;
  } else {
    printf("readBICfile: unknown line:%s\n",line);
  };
};
fclose(con);
}
