#!/bin/env python
# Operation: 
# no pars, stdout:
# fs fs_name fillNumber A_bunchCount A+C_bunchCount
# i.e. line 'fs...' available if all DIM services ok

# Hint for monitoring -two possible states ok/err:
# 1.(ok) DAQ services available (3 out of 4 are available ALL THE TIME)
#    a line to stdout starting 'fs ...', e.g.: 'fs  0 0 3'
#DIM Wrapper: src/dimmodule.cpp:1588 ::_dic_info_service_dummy: ERROR: Could not get new data to update service
#fs  0 0 3
# 2.(err) Any of 3 DAQ services down (i.e. not started at all or DIM bridge down):
#    stdout does not containg a 'fs ...' line, e.g.:
#DIM Wrapper: src/dimmodule.cpp:1588 ::_dic_info_service_dummy: ERROR: Could not get new data to update service
#DIM Wrapper: src/dimmodule.cpp:1588 ::_dic_info_service_dummy: ERROR: Could not get new data to update service
#
import sys,time,string,types,pydim
fsname=None
filln=0

beamA= None
beamC= None
def rmzero(strg):
  if strg[0]=='\0': return ""
  #rcstr = strg.translate(None, '\0')
  #rcstr = strg.translate(None, '\x002us')
  #rcstr = rcstr.translate(None, '\x00')
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

def service_cb1(*now):
  global beamA
  beamA= []
  fmt= type(now)
  #print "service_cb1 %s received: type: %slen:%d" % (gettimenow(), fmt, len(now)), now
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
  global beamC
  beamC= []
  #print "2:", now
  fmt= type(now)
  if type(now) is types.TupleType:
    #print "ok, tuple"
    if type(now[0]) == types.StringType:
      #print "ok, string"
      bucstr= string.split(rmzero(now[0]),',')
      for ix in range(len(bucstr)):
        if bucstr[ix]=='0': continue
        beamC.append(bucstr[ix])
def service_cb3(*now):
  global fsname
  fsname=""
  #print "3:", now
  fmt= type(now)
  if type(now) is types.TupleType:
    #print "ok, tuple"
    if type(now[0]) == types.StringType:
      #print "ok, string"
      fsname= str(rmzero(now[0]))
def service_cb4(*now):
  global filln
  fmt= type(now)
  #print "4:", now
  if type(now) is types.TupleType:
    #print "ok, tuple"
    if type(now[0]) == types.IntType:
      #print "ok, string"
      filln= now[0]

def main():
  global fsname
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
  servicename1="ALICE/LHC/CIRCULATING_BUNCHES_B1"   # always tupple ('0,...')
  servicename2="ALICE/LHC/CIRCULATING_BUNCHES_B2"
  servicename3="ALICEDAQ_LHCFillingSchemeName"   # "" in "BEAM SETUP: NO BEAM"
  servicename4="ALICEDAQ_LHCFillNumber"   # service NOT AVAILABLE in "BEAM SETUP: NO BEAM"
  # I.e: first 3 services ALWAYS ON (server always active!)
  try:
    serid3 = pydim.dic_info_service(servicename3, servicefmt, service_cb3)
    serid4 = pydim.dic_info_service(servicename4, "L", service_cb4)
    serid1 = pydim.dic_info_service(servicename1, servicefmt, service_cb1)
    serid2 = pydim.dic_info_service(servicename2, servicefmt, service_cb2)
    # seems always int returned (regardless of servicename status?)
  except:
    print "Error registering %s"%servicename
    #sys.exit(1)
  #print "serid:", type(serid1), serid1, serid2
  time.sleep(2)
  if fsname == None: sys.exit(1)  # shoudl wake alarm
  if beamA == None: sys.exit(1)   # should wake alarm
  if beamC == None: sys.exit(1)   # should wake alarm
  dipname= fsname+'.dip'
  #print "dipname:%s: filln lengths A/C:"%dipname, filln, len(beamA), len(beamC)
  #print "type:",type(dipname)
  fsdip= open(dipname, "w")
  nwritten=0
  for ix in range(len(beamA)):
    fsdip.write("A " + beamA[ix]+'\n')
    nwritten= nwritten+1
  nwrittena= nwritten
  for ix in range(len(beamC)):
    fsdip.write("C " + beamC[ix]+'\n')
    nwritten= nwritten+1
  fsdip.close()
  print "fs %s %s %s %s"%(fsname, filln, nwrittena, nwritten) 

if __name__ == "__main__":
    main()

