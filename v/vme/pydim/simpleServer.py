#!/bin/env python
# 4.11. when measurement not complete, restart telnet with next measurement
import sys,time,string,signal,os,pydim, pylog
quit= "nothing"

def signal_handler(signal, stack):
  global quit
  mylog.logm("signal:%d received."%signal)
  quit= str(signal)

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
    mylog.logm("callback function (i.e. Case1) for simpleServer.tag:%s:"%tag)
    # Calculate the value
    # This example returns a string with the current time
    now = time.strftime("%X")
    # Remember, the callback function must return a tuple
    return ("%s. %s"%(now,"blabla"),)
def epochtime():
  return "%.3f"%time.time()   #epoch
def loctime(epoch_str):   
  lt= time.localtime(float(epoch_str))
  rc="%2.2d.%2.2d %2.2d:%2.2d"%(lt[2], lt[1], lt[3], lt[4])
  return rc   # dd.mm hh:mm

def main():
  global mylog
  #mylog= pylog.Pylog("simpleServer","ttyYES")
  mylog= pylog.Pylog(None,"ttyYES")   # only tty (no file log)
  if not pydim.dis_get_dns_node():
    mylog("No Dim DNS node found. Please set the environment variable DIM_DNS_NODE")
    sys.exit(1)
  mypid= str(os.getpid())
  mylog.logm("dns:%s mypid:%s"%(pydim.dis_get_dns_node(), mypid))
  scopes = pydim.dis_add_service("simpleServer", "C", scope_cb, 33)
  # A service must be updated before using it.
  print "Updating the services ..."
  pydim.dis_update_service(scopes)
  pydim.dis_start_serving("example of simple server")
  mylog.logm("Starting the server ...")
  a=""
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
      #a='q'
      mylog.logm("exception:"+str(sys.exc_info()[0]))
    if a=='q': break
    elif a=='1':   # case1
      tist= epochtime()
      mylog.logm("time:%s = %s"%(loctime(tist), str(tist)))
      pydim.dis_update_service(scopes)
    elif a=='2':   # case2
      tist= epochtime()
      mylog.logm("time:%s = %s"%(loctime(tist), str(tist)))
      pydim.dis_update_service(scopes,("%s"%(tist+"\0"),))
    else:
      mylog.logm('bad input:%s...'%a) ; continue
  sys.stdout.flush()
  mylog.close()
        
if __name__ == "__main__":
  signal.signal(signal.SIGUSR1, signal_handler)  # 10         SIGUSR1
  signal.signal(signal.SIGHUP, signal_handler)   # 1  kill -s SIGHUP mypid
  signal.signal(signal.SIGINT, signal_handler)   # 2  CTRL-C
  main()

