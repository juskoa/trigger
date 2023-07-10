#!/usr/bin/env python
#from __future__ import print_function
#from builtins import input
import sys,os,os.path

# Import the pydim module
import pydim
def callbackF(now):
  #print "callbackF: '%s' (%s) len:%d" % (now, type(now),len(now))
  print("callbackF: '%f' (%s)" % (now, type(now)))
def callback2(now):
  print("callback2 TTCMI/MICLOCK: '%s' (%s) len:%d"%(now,type(now), len(now)))
def callbackC(now):
  print("callbackC:'%s' (%s) len:%d" % (now, type(now),len(now)))
def callbackL(now, now2):
  print("callbackL:%d %d  (%s) len: not for int"%(now, now2, type(now2)))
def callbackQ(now):
  print("callbackQ: TTCMI/QPLL '%s' = 0x%x (%s)" % (now, now, type(now)))

def main():
  ddnode= pydim.dis_get_dns_node()
  print("DIM_DNS_NODE:", ddnode)
  if not ddnode:
    print("Please set the environment variable DIM_DNS_NODE (alidcsdimdns)")
    sys.exit(1)
  #sid = pydim.dic_info_service("PHASE_SHIFT_BPTX1", "F", callbackF)
  try:
    #sid = pydim.dic_info_service("PHASE_SHIFT_BPTX1", "F", callbackF)
    sid = pydim.dic_info_service("ALICE/LHC/STATUS/BEAM_MODE", "C", callbackC)
    #sid = pydim.dic_info_service("ALICE/LHC/STATUS/MACHINE_MODE", "C", callbackC)
    #sid = pydim.dic_info_service("TTCMI/MICLOCK", "C", callback2,
    #  service_type=pydim.ONCE_ONLY)
    #sid = pydim.dic_info_service("TTCMI/QPLL", "L", callbackQ)
  except:
    print("sys.exc_info:",sys.exc_info()[0])
    sid=None
  if not sid:
    print("Error registering with info_services")
    sys.exit(1)
  print("sid:",sid)
  #res= pydim.dic_cmnd_service("TTCMI/MICLOCK_SET", arg, "C")
  while True:
    a= input("q to quit:")
    if a=='q': break
  pydim.dic_release_service(sid)

if __name__=="__main__":
  main()
