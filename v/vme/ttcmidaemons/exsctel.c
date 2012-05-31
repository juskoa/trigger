#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv)  {
int rc;
char cmd[]="$VMECFDIR/ttcmidaemons/sctel.py MININF";
rc= system(cmd);
printf("%s rc:%d\n", cmd, rc);
return(rc);
}
