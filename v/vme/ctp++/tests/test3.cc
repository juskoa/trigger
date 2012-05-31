#include <string>
#include <iostream>
using namespace std;
string strip(string s){
std::string::iterator it = std::remove_if(s.begin(), s.end(),
std::bind2nd(std::equal_to<char>(), ' '));
return (s = std::string(s.begin(), it));
}

int main(){
 cout << strip(" ko k ot ") << endl;
}
