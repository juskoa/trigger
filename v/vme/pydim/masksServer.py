#!/bin/env python
# 16.5.2012 services:
# CTPBCM/X string
# where X can be (like in VALID.BCMASKS): EMPTY B AC A C ACE S SA SC D ?
import sys,os,time,string,signal,pydim, pylog, trigdb

def rmzero(strg):
  if strg[-1]=='\0':
    rcstr= strg[:-1]
    #print "rmzero:%s:%s:"%(strg,rcstr)
  else:
    #print 'rmzero:%s'%strg
    rcstr= strg
  return rcstr
def epochtime():
  return "%.3f"%time.time()   #epoch
def loctime(epoch_str):   
  lt= time.localtime(float(epoch_str))
  rc="%2.2d.%2.2d %2.2d:%2.2d"%(lt[2], lt[1], lt[3], lt[4])
  return rc   # dd.mm hh:mm

services=[]
quit=None

def signal_handler(signal, stack):
  global quit
  if (signal==15) or (signal==2):   # 15:SIGTERM (default), 2: CTRL-C
    mylog.logm("Stopping, signal:%d"%signal)
    quit= 'q'
  else:
    mylog.logm("signal:%d received (10:SIGUSR1 = update)."%signal)
  if signal==10:
    updateAll()

class Service:
  def __init__(self, name, tag):
    self.name= name   # "A", "E",...
    servicename= "CTPBCM/" + self.name
    self.tag= tag
    #mylog.logm("Adding service "+self.name)
    self.sid = pydim.dis_add_service(servicename, "C", scope_cb, tag)
    mylog.logm("%s: %d"%(servicename, self.sid))
    # A service must be updated before using it.
    #pydim.dis_update_service(self.sid,(" ",))  -leads to error
    pydim.dis_update_service(self.sid)
    pass
  def update(self, value=None):
    if value==None or value=="":
      pydim.dis_update_service(self.sid)
    else:
      pydim.dis_update_service(self.sid,(str(value),))
  def bcmname(self):
    if self.name=='E':
      return "bcmEMPTY"
    else:
      return "bcm"+self.name

def scope_cb(tag):
    """
    Service callbacks are functions (in general, Python callable objects)
    that take one argument: the DIM tag used when the service was added,
    and returns a tuple with the values that corresponds to the service
    parameters definition in DIM.
    """
    mylog.logm("Running callback function (i.e. Case1) for xxx tag:%s"%str(tag))
    # Calculate the value
    # This example returns a string with the current time
    now = time.strftime("%X")
    # Remember, the callback function must return a tuple
    #return ("%s. %s"%(now,"blabla"),)
    return (" ",)
def updateAll():
  bcmasks= trigdb.TrgMasks() ; bcms=""
  for serv in services:
    bcmname= serv.bcmname() ; bcms= bcms + serv.name +' '
    value= bcmasks.getmask(bcmname)
    #print "msk:",bcmname, value,":"
    serv.update(value)
  mylog.logm("updateAll: "+bcms)
  mylog.flush()
def main():
  global mylog, services
  sids= {}
  mylogfn= os.path.join(os.environ["VMEWORKDIR"], "WORK/masksServer")
  #mylog= pylog.Pylog(None,"ttyYES")   # only tty (no file log)
  mylog= pylog.Pylog(mylogfn)
  dnsnode= pydim.dis_get_dns_node()
  if not dnsnode:
    mylog("No Dim DNS node found. Please set the environment variable DIM_DNS_NODE")
    sys.exit(1)
  mypid= str(os.getpid())
  mylog.logm("dns:"+dnsnode+ " mypid:"+mypid)
  mypidfn= os.path.join(os.environ["VMEWORKDIR"], "WORK/masksServer.pid")
  f= open(mypidfn,"w")
  f.write(mypid) ; f.close()
  signal.signal(signal.SIGUSR1, signal_handler)  # 10         SIGUSR1
  signal.signal(signal.SIGHUP, signal_handler)   # 1  kill -s SIGHUP mypid
  #signal.signal(signal.SIGKILL, signal_handler)   # ?
  signal.signal(signal.SIGQUIT, signal_handler)   # 3
  signal.signal(signal.SIGTERM, signal_handler)   # 15 -default
  signal.signal(signal.SIGINT, signal_handler)   # 2 CTRL C
  mtall= ['B','A','C','S','SA','SC','D','E','I']
  for mtag in range(len(mtall)):
    services.append(Service(mtall[mtag], mtag))
  updateAll()
  pydim.dis_start_serving("CTPBCM")
  mylog.logm("Starting the server ...")
  while True:
    a=""
    try:
      #a= raw_input('enter 1 2 3: update from VALID.BCMASKS or q:\n')
      time.sleep(600)
    except:
      mylog.logm("exception:"+str(sys.exc_info()[0]))
      if quit=='q':
        a='q'
      else:
        continue
    if a=='q' or quit=='q': break
    elif a=='1':   # case1
      tist= epochtime()
      mylog.logm("updating at time:%s = %s"%(loctime(tist), str(tist)))
      #pydim.dis_update_service(sids[mt])
      services[0].update()
    elif a=='2':   # case2, explicit update
      tist= epochtime()
      msg="update service directly,time:%s = %s"%(loctime(tist), str(tist))
      mylog.logm(msg)
      #pydim.dis_update_service(sids[mt],("%s"%(tist+"\0"),))
      services[0].update(msg)
    elif a=='3':   # read VALID.BCMASKS and update all service
      updateAll()
    else:
      #mylog.logm('bad input:%s'%a) ; continue
      pass
  pydim.dis_stop_serving()
  #sys.stdout.flush()
  mylog.close()
  os.remove(mypidfn)
        
if __name__ == "__main__":
    main()

