#!/usr/bin/python
import os,sys, popen2,time, string
from socket import *  
"""
see: /home/ref/ctp_proxy:
  rorcreset          -to bestarted on pcald40
  rorc_reset -b -s   - to be started on pcald36
  server.py          - to be started on pcald36
"""
serverHost = 'pcald36'          # server name, or: 'starship.python.net'
serverPort = 50009    

class iopipe:
  def __init__(self, nbcmd):
    self.nbcmd=nbcmd
    print "popen2("+nbcmd+")"
    self.iop= popen2.popen2(nbcmd, 1) #0- unbuffered, 1-line buffered
    self.printstdout()
  def printstdout(self, catch=None):
    iline=0; rc=None
    rc=None
    while(1):
      line= self.iop[0].readline()
      if line ==':\n':   #don't take last '\n:\n'
        break
      if line =='':
        #line="pipe from cmdline interface closed/broken\n"
        print 'empty line'
        break
      if catch!=None:
        if string.find(line,catch)!=-1:
        #print "PHYSEVENTS found",line
          if catch=='PHYSEVENTS:':
            a= string.split(line)
            rc=a[1]
          if catch=='event generated at':
            rc=line[:-1]
          if catch=='LTU after EOD':
            #print line
            a= string.split(line); rc= a[-2]   # L2a
      iline=iline+1
      #print line[:-1]
      #if iline>6: break
    return rc
  def cmd(self, cmd, catch=None):
    rc=None
    self.iop[1].write(cmd+'\n')
    #print self.nbcmd+':'+cmd
    if cmd!='q':
      rc= self.printstdout(catch)
    return rc
class remote:
  def __init__(self):
    self.sockobj = socket(AF_INET, SOCK_STREAM) # make a TCP/IP socket object
    self.sockobj.connect((serverHost, serverPort))  
  def cmd(self, cmd, getroc=None):
    """ Operation:
    - send cmd
    - receive answer
      if getroc==None:
        print answer; return None
      else:
        process answer (result in rc, if error: rc==None)
      if cmd==exit: close socket
      return rc  
    """
    self.sockobj.send(cmd)   # send line to server over socket
    #ri= raw_input("receive?")
    data = self.sockobj.recv(10024) # receive line from server: up to 1k
    #print "Data>",data,"<"

    rc= None
    if getroc==None:
       pass
       #  print data
    elif getroc=="roc": 
      #>0x40000004   0xb15e1004   0x00000004            4   0x010e4931     17713457
      if len(data)>60:
        rc=data[58]
      #print data
    elif getroc=="pid2":
      #print "pid2:",data
      rc= string.split(data[1:])[0]    # remove leading > before split
    elif getroc=="events":
     #The program has read 1 event(s).
      s= string.split(data)
      if len(s)>5:
        if s[1]=='program':
          rc= s[4]
    else:
      print "Unknown getroc:", getroc
      print data
    if cmd=="exit":
      print "exiting ", serverHost
      self.close()
    return rc
  def close(self):
    print 'closing self.sockobj'
    self.sockobj.close()                 

def byhand(self):
    while(1):
      il=raw_input(
        "'qq: quit'  't text': send to test  'r text':send to server\n")
      if il=='qq' or il=='': break
      elif il[:2]=='t ': 
        cmd= string.split(il,' ',1)
        test.cmd(cmd[1])
      elif il[:2]=='r ': 
        cmd= string.split(il,' ',1)
        rorc.cmd(cmd[1])
      else:
        print "bad input:", il
        continue
    time.sleep(1)
    rorc.cmd('exit')
    test.cmd("q")
    exit

def main():
  test= iopipe("linux/test")
  rorc= remote()
  rorc.cmd('cd /date/rorc/Linux')
  waitcmd="./rorc_receive -d -x 0 -e 1 |grep 'The program has read'"
  # version for physics events (rorc_receive started in background 
  # and killed than)
  dumpcmd="../../physmem/Linux/physmemDump -o 0 -L 10 |grep '0x40000008   0'"
  ltueodold=runs=0; okSOD=okEOD=allexpected=allgot=0
  #self.byhand()
  while(1):
    test.cmd("M")
    rorc.cmd(waitcmd)
    rc=rorc.cmd(dumpcmd,'roc')
    if rc==None: break
    if rc=='e': okSOD=okSOD+1
    rorc.cmd("./rorc_receive -d -x 0 >/tmp/rorclog & ")
    #print "PID2:"+pid+":"
    time.sleep(1)   # give it some time to start
    realevents= test.cmd("G", 'PHYSEVENTS:')
    if realevents==None: break
    #print "events from CTP L2a counter:",realevents
    time.sleep(1)   # give it some time to receive triggers
    pid= rorc.cmd("ps |grep rorc_receive0", "pid2")
    if pid==None: break
    killcmd= "kill -3 " + pid
    rorc.cmd(killcmd)
    time.sleep(1)
    ddg=rorc.cmd("tail -10 /tmp/rorclog | grep 'The program has read'", 'events')
    if ddg==None: break
    #bad events=rorc.cmd("grep 'The program has read' /tmp/rorclog", 'events')
    irealevents= int(realevents)
    if int(ddg) != irealevents:
      print 'exp. events:', realevents, " got:",ddg
    allexpected=allexpected +irealevents
    allgot=allgot +int(ddg)
    # now check end of data:
    #time.sleep(0.1)
    ltucnts= test.cmd("S", "LTU after EOD")
    ltul2abs= int(ltucnts)
    #print "ltul2a:",ltul2abs
    if ltul2abs >= ltueodold: ltul2a= ltul2abs - ltueodold
    else: ltul2a= ltueodold- ltul2abs
    #print "ltul2a:",ltul2a, ltueodold
    ltueodold= ltul2abs
    rorc.cmd(waitcmd)
    rc=rorc.cmd(dumpcmd, 'roc')
    if rc==None: break
    if rc=='f': okEOD=okEOD+1   # EOD expected now
    if runs>1:
      if (ltul2a-2) != irealevents:
        print "Run:%d CTP:%d LTU:%d DDG:%d"%\
          (runs, irealevents, ltul2a-2, int(ddg))
    #time.sleep(0.1)
    runs=runs+1
    if (runs%10)==0: 
      print "runs:",runs,"okSOD:",okSOD, "okEOD:", okEOD, "ph:",allexpected,\
        "ph received:", allgot
    if runs>=200:
      break
  time.sleep(1)
  rorc.cmd('exit')
  test.cmd("q")

if __name__ == "__main__":
    main()

