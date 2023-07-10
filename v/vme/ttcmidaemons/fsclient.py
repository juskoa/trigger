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
# 24.5.2022:
#[trigger@alidcsvme018 ttcmidaemons]$ ./fsclient.py 
#fs Single_4b_2_2_2_noLR 7635 4 8
#[trigger@alidcsvme018 ttcmidaemons]$ lth

from __future__ import print_function
from builtins import str
from builtins import range
import sys,os,time,types,pydim
fsname=None
filln=0

beamA= None
beamC= None
def rmzero(strg):
  if strg[0]=='\0': return ""
  #rcstr = strg.translate(None, '\0')
  #rcstr = strg.translate(None, '\x002us')
  #rcstr = rcstr.translate(None, '\x00')
  #eos= strg.find('\x00') #ctpapp: TypeError: a bytes-like object is required, not 'str'
  #eos= strg.find(b'0')  ctpapp: TypeError: must be str, not bytes
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

def do_beamAorC(now):
  beam= []
  if type(now) is tuple:
    #print "ok, tuple"
    #if type(now[0]) == bytes:
    #  #print "ok, string"
    #  #bucstr= string.split(rmzero(now[0]),',')
    if os.getenv("CTP3_WORK_DIRECTORY")==None:
      bucstr= rmzero(now[0]).split(',')
      zero= '0'
    else:
      bucstr= now
      zero= 0
    for ix in range(len(bucstr)):
      if bucstr[ix]==zero: continue
      #print "ix:%d %s"%(ix, bucstr[ix])
      beam.append(bucstr[ix])
    #print("beamlen:",len(beam))
    return beam
def writeAC(fsdip, AC, beamAC):
  nwritten=0
  for ix in range(len(beamAC)):
    #fsdip.write("A {:d}\n".format(beamA[ix]))
    fsdip.write("{} {}\n".format(AC,beamAC[ix]))
    nwritten= nwritten+1
  return nwritten
def service_cb1(*now):
  global beamA
  fmt= str(type(now))
  print("service_cb1 {:s} received: type: {:s} len:{:d}".format(gettimenow(), fmt, len(now)))
  #print("cb1:", now) # cb1: (b'\x01\x00\x00\x00\xd1\x07\x00\x00\xa1\x0f\x00\x009\
  # (1, 2001, 4001, 6201, 11111, 13111, 15111, 17851, 19851, 21051, 22051, 30701, 0,
  beamA= do_beamAorC(now)
def service_cb2(*now):
  global beamC
  beamC= do_beamAorC(now)
def service_cb3(*now):
  global fsname
  fsname=""
  #print("cb3:", now, type(now[0]))
  if type(now) is tuple:
    #print "ok, tuple"
    fsname= str(rmzero(now[0]))
    #print(fsname)
def service_cb4(*now):
  global filln
  #print("cb4:", now)
  if type(now) is tuple:
    filln= now[0]
  #print("cb4:",filln)
def main():
  global fsname
  if not pydim.dis_get_dns_node():
    print("No Dim DNS node found. Please set the environment variable DIM_DNS_NODE")
    sys.exit(1)
  #print "dns:",pydim.dis_get_dns_node()
  #if len(sys.argv)>1:
  #  servicename= sys.argv[1]
  #  if len(sys.argv)>2:
  #    servicefmt= sys.argv[2]
  #  else:
  if os.getenv("CTP3_WORK_DIRECTORY")==None:
  #servicename1="ALICE/LHC/CIRCULATING_BUNCHES_B1"   # always tupple ('0,...')
  #servicename2="ALICE/LHC/CIRCULATING_BUNCHES_B2"
  #servicename3="ALICEDAQ_LHCFillingSchemeName"   # "" in "BEAM SETUP: NO BEAM"
  #servicename4="ALICEDAQ_LHCFillNumber"   # service NOT AVAILABLE in "BEAM SETUP: NO BEAM"
    fmtI=fmtC= "C" 
    prefname= "ALICE/LHC/"
    servicename1="BUNCHES/CIRCULATING_BUNCHES_B1"
    servicename2="BUNCHES/CIRCULATING_BUNCHES_B2"
    servicename3="CONFIGURATION/INJECTION_SCHEMA"
    servicename4="CONFIGURATION/FILL_NUMBER"
  else:
    fmtI="I" ; fmtC= "C" 
    prefname= "ALICE/LHC/LHCIF/MF/"
    servicename1="CIRCULATING_BUNCHES_B1"
    servicename2="CIRCULATING_BUNCHES_B2"
    servicename3="INJECTION_SCHEMA"
    servicename4="FILL_NUMBER"
  # I.e: first 3 services ALWAYS ON (server always active!)
  try:
    serid3 = pydim.dic_info_service(prefname+servicename3, fmtC, service_cb3)
    #serid4 = pydim.dic_info_service(prefname+servicename4, fmtC, service_cb4)
    serid4 = pydim.dic_info_service(prefname+servicename4, fmtI, service_cb4)
    serid1 = pydim.dic_info_service(prefname+servicename1, fmtI, service_cb1)
    serid2 = pydim.dic_info_service(prefname+servicename2, fmtI, service_cb2)
    # seems always int returned (regardless of servicename status?)
  except:
    print("Error registering %s"%prefname)
    #sys.exit(1)
  #print "serid:", type(serid1), serid1, serid2
  time.sleep(2)
  if fsname == None: sys.exit(1)  # should wake alarm
  if beamA == None: sys.exit(1)   # should wake alarm
  if beamC == None: sys.exit(1)   # should wake alarm
  dipname= fsname+'.dip'
  if os.getenv("CTP3_WORK_DIRECTORY")==None:
    dippath= os.path.join(os.getenv("VMEWORKDIR"), "WORK", dipname)
  else:
    dippath= os.path.join(os.getenv("CTP3_WORK_DIRECTORY"), dipname)
  #print("dippath:{}: filln lengths A/C:".format(dippath), filln, len(beamA), len(beamC))
  #print "type:",type(dipname)
  fsdip= open(dippath, "w")
  nwritten= writeAC(fsdip, "A", beamA)
  nwrittena= nwritten
  nwritten= writeAC(fsdip, "C", beamC)
  fsdip.close()
  print("fs %s %s %s %s"%(fsname, filln, nwrittena, nwritten)) 

if __name__ == "__main__":
    main()
