/* mysqlwrap.c
To test: uncomment main() and: 
gcc mysqlwrap.c -o mysqlwrap `mysql_config --cflags --libs` mysqlwrap.c 
*/
#include <stdio.h>
#include "mysql.h"
//#include <unistd.h>

MYSQL mysql;

/*---------------------------------------------------*/ int msOpen() {
if(mysql_init(&mysql)==NULL) {
  printf("\nFailed to initate MySQL connection");
  exit(1);
}
if(!mysql_real_connect(&mysql,"pcald30","trg","trg","TRG",0,NULL,0)) { 
  printf( "Failed to connect to MySQL: Error: %s\n", mysql_error(&mysql)); 
  exit(1);
};
printf("Connect successful\n");
}
/*---------------------------------------------------*/ int msClose() {
mysql_close(&mysql);
}
/*---------------------------------------------------*/
MYSQL_RES *msSelect(char *fields, char *table, char *whereexp, int *nfields) {
/*  Usage:
fields=msSelect(fields, table,whereExp);
if(fields!=N) {   // error: "required # of fields not received"
};
while ((row = msFetch(result, &lengths)!=NULL) {    // unsigned long *lengths
  for(i = 0; i < fields; i++) { 
    printf("[%.*s] ", (int) lengths[i], row[i] ? row[i] : "NULL"); 
  } 
};
mysql_free_result(result);
-----------------------------------Input:
fields:   "name1,name2"    required columns
          "*"              all columns
table:    "tableName"
whereexp: "WHERE NameX='blabla'"
          ""  in case of all rows required
-----------------------------------Output:
nfields: <0 error
          0 no rows returned
*/
int rcfields;
MYSQL_RES *result;
char querydef[1000];
sprintf(querydef,"SELECT %s FROM %s ", fields, table);
if(whereexp[0]!='\0') {
  strcat(querydef, "WHERE ");
  strcat(querydef, whereexp);
};
if(mysql_query(&mysql,querydef)) {
  printf( "Select failed with error: %s\n", mysql_error(&mysql));
  rcfields=-1;
} else {
  result = mysql_store_result(&mysql);
  if (result) {             // there are rows
    rcfields= mysql_num_rows(result);
    //printf( "%ld rows found\n",(long) mysql_num_rows(result));
    if(rcfields!=0) {
      rcfields = mysql_num_fields(result);
    };
  } else {
    printf("result not returned (NULL)\n"); rcfields=-2;
  };
};
*nfields=rcfields;
return(result);
};
/*---------*/ MYSQL_ROW msFetch(MYSQL_RES *result, unsigned long **lengths) {
/* get next row (or NULL if no more rows)
*/
MYSQL_ROW rowrc;
rowrc = mysql_fetch_row(result); 
*lengths = mysql_fetch_lengths(result);
return(rowrc);
}
/*---------------------------------------------------*/
int msSelect1(char *fields, char *table, char *whereexp, char *outstr) {
/* Usage: 
Useful for retrieval of only 1 row (returned in outstr)
todo: fields is now only 1 name, more can be used -result can be returned
      as 1 string?
rc: <0 error
     0 not found (still mysql_free_result() to be called)
    >0 number of rows found
*/
int rc,nfields;
unsigned long *lengths;
MYSQL_RES *result;
MYSQL_ROW row;
result=msSelect(fields, table, whereexp, &nfields);
if(nfields<=0) {   // error: "required # of fields not received"
  rc= nfields; goto RET;
};
rc= mysql_num_rows(result);
if((row = msFetch(result, &lengths))!=NULL) { 
  int i;
  /*for(i = 0; i < nfields; i++) { 
    printf("[%.*s] ", (int) lengths[i], row[i] ? row[i] : "NULL"); 
  }; printf("\n");  */
  strcpy(outstr, row[0]);
} else {
  printf("row not received\n");
  rc=-3;
};
RET: 
mysql_free_result(result);
return(rc);
}
/*
int main(int argc, char **argv) {
int fields, id; unsigned long *lengths;
MYSQL_RES *result;
MYSQL_ROW row;
msOpen();
printf("TRG_LTUS:\n");
result=msSelect("*", "TRG_LTUS","", &fields);
if(fields!=5) {   // error: "required # of fields not received"
  printf("Error: 5 fields expected, got:%d\n", fields);
};
while ((row = msFetch(result, &lengths))!=NULL) { 
  int i;
  for(i = 0; i < fields; i++) { 
    printf("[%.*s] ", (int) lengths[i], row[i] ? row[i] : "NULL"); 
  }; printf("\n"); 
};
mysql_free_result(result);
printf("DETECTOR_CODES:\n");
for(id=0; id<24; id++) {
  char where[16];
  sprintf(where,"id=%d",id);
  result=msSelect("id,name", "DETECTOR_CODES",where, &fields);
  if(fields<0) {   // error: "required # of fields not received"
    printf("Error: 2 fields expected, got:%d\n", fields);
  };
  if((row = msFetch(result, &lengths))!=NULL) { 
    int i;
    for(i = 0; i < fields; i++) { 
      printf("[%.*s] ", (int) lengths[i], row[i] ? row[i] : "NULL"); 
    }; printf("\n"); 
  } else {
    printf("id:%d does not exist\n", id);
  };
  mysql_free_result(result);
};
printf("DETECTOR_CODES, msSelect1:\n");
for(id=0; id<24; id++) {
  int rc;
  char where[16];
  char outstr[80];
  sprintf(where,"id=%d",id);
  rc=msSelect1("name", "DETECTOR_CODES",where, outstr);
  if(rc<0) {   
    printf("mSelect1 error\n"); continue;
  };
  if(rc==0) {   // error: "required # of fields not received"
    printf("id:%d does not exist\n", id); continue;
  };
  printf("id:%d %s\n", id, outstr);
};
*/
/*--------------------------------------------------- generic:
strcpy(querydef,"SELECT * FROM TRG_LTUS");
if(mysql_query(&mysql,querydef)) {
  printf( "Select failed with error: %s\n", mysql_error(&mysql));
} else {
  printf( "%ld Record Found\n",(long) mysql_affected_rows(&mysql));
  result = mysql_store_result(&mysql);
  if (result) {             // there are rows
    int num_fields;
    num_fields = mysql_num_fields(result);
    while ((row = mysql_fetch_row(result))) { 
      int i;
      unsigned long *lengths = mysql_fetch_lengths(result);
      for(i = 0; i < num_fields; i++) { 
        printf("[%.*s] ", (int) lengths[i], row[i] ? row[i] : "NULL"); 
      } 
      printf("\n"); 
    }
    mysql_free_result(result);
  } else {                  // mysql_store_result() returned nothing
    if(mysql_field_count(&mysql) > 0) { // mysql_store_result() should have returned data
      printf( "Error getting records: %s\n", mysql_error(&mysql));
    }
  }
}
*/
/*
msClose();
}
*/
