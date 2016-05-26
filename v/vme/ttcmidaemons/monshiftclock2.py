#!/usr/bin/python
import types,sys,time,os

# Import the pydim module
import pydim
def callback1(now):
  global gshift
  #print "callback1: '%s' (%s) len:%d" % (now, type(now),len(now))
  #print "callback1: '%d' (%s)" % (now, type(now))
  lt= time.localtime(); ltim= "%2.2d.%2.2d. %2.2d:%2.2d"%(lt[2], lt[1], lt[3], lt[4])
  if type(now) is types.FloatType:
    print "%s: '%f' (%s)" % (ltim, now, type(now))
    gshift= now
  elif type(now) is types.IntType:
    print "%s: '%d' (%s)" % (ltim, now, type(now))
    difsecs= int(time.time()) - now
    days= difsecs/(24*3600) ; hours= (difsecs- days*24)/3600
    print "Age:%d secs. i.e. %d days %d hours"%(difsecs,days,hours)
  else:
    print "%s: '%d' (%s)" % (ltim, now, type(now))
  #ts= pydim.dic_get_timestamp(0)  nebavi
  #print "ts:",ts, type(ts)

def callback2(now):
    print "callback2: '%d' (%s)" % (now, type(now))
gshift=1000.0
def getShift(what=None):
  """rc: +/-float in [ns]
         "old" if age of mesurement too old
  what:force
       rc: +/-float in [ns]

  """
  global gshift
  gshift=1000.0
  if os.getenv("HOSTNAME")[:6] != "alidcs":
    import random
    ips= random.randint(-640,640)   # -6400, 6400
    if abs(ips) >6360.0: return "old"
    return str(ips/1000.)
  pydim.dic_set_dns_node("alidcsdimdns.cern.ch")
  #sid= pydim.dic_info_service("PHASE_SHIFT_BPTX1", "F:1", callback1, service_type=pydim.ONCE_ONLY)
  #time.sleep(1); pydim.dic_release_service(sid)
  gshift= pydim.dic_sync_info_service("PHASE_SHIFT_BPTX1", "F:1", timeout=1, default_value=(1000.0,))
  ts= pydim.dic_sync_info_service("TIMESTAMP_PHASE_SHIFT_BPTX1", "I:1", timeout=1, default_value=(0,))
  #print "gshift:", gshift,type(gshift), 'ts:',ts, type(ts)
  if ts==None: return "old"
  age= time.time() - ts[0]
  if what != "force":
    if age>181: return "old"   # was 91 till 3.11.
  rcshift= gshift[0]
  if (rcshift <= -23.5) and (-24.95 <= rcshift):
    rcshift = -24.95 - rcshift
  return "%6.4f"%rcshift
def main():
  #if not pydim.dis_get_dns_node():
  #  print "Please set the environment variable DIM_DNS_NODE (aldaqecs)"
  #  #sys.exit(1)
  #res= pydim.dic_cmnd_service("TTCMI/MICLOCK_SET", arg, "C")
  if len(sys.argv)>1:
    shift= getShift(sys.argv[1])
    print shift
    return
  while True:
    a= raw_input("get, q to quit:")
    if a=='q': break
    if a=='g' or a=='get':
      sid=8
      if os.getenv("HOSTNAME")[:6] != "alidcs":
        print "not in the pit"
        continue
      pydim.dic_set_dns_node("alidcsdimdns.cern.ch")
      #os.environ['DIM_DNS_NODE']="alidcsdimdns"
      print "dns: alidcsdimdns"
      sid= pydim.dic_info_service("PHASE_SHIFT_BPTX1", "F:1", 
        callback1, service_type=pydim.ONCE_ONLY)
      #cshift= pydim.dic_sync_info_service("PHASE_SHIFT_BPTX1", "F:1")
      #print "cshift:",cshift
      if not sid:
        print "Error registering with info_service"
        break
      sid2= pydim.dic_info_service("TIMESTAMP_PHASE_SHIFT_BPTX1", "I:1", 
        callback1, service_type=pydim.ONCE_ONLY)
      if not sid2:
        print "Error registering with TIMESTAMP info_service"
        break
      time.sleep(1); 
      pydim.dic_release_service(sid)
      pydim.dic_release_service(sid2)
      #ts= pydim.dic_get_timestamp(sid)   #ani tu nebavi
      #print "ts:",ts, type(ts)
      #pydim.dic_release_service(sid)
      #time.sleep(9)
      #pydim.dic_set_dns_node("aldaqecs.cern.ch")
      #os.environ['DIM_DNS_NODE']="aldaqecs"
      #print "dns: aldaqecs"
      #time.sleep(2)
      #sid2= pydim.dic_info_service("CTPDIM/BEAMMODE", "L:1", callback2, service_type=pydim.ONCE_ONLY)
      #bm= pydim.dic_sync_info_service("CTPDIM/BEAMMODE", "L:1")
      #print "bm:",bm
  #pydim.dic_release_service(sid2)

if __name__=="__main__":
  main()
