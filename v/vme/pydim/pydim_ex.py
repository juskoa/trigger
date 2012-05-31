#!/usr/bin/python
import time,string,clientpy

print "Init clientpy (once only)..."
clientpy.dicxinit()

class DimServer:
  def __init__(self, ltuname=None):
    """ rc: self.tag: 
     0 -> connection not successful
    >0 -> connection OK (i.e. execDO method can be used)
    """
    self.gettag1(ltuname)
    self.tag= clientpy.dicxinfo_service(self.resultservice)
    if self.tag==0: 
      print "Error: cannot register"
      return
    result= self.waitinfo()
    if result[:6] == 'failed':
      self.errprt("DImServer(%s)"%ltuname, result)
      self.tag=0
      return
    if ltuname!=None:   # LTU
      clientpy.dicxcmnd_callback(self.tag, self.ltuname+"/PIPE",
        "open "+self.ltuname+" pyclient 1.0\n")
      wimessage= self.waitinfo()
      self.dbgprt(wimessage)
  def dbgprt(self, *args):
    pass
    #print "DBG:", args
  def errprt(self, *args):
    pass
    print "Error:", args
  def waitinfo(self,secs=2):
    result= clientpy.waitinfocall(self.tag, secs);
    # from LTU, result is finished by: NL:NL or :NL
    if self.ltuname!=None:
      if result[-3:]=="\n:\n":
        result= result[:-3]   # remove NL:NL
      elif result[-2:]==":\n":
        result= result[:-2]   # remove :NL
      else:
        if result[:16]!='Server restarted':
          self.errprt("LTU result not ending :NL",result)
    return result
  def execDO(self, cmd):    # send cmd, wait for result and return it
    #result='1234'
    self.dbgprt("execDO:"+self.resultservice+', '+cmd)
    rc= clientpy.dicxcmnd_callback(self.tag, self.docmd, cmd)
    if rc != 1:
      self.errmsg("Bad cmd:",cmd)
      return ''
    result= self.waitinfo()
    self.dbgprt("res>%s<"%result)
    return result
  def __del__(self):
    time.sleep(1)
    clientpy.dicxrelease_service(self.tag)

class DimServerCtp(DimServer):
  def gettag1(self, ltuname):
    self.ltuname=None
    self.resultservice= "CTPDIM/RESULT"
    self.docmd= "CTPDIM/DO"

class DimServerLtu(DimServer):
  def gettag1(self, ltuname):
    self.ltuname= ltuname          # None: ctp connection
    self.resultservice= self.ltuname+"/RESULT"
    self.docmd= self.ltuname+"/DO"

