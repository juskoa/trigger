#include <iostream>
#include <string>

using namespace std;

int main(int argc, char *argv[])
{
 string aa("BCM1 aa dd aa iiiiii");
 cout << aa << endl;
 size_t ix;
 if((ix=aa.find("BCM")) != string::npos)cout << ix << endl;
 else cout << "not found" << endl;
 return 0;
} 


