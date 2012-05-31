#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lexan.h"

int floatON=1;
/*------------------------*/ void copy2nl(char *dest, char *src, int maxl) {
int ix=0;
while(1) {
  char c;
  c= src[ix];
  if((c=='\n') || (c=='\0')) { dest[ix]='\0'; break;};
  if(ix>=(maxl-1)) { dest[ix]='\0'; break;};
  dest[ix]= c; ix++;
};
}
/*----------------------------------------------*/ int isItalpha(char c) {
if( (c>='a' && c<='z') || (c>='A' && c<='Z') ||
    c == '_' ) return(1);
return(0);
}

/*----------------------------------------------*/ int isItdigit(char c) {
if( (c>='0' && c<='9') ) return(1);
return(0);
}
/*---------------*/ enum Ttokentype nxtoken(char *line, char *cmd, int *ix) {
/*I: line, ix: from where to start
  O: cmd: token as text
      ix: where to start the next token search
*/
int iline=*ix, icmd=0;
enum Ttokentype tt=tNONE;

/*------------------- skip space chars: */
while (line[iline] == ' ' || line[iline] == '\t' ) iline++;
cmd[icmd]= line[iline]; 

if( cmd[0] == '\0' || cmd[0] == '\n' || cmd[0]=='\r') { 
  cmd[0] = '\0';
  tt=tEOCMD;
  goto RET;
};

/*-------------------- 1,2 char lexicals: */
if( cmd[0] == '=') { 
  if( line[iline+1] == '=' ) {
    cmd[1]= '='; icmd=2; iline++; tt= tEQS;
  } else {
    icmd=1; tt=tASSIGN;
  };
  cmd[icmd]='\0'; iline++; goto RET;
};
if( cmd[0] == ',') { 
  cmd[1]='\0'; iline++; tt=tCOMMA; goto RET;
};
if( cmd[0] == '.') { 
  cmd[1]='\0'; iline++; tt=tDOT; goto RET;
};
if( cmd[0] == '+') { 
  cmd[1]='\0'; iline++; tt=tPLUS; goto RET;
};
if( cmd[0] == '-') { 
  cmd[1]='\0'; iline++; tt=tMINUS; goto RET;
};
if( cmd[0] == '*') { 
  cmd[1]='\0'; iline++; tt=tMULT; goto RET;
};
if( cmd[0] == '/') { 
  cmd[1]='\0'; iline++; tt=tDIV; goto RET;
};
if( cmd[0] == '(') { 
  cmd[1]='\0'; iline++; tt=tLEFT; goto RET;
};
if( cmd[0] == ')') { 
  cmd[1]='\0'; iline++; tt=tRIGHT; goto RET;
};
if( cmd[0] == '[') { 
  cmd[1]='\0'; iline++; tt=tLEFTA; goto RET;
};
if( cmd[0] == ']') { 
  cmd[1]='\0'; iline++; tt=tRIGHTA; goto RET;
};

/*------------------ hexadecimal number: */
if( line[iline] == '0' && line[iline+1] == 'x' ) {
  cmd[0]='0'; cmd[1]='x'; cmd[2]= '\0';
  iline= iline+2; icmd= 2;
  while( (line[iline] >= '0' && line[iline] <= '9') ||
         (line[iline] >= 'A' && line[iline] <= 'F') ||
         (line[iline] >= 'a' && line[iline] <= 'f') ) {
    cmd[icmd]= line[iline];
    iline++; icmd++; cmd[icmd]= '\0';
  };
  tt=tHEXNUM;
  goto RET;
}

/*--------------------- decimal  or float number: */
while( isItdigit(line[iline]) ) {
  cmd[icmd]=line[iline];
  iline++; icmd++; cmd[icmd]='\0';
};
if( icmd > 0) {
  if(floatON==0) {tt=tINTNUM; goto RET; };
  if(line[iline]=='.') {
    cmd[icmd]=line[iline];
    iline++; icmd++; cmd[icmd]='\0';
    while( isItdigit(line[iline]) ) {
      cmd[icmd]=line[iline];
      iline++; icmd++; cmd[icmd]='\0';
    };
    tt=tFLOAT;
  } else {
    tt=tINTNUM;
  };
  goto RET;
};

/*----------------------- char: */
if(line[iline]=='\'') {
/*  printf("tCHARA:%s:\n",&line[iline]); */
  cmd[icmd]= line[iline+1];
  icmd++; iline++;
  cmd[icmd]='\0';
  if(line[iline+1] != '\'') goto ERRLEX;
  iline= iline+2;
/*  printf("tCHARA2:%s:\n",&line[iline]); */
  tt=tCHAR;
  goto RET;
};
/*----------------------- string: */
if(line[iline]=='"') {
  char cs;
  int il,lwstr;  
  /*  iline++; icmd++; cmd[icmd]='\0'; */
/*    iline++; cmd[icmd]='\0'; */
  iline++; cmd[icmd]='"'; icmd++;
  lwstr=2; il=iline;
  while( (cs=line[il]) != '"' ) {
    if(cs == '\0')goto ERRLEX;
    lwstr++; il++;
  };
  /* lwstr: length of string including leading/trailing " */
  if(lwstr>MAXLINELENGTH) {
    printf("ERR: too long string");
    goto ERRLEX;
  };
  while(1) { 
    cs=line[iline];
    cmd[icmd]= cs;
    icmd++; cmd[icmd]='\0';
    if(cs == '\0')goto ERRLEX;
    iline++; 
    if(cs == '"') {
      break;
    };
  };
  /* cmd[icmd]= line[iline]; 17052005 */
  /*  iline++; icmd++; cmd[icmd]='\0'; */
  tt=tSTRING;
  goto RET;
};

/*----------------------- identifier: */
if( isItalpha(cmd[0]) ) {
  iline++; icmd++; cmd[icmd]='\0';
  while(isItalpha(line[iline]) || isItdigit(line[iline]) ) {
    cmd[icmd]= line[iline];
    iline++; icmd++; cmd[icmd]='\0';
  };
  tt=tSYMNAME;
  goto RET;
};
cmd[1]='\0'; 
ERRLEX: 
iline++; tt=tERR;
RET: 
*ix=iline; 
//printf("nxtoken:%s.\n    cmd:%s. tt: %d ix:%d\n", &line[*ix],cmd, tt, *ix);  
return(tt);
}
int str2int(char *str) {
int i=0; unsigned int var4=0; char c;
while( (c=str[i]) != '\0') {
    int ic;
    if( c>='0' && c<='9') ic= c-'0';
    var4= 10*var4+ic; i++;
  };
return(var4);
}

