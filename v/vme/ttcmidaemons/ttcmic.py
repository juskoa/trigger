#!/usr/bin/python
obsolete -see:
- bin/miclock   (no automatic change)
- ttcmidaemons/miclock.py -with automatic change
import sys,os
import time

# Import the pydim module
import pydim

def callback1(now):
  #print "callback1: '%s' (%s)" % (now, type(now))
  print "TTCMI/MICLOCK: %s"%(now)
def cbtran(now):
  print "TTCMI/MICLOCK_TRANSITION: %s"%(now)
def main():
  if not pydim.dis_get_dns_node():
    print "Please set the environment variable DIM_DNS_NODE (aldaqecs)"
    sys.exit(1)
  res = pydim.dic_info_service("TTCMI/MICLOCK", "C", callback1)
  restran = pydim.dic_info_service("TTCMI/MICLOCK_TRANSITION", "C", cbtran)
  if not res or not restran:
    print "Error registering client"
    sys.exit(1)
  time.sleep(1)
  # authenticate:
  if os.environ['USER']=="trigger":
    pid= str(os.getpid())
    print "my pid:", pid
    if os.environ['VMESITE']=='ALICE':
      f= open("/data/ClientLocalRootFs/alidcsvme017/home/alice/trigger/v/vme/WORK/miclockid", "w")
    else:
      print "VMESITE:", os.environ['VMESITE']
      f= open("/home/trigger/v/vme/WORK/miclockid", "w")
    f.write(pid+'\n')
    f.close()
  while True:
    #time.sleep(10)
    a= raw_input('enter BEAM1 BEAM2 REF LOCAL or q:\n')
    if a=='q': break
    #print "entered:",a
    arg= (a,)
    res= pydim.dic_cmnd_service("TTCMI/MICLOCK_SET", arg, "C")
if __name__=="__main__":
  main()


