#include <stdio.h>
#include <string.h>
#include "vmewrap.h"
#include "ctplib.h"

#define MAXNAMELEN 32
#define MAXITEMS 500
#define MAXALIASES 300
// average number of aliases per class
#define AVAPC 5
#define MAXLINELENGTH 200

int aliasNamesN=0;   // number of items in aliasNames
char aliasNames[MAXALIASES][MAXNAMELEN];
// aliases lists, each list finished by -1. Start of the list
// is pointed by array[].value
int cls2aliasesN=0;   // number of items in cls2aliases
int cls2aliases[MAXITEMS*(AVAPC+1)];   

int arrayN=0;   // number of items in array
typedef struct item {
  char name[MAXNAMELEN];
  int value;   // 0.. points to cls2aliases
} Titem;
Titem array[MAXITEMS];

/* rc: -1: not found, >=0: index to aliasNames
int findAlias(char *alname) {
int i;
for(i=0;i<aliasNamesN;i++) {  
  if(strcmp(aliasNames[i], alname)==0) {
    return(i);
  };
}; return(-1);
}*/
// rc: -1: not found or added, i.e. error. >=0: index to aliasNames
int findAddAlias(char *alname) {
int i;
for(i=0;i<aliasNamesN;i++) {  
  if(strcmp(aliasNames[i], alname)==0) {
    return(i);
  };
};
if(aliasNamesN < MAXALIASES) {
  aliasNamesN++; strcpy(aliasNames[aliasNamesN], alname);
  return(aliasNamesN);
};
printf("ERROR: findAddAlias() too many aliases. MAXALIASES:%d\n",MAXALIASES);
return(-1);
}

void bubble(Titem *a,int n)  {  
int i,j;
for(i=n-2;i>=0;i--) {  
  int nexch=0;
  for(j=0;j<=i;j++) {  
    if(strcmp(a[j].name, a[j+1].name)>0) {  
      Titem t;
      strcpy(t.name, a[j].name); t.value= a[j].value; 
      strcpy(a[j].name, a[j+1].name);  
      strcpy(a[j+1].name, t.name);  a[j+1].value= t.value;  
      nexch++;
    };
  }; 
  //printf("i:%11d nexch:%4d\n", i, nexch);
  if(nexch==0) break;
}
}

int readAliases() {
FILE *alfi;
char line[MAXLINELENGTH];
alfi= openFile("aliases.txt","r");
//alfi= fopen("testa.txt","r");
//alfi= fopen("aliases.txt","r");
while(fgets(line, MAXLINELENGTH, alfi)){
  int rc,ix;
  int aliasn,aliasix;
  char clsname[MAXNAMELEN];
  char apart[MAXLINELENGTH];
  char aliases[MAXALIASES][MAXNAMELEN];
  rc= sscanf(line, "%s %s", clsname, apart);
  //printf("%d:%s:%s:\n", rc,clsname, apart);
  if(rc!=2) {
    printf("ERROR: bad line:%s\n", line); continue;
  };
  aliasn=0; aliasix=0; ix=0;
  while(1) {
    char c; 
    c= apart[ix];
    if((c==',') || (c=='\0')) {
      aliases[aliasn][aliasix]= '\0';
      aliasn++; aliasix=0;
    } else {
      aliases[aliasn][aliasix]= c;
      aliasix++;
    };
    if(c=='\0') break;
    ix++;
  };
  /*
  clsname -class name
  aliases[0..aliasn-1] -corresponding alias' names
  */
  if(arrayN<MAXITEMS) {
    strcpy(array[arrayN].name, clsname);
    array[arrayN].value= cls2aliasesN; arrayN++;
  } else {
    printf("ERROR readAliases() too many classes. MAXITEMS:%d\n",MAXITEMS);
    return(-1);
  };
  for(ix=0; ix<aliasn; ix++) {
    //printf("%8d: %s\n", ix, aliases[ix]);
    // store alias name:
    rc= findAddAlias(aliases[ix]);
    if(rc==-1) return(-1);
    // create list in cls2aliases:
    cls2aliases[cls2aliasesN]= rc; cls2aliasesN++;
  };
  cls2aliases[cls2aliasesN]= -1; cls2aliasesN++;  // the list termination
  //
};
printf("INFO classes: %d, aliases: %d, cls2aliases dim: %d\n", 
  arrayN, aliasNamesN, cls2aliasesN);
fclose(alfi);
bubble(array, arrayN);
return(0);
}
void printAliases() {
int ix;
for(ix=0; ix<arrayN; ix++) {
  int ix2,ix3; char aliases[MAXLINELENGTH]="";
  aliases[0]='\0';
  ix2= array[ix].value; ix3= cls2aliases[ix2];
  while( ix3!= -1) {
    if(ix2>array[ix].value) sprintf(aliases,"%s,", aliases);
    sprintf(aliases,"%s%s", aliases, aliasNames[ix3]);
    ix2++; ix3= cls2aliases[ix2];
  };
  printf("%s %s\n", array[ix].name, aliases);
};
}
/* Prepare alist for new parameter specifying aliases
for DAQ function: DAQlogbook_update_triggerClassName(...,alist)
name: class name (input)
alist: {NULL} if no aliases defined, or class name not found in array
       {aname1,aname2,...,NULL} if aliases found 
*/
void getClassAliases(char *name, char **alist) {
int ix,ix2,ix3,ixalist,ixl=0,ixh=arrayN;
while(1) {
  int cmprc;
  ix = (ixl+ixh)/2;
  printf("ixlh:%d %d -> %d\n", ixl, ixh, ix);
  if((ix==ixl) || (ix==ixh)) {ix=-1; break; }; 
  cmprc= strcmp(array[ix].name, name);
  if(cmprc == 0) {break; 
  } else if(cmprc < 0) {
    ixl= ix;
  } else {
    ixh= ix;
  };
};
if(ix== -1) {
  alist[0]= NULL;
  return;
};
ix2= array[ix].value; ix3= cls2aliases[ix2]; ixalist=0;
while( ix3!= -1) {
  alist[ixalist]= aliasNames[ix3]; ixalist++;
  ix2++; ix3= cls2aliases[ix2];
  if(ixalist>MAXALIASES) {
    printf("ERROR: getClassAliases internal error for class %s\n",
      array[ix].name); break;
  };
};
alist[ixalist]= NULL; ixalist++;
}
void printalist(char **alist) {
char *ap=alist[0];int ix=0;
while(ap != NULL) {
  printf("%s ", ap); ix++; ap= alist[ix];
};
}
/* g++ -g aliases.c 
int main() {
int rc;
char *daqlist[MAXALIASES];
rc= readAliases(); if(rc==-1) return(8);
//printAliases();
getClassAliases("CMLL8-S-NOPF-MUOo", daqlist);
printalist(daqlist);
getClassAliases("CMLL8-S-NOPF-MUON", daqlist);
printalist(daqlist);
} */
