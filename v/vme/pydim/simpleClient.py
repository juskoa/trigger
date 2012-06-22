#!/bin/env python
# 4.11. when measurement not complete, restart telnet with next measurement
import sys,time,string,pydim
AliceClock="not defined"

def rmzero(strg):
  if strg[-1]=='\0':
    rcstr= strg[:-1]
    #print "rmzero:%s:%s:"%(strg,rcstr)
  else:
    #print 'rmzero:%s'%strg
    rcstr= strg
  return rcstr
def miclock_cb(now):
  global AliceClock
  print "miclock_cb. Message received: '%s' (%s)" % (now, type(now)) 
  AliceClock= rmzero(now)

def main():
  if not pydim.dis_get_dns_node():
    print "No Dim DNS node found. Please set the environment variable DIM_DNS_NODE"
    sys.exit(1)
  print "dns:",pydim.dis_get_dns_node()
  if len(sys.argv)>1:
    servicename= sys.argv[1]
  else:
    servicename= "simpleServer"
  print "connecting to %s service"%(servicename)
  try:
    miclock = pydim.dic_info_service(servicename, "C", miclock_cb)
    # seems always int returned (regardless of servicename status?)
  except:
    print "Error registering %s"%servicename
    #sys.exit(1)
  print "miclock:", type(miclock), miclock
  while True:
    a= raw_input('enter any text or q:\n')
    print a
    if a=='q': break

if __name__ == "__main__":
    main()

