/* gcc myudplib.c -o myudplib.exe */
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>      // gethostbyname
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>      //EAGAIN
//#include <unistd.h>
#include <string.h>     // memset
#include <unistd.h>     // close
#include <stdlib.h>     //exit
#include <sys/time.h>   /* gettimeofday */

//#include "vmewrap.h"
#include "vmeblib.h"

#define BUFLEN 512
#define NPACK 2 

#define PORTs 9930
#define PORTr 9930
/*pit:
mindiffus is around 130

#define CTP_IP "10.160.130.18"
#define TTCMI_IP "10.160.130.36"
*/
/*lab (TTCMI=  altri1, CTP=pcalicebhm05) 
mindiffus is around 251 for both modes (server block/unblock)
*/
//#define CTP_IP "137.138.93.77"   //pcalicebhm11
//#define CTP_IP "137.138.93.141"    //  05
//#define TTCMI_IP "137.138.140.218"
#define CTP_IP "pcalicebhm05"    //  05
#define TTCMI_IP "altri1"

/*----------------------------------------------*/
void GetMicSec(w32 *tsec, w32 *tusec) {
int rc;
struct timeval tv; struct timezone tz;
rc=gettimeofday(&tv,&tz);
*tsec=tv.tv_sec; *tusec=tv.tv_usec;
}
/*----------------------------------------------*/
w32 DiffSecUsec(w32 tsec,w32 tusec,w32 prevtsec,w32 prevtusec) {
w32 usecdiff;
if( tusec >= prevtusec) {
  usecdiff= 1000000*(tsec-prevtsec) + tusec-prevtusec;
} else {
  usecdiff= 1000000*(tsec-prevtsec-1) + prevtusec-tusec;
};
return(usecdiff);   /* in microseconds */
}

void diep(char *s) {
  perror(s);
  exit(1);
}
// sending:
struct sockaddr_in si_send;
// receiving side:
struct sockaddr_in si_me, si_other;
socklen_t srlen= sizeof(si_other);

/*------------------------------------------------------------- udpopens */
/* Usage:   client starts with sending the message
sck= udpopens(); rc= udpsend(sck, buf);
if(sck or rc is -1): error
*/
int udpopens(char *server) {
int s;
struct hostent *phe;
struct in_addr h_addr;
char serverip[16]; // 255.255.255.255 + \0
if((phe = (struct hostent *)gethostbyname(server))==NULL) { 
  return(-1); 
} else {
  h_addr.s_addr= *((unsigned long *) phe->h_addr_list[0]);
  //h_addr.s_addr= *((unsigned long *) host->h_addr_list[0]);
  //memcpy(serverip, phe->h_addr, phe->h_length);
  strcpy(serverip, inet_ntoa(h_addr));
};
//strcpy(serverip, server);
s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
//s=socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
if (s==-1) return(s);
memset((char *) &si_send, 0, sizeof(si_send));
si_send.sin_family = AF_INET; si_send.sin_port = htons(PORTs);
if (inet_aton(serverip, (struct in_addr *)&si_send.sin_addr)==0) { s=-1; };
return(s);
}

/*------------------------------------------------------------- udpopenr */
/* Usage:   server starts with waiting
1. start server:
sck= udpopenr(); rc= udpwaitr[block](sck, buf, buflen);
if(sck or rc is -1): error
2. start client (see above)
*/
int udpopenr() {
int s,rc;
s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  //returns file descriptor
//fcntl( not here (see MSG_NOWAIT
//s=socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
if(s==-1) {
  //diep("socket");
  return(s);
};
memset((char *) &si_me, 0, sizeof(si_me));
si_me.sin_family = AF_INET; si_me.sin_port = htons(PORTr);
si_me.sin_addr.s_addr = htonl(INADDR_ANY);
rc= bind(s, (struct sockaddr *)&si_me, sizeof(si_me));
if(rc==-1) {
  //diep("bind");
  return(-1);
};
return(s);
}
/* ---------------------------------------------------------- udpclose */
void udpclose(int s) {
close(s);
}
/* ---------------------------------------------------------- udpsend
rc:-1 -> error
*/
int udpsend(int sock, unsigned char *buf, int buflen) {
int rc;
printf("Sending packet length: %d\n", buflen);
rc= sendto(sock, buf, buflen, 0, (struct sockaddr *)&si_send, sizeof(si_send));
return(rc);
}
/* ---------------------------------------------------------- udpwaitr
Blocking udp read:
rc: length of received message (max. buflen) or 
    -1 error 
*/
int udpwaitr(int s, unsigned char *buf, int buflen) {
int rc;
rc=recvfrom(s, buf, buflen, 0, (struct sockaddr *)&si_other, &srlen);
return(rc);
}
/* ---------------------------------------------------------- udpwait_unblock
Unblocking udp read:
rc: length of received message (max. buflen) or 
    -1 error 
*/
int udpwait_unblock(int s, unsigned char *buf, int buflen) {
int rc, ixrep=0;
//rc=recvfrom(s, buf, buflen, 0, (struct sockaddr *)&si_other, &srlen);
while(1) {
  rc=recvfrom(s, buf, buflen, MSG_DONTWAIT, 
    (struct sockaddr *)&si_other, &srlen);
  if(rc==-1) {
    char msg[200];
    if(errno==EAGAIN) {
      //printf("EAGAIN:%d ixrep:%d\n", errno, ixrep); EAGAIN is 11
      ; //usleep(10);
    } else {
      sprintf(msg, "recvfrom() %d",ixrep);
      diep(msg);
      return(-1);
    };
  } else {
    break;
  };
  ixrep++;
};
// printf (on both sides) adds 300 usecs
/* printf("Received packet from %s:%d\nData: %s\n\n", 
  inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf); */
//printf("ixrep:%d\n", ixrep);
return(rc);
}

