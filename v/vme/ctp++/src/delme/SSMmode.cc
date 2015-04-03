#include "SSMmode.h"
SSMmode::SSMmode()
{
 for(int i=0;i<32;i++)channels[i]="empty";
}
SSMmode::SSMmode(string const name,string const board,w32 const modecode):
modename(name),boardname(board),modecode(modecode):
{
 
 for(int i=0;i<32;i++)channels[i]="empty";
}
//-----------------------------------------------------------------------------
//w32 SSMmode::Init(string boardname, string modename,w32 mode) 
w32 SSMmode::Init() 
/*
 *  Get Channels from file
 */ 
{
 //this->boardname=boardname;
 //this->modename=modename;
 //modecode=mode;
 // this should be done via $VME???
 //string name="CFG/ctp/ssmsigs/"+d_name+"_"+modename+".sig";
 string name="../CFG/ctp/ssmsigs/"+boardname+"_"+modename+".sig";
 //cout << "Mode file:"<< name << endl;
 modefile.open(name.c_str());
 if(!modefile.is_open()){
  cout << "File "<< name << " cannot be opened, exiting." << endl;
  exit(1);
 }else{
  //cout << "File "<< name << " opened successfully" << endl;
 }
 bool modflag=1;
 while(! modefile.eof()){
  string buffer;
  getline(modefile,buffer);
  //cout << buffer << endl;
  int channel=atoi(buffer.substr(0,2).c_str());
  //cout << channel << endl;  
  if(buffer.length() < 3) continue;
  if(channel <= 32){
    int i=0;while(buffer[i] != ' ')i++;
    //cout << buffer.length() << endl;
    string name=stripstring(buffer.substr(i,buffer.length()));
    //string name=buffer.substr(3,buffer.length());
    //cout << name << endl;
    if(channel < 32){
      channels[channel]=name;
    }
    else if(channel == 32){    // mode
      modflag=0;
      modecode=parsemode(name);
    }
  }
 }
 modefile.clear();
 modefile.close();
 if(modflag){
   cout << "GetCHannels: no mode in file " << modename << endl;
  return 1;
 }
}
//-----------------------------------------------------------------------------
int SSMmode::parsemode(string const &mode) const
{
 w32 modecode=0;
 for(int i=0;i<6;i++){
    if(mode[i] == '1') modecode=modecode + (1<<i);
 }
 if(mode[6] == '1') modecode=modecode + (1<<8);
 if(mode[7] == '1') modecode=modecode + (1<<9);
 //cout << "parsemode: " << hex << modecode << endl;
 return modecode;
}

