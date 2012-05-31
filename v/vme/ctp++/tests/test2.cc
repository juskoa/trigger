#include <fstream>
#include <iostream>
#include <string>
using namespace std;
//-----------------------------------------------------------------------
char *string2char(string s){
 int n=s.size()+1;
 char *c=new char[n];
 for(int i=0;i<(n-1);i++)c[i]=s[i];
 c[n]='\0';
 //cout << s << " " << c << endl;
 return c;
}
//--------------------------------------------------------------------
class test{
 public:
        ifstream file;
        void open(int i,string const name);
        void close();
        char const *nam;
};
void test::open(int i,string const name){
 string name2="../../CFG/ctp/ssmsigs/ltu_"+name+".sig";
 nam=string2char(name2);
 cout << "1 state: " << hex<< file.rdstate() << endl;;
 file.open(name2.c_str());
 cout << "2 state: " << hex<< file.rdstate() << endl;;
 if(! file) cout << i<< " failed" << endl; 
 while(!file.eof()){
   string line;
   getline(file,line);
 }
 
 cout << "3 state: " << hex<< file.rdstate() << endl;;
 file.close();
 cout << "4 state: " << hex<< file.rdstate() << endl;;
 file.clear();
 cout << "5 state: " << hex<< file.rdstate() << endl;;
}
void test::close(){
 file.close();
}
int main(){
 test t;
 t.open(1,"inmon");
 t.open(2,"inmon");
 t.open(3,"inmon");
}
