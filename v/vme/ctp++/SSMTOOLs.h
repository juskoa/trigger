#ifndef _SSMTOOLs_h_
#define _SSMTOOLs_h_
#include "libctp++.h"
#include <list>
#define NUMOFCHAN 32
#define HEADERLEN 8
#define SIGNATLEN 7
#define HEADER 0xb1
// 0xb1=0x8d !
class SSMTOOLs
{
public:
         SSMTOOLs();
         void setssm(w32 *ssm);
         int genSeq(int Period,int Start);
         int findOffset() const;
	 int find0();
         int CompSSM(w32 *ssm2);
	 int CompSSM(w32 *ssm2,int start);
	 int CompSSMch(w32 *ssm2,int offset,int channel);
	 int CompSSMch(w32 *ssm2,w32 const mask1,w32 const mask2) const;
         int CompSSMch(w32 *ssm2,int const offset,list<Connection> &Connections,int *channel,int *position) const;
	 int writeSPP(int Start,int Channel,char *Pattern);
	 int genSignatureAll();
         int checkSignatureCon(list<Connection> &Connections);
         int dumpSSM(char *name);
private:
        w32 *ssm;
        w32 getNumber(int i0,int length,w32 mask);
	void writeNumber(int i0,int length,w32 mask, w32 Number);
	int genSignature(int *channels,int *start,int *dist);
	int checkSignature(int *channels,int offset);
};
#endif
