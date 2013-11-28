#include "CONNECT.h"
CONNECT::CONNECT(){
 CheckAll();
}
CONNECT::~CONNECT(){}
//-----------------------------------------------------------------------------
void CONNECT::CheckAll(){
  int rc=0;
  error_count=0;
  list<BOARD*>::iterator bo1;
  for(int i=0;i<200;i++){
	  cout << "Loop number " << i << "#################################################" << endl;
	  for(bo1 = ctp.boards.begin();bo1 != ctp.boards.end();++bo1){
		  string name=(*bo1)->getName();
		  if(name == "l0"){
			  if(ctp.l1)rc=rc+CheckConnections(ctp.l0,ctp.l1,"inmon");
			  if(ctp.l2)rc=rc+CheckConnections(ctp.l0,ctp.l2,"inmon");
			  if(ctp.inter)rc=rc+CheckConnections(ctp.l0,ctp.inter,"inmon");
			  for(int i=0;i<NUMOFFO;i++)
				  if(ctp.fo[i])rc=rc+CheckConnections(ctp.l0,ctp.fo[i],"inmonl0");
		  }else if(name == "l1"){
			  if(ctp.l2)rc=rc+CheckConnections(ctp.l1,ctp.l2,"inmon");
			  for(int i=0;i<NUMOFFO;i++)
				  if(ctp.fo[i])rc=rc+CheckConnections(ctp.l1,ctp.fo[i],"inmonl1");
			  if(ctp.inter)rc=rc+CheckConnections(ctp.l1,ctp.inter,"inmon"); 
		  }else if(name == "l2"){
			  for(int i=0;i<NUMOFFO;i++)
				  if(ctp.fo[i])rc=rc+CheckConnections(ctp.l2,ctp.fo[i],"inmonl2");
			  if(ctp.inter)rc=rc+CheckConnections(ctp.l2,ctp.inter,"inmon"); 
		  }else if(name == "busy"){
			  if(ctp.l0)rc=rc+CheckConnections(ctp.busy,ctp.l0,"inmon"); 
		  }else if(name == "fo"){
			  //for(int i=0;i<NUMOFLTUS;i++)
			  //   if(ctp.ltu[i])rc=rc+CheckConnections((*bo1),ctp.ltu[i],"inmon"); 
		  }
	  }
	  // LTU connection not ready
	  //return ;
	  for(int i=0;i<NUMOFFO;i++){
		  for(int j=0;j<NUMOFCON;j++){
			  if(ctp.fo2det[i][j] && ((ctp.fo2det[i][j]->ltunum) >= 0)){
				  rc=rc+CheckConnections(ctp.fo[i],ctp.ltu[ctp.fo2det[i][j]->ltunum],"inmon");
			  }
		  }
	  }
       //if(error_count)exit(1);
  }
}
//----------------------------------------------------------------------------
void CONNECT::PrintConnections(list<Connection> &Connections) const
{
	list<Connection>::iterator con;
	for(con = Connections.begin(); con != Connections.end(); ++con){
		cout << con->name << " " << dec << con->channel1 << " " << con->channel2 << endl;
	}
}
//----------------------------------------------------------------------------
int CONNECT::CheckConnections(BOARD *b1,BOARD *b2,string const &mode){
	list<Connection> Connections;
	if((b1==0) || (b2==0)){
		cout << "CheckConnection: wrong boards " << b1 << " " << b2 << endl;
		return 0;
	}
	FindConnections(b1,"outgen",b2,mode,Connections);
	if(Connections.empty()){
		cout << "No connections between "<< b1->getName() << " and ";
		cout << b2->getName() << endl;
		return 1;
	}
	if(Connect(b1,b2,mode)) return 1;
	//PrintConnections(Connections);
	//int offset=b2->ssmtools.findOffset();
	int offset=100;
        int ch1=0,ch2=0,p1=0,p2=0;
        //int rc1=b2->ssmtools.checkSignatureCon(Connections);
        //int rc2=3;
	int rc1=b1->ssmtools.CompSSMch(b2->GetSSM(),offset,Connections,&ch1,&p1);
	int rc2=b1->ssmtools.CompSSMch(b2->GetSSM(),offset+1,Connections,&ch2,&p2);
        
	//cout << "ret= " << rc1 << " " << rc2 << endl;
	if((rc1+rc2) != 1){
                int ch,p;
                if(p1>offset+10){ 
                   p=p1;
                   ch=ch1;
                }else{
                   p=p2;
                   ch=ch2;
                }
		cout << " ----------------------------------> Error detected." << endl;
                char error[1024]="";
                sprintf(error,"%sto%s_%i_%i_%i",b1->getName().c_str(),b2->getName().c_str(),error_count,ch,p);
                b2->ssmtools.dumpSSM(error);
                //sprintf(error,"%s_%i_%i_%i",b1->getName().c_str(),error_count,ch,p);
                //b1->ssmtools.dumpSSM(error);
                error_count=error_count+1;
		//return 1;
                exit(1);
	}
	cout << " OK." << endl;
	return 0;
}
//----------------------------------------------------------------------------
int CONNECT::FindConnections(BOARD *b1,string const &mode1,BOARD *b2,string const &mode2,list<Connection> &Connections)
{
	string *b1chans,*b2chans;
	b1chans=b1->GetChannels(mode1);
	b2chans=b2->GetChannels(mode2);
	//b1->PrintChannels(mode1);
	//b2->PrintChannels(mode2);
	for(int i=0;i<32;i++){
		if(b1chans[i] != "empty"){
			for(int j=0;j<32;j++){
				if(b1chans[i] == b2chans[j]){
					//cout << i << " " << j << " " << b1chans[i] << endl;
					Connection connection={b1chans[i],i,j};
					Connections.push_back(connection);
				}
			}
		}
	}
	return 0;
}
//-----------------------------------------------------------------------------
int CONNECT::Connect(BOARD *b1,BOARD *b2,string const &mode){
	cout << "Testing: "<< b1->getName()<<"["<<b1->getboardbase()<<"]";
	cout <<"->"<< b2->getName()<<"["<<b2->getboardbase()<<"]";
	b1->ssmtools.genSeq(2,0);
	//b1->ssmtools.genSignatureAll();
	b1->WritehwSSM();
	if(b1->SetMode("outgen",'c')) return 1;
	b2->WriteSSM(0);
	b2->WritehwSSM();
	if(b2->SetMode(mode,'1')) return 1;
	b1->StartSSM();
	mysleep(300);
	b2->StartSSM();
	mysleep(40000);
	b2->StopSSM();
	b1->StopSSM(); 
	b2->ReadSSM();
	//b2->WriteSSM(0); // for debug
	//b2->PrintSSM(first,10);
	return 0;	
}
