#!/bin/env python
# 4.11. when measurement not complete, restart telnet with next measurement
import sys,time,string,signal,os,random,pydim, pylog
quit= ""
lastbusy= 0.
avlt= {"trd":0.05, "zdc" :0.1, "emcal":0.15, "tpc":0.20, "pmd":0.25,
  "acorde":0.30, "sdd":0.35, "muon_trk":0.40, "muon_trg":0.45,
  "daq":0.50, "ssd":0.55, "fmd":0.60, "t0":0.65, "hmpid":0.70,
  "phos":0.75, "cpv":0.80, "ad":0.85, "spd":0.90, "tof":0.95, "v0":0.0}
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
def monbusy_cb(tag):
  global lastbusy
  monbusy= lastbusy
  avbusy= lastbusy*1000   # us
  if lastbusy<0.01:
    l2arate= 9999
  else:
    l2arate= 100/lastbusy
  t= time.time()
  secs= int(t)
  micsecs= int((10**6)*(t-secs))
  #monbusy= float(random.randint(0,1000))/1000.
  #monbusy= monbusy+1.
  #mylog.logm("monbusy_cb tag:%s rc:%f"%(tag,monbusy))
  #return (monbusy,)
  print "  ", secs, micsecs, monbusy,avbusy,l2arate
  return (secs, micsecs, monbusy,avbusy,l2arate)
def epochtime():
  return "%.3f"%time.time()   #epoch
def loctime(epoch_str):   
  lt= time.localtime(float(epoch_str))
  rc="%2.2d.%2.2d %2.2d:%2.2d"%(lt[2], lt[1], lt[3], lt[4])
  return rc   # dd.mm hh:mm

def main(servicename):
  global mylog,lastbusy,avlt
  #mylog= pylog.Pylog("simpleServer","ttyYES")
  mylog= pylog.Pylog(None,"ttyYES")   # only tty (no file log)
  if not pydim.dis_get_dns_node():
    mylog("No Dim DNS node found. Please set the environment variable DIM_DNS_NODE")
    sys.exit(1)
  mypid= str(os.getpid())
  mylog.logm("dns:%s service:%s mypid:%s"%(pydim.dis_get_dns_node(), servicename, mypid))
  if servicename=="simpleServer":
    scopes = pydim.dis_add_service("simpleServer", "C", scope_cb, 33)
  else:
    #scopes = pydim.dis_add_service(servicename+"/MONBUSY", "F", monbusy_cb, 34)
    scopes = pydim.dis_add_service(servicename+"/MONBUSY", "I:2;F:3", monbusy_cb, 34)
  # A service must be updated before using it.
  print "Updating the services ..."
  pydim.dis_update_service(scopes)
  pydim.dis_start_serving(servicename)
  mylog.logm("Starting the server "+servicename)
  a=""
  if servicename!="simpleServer":
    while True:
      time.sleep(1)
      monbusy= float(random.randint(0,1000))/1000.   # 0 .. 1
      if avlt.has_key(servicename):
        monbusy= avlt[servicename] + monbusy/10.
        if monbusy>1.: monbusy= 1.
      #if (monbusy>=0.2) and (monbusy<=0.9):
      if abs(lastbusy - monbusy)<0.04:  # 0.01
        pass # no update if 0.6 ..0.9
        print "no update last: %f now:%f"%(lastbusy, monbusy)
      else:
        lastbusy= monbusy
        pydim.dis_update_service(scopes)
        print "updated       : %f now:%f"%(lastbusy, monbusy)
      if quit: break
  else:
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
        mylog.logm("raw_input exception:"+str(sys.exc_info()[0]))
        a='q'
      if a=='q': break
      elif a=='1':   # case1
        tist= epochtime()
        mylog.logm("time:%s = %s"%(loctime(tist), str(tist)))
        pydim.dis_update_service(scopes)
      elif a=='2':   # case2
        tist= epochtime()
        tistmsg="epochtime:%s of type:%s"%(tist, str(type(tist)))
        mylog.logm("time:%s = %s"%(loctime(tist), str(tist)))
        # "\0" for C-clients ? (not needed for py-clients)
        pydim.dis_update_service(scopes,("%s"%(tistmsg+"\0"),))
      else:
        mylog.logm('bad input:%s...'%a) ; continue
  pydim.dis_stop_serving()
  sys.stdout.flush()
  mylog.close()
        
if __name__ == "__main__":
  signal.signal(signal.SIGUSR1, signal_handler)  # 10         SIGUSR1
  signal.signal(signal.SIGHUP, signal_handler)   # 1  kill -s SIGHUP mypid
  signal.signal(signal.SIGINT, signal_handler)   # 2  CTRL-C
  if len(sys.argv)>1:
    servicename= sys.argv[1]
    main(servicename)
  else:
    print """ Usage:
simpleServer.py simpleServer  -old run1 case for scope telnet tests
                daq/MONBUSY   -start this service updates 1/sec with float number 0..1
"""

