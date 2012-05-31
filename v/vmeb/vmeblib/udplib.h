// defualt port where to send mon. messages
#define send2PORT 9930

int udpopens(char *server, int port);
int udpopenr(int port);
void udpclose(int s);
int udpsend(int sock, unsigned char *buf, int buflen);
int udpwaitr(int s, unsigned char *buf, int buflen);

