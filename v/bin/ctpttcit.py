#!/usr/bin/python
# start ttcit.e waiting for triggers
# send burst to fit one ssm ctpstart.e
# assumes that ctp is configured and detector is connectedon ttcit switch
import os,sys,string,time,shlex,subprocess,thread
class iopipe:
  """
     Send command to system . Using subprocess insetad poopen2.
  """
  def __init__(self,cmd,myshell=False):
    self.ret = 0
    args=shlex.split(cmd)
    #print "iopipe args: ",args
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
  def ioprint(self):
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
    for line in self.p.stdout:
      ix2 = string.find(line,text)
      if ix2==0: 
       print line
       return line
    return None
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
    #iop=iopipe(sys.argv[1])
    iottcstart = iopipe("ssh -2 -q alidcsvme007 /local/trigger/v/vme/ctp++/startttc.e")
    if iottcstart.ret == 1: return;    
    ioctp = iopipe("ssh -2 -q alidcsvme001 /local/trigger/v/vme/ctp++/ctpstart.e")
    if ioctp.ret == 1: return;    
    iottcread = iopipe("ssh -2 -q alidcsvme007 /local/trigger/v/vme/ctp++/readttc.e")
    if iottcread.ret == 1: return;    
    #time.sleep(1)
    #iottcstart.ioprint()
    #ioctp.ioprint()
    iottcread.printlast4()
    return
if __name__ == "__main__":
  rc=main()
  sys.exit(rc)
