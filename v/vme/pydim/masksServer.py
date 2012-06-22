#!/bin/env python
# 16.5.2012 services:
# CTPBCM/X string
# where X can be (like in VALID.BCMASKS): EMPTY B AC A C ACE S SA SC D ?
import sys,os,time,string,signal,pydim, pylog

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
def signal_handler(signal, stack):
  global quit
  mylog.logm("signal:%d received (1:SIGHUP, 10:SIGUSR1)."%signal)
  if signal==1:
    quit= 'q'

class service:
  def __init__(self, name, tag):
    self.name= name
    self.tag= tag
    mylog.logm("Adding service "+self.name)
    self.sid = pydim.dis_add_service(self.name, "C", scope_cb, tag)
    # A service must be updated before using it.
    pydim.dis_update_service(self.sid)
    pass
  def update(self, value=None):
    if value==None:
      pydim.dis_update_service(self.sid)
    else:
      pydim.dis_update_service(self.sid,("%s"%(value),))

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
    return ("%s. %s"%(now,"blabla"),)
#def updateAll():
#  for line in 

def main():
  global mylog
  sids= {}
  mylog= pylog.Pylog(None,"ttyYES")   # only tty (no file log)
  dnsnode= pydim.dis_get_dns_node()
  if not dnsnode:
    mylog("No Dim DNS node found. Please set the environment variable DIM_DNS_NODE")
    sys.exit(1)
  mypid= str(os.getpid())
  mylog.logm("dns:"+dnsnode+ " mypid:"+mypid)
  signal.signal(signal.SIGUSR1, signal_handler)  # 10         SIGUSR1
  signal.signal(signal.SIGHUP, signal_handler)   # 1  kill -s SIGHUP mypid
  services=[]
  mt= 'B' ; mtag=1
  servicename= "CTPBCM/" + mt
  #sids[mt] = pydim.dis_add_service(servicename, "C", scope_cb, mtag)
  services.append(service(servicename, mtag))
  pydim.dis_start_serving("example of simple server")
  mylog.logm("Starting the server ...")
  while True:
    try:
      a= raw_input('enter 1 2 or q:\n')
    except:
      mylog.logm("exception:"+str(sys.exc_info()[0]))
      if quit=='q':
        a='q'
      else:
        continue
    if a=='q': break
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
    else:
      mylog.logm('bad input:%s...'%a) ; continue
  pydim.dis_stop_serving()
  #sys.stdout.flush()
  mylog.close()
        
if __name__ == "__main__":
    main()

