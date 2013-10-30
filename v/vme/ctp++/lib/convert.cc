#include "libctp++.h"
#include <algorithm>
//------------------------------------------------------------------------------
// Converts string hex number to int
w32 convertS2H(w32 &number,string const ss){
 char c;
 int n,dig;
 if((ss[1] != 'x' && ss[1] != 'X') || ss[0] != '0'){
  cout << "convertS2H: " << ss << " Not hexa number" << endl;
  return 1;
 }
 n=ss.size();
 //cout << "n= " << n << endl;
 number=0;
 for(int i=2;i<n;i++){
  c=ss[i];
  switch(c){
	case '0':dig=0;break;
	case '1':dig=1;break;
	case '2':dig=2;break;
  	case '3':dig=3;break;
	case '4':dig=4;break;
	case '5':dig=5;break;
	case '6':dig=6;break;
	case '7':dig=7;break;
	case '8':dig=8;break;
	case '9':dig=9;break;
        case 'a':dig=10;break;
	case 'A':dig=10;break;
	case 'b':dig=11;break;
	case 'B':dig=11;break;
	case 'c':dig=12;break;
	case 'C':dig=12;break;
	case 'd':dig=13;break;
	case 'D':dig=13;break;
	case 'e':dig=14;break;
	case 'E':dig=14;break;
	case 'f':dig=15;break;
	case 'F':dig=15;break;
	default: cout << "Wrong character'"<<c<<"' in hexa " << ss << endl;
  }
  number=number+dig*(1<<((n-i-1)*4));
 }
 return 0; 
}
//-----------------------------------------------------------------------
char *string2char(string s){
 int n=s.size()+1;
 char *c=new char[n];
 for(int i=0;i<(n-1);i++)c[i]=s[i];
 c[n]='\0';
 //cout << s << " " << c << endl;
 return c; 
}
//-------------------------------------------------------------------------
char int2char(int i){
 char c=' ';
 switch(i){
  case 0: c='0';break;
  case 1: c='1';break;
  case 2: c='2';break;
  case 3: c='3';break;
  case 4: c='4';break;
  case 5: c='5';break;
  case 6: c='6';break;
  case 7: c='7';break;
  case 8: c='8';break;
  case 9: c='9';break;
  default: cout << "integer " << i << " not in table" << endl;
 }
 return c;
}
//-------------------------------------------------------------------------
// convert digit to string of size 1
string int2str(int i){
 string c="";
 switch(i){
  case 0: c='0';break;
  case 1: c='1';break;
  case 2: c='2';break;
  case 3: c='3';break;
  case 4: c='4';break;
  case 5: c='5';break;
  case 6: c='6';break;
  case 7: c='7';break;
  case 8: c='8';break;
  case 9: c='9';break;
  default: cout << "integer " << i << " not in table" << endl;
 }
 return c;
}
//-------------------------------------------------------------------------
int char2int(char c){
 int i=-1;
 switch(c){
  case '0': i=0;break;
  case '1': i=1;break;
  case '2': i=2;break;
  case '3': i=3;break;
  case '4': i=4;break;
  case '5': i=5;break;
  case '6': i=6;break;
  case '7': i=7;break;
  case '8': i=8;break;
  case '9': i=9;break;
  case 'a': i=10;break;
  case 'b': i=11;break;
  case 'c': i=12;break;
  case 'd': i=13;break;
  case 'e': i=14;break;
  case 'f': i=15;break;
  default: cout << "character " << c << " not in table" << endl;
 }
 return i;
}
//---------------------------------------------------------------
void int2string(int num,string s){
 int i;
 if(num==0){
  s="0";
  return;
 }
 s="";
 while(num != 0){
  i=num-(num/10)*10;
  s=s.append(int2str(i));
  num=num/10;
 }
}
//------------------------------------------------------------
// Converts decadic string>0  to int
int string2int(string s){
 int n=s.length();
 int num=0;
 int d=1;
 for(int i=0;i<n;i++){
  int digit=char2int(s[n-1-i]);
  if(digit == -1) return -1;
  num=num + d*digit;
  d=d*10;
 }
 return num;
}
//-----------------------------------------------------------
void splitstring(const string& str,
                      vector<string>& tokens,
                      const string& delimiters = " ")
{
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}
//-----------------------------------------------------------
string stripstring(string s){
std::string::iterator it = std::remove_if(s.begin(), s.end(),
std::bind2nd(std::equal_to<char>(), ' '));
return (s = std::string(s.begin(), it));
}
