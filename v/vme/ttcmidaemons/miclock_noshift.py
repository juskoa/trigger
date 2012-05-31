#!/usr/bin/python
import sys,os,os.path,string
import signal,time

# Import the pydim module
import pydim
if os.environ['VMESITE']=='ALICE':
  MICLOCKID="/data/dl/snapshot/alidcsvme017/home/alice/trigger/v/vme/WORK/miclockid"
else:
  #print "VMESITE:", os.environ['VMESITE']
  MICLOCKID="/home/alice/trigger/v/vme/WORK/miclockid"

def signal_handler(signal, stack):
  global MICLOCKID
  print "signal:",signal
  print "/n    Ctrl+C pressed, press q  to quit miclock script.../n"
  #os.remove(MICLOCKID) ; sys.exit(0)

def rmzero(strg):
  if strg[-1]=='\0':
    rcstr= strg[:-1]
    #print "rmzero:%s:%s:"%(strg,rcstr)
  else:
    #print 'rmzero:%s'%strg
    rcstr= strg
  return rcstr
class web:
  def __init__(self):
    self.miclock='none'
    self.transition='none'
    self.newclock='none'
    self.clockchangemode='man'   #'man' (manual) or  'auto': automatic
    self.lastbmname='none'
  def save(self):
    fn= os.path.join(os.environ['HOME'],"CNTRRD/htmls/clockinfo")
    f= open(fn,"w")
    if self.miclock=='LOCAL':
      line='clock: <big><FONT COLOR="green">%s</FONT><br>'%(self.miclock)
    elif self.miclock=='BEAM1':
      line='clock: <big><FONT COLOR="blue">%s</FONT><br>'%(self.miclock)
    elif self.miclock=='BEAM2':
      line='clock: <big><FONT COLOR="red">%s</FONT><br>'%(self.miclock)
    else:
      line='clock: <big>%s<br>'%(self.miclock)
    if self.transition[0] != '0':
      line= line +'(%s in %s min)'%(self.newclock, self.transition)
    line= line + '</big>'
    f.write(line)
    #print "Saving to:",fn
    #print line
    f.close()
  def show(self):
    print "miclock transition newclock bmname mode:",\
      self.miclock, self.transition, self.newclock, self.lastbmname, self.clockchangemode
#WEB= None; web()
# see LHC-OP-ES-0005 rev 1.1 2009-10-07
# LHC-BOB-ES-0001 ver.0.1 2005-09-29
#LHC-BOB-ES-0001-20-00.pdf https://edms.cern.ch/document/638899/2.0
bm2clock={
1:(0,'NO MODE'),    # LOCAL
2:(0,'SETUP'), 
3:(0,'INJECTION PROBE BEAM'),  # was 1 till 24.11.
4:(1,'INJECTION SETUP BEAM'), 
5:(1,'INJECTION PHYSICS BEAM'), 
6:(1,'PREPARE RAMP'), 
7:(1,'RAMP'), 
8:(1,'FLAT TOP'),   # RAMP->FLATTOP: adjust clock (see monshiftclock)
9:(1,'SQUEEZE'), 
10:(1,'ADJUST'), 
11:(1,'STABLE BEAMS'), 
12:(1,'UNSTABLE BEAMS'), 
13:(0,'BEAM DUMP'),
14:(0,'RAMP DOWN'),    # ->RAMP_DOWN: adjust clock
15:(0,'RECOVERY'), 
16:(0,'INJECT & DUMP'),    # was 1 till 24.11.
17:(0,'CIRCULATE & DUMP'),    # was 1 till 24.11.
18:(0,'ABORT'), 
19:(0,'CYCLING'), 
20:(0,'BEAM DUMP WARNING'), # seems never happen
21:(0,'NO BEAM')}
def callback1(now):
  #print "callback1: '%s' (%s)" % (now, type(now))
  WEB.miclock= rmzero(now) ; WEB.save()
  print "TTCMI/MICLOCK: %s."%(now)
