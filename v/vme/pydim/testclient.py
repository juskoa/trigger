#!/usr/bin/python
import time,clientpy

class ltuproxy:
  def __init__(self, ltu):
    self.ltu= ltu
    self.RESULTid= clientpy.dicxinfo_service(ltu+"/RESULT")
    print self.waitinfo()
    clientpy.dicxcmnd_callback(ltu+"/PIPE","open "+ltu+" pyclient 1.0\n")
    print self.waitinfo()
  def waitinfo(self,secs=2):
    #result='a' ; result.zfill(80) nebavi
    #result='1234567890abcdefghijklmnopqrstuvwxyz'
    result= clientpy.waitinfocall(secs);
    return result
  #def execcmd(self, cmd):   # only send (i.e. dic_cmnd_callback)
  #  clientpy.dicxcmnd_callback(self.ltu+"/DO", cmd)
  #  time.sleep(2)
  def execDO(self, cmd):    # send and wait for result
    #result='1234'
    rc= clientpy.dicxcmnd_callback(self.ltu+"/DO", cmd)
    if rc != 1:
      return ''
    result= self.waitinfo()
    return result
  def __del__(self):
    #clientpy.dicxcmnd_callback(self.ltu+"/PIPE","close ltuname pyclient 1.0\n")
    time.sleep(1)
    clientpy.dicxrelease_service(self.RESULTid)

#clientpy.dicxcmnd_callback("CTPRCFG/RCFG","pclieEJJJw\n")
#time.sleep(5)
#init hmpid ltu connection:
print "--------------------------- connecting..."
proxy= ltuproxy("hmpid")
#do something
#proxy.execcmd("vmeopr32(VERSION_ADD)\n")
print "--------------------------- VERSION_ADD..."
#answer=proxy.execDO("vmeopr32(VERSION_ADD)\n")
answer=proxy.execDO("getbcphase(1)\n")
print "answer>%s<"%answer
#answer>0xffffffa7
#:
#<
for ix in range(100):
  answer=proxy.execDO("vmeopr32(VERSION_ADD)\n")
#close connection:
proxy=None
