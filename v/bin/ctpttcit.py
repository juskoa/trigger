#!/usr/bin/python
# start ttcit.e waiting for triggers
# send burst to fit one ssm ctpstart.e
# assumes that ctp is configured and detector is connectedon ttcit switch
import os,sys,string,time,shlex,subprocess,thread
class command:
  """
     Send command to system . Using subprocess insetad poopen2.
  """
  def __init__(self,cmd,myshell=False):
    self.ret = 0
    args=shlex.split(cmd)
    #print "command args: ",args
    self.p = subprocess.Popen(args,bufsize=-1,stdout=subprocess.PIPE,stderr=subprocess.STDOUT).communicate()
    if len(self.p) != 2:
       print "Unexpeced len of subprocess output: ", len(self.p)
       self.ret = 1
    if self.p[1] != None:
       print "Error: ", self.p[1]
       self.ret =2
    # another way how to call
    #  self.p = subprocess.Popen(cmd,bufsize=-1,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,shell=True) 
    return
  def poll(self):
    i=0
    while(i<5):
      print self.p.poll()
      time.sleep(1)
      i+=1
    return self.p.poll()  
  def cmdprint(self):
    i=0
    for item in self.p: 
       print i,' ',item
       i+=1
  def printlast4(self):
    """
      Print last 4 linesof output.
    """
    print self.p[0]
  def check(self,text):
    """
      Find text in command output. return line or None if not found.
    """
    ix2 = string.find(self.p[0],text)
    if ix2==0: return 1 
    return 0
def main():
  print """
This script assumes that:
1.) CTP is correctly configured, i.e. sending some triggers,
2.) Required detector is connected to TTCIT.
"""
  rc=0
  if len(sys.argv) != 1:
    print sys.argv
  else:
    print sys.argv
    host = command("hostname")
    #host.cmdprint()
    if host.check("avmes"):
      print "Running from avmes"
      ctpcpu="altri2"
      ttcitcpu = "altri1"
      dir1 = " /localavmes/trigger/v/vme/ctp++"
    else:
      ctpcpu="alidcsvme001"
      ttcitcpu = "alidcsvme007"
      dir1 = " /local/trigger/v/vme/ctp++"
    dir = " /local/trigger/v/vme/ctp++"
    #cmd=command(sys.argv[1])
    cmdttcstart = "ssh -2 -q " + ttcitcpu + dir + "/startttc.e"
    cmdctpstart = "ssh -2 -q " + ctpcpu   + dir1 + "/ctpstart.e"
    cmdreadttc = "ssh -2 -q " + ttcitcpu + dir + "/readttc.e"
    iottcstart = command(cmdttcstart)
    if iottcstart.ret == 1: return;    
    ioctp = command(cmdctpstart)
    if ioctp.ret == 1: return;    
    iottcread = command(cmdreadttc)
    if iottcread.ret == 1: return;    
    #time.sleep(1)
    iottcstart.cmdprint()
    ioctp.cmdprint()
    iottcread.printlast4()
    return
if __name__ == "__main__":
  rc=main()
  sys.exit(rc)
