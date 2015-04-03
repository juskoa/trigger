#!/bin/env python
# 4.11. when measurement not complete, restart telnet with next measurement
import sys,time,string,types,pydim
AliceClock=None

def rmzero(strg):
  if strg[-1]=='\0':
    rcstr= strg[:-1]
    #print "rmzero:%s:%s:"%(strg,rcstr)
  else:
    #print 'rmzero:%s'%strg
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
  print "dns:",pydim.dis_get_dns_node()
  if len(sys.argv)>1:
    servicename= sys.argv[1]
    if len(sys.argv)>2:
      servicefmt= sys.argv[2]
    else:
      servicefmt= "C"
  else:
    print """
simpleClient.py service [fmt]
fmt default: "C" 
"""
    sys.exit()
  print "connecting to %s service fmt: %s"%(servicename, servicefmt)
  try:
    serid = pydim.dic_info_service(servicename, servicefmt, service_cb)
    # seems always int returned (regardless of servicename status?)
  except:
    print "Error registering %s"%servicename
    #sys.exit(1)
  print "serid:", type(serid), serid
  while True:
    a= raw_input('enter any text or q:\n')
    print a
    if a=='q': break
  #pydim.dic_relese_service(serid) not available in pydim

if __name__ == "__main__":
    main()

