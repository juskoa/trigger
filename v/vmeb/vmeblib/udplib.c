/* udplib.c
*/
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>      // gethostbyname
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>      //EAGAIN
#include <unistd.h>     //close
#include <string.h> 

/*void diep(char *s) {
  perror(s);
  exit(1);
}*/
// sending:
struct sockaddr_in si_send;
// receiving side:
struct sockaddr_in si_me, si_other;
int srlen=sizeof(si_other);

/*------------------------------------------------------------- udpopens */
/* Usage:   client starts with sending the message
sck= udpopens(); rc= udpsend(sck, buf);
if(sck or rc is -1): error
*/
int udpopens(char *server, int PORTs) {
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
if(s==-1) return(s);
memset((char *) &si_send, 0, sizeof(si_send));
si_send.sin_family = AF_INET; si_send.sin_port= htons(PORTs);
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
int udpopenr(int PORTr) {
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
//printf("Sending packet %s length: %d\n", buf, buflen);
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
rc=recvfrom(s, buf, buflen, 0, (struct sockaddr *)&si_other, (socklen_t *)&srlen);
return(rc);
}

