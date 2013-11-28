#include "CONNECT.h"
int main(){
 string answer;
 cout << endl;
 cout << "Connection test interfere with CTP operation !" << endl;
 cout << "Would you like to continue ? (yes, no)" ;
 cin >> answer;
 if(answer != "yes") exit(1);

 CONNECT connect;
}
