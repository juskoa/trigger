#!/bin/env python
import sys,time,pydim

def scope_cb(now):
  """
  Callback functions receive as many arguments as values are returned by the
  service. For example, as the service 1 returns only one string this callback
  function has only one argument. 
  """
  print "scope_cb: '%s' (%s)" % (now, type(now)) 
def qpll_cb(now):
  print "qpll_cb: '%s' (%s)" % (now, type(now)) 

def main():
  if not pydim.dis_get_dns_node():
    print "No Dim DNS node found. Please set the environment variable DIM_DNS_NODE"
    sys.exit(1)
  print "dns:",pydim.dis_get_dns_node()
  scope = pydim.dic_info_service("TTCMI/SCOPE", "C", scope_cb)
  qpll = pydim.dic_info_service("TTCMI/QPLL", "C", qpll_cb)
  if not scope:
    print "There was an error registering the clients"
    sys.exit(1)
  while True:
    time.sleep(10)

if __name__=="__main__":
    main()

