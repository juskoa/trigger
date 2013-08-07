#include "stdio.h"      // printf
#include "vmewrap.h"
#include "vmeblib.h"
#include "daqlogbook.h"
int main() {
int rcdaq,runN;
rcdaq= daqlogbook_open();
if(rcdaq!=0) {
  printf("ERROR DAQlogbook_open failed:%d\n", rcdaq);
} else {
runN=0;
rcdaq=daqlogbook_add_comment(runN,"title test","test message");
rcdaq= daqlogbook_close();
};
}

