#!/bin/env python
#
import sys,time,string,types,pydim
AliceClock=None

beamA= []
beamC= []
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

def service_cb1(*now):
  global AliceClock
  fmt= type(now)
  #print "service_cb1 %s received: type: %s" % (gettimenow(), fmt), now
  if type(now) is types.TupleType:
    #print "ok, tuple"
    if type(now[0]) == types.StringType:
      #print "ok, string"
      bucstr= string.split(rmzero(now[0]),',')
      for ix in range(len(bucstr)):
        if bucstr[ix]=='0': continue
        #print "ix:%d %s"%(ix, bucstr[ix])
        beamA.append(bucstr[ix])
def service_cb2(*now):
  global AliceClock
  fmt= type(now)
  if type(now) is types.TupleType:
    #print "ok, tuple"
    if type(now[0]) == types.StringType:
      #print "ok, string"
      bucstr= string.split(rmzero(now[0]),',')
      for ix in range(len(bucstr)):
        if bucstr[ix]=='0': continue
        beamC.append(bucstr[ix])

def main():
  if not pydim.dis_get_dns_node():
    print "No Dim DNS node found. Please set the environment variable DIM_DNS_NODE"
    sys.exit(1)
  #print "dns:",pydim.dis_get_dns_node()
  #if len(sys.argv)>1:
  #  servicename= sys.argv[1]
  #  if len(sys.argv)>2:
  #    servicefmt= sys.argv[2]
  #  else:
  servicefmt= "C"
  servicename1="ALICE/LHC/CIRCULATING_BUNCHES_B1"
  servicename2="ALICE/LHC/CIRCULATING_BUNCHES_B2"
  #print "connecting to %s service fmt: %s"%(servicename1, servicefmt)
  try:
    serid1 = pydim.dic_info_service(servicename1, servicefmt, service_cb1)
    serid2 = pydim.dic_info_service(servicename2, servicefmt, service_cb2)
    # seems always int returned (regardless of servicename status?)
  except:
    print "Error registering %s"%servicename
    #sys.exit(1)
  #print "serid:", type(serid1), serid1, serid2
  time.sleep(2)
  print "lengths A/C:", len(beamA), len(beamC)
  fsdip= open("fs.dip", "w")
  for ix in range(len(beamA)):
    fsdip.write("A " + beamA[ix]+'\n')
  for ix in range(len(beamC)):
    fsdip.write("C " + beamC[ix]+'\n')
  fsdip.close()
  print "fs.dip created"

if __name__ == "__main__":
    main()

