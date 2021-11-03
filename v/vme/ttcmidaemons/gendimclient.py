#!/usr/bin/python
import sys,os,os.path

# Import the pydim module
import pydim
def callback1(now):
  #print "callback1: '%s' (%s) len:%d" % (now, type(now),len(now))
  print "callback1: '%f' (%s)" % (now, type(now))
def callback2(now):
  print "callback2 TTCMI/MICLOCK: '%s' (%s) len:%d" % (now, type(now), len(now))
def callbackC(now):
  print "callbackC: TTCMI/SHIFT '%s' (%s) len:%d" % (now, type(now),len(now))
def callbackgen(now):
  print "callbackgen: '%s' (%s) len:%d" % (now, type(now),len(now))
def dicinfo(sname, stype="C"):
  print "registering with info_service:", sname, "stype:",stype,"..."
  try:
    sid = pydim.dic_info_service(sname, stype, callbackgen,
      service_type=pydim.ONCE_ONLY)
  except:
    print "sys.exc_info:",sys.exc_info()[0]
    sid=None
  if not sid:
    print "Error registering with info_service:", sname, "stype:",stype
    sys.exit(1)
  #print "sid:",sid, "releasing..."
  #pydim.dic_release_service(sid)
def main():
  ddnode= pydim.dis_get_dns_node()
  print "DIM_DNS_NODE:", ddnode
  if not ddnode:
    print "Please set the environment variable DIM_DNS_NODE (aldaqecs)"
    sys.exit(1)
  #sid = pydim.dic_info_service("PHASE_SHIFT_BPTX1", "F", callback1)
  try:
    #sid = pydim.dic_info_service("PHASE_SHIFT_BPTX1", "F", callback1)
    sid = pydim.dic_info_service("TTCMI/SHIFT", "C", callbackC,
      service_type=pydim.ONCE_ONLY)
    sid = pydim.dic_info_service("TTCMI/MICLOCK", "C", callback2,
      service_type=pydim.ONCE_ONLY)
  except:
    print "sys.exc_info:",sys.exc_info()[0]
    sid=None
  if not sid:
    print "Error registering with info_services"
    sys.exit(1)
  print "sid:",sid
  #res= pydim.dic_cmnd_service("TTCMI/MICLOCK_SET", arg, "C")
  while True:
    a= raw_input("g servicename or q to quit:")
    asplit= a.split()
    print "asplit:",asplit
    if len(asplit)==1:
      if asplit[0]=='q': break
    elif len(asplit)==0:
      pass
    else:
      if asplit[0]=='g':
        sname= asplit[1]
        dicinfo(sname) 
  pydim.dic_release_service(sid)

if __name__=="__main__":
  main()
