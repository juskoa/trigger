/* 
BST1/ORB1:
4:12 0x5d89a9   40
5:12 0x5d89a8   21
6:12 0x5d89a9   52
7:12 0x5d89a8   33
8:12 0x5d89a9   31
- shift after ~5 mins:
1:12 0x5d89ac   28
2:12 0x5d89ad   59
3:12 0x5d89ac   39
4:12 0x5d89ad   36
5:12 0x5d89ac   15
6:12 0x5d89ad   49

TTCMI/ORBmain_SELECT:
1:70 0x5025f6   36
2:70 0x5025f6   69
3:70 0x5025f5   49
5:70 0x5025f6   26
6:70 0x5025f5   1
7:70 0x5025f6   39
8:70 0x5025f5   19
9:70 0x5025f5   52
- no shift during several 10s minutes
+-1 fluctuation: 
seems, 5025f5 appears only for smaller UDP difference
(<=56).
5025f4 appears only with UDPdif <4
So it seems, we should through away all measurements with UDPdif<58
- we loose some good measurements (5025f6 appears sometimes with 36 micsecs)
*/
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>      //EAGAIN
#include <unistd.h>     //close
#include <string.h>     // memset
#include <stdlib.h>     //exit
#include <sys/time.h>   /* gettimeofday */
#include <signal.h>    /* signal name macros, and the signal() prototype */

#include "vmewrap.h"
#define L2_ORBIT_READ  0xb140    /* synced with INT */
//#include "ctp.h"

/* receive buffer: */
//#define BUFLEN 512
#define BUFILEN 6   // place for packetn + Orbit (2words) + 2xTime (4 words)
#define BUFLEN (BUFILEN*4)
#define NPACK 2 

#define PORTs 9930
#define PORTr 9930
/*pit:
mindiffus is around 130
*/
#define CTP_IP "10.160.130.18"
#define TTCMI_IP "10.160.130.36"

/*lab (TTCMI=  altri1, CTP=pcalicebhm11) 
mindiffus is around 251 for both modes (server block/unblock)

#define CTP_IP "137.138.93.77"
#define TTCMI_IP "137.138.140.218"
*/
#define MINUDPDIF 60      // in micsecs
#define MAXUDPDIF 120
w32 packetNumber=0;  
int QUIT=0;
/*----------------*/void catch_int(int sig_num) {
/* re-set the signal handler again to catch_int, for next time */
signal(SIGINT, catch_int);
printf("CTRL-C\n"); fflush(stdout);
QUIT=1;
}

/*------------------------------------------------------- dodif32()
Substract 2 32 bits values (representing counters)
*/
w32 dodif32(w32 before, w32 now) {
w32 dif;
if(now >= before) dif= now-before;
else dif= now+ (0xffffffff-before) +1;
//if(DBGcnts) printf("dodif32:%d\n", dif);
return(dif);
}

/* calculate (turn-orbit)mod24bits 
i.e.:
Input: orbit, turn
Output: mult= turn / bit24; turn24= turn % bit24; dif24= turn24-orbit
backwards:
turn= mult*bit24 + dif24
----------------- */ void dif3224(w32 orbit, w32 turn, int *mult, w32 *dif24) {
#define bit24 0x1000000
w32 turn24, dif,mlt;
mlt= turn / bit24;
//mlt= turn- (turn24*bit24);
turn24= turn % bit24; //printf("turn24:%d 0x%x\n", turn24, turn24);
if(turn24 >= orbit) dif= turn24-orbit;
//else dif= before  now;
else dif= turn24+ (0xffffff-orbit) +1;
//printf("%d - %d dif3224:%d %d\n", turn, orbit, mlt, dif);
*mult=mlt; *dif24= dif;
return;
}