/*---------------*/ enum Ttokentype nxtoken1(char *line, char *cmd, int *ix) {
/* similar to nxtoken(), but any sequence of non-white characters on output
I: line, ix: from where to start
O: cmd: token as text
      ix: where to start the next token search
*/
int iline=*ix, icmd=0;
enum Ttokentype tt=tNONE;
/*------------------- skip space chars: */
while (line[iline] == ' ' || line[iline] == '\t' ) iline++;
cmd[icmd]= line[iline]; 

if( cmd[0] == '\0' || cmd[0] == '\n' || cmd[0]=='\r') { 
  cmd[0] = '\0';
  tt=tEOCMD;
  goto RET;
};
/* if( cmd[0]>='0' && cmd[0]<='9') {
} else {
};*/
iline++; icmd++; cmd[icmd]='\0';
while((line[iline] != ' ') && (line[iline]!='\t') 
       && (line[iline]!='\n') && (line[iline]!='\0') ) {
  cmd[icmd]= line[iline];
  iline++; icmd++; cmd[icmd]='\0';
};
tt=tSYMNAME;
RET: *ix=iline; return(tt);
}
/*In:
&line[0] -start search from here (skip leading ' ' '\t')
delim    -finish just before this char or '\0', '\n'
--------------------*/ void getRestLine(char *line, char delim, char *rest) {
int start,lng=0,iline=0;
while( (line[iline] == ' ') || (line[iline] == '\t') ) iline++;
start=iline;
while(1) {
  if(line[iline]=='\0') break;
  if(line[iline]=='\n') break;
  if(line[iline]==delim) break;
  rest[lng]= line[iline]; rest[lng+1]='\0';
  lng++; iline++;
}; 
}
/*-------------------------------------------------------------*/
char int12hex(int numint)
{
 switch(numint){
    case 0: return '0';
    case 1: return '1';
    case 2: return '2';
    case 3: return '3';
    case 4: return '4';
    case 5: return '5';
    case 6: return '6';
    case 7: return '7';
    case 8: return '8';
    case 9: return '9';
    case 10: return 'a';
    case 11: return 'b';
    case 12: return 'c';
    case 13: return 'd';
    case 14: return 'e';
    case 15: return 'f';
 }
 return '\0';
}
/*-------------------------------------------------------------*/
int hex12int(char c) {
int ic;
if( c>='0' && c<='9') {
  ic= c-'0';
} else if( c>='a' && c<='f') {
  ic= c-'a'+ 10;
} else if( c>='A' && c<='F') {
  ic= c-'A'+ 10;
} else {
  char errmsg[100];
  printf(errmsg,"hex12int(): wrong hexa-digit:%c.\n",c);
  ic= 0xff;
}; return(ic);
}
/*------------------------------------------------------------ getNextline()
In: str search \n from here
Out: rc: 0 -\0 found (no \n was found)
        >0 -position of \n is: rc-1
*/
int getNextLine(char *strline) {
int rc=0, ix=0;
while(strline[ix]!='\0') {
  if(strline[ix]=='\n') {rc=ix+1; break; };
  ix++;
};
return(rc);
}
/*------------------------------------------------------------ hex2int
I: "fA01" -hexadecimal number (no leading 0x ! )
O: rc: internal representation of hex. number
   in case of error, message is printed to stdout and rc is 0xffffffff
*/
int hex2int(char *token) {
int i=0; unsigned int var4=0;
char c;
if(strlen(token)>8) {
  printf("hex2int(): wrong hexa-number (>8chars):%s.\n",token);
  var4= 0xffffffff; return(var4);
};
c=token[i];
while( (c != '\0') && (c != '\n') ) {
  unsigned int ic;
  if( c>='0' && c<='9') {
    ic= c-'0';
  } else if( c>='a' && c<='f') {
    ic= c-'a'+ 10;
  } else if( c>='A' && c<='F') {
    ic= c-'A'+ 10;
  } else {
    char errmsg[100];
    printf(errmsg,"hex2int(): wrong hexa-number:%s.\n",token);
    var4= 0xffffffff; break;
  };
  var4= (var4<<4) | ic;
  i++; c=token[i];
};
return(var4);
}