/*--------------------------------------------------------------------- main */
#define loops 2
int main(int argc, char **argv) {
int udpsocks,udpsockr, rc;
w32 tusec1,tsec1,tusec2,tsec2,diffus, maxdiffus,mindiffus,avdiffus;
//int mes[2]={8,9};
unsigned char mes[]="message1";
unsigned char mesok[]="OK";
unsigned char bufrec[BUFLEN];
if(argc < 2) {
  printf("Usage: \n\
udplib s -start server waiting message + sending response (11 or ctp crate)\n\
udplib c -start client sending message+waiting response(altri1 or TTCmi crate)\n\
Note:\n\
Server can be started in 'unblocking mode' : udplib s u\n\
In this mode, the waiting for UDP message is in unblocking mode (endless loop)\n");
  exit(4);
};
if(strcmp(argv[1],"c")==0) {         //-------------------------------------- client
  int ix;
  printf("Client, server should be active already,...\n");
  udpsocks= udpopens(CTP_IP); if(udpsocks==-1) diep("udpopens CTP_IP");
  udpsockr= udpopenr(); if(rc==-1) diep("udpopenr()"); 

  avdiffus=0; mindiffus=10000000; maxdiffus=0;
  for(ix=0; ix<loops; ix++) {
    GetMicSec(&tsec1, &tusec1);
    rc= udpsend(udpsocks, mes, strlen((const char *)mes)+1); if(rc==-1)  diep("udpsend()"); 
    rc= udpwaitr(udpsockr, bufrec, BUFLEN); if(rc==-1) diep("udpwaitr()"); 
    if((rc!= (int)strlen((const char *)mesok)+1)) {printf("length of rec. message wrong:%d\n",rc);};
    GetMicSec(&tsec2, &tusec2);
    diffus= DiffSecUsec(tsec2, tusec2, tsec1, tusec1);
    //printf("(send+wait) micsecs: %d\n", diffus);
    if(diffus> maxdiffus) maxdiffus= diffus;
    if(diffus< mindiffus) mindiffus= diffus;
    avdiffus= avdiffus+ diffus;
  };
  udpclose(udpsocks); udpclose(udpsockr);
  printf("(send+wait) micsecs loops:%d average:%9.3f min:%d max:%d\n", 
    loops, 1.0*avdiffus/loops, (int)mindiffus, (int)maxdiffus);
} else if(strcmp(argv[1],"s")==0) {  //------------------------------ server
  int msgs=0; int unblockingmode=0;
  if(argc>2) {
    if(strcmp(argv[2],"u")==0) unblockingmode=1;
  };
  if(unblockingmode==1) {
    printf("Server. Starting in unblocking mode...\n");
  } else {
    printf("Server. Starting in blocking mode...\n");
  };
  udpsocks= udpopens(TTCMI_IP); if(udpsocks==-1) diep("udpopens TTCMI_IP");
  udpsockr= udpopenr(); if(rc==-1) diep("udpopenr()"); 
  while(1) {
    if(unblockingmode==1) {
      rc= udpwait_unblock(udpsockr, bufrec, BUFLEN); 
      if(rc==-1) diep("udpwait_unblock()"); 
    } else {
      rc= udpwaitr(udpsockr, bufrec, BUFLEN); if(rc==-1) diep("udpwaitr()"); 
      printf("got %d bytes: %s\n", rc, bufrec);
    };
    rc= udpsend(udpsocks, mesok, strlen((const char *)mesok)+1); 
    if(rc==-1)  diep("udpsend()"); 
    msgs++;
  };
  printf("%d messages received (OK sent back)\n", msgs);

  udpclose(udpsocks); udpclose(udpsockr);
} else {
  printf("Bad parameter: %s\n", argv[1]);
};
return(0);
}

