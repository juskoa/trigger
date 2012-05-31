#!/usr/bin/python
import time,clientpy

class ctpdim:
  def __init__(self):
    self.tag= clientpy.dicxinfo_service("CTPDIM/RESULT")
    if self.tag==0: 
      print "Error: cannot register"
      return
    result= self.waitinfo()
    if result[:6] == 'failed'
      print "Error: "+result
      self.tag=0
  def waitinfo(self,secs=2):
    result= clientpy.waitinfocall(self.tag, secs);
    return result
  def execDO(self, cmd):    # send and wait for result
    #result='1234'
    print "execDO:CTPDIM/DO:"+cmd
    rc= clientpy.dicxcmnd_callback(self.tag,"CTPDIM/DO", cmd)
    if rc != 1:
      return ''
    result= self.waitinfo()
    print "res>%s<"%result
    return result
  def __del__(self):
    time.sleep(1)
    clientpy.dicxrelease_service(self.tag)
 
class ltuproxy:
  def __init__(self, ltu):
    self.ltuname= ltu
    self.tag= clientpy.dicxinfo_service(self.ltuname+"/RESULT")
    if self.tag==0: 
      print "Error: cannot register"
      return
    print self.waitinfo()
    clientpy.dicxcmnd_callback(self.tag, self.ltuname+"/PIPE",
      "open "+self.ltuname+" pyclient 1.0\n")
    print self.waitinfo()
  def waitinfo(self,secs=2):
    result= clientpy.waitinfocall(self.tag, secs);
    return result
  #def execcmd(self, cmd):   # only send (i.e. dic_cmnd_callback)
  #  clientpy.dicxcmnd_callback(self.tag, self.ltuname+"/DO", cmd)
  #  time.sleep(2)
  def execDO(self, cmd):    # send and wait for result
    #result='1234'
    print "execDO:"+self.ltuname+"/DO:"+cmd
    rc= clientpy.dicxcmnd_callback(self.tag, self.ltuname+"/DO", cmd)
    if rc != 1:
      return ''
    result= self.waitinfo()
    print "res>%s<"%result
    return result
  def __del__(self):
    #clientpy.dicxcmnd_callback(self.tag, self.ltuname+"/PIPE","close ltunam pyclient 1.0\n")
    time.sleep(1)
    clientpy.dicxrelease_service(self.tag)

def main():
  clientpy.dicxinit()
  #clientpy.dicxcmnd_callback("CTPRCFG/RCFG","pclieEJJJw\n")
  #time.sleep(5)
  #init hmpid ltu connection:
  print "--------------------------- CTPDIM connecting..."
  ctp= ctpdim()
  #do something
  #proxy.execcmd("vmeopr32(VERSION_ADD)\n")
  print "--------------------------- HMPID connecting..."
  ltu= ltuproxy("hmpid")
  # measure busy L0/1/2 INT:
  answer=ctp.execDO("CHECKPHASES")
  print "ctp:BUSY L0/1/2 INT phases:>%s<"%answer
  # measure FO1:
  answer=ctp.execDO("TOGGLE 1 HMPID")
  answer=ltu.execDO("setglobalmode()\n")
  answer=ltu.execDO("getbcphase(0)\n")
  print "%s:FO1(hmpid) phase>%s<"%(ltu.ltuname,answer)
  answer=ctp.execDO("TOGGLE 0 HMPID")
  answer=ltu.execDO("setstdalonemode()\n")
  #answer=ctp.execDO("W 1\n")
  answer=ltu.execDO("getbcphase(1)\n")
  print "%s:phase>%s<"%(ltu.ltuname,answer)
  #for ix in range(100):
  #  answer=proxy.execDO("vmeopr32(VERSION_ADD)\n")
  #close connection:
  ctp=None; ltu=None
  #proxy=None

if __name__=="__main__":
  main()

