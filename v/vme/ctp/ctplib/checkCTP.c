#include <stdio.h>
#include <string.h>
#include "vmewrap.h"
#include "ctp.h"
#include "vmeblib.h"

/* check/configure/print FPGAs versions for all the ctp boards */
void checkCTP() {
printenvironment(); 
readBICfile();
//readTables();  only in ctp_proxy!
}

