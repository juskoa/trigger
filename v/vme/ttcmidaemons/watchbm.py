#!/usr/bin/env python
# 2.2.2012 and 20.1.2013 (DLL_RESYNC removed):
# look for #CJI -Clock Jitter Invetsigation modifications (no DLL_RESYNC)
#
# 26.4.2015 CTPDIM/BEAMMODE replaced by ALICEDAQ_LHCBeamMode (100chars)
#  5.11.2015 DLL RESET enabled (2x: in checkandsave + SQUEEZE)
# 17.11.2016  we use SQUEEZE again (not ADJUST) for clockshift adjustment
# 26.5. 2017 FLAT TOP instead of SQUEEZE time forclock adjustment
# 26.5. 2017 FLAT TOP + 60 secs bug
#  1.6. 2017 FLAT TOP + 50 secs check +30s, tested in lab (using pydim/simpleClient.py )
from __future__ import division
from __future__ import print_function
from builtins import input
from builtins import str
from past.utils import old_div
from builtins import object
import sys,os,os.path,pylog
import signal,time,subprocess,threading,socket

# Import the pydim module
import pydim

mylog= None
VMECFDIR= os.environ["VMECFDIR"]
if os.environ['VMESITE']=='ALICE':
  #MICLOCKID="/home/dl6/snapshot/alidcsvme017/home/alice/trigger/v/vme/WORK/miclockid"
  MICLOCKID="/home/alice/trigger/v/vme/WORK/watchbmid"
else:
  MICLOCKID= os.path.join(os.environ['VMEWORKDIR'], "WORK/miclockid")

def signal_handler(signal, stack):
  global MICLOCKID
  mylog.logm("signal:%d Ctrl+C pressed, press q  to quit miclock script.../n"%signal)
  mylog.flush()
  #os.remove(MICLOCKID) ; sys.exit(0)

def rmzero(strg):
  rcstr=""
  #old version (till sep1 2015):
  #for ix in range(len(strg)):
  #  if strg[ix]=='\0': break
  #  rcstr= rcstr+strg[ix]
  # new version (ok in pydim/fsclient.py)
  if strg[0]=='\0': return ""
  eos= strg.find('\x00')
  if eos >=0:
    rcstr= strg[:eos]
  else:
    rcstr= strg
  #
  return rcstr
class MyStatus:
  def __init__(self):
    self.filln= 0
    self.fs= ""
class myThread (threading.Thread):
  def __init__(self, threadID, name, delay):
    threading.Thread.__init__(self)
    self.threadID = threadID
    self.name = name
    self.delay = delay
    self.exitflag= 0
  def run(self):
    #print "Starting " + self.name
    mylog.logm("Starting thread %s"%self.name)
    self.udp2monitor()
    #print "Exiting " + self.name
    mylog.logm("Exiting thread %s"%self.name)
  def stop(self):
      self.exitflag= 1
      self.join()
  def udp2monitor(self):
    global WEB
    host = "localhost"
    port = 9931
    addr = (host,port)
    sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    udpcount=0
    while True:
      #if exitFlag:
      if self.exitflag:
          break
      time.sleep(self.delay)
      message="miclock %s %d"%(WEB.lastbmname, udpcount)
      #sentrc= sock.sendto(message, addr)
      sentrc= sock.sendto(bytes(message,'utf-8'), addr)
      #sentrc= sock.sendto(message.encode('utf-8'), addr))   also ok
      udpcount= udpcount+1
      #print "%s: %s sentrc:" % (time.ctime(time.time()), message), sentrc
      #if udpcount<4:   # log only first few msgs
      #  mylog.logm("%s: %s sentrc:%d" % (time.ctime(time.time()), message, sentrc))

bm2clock={
1:(0,'NO MODE'),    # LOCAL
2:(0,'SETUP'), 
3:(0,'INJECTION PROBE BEAM'),  # was 1 till 24.11.
4:(1,'INJECTION SETUP BEAM'), 
5:(1,'INJECTION PHYSICS BEAM'), 
6:(1,'PREPARE RAMP'), 
7:(1,'RAMP'), 
8:(1,'FLAT TOP'),        # CLOCKSHIFT VALID only in these 4 modes (PHASE_SHIFT_BPTX1 dim)
9:(1,'SQUEEZE'),         # CLOCKSHIFT VALID
10:(1,'ADJUST'),         # CLOCKSHIFT VALID
11:(1,'STABLE BEAMS'),   # CLOCKSHIFT VALID
12:(1,'UNSTABLE BEAMS'), # 5..12: if not BEAM1 FATAL sent to operator!
13:(0,'BEAM DUMP'),    # -> BEAM_DUMP: ajust clock after switching to local
14:(0,'RAMP DOWN'),    #    see ttcmidims.c, ctplib/ttmisubs.c
15:(0,'RECOVERY'), 
16:(0,'INJECT & DUMP'),    # was 1 till 24.11.
17:(0,'CIRCULATE & DUMP'),    # was 1 till 24.11.
18:(0,'ABORT'), 
19:(0,'CYCLING'), 
20:(0,'BEAM DUMP WARNING'), # seems never happen
21:(0,'NO BEAM')}
bm2clocknames= {}   # { 'NO MODE':1, 'SETUP':2,... }

