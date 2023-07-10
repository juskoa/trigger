#!/usr/bin/env python
from __future__ import print_function
from builtins import input
import sys,os,os.path

# Import the pydim module
import pydim
servname= ""
def callback(*args):
  #print("callback: {} {} len:{}".format(args, type(args), len(args)))
  lin= ""
  for v in args:
    if v==0: continue
    lin += str(v) + " "
  fn= os.path.join(os.getenv("HOME"), sys.argv[1] + ".bus")
  f= open(fn,"w")
  f.write(lin)
  print(lin)
  f.close()
def callback1(now):
  #print "callback1: '%s' (%s) len:%d" % (now, type(now),len(now))
  print("callback1: '%f' (%s)" % (now, type(now)))
def callback2(now):
  print("callback2 TTCMI/MICLOCK: '%s' (%s) len:%d" % (now, type(now), len(now)))
def callbackC(now):
  print("callbackC: '%s' (%s) len:%d" % (now, type(now),len(now)))
def callbackQ(now):
  print("callbackQ: TTCMI/QPLL '%s' = 0x%x (%s)" % (now, now, type(now)))

def main():
  global servname
  ddnode= pydim.dis_get_dns_node()
  print("DIM_DNS_NODE:", ddnode)
  if not ddnode:
    print("Please set the environment variable DIM_DNS_NODE (aldaqecs)")
    sys.exit(1)
  #sid = pydim.dic_info_service("PHASE_SHIFT_BPTX1", "F", callback1)
  if True: #try:
    if len(sys.argv)>1:
      if sys.argv[1]=="fs":
        sid = pydim.dic_info_service("ALICE/LHC/CONFIGURATION/INJECTION_SCHEMA",
          "C", callbackC, service_type=pydim.ONCE_ONLY)
        sid = pydim.dic_info_service("ALICE/LHC/CONFIGURATION/FILL_NUMBER",
          "C", callbackC, service_type=pydim.ONCE_ONLY)
        # vyzera ze nasledovne sa vola callbakck iba raz (avsak hhoere sa vola
        # callback raz za cca niekolko secs)
        sid = pydim.dic_info_service("ALICE/LHC/STATUS/BEAM_MODE",
          "C", callbackC) #, service_type=pydim.ONCE_ONLY)
        sid = pydim.dic_info_service("ALICE/LHC/STATUS/MACHINE_MODE",
          "C", callbackC) #, service_type=pydim.ONCE_ONLY)
      elif sys.argv[1]=="b1":   # '1,0,...,0' 5616 chars
        sid = pydim.dic_info_service("ALICE/LHC/BUNCHES/CIRCULATING_BUNCHES_B1",
          "C", callbackC, service_type=pydim.ONCE_ONLY)
      elif sys.argv[1]=="b2":   # '321,0,...0' 5618 chars
        sid = pydim.dic_info_service("ALICE/LHC/BUNCHES/CIRCULATING_BUNCHES_B2",
          "C", callbackC, service_type=pydim.ONCE_ONLY)
      elif sys.argv[1]=="b1v":  #  [1,0,...] 2808 ints
        servname = "ALICE/LHC/BUNCHES/CIRCULATING_BUNCHES_B1_VALUES"
        sid = pydim.dic_info_service(servname, "I:2808", callback, service_type=pydim.ONCE_ONLY)
      elif sys.argv[1]=="b2v":  # [321,0,...] 2808 ints
        servname = "ALICE/LHC/BUNCHES/CIRCULATING_BUNCHES_B2_VALUES"
        sid = pydim.dic_info_service(servname, "I:2808", callback) #, service_type=pydim.ONCE_ONLY)
    else:
      #sid = pydim.dic_info_service("PHASE_SHIFT_BPTX1", "F", callback1)
      sid = pydim.dic_info_service("TTCMI/SHIFT", "C", callbackC,
        service_type=pydim.ONCE_ONLY)
      sid = pydim.dic_info_service("TTCMI/MICLOCK", "C", callback2,
        service_type=pydim.ONCE_ONLY)
      sid = pydim.dic_info_service("TTCMI/QPLL", "L", callbackQ)
      #service_type=pydim.ONCE_ONLY)
  #except:
  #  print("sys.exc_info:",sys.exc_info()[0])
  #  sid=None
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