/*------------------------------------------------------------ gethexdec
 I: hdnum: 0xhexa or 1023   (hex. or dec. number as string)
 O: rc:0, *num is internal representation of the number
    rc:2  syntax error */
int gethexdec(char *hdnum, unsigned int *num) {
enum Ttokentype token;
int rc=0, ix=0;
char value[80];
token= nxtoken(hdnum, value, &ix);
if(token == tHEXNUM)  {
  *num= hex2int(&value[2]);
} else if(token==tINTNUM) {
  *num= str2int(value);
} else rc=2;
return(rc);
}

/*------------------------------------------------ */void UPPER (char *name) {
char *p;
p=name;  
while ( *p != '\0' ) { *p=toupper(*p); p++; };
} 
/*------------------------------------------------ */void LOWER (char *name) {
char *p;
p=name;  
while ( *p != '\0' ) { *p=tolower(*p); p++; };
} 

/*---------------------------- */int stringStarts(char *str, char *strc) {
if(strncmp(str, strc, strlen(strc))==0) {
  return(1);
} else {
  return(0);
};
}
/*------------------------------
Input: line:l0f e.g. "0SH1& (~0VGA |0VBC)"
Out: name -first L0 name found
     rc: -1 no name found
Usage:
ixn=0;
while(1) {
  int rc;
  rc= getNextFunName(&line[ixn], name);
  if(rc==-1) { break; }; // no meaningfull name found
  //ixn points just after name in line string (' ', '\0', ':', '\n')
  //name contains next name.
  printf("name:%s\n", name);
  ixn=rc+ixn;
};  
----------------------------*/ int getNextFunName(char *line, char *name) {
int ix=0,rc=0; enum Ttokentype token; char value[100]; 
while(1) {
  char c;
  c= line[ix];
  if((c=='|') || (c=='&') || (c==' ') || (c=='~') || (c=='(') ||(c==')')) {
    ix++;
  } else if((c==':') || (c=='\n') || (c=='\0') ) {
    rc=-1; return(-1);
  } else if(c=='0') {   // names starts with '0'
    ix++ ; break;
  };
};
//printf("gNFN:line:%s:\n",line);
strcpy(name,"0"); rc=ix;
while(1) {
  token= nxtoken(line, value, &rc);
  //printf("gNFN:%d:%s:\n", token, value);
  if(token==tEOCMD) {rc=-1; break;
  } else if((token==tERR) && (value[0]==':')) {rc=-1; break;
  } else {strcat(name, value); break;
  };
};
return(rc);
}

