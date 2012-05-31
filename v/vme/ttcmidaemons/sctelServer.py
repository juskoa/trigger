#!/bin/env python
# 4.11. when measurement not complete, restart telnet with next measurement
import sys,time,string,pydim
import sctel
AliceClock="not defined"
BeamMode=None

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
    print "Running callback function for TTCMI/SCOPE"
    # Calculate the value
    # This example returns a string with the current time
    now = time.strftime("%X")
    # Remember, the callback function must return a tuple
    return ("%s. time is %s"%(AliceClock,now),)
def miclock_cb(now):
  global AliceClock
  print "miclock_cb. Message received: '%s' (%s)" % (now, type(now)) 
  AliceClock= rmzero(now)
def beammode_cb(now):
  global BeamMode
  print "beammode_cb. Message received: '%s' (%s)" % (now, type(now)) 
  BeamMode= str(now)
def epochtime():
  return "%.3f"%time.time()   #epoch
def loctime(epoch_str):   
  lt= time.localtime(float(epoch_str))
  rc="%2.2d.%2.2d %2.2d:%2.2d"%(lt[2], lt[1], lt[3], lt[4])
  return rc   # dd.mm hh:mm

def main():
  if not pydim.dis_get_dns_node():
    print "No Dim DNS node found. Please set the environment variable DIM_DNS_NODE"
    sys.exit(1)
  print "dns:",pydim.dis_get_dns_node()
  scopes = pydim.dis_add_service("TTCMI/SCOPE", "C", scope_cb, 0)
  if not scopes:
    sys.stderr.write("Error registering the service TTCMI/SCOPE\n")
    sys.exit(1)
  miclock = pydim.dic_info_service("TTCMI/MICLOCK", "C", miclock_cb)
  beammode = pydim.dic_info_service("CTPDIM/BEAMMODE", "L:1", beammode_cb)
  if not miclock:
    print "Error registering TTCMI/MICLOCK"
    sys.exit(1)
  # A service must be updated before using it.
  print "Updating the services ..."
  pydim.dis_update_service(scopes)
  pydim.dis_start_serving("TTCMISCOPE")
  print "Starting the server ..."
  npass=0; tn=None    # i.e. telnet closed
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
    #print "pass:",npass, AliceClock, (npass%10), tn
    # 9,10,11: SQUEEZE,ADJUST,SATBLE_BEAMS
    #if (AliceClock=="BEAM1") and ((BeamMode>="9") and (BeamMode<=11)):
    #
    # in dbg mode: alwasy with BEAM1 and flip/flop with LOCAL:
    if ((AliceClock=="LOCAL") and ((npass % 10)<5)) or\
        (AliceClock=="BEAM1"):
      if tn == None:
        print "opening telnet..."
        tn= sctel.TN()
        if tn.prompt1=="":
          tn= None
          # can't open telnet, it's time to restart infinium:
          tist= epochtime()
          print "%s restart:"%loctime(tist), tist
          pydim.dis_update_service(scopes,("%s"%(tist+"\0"),))
      else:
        scopedata= tn.measure()
        sd_ar= string.split(scopedata)
        print "%s measured:"%loctime(sd_ar[0]), scopedata
        if len(sd_ar)<4:
          print "restarting telnet (close + open with next measurement)..."
          tn.close() 
          tn= None
        else:
          pydim.dis_update_service(scopes,("%s"%(scopedata+"\0"),))
    else:
      if tn != None:
        print "not closing telnet..."
        #tn.close() ; tn= None
    #ts= time.strftime("%X")   # hh:mm:ss
    #pydim.dis_update_service(scopes,("%s %s %s"%(ts, AliceClock,BeamMode),))
    sys.stdout.flush()
    time.sleep(10) ; npass= npass+1
        
if __name__ == "__main__":
    main()

