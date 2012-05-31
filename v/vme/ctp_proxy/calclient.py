#!/usr/bin/python
import sys,os,os.path
import time

# Import the pydim module
import pydim
inCaseService="""
def rmzero(strg):
  if strg[-1]=='\0':
    rcstr= strg[:-1]
    #print "rmzero:%s:%s:"%(strg,rcstr)
  else:
    #print 'rmzero:%s'%strg
    rcstr= strg
  return rcstr

def callback1(now):
  #print "callback1: '%s' (%s)" % (now, type(now))
  WEB.miclock= rmzero(now) ; WEB.save()
  print "TTCMI/MICLOCK: %s."%(now)
"""

def main():
  if not pydim.dis_get_dns_node():
    print "Please set the environment variable DIM_DNS_NODE (aldaqecs)"
    sys.exit(1)
  #res = pydim.dic_info_service("TTCMI/MICLOCK", "C", callback1)
  #if not res:
  #  print "Error registering client"
  #  sys.exit(1)
  time.sleep(1)
  # authenticate:
  while True:
    #time.sleep(10)
    a= raw_input('[a|d] 1 2..., u, p, help or q:\n')
    if (a[0]!='q') and (a[0]!='a') and (a[0]!='d') and (a[0]!='p') \
      and (a[0]!='u') and (a[0]!='h'):
      print 'bad input:%s. Not sent.'%a ; continue
    if a[0]=='h': 
      print """
SDD:2 
TOF:5 
MTR:11
T00:13
ZDC:15
EMC:18
Examples:
a 2 5     -add SDD+TOF to the list of calibrated detectors
d 13 15   -delete T0+ZDC from the list of calibrated detectors
u         -update the list of calibrated detectors from current global runs
p         -print status into alidcsvme001:v/vme/WORK/gcalib.log file
h         -this message
q         -quit this DIM client
"""
      continue
    if a[0]=='q': break
    if (a[0]=='a')or (a[0]=='d'): 
      print "Add/Del dets:",a
    arg= (a,)
    res= pydim.dic_cmnd_service("CTPCALIB/DO", arg, "C")
    time.sleep(1)
if __name__=="__main__":
  main()


