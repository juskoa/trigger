#!/bin/env python
# 4.11. when measurement not complete, restart telnet with next measurement
import sys,time,string,types,pydim
AliceClock=None

servicename= "IR_MONITOR/CTP/Luminosity"
def rmzero(strg):
  if strg[0]=='\0': return ""
  eos= strg.find('\x00')
  if eos >=0:
    rcstr= strg[:eos]
  else:
    rcstr= strg
  return rcstr
def gettimenow(secs="yes"):
  lt= time.localtime()
  if secs!=None:
    ltim= "%2.2d.%2.2d.%2.2d %2.2d:%2.2d:%2d "%(lt[2], lt[1], lt[0], lt[3], lt[4], lt[5])
  else:
    ltim= "%2.2d.%2.2d. %2.2d:%2.2d "%(lt[2], lt[1], lt[3], lt[4])
  return ltim

def service_cb(*now):
  global AliceClock
  fmt= type(now)
  print "service_cb %s received: type: %s" % (gettimenow(), fmt), now
  #if type(now) == types.StringType:
  #  AliceClock= rmzero(now)
  if (type(now) == types.TupleType) and (len(now)==1):
    if type(now[0]) == types.StringType:
      AliceClock= rmzero(now[0])
      print "AliceClock is:", AliceClock

def main():
  if not pydim.dis_get_dns_node():
    print "No Dim DNS node found. Please set the environment variable DIM_DNS_NODE"
    sys.exit(1)
  print "dns:",pydim.dis_get_dns_node(), "service:", servicename
  if len(sys.argv)==1:
    print """
Usage:
testIR_MONITORclient.py sync
pydim.dic_sync_info_service("%s", "F", 2)
(used to work in run1 with ("F",) )
"""%servicename
    #sys.exit()
  elif sys.argv[1]=="sync":
    #if len(sys.argv)>2:
    #  servicefmt= sys.argv[2]
    #else:
    #  servicefmt= "C"
    #print "connecting to %s service fmt: %s"%(servicename, servicefmt)
    while True:
      a= raw_input('enter q, or sync ":\n')
      print a
      if a=='q': break
      if a=='sync':
        try:
          #rclumi = pydim.dic_sync_info_service("IR_MONITOR/CTP/Luminosity", ("F",), 2)
          rclumi = pydim.dic_sync_info_service(servicename, "F", 2)
          print "rclumi len:", len(rclumi)
          if len(rclumi)>=5:
            print rclumi[:5]
          #serid = pydim.dic_info_service(servicename, servicefmt, service_cb)
        except:
          print "exception ... "
      #pydim.dic_relese_service(serid) not available in pydim
  else:
    print "sync expected..."

if __name__ == "__main__":
    main()

