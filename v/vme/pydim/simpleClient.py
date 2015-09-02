#!/bin/env python
# 4.11. when measurement not complete, restart telnet with next measurement
import sys,time,string,types,pydim
AliceClock=None

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
  print "dns:",pydim.dis_get_dns_node()
  if len(sys.argv)>1:
    servicename= sys.argv[1]
    if len(sys.argv)>2:
      servicefmt= sys.argv[2]
    else:
      servicefmt= "C"
    print "connecting to %s service fmt: %s"%(servicename, servicefmt)
    try:
      serid = pydim.dic_info_service(servicename, servicefmt, service_cb)
      # seems always int returned (regardless of servicename status?)
    except:
      print "Error registering %s"%servicename
      #sys.exit(1)
    print "serid:", type(serid), serid
  else:
    print """
simpleClient.py service [fmt]
fmt default: "C" 
"""
    #sys.exit()
  while True:
    a= raw_input('enter q, cmd CTPDIM/DO XXX YYY, sec1 or sec60":\n')
    print a
    if a=='q': break
    asp= string.split(a)
    if asp[0]=="cmd":
      # cmd CTPDIM/DO W 3
      pydim.dic_cmnd_service(asp[1], (string.join(asp[2:])+"\x00",),"C")
    elif asp[0]=="sec1":
      pydim.dic_cmnd_service("CTPDIM/DO", ("SLEEP 1\x00",),"C")
    elif asp[0]=="sec60":
      pydim.dic_cmnd_service("CTPDIM/DO", ("SLEEP 60\x00",),"C")
  #pydim.dic_relese_service(serid) not available in pydim

if __name__ == "__main__":
    main()