def callback_bm(bm):
  #print "callback_bm: '%s' (%s)" % (bm, type(bm))
  #print "callback_bm: '%s' (%s)" % (p2, type(p2))
  #WEB.miclock= rmzero(now) ; WEB.save()
  if bm2clock.has_key(bm):
    i01= bm2clock[bm][0]
    if i01==0:
      expclock= "LOCAL"
    else:
      expclock= "BEAM1"
    bmname= bm2clock[bm][1]
  else:
    expclock= "?" ; bmname="???"
  WEB.lastbmname= bmname
  lt= time.localtime(); ltim= "%2d.%2d. %2d:%2d"%(lt[2], lt[1], lt[3], lt[4])
  if WEB.miclock==expclock:
    print "%s BEAM MODE:%s clock %s OK"%(ltim, bmname, expclock) 
  else:
    print "%s BEAM MODE:%s clock %s not correct. miclock mode:%s"%\
      (ltim, bmname, WEB.miclock,WEB.clockchangemode) 
    #print "%s bad clock:%s for beam mode:%s(%d) clock_change_mode:%s"%\
    #  (ltim, WEB.miclock, bmname,bm, WEB.clockchangemode)
    if WEB.clockchangemode=='auto':
      # change clock
      print "changing clock to %s. Wait 3 minutes please..."%(expclock)
      res= pydim.dic_cmnd_service("TTCMI/MICLOCK_SET", (expclock,), "C")

def cbtran(now):
  WEB.transition= rmzero(now) ; WEB.save()
  print "TTCMI/MICLOCK_TRANSITION: %s."%(now) #,type(now)
def main():
  global WEB
  if not pydim.dis_get_dns_node():
    print "Please set the environment variable DIM_DNS_NODE (aldaqecs)"
    sys.exit(1)
  #signal.signal(signal.SIGKILL, signal_handler)
  # authenticate:
  if os.environ['USER']=="trigger":
    if os.path.exists(MICLOCKID):
      lsf= open(MICLOCKID,"r"); pid=lsf.read(); lsf.close; 
      pid= string.strip(pid,"\n")
      print """
It seems, miclock process already started, pid:%s
If you cannot locate window, where %s is started, please
remove file and kill miclock process, i.e.:
kill %s
rm %s

Than start miclock again.
"""%(pid,pid,pid,MICLOCKID)
      return
    pid= str(os.getpid())
    print "my pid:", pid, "MICLOCKID:",MICLOCKID
    f= open(MICLOCKID, "w"); f.write(pid+'\n'); f.close()
  time.sleep(2)   # 1sec was enough
  WEB=web()
  res = pydim.dic_info_service("TTCMI/MICLOCK", "C", callback1)
  restran = pydim.dic_info_service("TTCMI/MICLOCK_TRANSITION", "C", cbtran)
  # next line after res service (i.e. current clock retrieved already)
  resbm = pydim.dic_info_service("CTPDIM/BEAMMODE", "L:1", callback_bm)
  #print "res...:", resbm, res, restran
  if not res or not restran or not resbm:
    print "Error registering with info_services", resbm, res, restran
    sys.exit(1)
  while True:
    #time.sleep(10)
    try:
      a= raw_input('enter BEAM1 BEAM2 REF LOCAL man auto (now:%s) or q:\n'%\
        WEB.clockchangemode)
    except:
      a='q'
      print "exception:",sys.exc_info()[0]
    if (a!='q') and (a!='BEAM1') and (a!='BEAM2') and (a!='LOCAL') and \
      (a!='REF') and (a!='man') and (a!='auto') and (a!='show') :
      print 'bad input:%s...'%a ; continue
    if a=='q': break
    if a=='auto':
      WEB.clockchangemode='auto'
    elif a=='man':
      WEB.clockchangemode='man'
    elif a=='show':
      WEB.show()
    else:
      print "Wait 3 minutes till MICLOCK_TRANSITION is 0. Switching to ",a," ..."
      arg= (a,)
      res= pydim.dic_cmnd_service("TTCMI/MICLOCK_SET", arg, "C")
      time.sleep(1)
      WEB.newclock= a; WEB.save()
  os.remove(MICLOCKID)
  #pydim.dic_release_service(resbm)    -Segmentation fault when 'q'
  #pydim.dic_release_service(res)
  #pydim.dic_release_service(restran)
if __name__=="__main__":
  main()


