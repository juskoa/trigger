#include "CTP.h"
class CONNECT{
 public:
         CONNECT();
         ~CONNECT();
 private:
         CTP ctp;
         int Connect(BOARD *b1,BOARD *b2,string const &mode);
         void CheckAll();
         int CheckConnections(BOARD *b1,BOARD *b2,string const &mode);
         int FindConnections(BOARD *b1,string const &mode1,BOARD *b2,string const &mode2,list<Connection> &Connections);
         void PrintConnections(list<Connection> &Connections) const;
         int error_count;
};
