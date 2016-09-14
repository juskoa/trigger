#include <string>
#include <iostream>
#include <cstdlib>
using namespace std;
int main(){
 string ss("123");
 cout << ss << " " << atoi(ss.c_str()) << endl;
}
