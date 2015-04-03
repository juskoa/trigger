#!/usr/bin/python
# start ttcit.e waiting for triggers
# send burst to fit one ssm ctpstart.e
# assumes that ctp is configured and detector is connectedon ttcit switch
import os,sys,string,time,shlex,subprocess
class command:
  """
     Send command to system . Using subprocess insetad poopen2.
  """
  def __init__(self,cmd,par="",myshell=False):
    self.ret = 0
    args=shlex.split(cmd)
    #print "command args: ",args
    #print "par= ", par
    self.p = subprocess.Popen(args,bufsize=-1,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,stdin=subprocess.PIPE).communicate(par)
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
  def cmdprint0(self):
    """
      Print last 4 linesof output.
    """
    print self.p[0]
  def check(self,text):
    """
      Find text in command output. return line or None if not found.
    """
    ix2 = string.find(self.p[0],text)
    #print ix2
    if ix2 >= 0: return 1 
    return 0
cpu2="alidcsvme002"
cpu5="alidcsvme005"
ltu0="0x810000"
ltu1="0x811000"
ltu2="0x812000"
detectors={ 't0':[cpu2,ltu2],'ssd':[cpu2,ltu0],'fmd':[cpu2,ltu1],
           'tpc':[cpu5,ltu0],'pmd':[cpu5,ltu1],'acorde':[cpu5,ltu2]
          }
def main():
  #print """
#This script assumes that:
#1.) LTU emulation is configured,
#2.) Required detector is connected to TTCIT using ttcitswitch.
#"""
  rc=0
  if len(sys.argv) != 2:
    print "Expected one argument - name of the detctor to test."
    #print sys.argv
  else:
    #print sys.argv
    det=sys.argv[1]
    host = command("hostname -s")
    #host.cmdprint()
    if host.check("avmes"):
      print "Running from avmes"
      ltucpu="altri2"
      ttcitcpu = "altri1"
      dir1 = " /localavmes/trigger/v/vme/ctp++"
      print "To be done"
      return 
    elif host.check("alidcscom835"):
      if detectors.has_key(det):
       ltucpu=detectors[det][0]
       ltu=detectors[det][1]
      ttcitcpu = "alidcsvme007"
      dir1 = " /local/trigger/v/vme/"
    else:
      print "Unknown host: "
      host.cmdprint0()
      return
    dir = " /local/trigger/v/vme/"
    #
    # Do ttcit switch
    #
    ans = raw_input("Is detector ltu  connected to ttcitswitch ? \n If yes type Y \n")
    if ans != 'Y':
      cmdttcswi = "ssh -2 -q " + ttcitcpu + " /local/trigger/v/bin" + "/ttcitsw.py "+det
      iottcsw = command(cmdttcswi)
      #print iottcsw
      if iottcsw.ret != 0: return
      if iottcsw.check("ERR"): 
        print "ERROR: "
        iottcsw.cmdprint()
        return
      else: print "ttcitswitch set OK."
      #
      # Do TTCinit
      #
      #cmdltu = "ssh -2 -q " + ltucpu   + dir1 + "ltu/ltu.exe -noboardInit " + ltu
      #ioltuttcinit = command(cmdltu,par="TTCinit()")
      #print ioltuttcinit.cmdprint()
      #ioltuload = command(cmdltu,par="SLMload(\"WORK/slmseq.seq\")\n")
      #print ioltuload.cmdprint()
      #return
      print "\n"
      #text = "Start ltu GUI: vmecrate "+det+ "\n When ready type: Y \n"
      #ans = raw_input(text)
      #if ans != 'Y': return
      ans = raw_input("Do TTCinit and type Y: ")
      if ans != 'Y': return
    print "Set up emulator: \n 1. choose sequence \n 2. select start signal and frequncy \n 3. Make sure BUSYs are disabled\n"
    ans = raw_input("When readt type Y \n" )
    if ans != 'Y': return
    # 
    # TTC start
    #
    cmdttcstart = "ssh -2 -q " + ttcitcpu + dir + "ctp++/startttc.e"
    iottcstart = command(cmdttcstart)
    if iottcstart.ret == 1: return;    
    iottcstart.cmdprint()
    #
    #  LTU start
    #
    cmdltustart = "ssh -2 -q " + ltucpu   + dir1 + "ctp++/ltustart.e " + ltu
    #print cmdltustart
    ioltu = command(cmdltustart)
    if ioltu.ret == 1: return; 
    ioltu.cmdprint()
    #
    #  Read TTC and analyse
    #   
    cmdreadttc = "ssh -2 -q " + ttcitcpu + dir + "ctp++/readttc.e"
    iottcread = command(cmdreadttc)
    if iottcread.ret == 1: return;    
    iottcread.cmdprint0()
    return
if __name__ == "__main__":
  rc=main()
  sys.exit(rc)
