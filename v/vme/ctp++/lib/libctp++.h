#ifndef _libctp_h_
#define _libctp_h_
#include <iostream>
#include <string>
#include <vector>
#include "vmeblib.h"
#define Mega (1024*1024)
using namespace std;
//typedef unsigned int w32;
/*
typedef struct SSMmode{
 string name;
 w32 modecode;
 string channels[32];
}SSMmode;
*/
//
typedef struct Connection{
 string name;
 w32 channel1;
 w32 channel2;
}Connection;
//
void GetMicSec(w32 *tsec, w32 *tusec);
w32 CountTime();
void mysleep(w32 delta);
//
w32 convertS2H(w32 &number,string const ss);
char *string2char(string s);
char int2char(int i);
string int2str(int i);
void int2string(int num,string s);
int char2int(char c);
int string2int(string s);
void splitstring(const string& str,vector<string>& tokens,const string& delimiters);
string stripstring(string s);
#endif

