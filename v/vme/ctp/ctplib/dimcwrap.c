int dic_cmnd(
#define LTUNCOUNTERSall 26;
unsigned int cnts[NCOUNTERS];       // allocate maximum (i.e. for CTP)
int realcnts=NCOUNTERS;
unsigned int cntsFailed=0xdeaddeed;
strcpy(DNGET,DETNAME); strcat(DNGET, "/GETCOUNTERS");
    rc= dic_cmnd_service(DNGET, cnts,0);
    printf("RC from dic_cmnd_service:%d\n", rc);