def callback_bm(ecsbm):
  #print "callback_bm: '%s' (%s)" % (p2, type(p2))
  #WEB.miclock= rmzero(now) ; WEB.save()
  #print("callback_bm: '%s' (%s)" % (ecsbm, type(ecsbm)))
  bmname= rmzero(ecsbm)
  #print("callback_bm: '%s' (%s)" % (bmname, type(bmname)))
  bm= bm2clocknames[bmname]
  if bm in bm2clock:
    i01= bm2clock[bm][0]
    if i01==0:
      expclock= "LOCAL"
    else:
      expclock= "BEAM1"
    bmname= bm2clock[bm][1]
  else:
    expclock= "?" ; bmname="???"
  arg= ("%d %s"%(bm, bmname),)
  mylog.logm("callback_bm:" + arg[0])
  if (bmname=="PREPARE RAMP") or (bmname=="RAMP"):
    sys.path.append(os.path.join(os.environ['VMECFDIR'],"filling"))
    if True:   #os.environ['VMESITE'] != "ALICE":
      mylog.logm("lab env, 'getfsdip.py act' call skipped")
    else:
      import getfsdip
      reload(getfsdip)
      mylog.logm("getfsdip.py act...")
      getfsdip.main("act")
  mylog.flush()

def main():
  global WEB,mylog
  if not pydim.dis_get_dns_node():
    print("Please set the environment variable DIM_DNS_NODE (alidcsdimdns)")
    sys.exit(1)
  #signal.signal(signal.SIGKILL, signal_handler)
  #signal.signal(signal.SIGUSR1, signal_handler)
  for bmix in list(bm2clock.keys()):
    bmnamx= bm2clock[bmix][1]
    bm2clocknames[bmnamx]= bmix
  if os.path.exists(MICLOCKID):
    lsf= open(MICLOCKID,"r"); pid=lsf.read(); lsf.close; 
    pid= pid.strip("\n")
    print("""
It seems, watchbm process already started, pid:%s
If you cannot locate window, where %s is started, please
remove file and kill miclock process, i.e.:
kill %s
rm %s

Than start miclock again.
"""%(pid,pid,pid,MICLOCKID))
    sys.exit(1)
  mylog= pylog.Pylog("watchbm","ttyYES")
  pid= str(os.getpid())
  mylog.logm("my pid:"+ pid+ " MICLOCKID:"+ MICLOCKID)
  f= open(MICLOCKID, "w"); f.write(pid+'\n'); f.close()
  # authenticate:
  if os.environ['USER']!="trigger":
    print("Warning: not trigger account:",os.environ['USER'])
  ##mylog.logm("## vesion -i.e. miclock_shift.py")
  mylog.logm("watchbm.py started...")
  time.sleep(2)   # 1sec was enough
  if os.environ['VMESITE']=='ALICE':
    resbm = pydim.dic_info_service("ALICE/LHC/STATUS/BEAM_MODE", "C", callback_bm)
    if not resbm:
      mylog.logm("Error registering with info_services: "+str((resbm)))
      sys.exit(1)
  else:
    mylog.logm("VMESITE: %s"%os.environ['VMESITE'])
  udpmon_thread= myThread(1, "udpmon", 30)
  udpmon_thread.start()
  while True:
    #time.sleep(10)
    #man/auto     -change operation mode (manual or automatic) now:%s
    #             auto is forbidden from 28.4.2015
    try:
      a= input("""
   enter:
   q            -quit this script (takes ~1 minute to exit)
""")
    except:
      a='q'
      mylog.logm("exception:"+str(sys.exc_info()[0]))
    #if string.find("getshift",a)==0: a="getshift"
    if "getshift".find(a)==0: a="getshift"
    if (a!='q') and (a!=''):
      mylog.logm('bad input:%s'%a) ; continue
    if a=='q': 
      print("wait a minute until stop done properly...")
      break
    if a=='': continue
    elif a=='bla':
      pass
    else:
      mylog.logm("Wait 3 half-minutes till MICLOCK_TRANSITION is 0. Switching to "+a+" ...");
      mylog.flush()
      time.sleep(1)
  ##os.remove(MICLOCKID)
  udpmon_thread.stop()
  os.remove(MICLOCKID)
  #pydim.dic_release_service(resbm)    -Segmentation fault when 'q'
  mylog.close()

if __name__=="__main__":
  main()

