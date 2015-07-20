#ifndef _libctp_h_
#define _libctp_h_
#include <iostream>
#include <cstdio>
#include <string>
#include <cstdlib>
#include <vector>
#include "vmewrap.h"
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
// MOre genarally used for
// - l2data on backplane
// - l2data serial data
// - ctp readout
typedef struct L2Data{
 	int l2clusters;
 	w64 l2classes1;
 	w64 l2classes2;
 	int bcid;
 	int orbit;
 	int eob;   // used as eob flag, other items shooud be zero
 	int esr;
 	int clt;    // calibration trigger
 	int swc;    // software class
 	int issm;   // position of the first word in ssm
}L2Data;
//Interaction Record
typedef struct IRDda{
	int error1;
	int error2;
	int incomplete;
	int orbit;
	int Inter[251];
	int bc[251];
	int issm;
}IRDa;
void printL2Data(L2Data &ctpr);
void clearL2Data(L2Data &ctpr);
void clearIRDda(IRDda &irda);
void printIRDda(IRDda &irda);

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
void getdatetime(char* dmyhms);
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
// parse logical expression
int evaluate(string& eval);
int parse(const char* desc,int level);
#endif

