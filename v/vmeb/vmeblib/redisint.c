#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hiredis.h"
void readpw(char *facility, char *mem);

char host[24]="adls";   // "pcalicebhm10";
static redisContext *context=NULL;
static struct config {
    char *hostip;
    int hostport;
} config={host,6379};

void printReply1(redisReply *reply, int level) {
#define MAXLEVEL 10
int ix;
char clevel[MAXLEVEL*2+1]="";
for(ix=0; ix<level; ix++) {
  strcat(clevel,"  ");
};
if(reply==NULL) {
  printf("INFO %sreply: NULL reply\n", clevel);
} else if(reply->type== REDIS_REPLY_NIL) {
  printf("INFO %sreply: NULL\n", clevel);
} else if(reply->type== REDIS_REPLY_STATUS) {
  printf("INFO %sreply->type:%d (STATUS) str:%s len:%d\n", 
    clevel, reply->type, reply->str, reply->len);
} else if(reply->type== REDIS_REPLY_STRING) {
  printf("INFO %sreply->type:%d (STRING) str:%s len:%d\n", 
    clevel, reply->type, reply->str, reply->len);
} else if(reply->type== REDIS_REPLY_INTEGER) {
  // long long: lld unsigned long long: llu
  printf("INFO %sreply->type:%d (INTEGER) int:%lld len:%d\n", 
    clevel, reply->type, reply->integer, reply->len);
} else if(reply->type== REDIS_REPLY_ARRAY) {
  unsigned int ixa;
  printf("INFO %sreply->type:%d (ARRAY) elements:%d\n", 
    clevel, reply->type, (int)reply->elements);
  for(ixa=0; ixa<reply->elements; ixa++) {
    printReply1(reply->element[ixa], level+1);
  };
} else {
  printf("INFO %sreply->type:%d (?) str:%s len:%d\n",
    clevel, reply->type, reply->str, reply->len);
};
}
void printReply(redisReply *reply) {
printReply1(reply, 0);
}

void mydbDisconnect() {
if(context !=NULL) {
  printf("INFO redisFree...\n");
  redisFree(context);
};
}
void mydbShutdown() {
redisReply *reply;// int rc;
reply= (redisReply *)redisCommand(context, "SHUTDOWN");
freeReplyObject(reply);
redisFree(context);
}
int mydbConnect() {
int rc=REDIS_OK;
char *site;
site= getenv("VMESITE");
if(strcmp(site, "ALICE")==0) {
  strcpy(config.hostip, "alitri");
} else if(strcmp(site, "SERVER")==0) {
  strcpy(config.hostip, "adls");
} else if(strcmp(site, "SERVER2")==0) {
  strcpy(config.hostip, "pcalicebhm10");
} else {
  printf("ERROR VMESITE:%s (SERVER or ALICE expected)\n", site);
  return(16);
};
if(context==NULL) {
  //config.hostip = host; config.hostport = 6379;
  context = redisConnect(config.hostip,config.hostport);
  if(context->err) {
    fprintf(stdout,"ERROR Could not connect to Redis at ");
    fprintf(stdout,"%s:%d: %s\n",config.hostip,config.hostport,context->errstr);
    redisFree(context);
    context = NULL;
    rc= REDIS_ERR;
  } else {
    char pmsg[80];
    redisReply *reply; char cmd[90]; char pwd[80];
    readpw("redis", pwd);
    if(strlen(pwd)>0) {
      sprintf(cmd, "AUTH %s", pwd); 
      reply= (redisReply *)redisCommand(context, cmd);
      printReply(reply);
      if((reply->type== REDIS_REPLY_STATUS) && (strcmp(reply->str,"OK")==0)) {
        strcpy(pmsg,"ok, auth");
      } else {
        strcpy(pmsg,"ok, not auth (bad passwd)");
      };
    } else {
      strcpy(pmsg,"ok, not auth (no passwd given)");
    };
    printf("INFO redisConnect(%s,%d) %s.\n", config.hostip,config.hostport, pmsg);
  };
}; return(rc);
}

/*===========================================*/
#define gdetsname "gruns_dets"
void red_update_detsinrun(int runn, unsigned int detpattern) {
redisReply *reply; char cmd[80];
//rc= mydbConnect();
sprintf(cmd, "HSET %s %d 0x%x", gdetsname, runn, detpattern); 
reply= (redisReply *)redisCommand(context, cmd);
printReply(reply);
//if(reply->type == REDIS_REPLY_INTEGER) 
freeReplyObject(reply);
}
/* runn:0 -clear all runs
*/
void red_clear_detsinrun(int runn) {
redisReply *reply; char cmd[80];
if(runn==0) {
  sprintf(cmd, "DEL %s", gdetsname); 
} else {
  sprintf(cmd, "HDEL %s %d", gdetsname, runn); 
};
reply= (redisReply *)redisCommand(context, cmd);
printReply(reply);
freeReplyObject(reply);
}
/* ret: 0xffffffff in case of error */
unsigned int red_get_detsinrun(int runn) {
redisReply *reply; char cmd[80];
unsigned int dets; int n;
sprintf(cmd, "HGET %s %d", gdetsname, runn); 
reply= (redisReply *)redisCommand(context, cmd);
printReply(reply);
if(reply->type == REDIS_REPLY_STRING) {
  n= sscanf(reply->str,"%d", &dets);
  if(n!=1) {
    dets= 0xffffffff;
  }
} else {
  dets= 0xffffffff;
};
freeReplyObject(reply);
return(dets);
}
void red_get_runs(int *runs, int *dets){
char cmd[40]; int ixa;
redisReply *reply;
for(ixa=0; ixa<6; ixa++) {
  runs[ixa]=0; dets[ixa]=0;
};
sprintf(cmd, "HGETALL %s", gdetsname);
reply= (redisReply *)redisCommand(context, cmd);
if(reply->type== REDIS_REPLY_ARRAY) {
  unsigned int ixa;
  //printf("reply->type:%d (ARRAY) elements:%d\n", reply->type, (int)reply->elements);
  for(ixa=0; ixa<reply->elements; ixa=ixa+2) {
    redisReply *reply2;
    if(ixa>10) {   // max. 6 runs active
      printf("ERROR: too many elements in %s", gdetsname);
      break;
    };
    // key
    reply2= reply->element[ixa];
    //printReply(reply2);
    if(reply2->type== REDIS_REPLY_STRING) {
      int run, ni;
      ni= sscanf(reply2->str, "%d", &run);
      if(ni==1) runs[ixa/2]= run;
    } else {
      break; // error
    };
    // value
    reply2= reply->element[ixa+1];
    //printReply(reply2);
    if(reply2->type== REDIS_REPLY_STRING) {
      int det, ni;
      ni= sscanf(reply2->str, "0x%x", &det);
      if(ni==1) dets[ixa/2]= det;
    } else {
      break; // error
    };
  };
} else {
  ; // error
};
freeReplyObject(reply);
}
