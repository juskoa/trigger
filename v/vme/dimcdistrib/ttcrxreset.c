//ttcrxreset.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef CPLUSPLUS
#include <dic.hxx>
#else
#include <dic.h>
#endif

#define TAGdo 33
char DETNAME[10]="";
char DNCMD[20];   // detname/CMD
int rcglob=9;

void prerr(char *msg) {
	printf("ERROR:%s\n", msg); fflush(stdout);
}

void callback(void *tag, int *rc) {
	//printf("callback tag:%d rc:%d\n", *tag, *rc);
	if(*rc != 1) {
		char errmsg[200];
		sprintf(errmsg,"callback for tag:%d not executed by server %s. rc:%d",
				*(int *)tag, DETNAME, *rc);
		prerr(errmsg);
		rcglob=8;
	} else {
		printf("OK, sent over DIM to %s\n", DETNAME);
		rcglob=0;
	};
}


int execute(char *cmd, char *inpline) {
	/* rc: 1:ok   0: not executed */
	int rc;
	rc= dic_cmnd_callback(cmd, inpline, strlen(inpline)+1, callback, TAGdo);
	return(rc);
}


int main(int argc, char **argv) {
	int rc;
	strcpy(DETNAME,argv[1]);
	strcpy(DNCMD,DETNAME); strcat(DNCMD, "/CMD");
	rc=execute(DNCMD, "ttcrxreset");
	if(rc==1) {
		sleep(3);   // at least 1secs, if not, server does not see the request!
	} else {
		printf("not executed. rc:%d\n", rc);
		rcglob=8;
	};
	return(rcglob);
} 
