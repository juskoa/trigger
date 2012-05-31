#!/bin/env python
# 16.5.2012 services:
# CTPRCFG/bcmX string
# where X can be: EMPTY B AC A C ACE S SA SC D ?
import sys,time,string,pydim, pylog

def rmzero(strg):
  if strg[-1]=='\0':
    rcstr= strg[:-1]
    #print "rmzero:%s:%s:"%(strg,rcstr)
  else:
    #print 'rmzero:%s'%strg
    rcstr= strg
  return rcstr
def scope_cb(tag):
    """
    Service callbacks are functions (in general, Python callable objects)
    that take one argument: the DIM tag used when the service was added,
    and returns a tuple with the values that corresponds to the service
    parameters definition in DIM.
    """
    mylog.logm("Running callback function (i.e. Case1) for simpleServer tag:%s"%str(tag))
    # Calculate the value
    # This example returns a string with the current time
    now = time.strftime("%X")
    # Remember, the callback function must return a tuple
    return ("%s. %s"%(now,"blabla"),)
def updateAll():
  for line in 

def main():
  global mylog
  mylog= pylog.Pylog(None,"ttyYES")   # only tty (no file log)
  if not pydim.dis_get_dns_node():
    mylog("No Dim DNS node found. Please set the environment variable DIM_DNS_NODE")
    sys.exit(1)
  mylog.logm("dns:"+pydim.dis_get_dns_node())
  mt= 'B' ; mtag=1
  sid[mt] = pydim.dis_add_service("CTPBCM/"+mt, "C", scope_cb, mtag)
  # A service must be updated before using it.
  print "Updating the services ..."
  pydim.dis_update_service(sid[mt])
  pydim.dis_start_serving("example of simple server")
  mylog.logm("Starting the server ...")
  while True:
    # Update the services periodically
    # Case 1: When `dis_update_service` is called without arguments the 
    # callback function will be executed and its return value
    # will be sent to the clients.
    #pydim.dis_update_service(scopes)
    #time.sleep(1)
    # Case 2: When `dis_update_service` is called with arguments, they are
    # sent directly to the clients as the service value, *without* executing the
    # callback function. Please note that the number and the type of the 
    # arguments must correspond to the service description. 
    #
    try:
      a= raw_input('enter 1 2 or q:\n')
    except:
      a='q'
      mylog.logm("exception:"+str(sys.exc_info()[0]))
    if a=='q': break
    elif a=='1':   # case1
      tist= epochtime()
      mylog.logm("time:%s = %s"%(loctime(tist), str(tist)))
      pydim.dis_update_service(sid[mt])
    elif a=='2':   # case2
      tist= epochtime()
      mylog.logm("time:%s = %s"%(loctime(tist), str(tist)))
      pydim.dis_update_service(sid[mt],("%s"%(tist+"\0"),))
    else:
      mylog.logm('bad input:%s...'%a) ; continue
  sys.stdout.flush()
  mylog.close()
        
if __name__ == "__main__":
    main()

