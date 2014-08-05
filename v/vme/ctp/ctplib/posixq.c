/* gcc -D MAIN -o posixq posixq.c -lrt
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <mqueue.h>
#define QUEUE_NAME  "/ctpproxy_queue"
#define MAX_SIZE    128
#define CHECK(x) \
do { \
  if (!(x)) { \
    fprintf(stderr, "%s:%d: ", __func__, __LINE__); \
    perror(#x); \
    exit(-1); \
  } \
} while (0) \

struct mq_attr attr;

mqd_t pq_open() {
mqd_t mq;
/* initialize the queue attributes */
attr.mq_flags = 0;
attr.mq_maxmsg = 10;
attr.mq_msgsize = MAX_SIZE;
attr.mq_curmsgs = 0;
mq = mq_open(QUEUE_NAME, O_CREAT | O_RDONLY, 0644, &attr);
return(mq);
}
mqd_t pq_connect() {
mqd_t mq;
mq = mq_open(QUEUE_NAME, O_WRONLY);
return(mq);
}
void pq_close(mqd_t mq, int unlink) {
CHECK((mqd_t)-1 != mq_close(mq));
if(unlink==1) {
  CHECK((mqd_t)-1 != mq_unlink(QUEUE_NAME));
};
}
int pq_send(mqd_t mq, char *msg) {
int msize,rc;
//CHECK(0 <= mq_send(mq, buffer, MAX_SIZE, 0));
msize = strlen(msg); if(msize>MAX_SIZE) msize= MAX_SIZE;
rc= mq_send(mq, msg, msize, 0);   // send
return(rc);
}
void pq_receive(mqd_t mq, char *msg) {
char buffer[MAX_SIZE + 1];
ssize_t bytes_read;
bytes_read = mq_receive(mq, buffer, MAX_SIZE, NULL);   //receive
CHECK(bytes_read >= 0);
strncpy(msg, buffer, bytes_read); msg[bytes_read]='\0';
return;
}
#ifdef MAIN
int main(int argc, char **argv) {
char msg[MAX_SIZE+1];
if(argc!=2) {
printf("posixq s   -start server\n\
posixq c   -startclient\n\
");
exit(4);
};
if(strcmp("s",argv[1])==0) {
  int must_stop = 0; 
  mqd_t mq_rec;
  mq_rec= pq_open(); // create the message queue
  CHECK((mqd_t)-1 != mq_rec);
  do {
    ssize_t bytes_read;
    pq_receive(mq_rec, msg);
    if (strcmp(msg, "qs")==0) {
        must_stop = 1;
    };
    printf("Received: %s.\n", msg);
  } while (!must_stop);
  pq_close(mq_rec, 1); // cleanup
} else if(strcmp("c",argv[1])==0) {
  mqd_t mq_send;
  mq_send = pq_connect();
  CHECK((mqd_t)-1 != mq_send);
  printf("Send to server (enter q to stop, qs to stop also server):\n");
  do {
    printf("> "); fflush(stdout);
    //memset(msg, 0, MAX_SIZE);
    fgets(msg, MAX_SIZE, stdin); msg[strlen(msg)-1]='\0'; // remove \n
    CHECK(0 <= pq_send(mq_send, msg));
  } while (strncmp(msg, "q", strlen("q")));
  pq_close(mq_send, 0);
};
return 0;
}
#endif