/*-----------------------------------*/ void GetMicSec(w32 *tsec, w32 *tusec) {
int rc;
struct timeval tv; struct timezone tz;
rc=gettimeofday(&tv,&tz);
*tsec=tv.tv_sec; *tusec=tv.tv_usec;
}
/*----------*/ w32 DiffSecUsec(w32 tsec,w32 tusec,w32 prevtsec,w32 prevtusec) {
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
socklen_t srlen=sizeof(si_other);

/*------------------------------------------------------------- udpopens */
/* Usage:   client starts with sending the message
sck= udpopens(); rc= udpsend(sck, buf);
if(sck or rc is -1): error
*/
int udpopens(char *server) {
int s;
packetNumber=0;
s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
//s=socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
if (s==-1) return(s);
memset((char *) &si_send, 0, sizeof(si_send));
si_send.sin_family = AF_INET; si_send.sin_port = htons(PORTs);
if (inet_aton(server, (struct in_addr *)&si_send.sin_addr)==0) {s=-1; };
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
    0  -no msg received (returns, becasue nonblocking mode)
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
      return(0);
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

/* First int reserved for packet number.
---------------------*/ int udpsend(int sock, unsigned int *buf, int buflen) {
int rc;
buf[0]= packetNumber;
//printf("Sending packet %d\n", i);bstorbit&0xffffff
rc= sendto(sock, (char *)buf, buflen, 0, (struct sockaddr *)&si_send, 
  sizeof(si_send));
packetNumber++;
return(rc);
}
void udpclose(int s) {
close(s);
}

#define MAXSTORE 10
typedef struct{
 w32 packetn;
 w32 orbit, turn;
 w32 orbitcsec,orbitcmic;  // T2
 w32 turnsec,turnmic;      // T1
 w32 udpsec,udpmic;        // T3
} Tstore;

Tstore *store;             // pointer to current item
Tstore Store[MAXSTORE];

void storeorbit(w32 orbit) {
//if(store => &Store[MAXSTORE]) store= &Store[0];
store->orbit= orbit;
GetMicSec(&store->orbitcsec, &store->orbitcmic);
}
/* msgok: 0: message not arrived yet, check timeout
         >0: message arrived, store
rc: 0: timeout or message arrived/stored
    1: message not arrived, still wait for it
-----------------------------------*/ int storeturn(int msgok, w32 *turnmsg) {
w32 micdif;
GetMicSec(&store->udpsec, &store->udpmic);
micdif= DiffSecUsec(store->udpsec, store->udpmic,
  store->orbitcsec, store->orbitcmic);
if(msgok==0) {   // check timeout
  if(micdif>80) { 
    //printf("timeout1:%d mics\n", micdif);
    return(0); 
  } else { return(1); };
}; /*
  if(micdif>MAXUDPDIF) { 
    printf("storeturn(): too slow:%d mics\n", micdif);
    return(0); 
  } else if(micdif<MINUDPDIF) {
    printf("storeturn(): too fast:%d mics\n", micdif);
    return(0); 
  };*/
store->packetn= turnmsg[0];
store->turn= turnmsg[1];
store->turnsec= turnmsg[2];
store->turnmic= turnmsg[3];
store++;
return(0);
}
/*-----------------------------------------------*/ void printresetStore() {
Tstore *st;
for(st= &Store[0]; st < &Store[MAXSTORE]; st++) {
  int mult; w32 dif24; w32 difmic;
  dif3224(st->orbit, st->turn, &mult, &dif24);
  /*printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
    st->orbit, st->turn, dif24,
    st->orbitcsec, st->orbitcmic,
    st->udpsec, st->udpmic);*/
  /*printf("%d\t%d\t%d\t%d\t%d\t%d\n",
    mult, dif24, 
    st->orbitcsec, st->orbitcmic,
    st->udpsec, st->udpmic);*/
  difmic= DiffSecUsec(st->udpsec,st->udpmic,st->orbitcsec,st->orbitcmic);
  printf("%d:%d 0x%x\t%d\n", st->packetn, mult, dif24, difmic);
};
store= &Store[0];
}
/* orbit1,orbitsec/mic -orbit # + time of its start
Operation:
1. read orbit until 2 equal readings
2. read orbit until changed (+1) between 2 readings
3. save orbit, time
RC:
0xffffffff -orbit increase >1,  UDP packet,if received, to be discarded
0-0xffffff -current orbit, just started
-----------------------------------------------------*/ w32 findorbit_ctp() {
w32 o1,o2,orbit2,o1p1;
FINDNEXT: while(1) {
  o1= vmer32(L2_ORBIT_READ); o2= vmer32(L2_ORBIT_READ);
  if(o1==o2) break;
}; o1p1=o1+1;
while(1) {
  orbit2= vmer32(L2_ORBIT_READ);
  if(orbit2 == o1) continue;
  if(orbit2 == o1p1) {
    storeorbit(orbit2); break;
  } else {
    //printf("findorbit:%u %u\n", o1, orbit2);  //quite often (>1/s rate)
    // if this happen, discard UDP too:
    return(0xffffffff);
    goto FINDNEXT;
  };
};
return(orbit2);
}
/*
Find the change of turn number, reading bobr's BST
-----------------------------------------------------*/ w32 findorbit_lhc() {
#define MessageInput 0x0800
w32 o1,o2,orbit2;
FINDNEXT: while(1) {
  o1= (vmer32(MessageInput+16)>>16) | (vmer32(MessageInput+20)<<16);
  o2= (vmer32(MessageInput+16)>>16) | (vmer32(MessageInput+20)<<16);
  if(o1==o2) break;
};
while(1) {
  orbit2= (vmer32(MessageInput+16)>>16) | (vmer32(MessageInput+20)<<16);
  if(orbit2 == o1) continue;
  if(orbit2 == (o1+1)) {
    return(orbit2);
  } else {
    printf("findorbit:%u %u\n", o1, orbit2);
    goto FINDNEXT;
  };
};
}

/*
Find the change of turn number, reading RF2TTC ORBmain_COUNTER
-----------------------------------------------------*/ w32 findorbit_rf2ttc() {
#define ORBmain_COUNTER 0x7facc
w32 o1,o2,orbit2;
FINDNEXT: while(1) {
  o1= vmer32(ORBmain_COUNTER); o2= vmer32(ORBmain_COUNTER);
  if(o1==o2) break;
};
while(1) {
  orbit2= vmer32(ORBmain_COUNTER);
  if(orbit2 == o1) continue;
  if(orbit2 == (o1+1)) {
    return(orbit2);
  } else {
    printf("findorbit:%u %u\n", o1, orbit2);
    goto FINDNEXT;
  };
};
}
w32 bstread4(int vsp, w32 adr) {
int ix; w32 rc=0;
for(ix=0; ix<4; ix++) {
  rc= rc | ((vmxr32(vsp, 0x800+adr+4*ix)&0xff)<<(8*ix));
};
return(rc);
}

//------------------------------------------------------------- main
#define loops 900 
#define usclientwait 10000 
int main(int argc, char **argv) {
int udpsocks,udpsockr, rc, rcmain=0, vsp=0;
//w32 tusec1,tsec1; 
w32 tusec2,tsec2,diffus, maxdiffus,mindiffus,avdiffus;
// unsigned char mes[]="message1"; unsigned char mesok[]="OK";
w32 mesout[3];
w32  bufrec[BUFILEN];   //char bufrec[BUFLEN];
if(argc < 2) {
  printf("Usage: \n\
OBSOLETE (bobr is now in CTP crate):\n\
orbitsyn s -start server waiting message + sending response (11 or ctp crate)\n\
orbitsyn c -start client sending message+waiting response(altri1 or TTCmi crate)\n\
\n\
orbitsyn c1 -read ctp and BST1 orbit, then make the difference\n\
orbitsyn c2 -read ctp and BST2 orbit, then make the difference\n\
Note:\n\
Server is started in 'unblocking mode' -i.e.\n\
the waiting for UDP message is in unblocking mode (endless loop)\n");
  exit(4);
};
signal(SIGINT, catch_int); /* Ctrl-C signal handler to 'catch_int' */
store= &Store[0];
if(strcmp(argv[1],"c")==0) {   //------------------------------------- client
  int ix;
  printf("Client, server should be active already,...\n");
  udpsocks= udpopens(CTP_IP); if(udpsocks==-1) diep("udpopens CTP_IP");
  udpsockr= udpopenr(); if(udpsockr==-1) diep("udpopenr()"); 
  //
  //rc= vmxopenam(&vsp, "0xb00000","0x4000","A24");
  //if(rc!=0) { printf("bobr vmeopen rc:%d\n", rc); exit(8); };
  rc= vmxopenam(&vsp, "0xf00000","0x100000","A32");
  if(rc!=0) { printf("rf2ttc vmeopen rc:%d\n", rc); exit(8); };
  avdiffus=0; mindiffus=10000000; maxdiffus=0;
  for(ix=0; ix<loops; ix++) {
    //mesout[1]= findorbit_lhc();
    mesout[1]= findorbit_rf2ttc();
    //GetMicSec(&mesout[2], &mesout[3]);
    // send: turn, time
    usleep(10); // increase travel time
    rc= udpsend(udpsocks, mesout, 4+4); if(rc==-1)  diep("udpsend()"); 
    //printf("sent: %d %d %d %d\n", mesout[0], mesout[1], mesout[2],mesout[3]);
    //just send (once per 1000 turns) without waiting confirmation packet
    //rc= udpwaitr(udpsockr, (char *)bufrec, BUFLEN); if(rc==-1) diep("udpwaitr()"); 
    //if((rc!= strlen(mesok)+1)) {printf("length of rec. message wrong:%d\n",rc);};
    //
    GetMicSec(&tsec2, &tusec2);
    diffus= DiffSecUsec(tsec2, tusec2, mesout[1], mesout[2]);
    //printf("(send+wait) micsecs: %d\n", diffus);
    if(diffus> maxdiffus) maxdiffus= diffus;
    if(diffus< mindiffus) mindiffus= diffus;
    avdiffus= avdiffus+ diffus;
    usleep(usclientwait);
  };
  udpclose(udpsocks); udpclose(udpsockr);
  printf("(send+wait) micsecs loops:%d average:%9.3f min:%d max:%d\n", 
    loops, 1.0*avdiffus/loops, mindiffus, maxdiffus);
} else if(strcmp(argv[1],"s")==0) {   //------------------------------- server
  int msgs=0;
  /*if(argc>2) {
    if(strcmp(argv[2],"u")==0) unblockingmode=1;
  };*/
  udpsocks= udpopens(TTCMI_IP); if(udpsocks==-1) diep("udpopens TTCMI_IP");
  udpsockr= udpopenr(); if(rc==-1) diep("udpopenr()"); 
  rc= vmxopenam(&vsp, "0x820000","0xd000","A24");
  if(rc!=0) { printf("ctp vmeopen rc:%d\n", rc); exit(8); };
  while(1) {
    w32 orbit;
    orbit= findorbit_ctp();   // & (re)store orbit + T2
    while(1) {
      rc= udpwait_unblock(udpsockr, (unsigned char *)bufrec, BUFLEN); 
      //rc= udpwaitr(udpsockr, (char *)bufrec, BUFLEN); 
      if(rc==-1) diep("server: udpwait_unblock()"); 
      if(orbit==0xffffffff) {        // discard (if any) UDP packet
        break;
      };
      rc= storeturn(rc, &bufrec[0]);    // store turn + T3 or just return
      if(rc==0) break;  // udp timeout or message arrived/stored
      /*} else if(rc!=0) {
        storeturn(&bufrec[0]);    // store turn + T3
        break;
      };*/
      //if((store->orbit % 10000)==0) printf("no udp yet:%d\n", store->orbit);
    };
    //printf("sending ok...\n");
    //rc= udpsend(udpsocks, mesout, 0+4); if(rc==-1)  diep("udpsend()"); 
    msgs++;
    if(store >= &Store[MAXSTORE]) printresetStore();
    if(QUIT==1) break;
  };
  //printf("Messages received/sent (OK sent back):%d\n", msgs);
  printf("Messages received:%d loops:%d usclientwait:%d us\n", 
    msgs, loops, usclientwait);
  udpclose(udpsocks); udpclose(udpsockr);
} else if((strcmp(argv[1],"c1")==0) ||
  (strcmp(argv[1],"c2")==0)) {   //----- just compare rf2ttc vs. BST1/2
  int ix,vspbobr=1; char bstx= argv[1][1];
  char mibase[10];
  if(strcmp(argv[1],"c1")==0) {
    strcpy(mibase, "0xb00000");
  } else {
    strcpy(mibase, "0xb10000");
  };
  printf("Compare local ctp orbit counter with Orbit from BST%c\n",bstx);
  // has to be first (to open vsp 0):
  //rc= vmxopenam(&vsp, "0xf00000","0x100000","A32");
  rc= vmxopenam(&vsp, "0x820000","0xd000","A24");
  if(rc!=0) { printf("ctp vmeopen rc:%d\n", rc); exit(8); };
  rc= vmxopenam(&vspbobr, mibase,"0x4000","A24");
  if(rc!=0) { printf("bobr vmeopen rc:%d\n", rc); exit(8); };
  //von avdiffus=0; mindiffus=10000000; maxdiffus=0;
  for(ix=0; ix<100; ix++) {
    w32 rf2ttcorbit,bstorbit; w32 diff32;
    //rf2ttcorbit= findorbit_rf2ttc();   // find change
    rf2ttcorbit= findorbit_ctp();   // find change
    bstorbit= bstread4(vspbobr, 18*4);
    //diff32= dodif32(rf2ttcorbit, bstorbit);
    diff32= rf2ttcorbit - (bstorbit&0xffffff);
    //diff32= (bstorbit&0xffffff) - rf2ttcorbit;
    //GetMicSec(&mesout[2], &mesout[3]);
    usleep(1000000);
    //printf("rf2ttc:%d rf2ttc-bst orbit: %d=%x\n", rf2ttcorbit,diff32,diff32);
    printf("ctp:%x bst:%x ctp-bst orbit: %d=%x bst31..24:%x\n", 
      rf2ttcorbit,bstorbit, diff32,diff32, bstorbit&0xff000000);
    if(QUIT==1) break;
  };
  vmxclose(vspbobr);
} else {
  printf("Bad parameter: %s\n", argv[1]);
};
rc= vmeclose();
return(rcmain);
}

