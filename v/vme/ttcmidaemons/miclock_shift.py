#!/usr/bin/python
# 2.2.2012 and 20.1.2013 (DLL_RESYNC removed):
# look for #CJI -Clock Jitter Invetsigation modifications (no DLL_RESYNC)
#
# This should run all the time:
# - follows BEAM MODES (from ctpdim CTPDIM/BEAMMODE
#   - changes clock (in auto mode) TTCMI/MICLOCK_SET
#   - set $dbctp/clockshift (file change)
#   - set corde delay at the end of FLAT TOP(CORDE reg changed) TTCMI/CORDE_SET
#   - calculate new VALID.BCMASK (at the start of FLAT TOP?)
# - follows user input (in man or in auto mode)
#   - clock change
# 26.4.2015 CTPDIM/BEAMMODE replaced by ALICEDAQ_LHCBeamMode (100chars)
import sys,os,os.path,string,pylog
import signal,time,popen2,threading

# Import the pydim module
import pydim

mylog= None
VMECFDIR= os.environ["VMECFDIR"]
if os.environ['VMESITE']=='ALICE':
  #MICLOCKID="/data/dl/snapshot/alidcsvme017/home/alice/trigger/v/vme/WORK/miclockid"
  MICLOCKID="/home/dl6/snapshot/alidcsvme017/home/alice/trigger/v/vme/WORK/miclockid"
elif os.environ['VMESITE']=='SERVER':
  MICLOCKID="/home/dl6/snapshot/altri1/home/alice/trigger/v/vme/WORK/miclockid"
else:
  #print "VMESITE:", os.environ['VMESITE']
  if os.environ["USER"]=="oerjan":
    MICLOCKID="/home/dl/snapshot/altri1/home/alice/trigger/v/vme/WORK/oerjan/miclockid"
  else:
    #MICLOCKID="/home/dl6/snapshot/altri1/home/alice/trigger/v/vme/WORK/miclockid"
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
class web:
  def __init__(self):
    self.miclock='none'
    self.transition='none'
    self.newclock='none'
    self.clockchangemode='???'   #'man' (manual) or  'auto': automatic
    self.lastbmname='none'
  def save(self):
    if self.clockchangemode=='MANUAL':
      ccm='<FONT COLOR="red">/MANUAL</FONT>'
    else:
      ccm=''
    fn= os.path.join(os.environ['HOME'],"CNTRRD/htmls/clockinfo")
    f= open(fn,"w")
    if self.miclock=='LOCAL':
      line='clock: <big><FONT COLOR="green">%s</FONT>%s<br>'%(self.miclock,ccm)
    elif self.miclock=='BEAM1':
      line='clock: <big><FONT COLOR="blue">%s</FONT>%s<br>'%(self.miclock,ccm)
    elif self.miclock=='BEAM2':
      line='clock: <big><FONT COLOR="red">%s</FONT>%s<br>'%(self.miclock,ccm)
    else:
      line='clock: <big>%s<br>'%(self.miclock)
    if self.transition != '0':
      if self.transition=='none': 
        secs1=0
      else:
        secs1= int(self.transition)
      secs= secs1*30
      #line= line +'(%s in %s min)'%(self.newclock, self.transition)
      line= line +'(%s in %ds)'%(self.newclock, secs)
    line= line + '</big>'
    f.write(line)
    #print "Saving to:",fn
    #print line
    f.close()
    mylog.logm("web.save:"+line, 1)
  def show(self):
    mylog.logm("miclock:%s transition:%s newclock:%s bmname:%s mode::%s"%\
      (self.miclock, self.transition, self.newclock, 
      self.lastbmname, self.clockchangemode))
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
def callback1(now):
  #mylog.logm("callback1: '%s' (%s)" % (now, type(now)))
  WEB.miclock= rmzero(now) 
  WEB.save() ; mylog.logm("TTCMI/MICLOCK: %s."%(WEB.miclock))
  ##mylog.logm("TTCMI/MICLOCK: %s. -WEB NOT UPDATED ##"%(now))
def callback_manauto(auma):
  #if auma=='AUTO' or auma=='MANUAL':
  aumanz= rmzero(auma)
  mylog.logm("callback_manauto:%s:%s:"%(auma,aumanz))
  WEB.clockchangemode= aumanz
  WEB.save()

def getShift():
  mcmd= os.path.join(VMECFDIR,"ttcmidaemons/monshiftclock2.py")
  iop= popen2.popen2(mcmd+" s", 1) #0- unbuffered, 1-line buffered
  line= iop[0].readline()
  iop[0].close()
  iop[1].close()
  #print "getShift",line,":"
  return line[:-1]
def checkandsave(csf_string, fineshift="None", force=None):
  """csf_string: clock shift (float string:  e.g.: '0.923819' )
  fineshift: 
  "None"   change $dbctp/clockshift (if change is different from the last one!)
  != 'None' apply also CORDE modification
  force:
  None (default) -apply change only when outside of MIN,MAX interval
  !=None         -apply any change (still the check for difference is done)
          force option is used from ttcmidaemons/apply_any_shift.py 
          to change corde shift in $dbctp/clockshift

  Operation:
  If clock shift too big and last applied change was different: 
     modify $dbctp/clockshift file
  If we arrange this function to be called in time of clock change,
  the registers on CORDE board will be changed 3min later -when clock
  is changed
  29.6.: logic changed:
  - checkandsave(csf_string, beammode) is called only once: at SQUEEZE
    (i.e. ALWAYS fine shift done (if change too big)
  - no action in time of the clock change (BEAM1/LOCAL)
  - check if 'changed' removed (see: 'if False'...)
  """
  csps= int(eval(csf_string)*1000)   #ns-> ps
  csps_applied_new= int(eval(csf_string)*10000)   #ns-> 0.1ps
  #if ((csps < -250) or (csps > 250)) or (force != None):
  if ((csps < -10) or (csps > 10)) or (force != None):
    if ((csps < -1500) or (csps > 1500)) and (force == None):
      mylog.logm("csps:%dps too big (max 1500(now 1500) ps allowed). No action."%csps)
    else:
      fn= os.path.join(os.environ['dbctp'],"clockshift")
      f= open(fn,"r")
      (ttcmi_hns, corde_10ps_str, last_applied)= f.readline().split()     
      corde_10ps= int(corde_10ps_str)
      f.close()
      mylog.logm("csps:%d _applied_new:%d"%(csps, csps_applied_new))
      if last_applied:
        if False:   #do not check change! #eval(last_applied)==csps_applied_new:
          mylog.logm("db (%s %d %s) not changed, already applied. "%\
            (ttcmi_hns, corde_10ps, last_applied))
        else:
          newcorde= corde_10ps - csps/10
          #f= open(fn,"w")   -written in .c (CORDE_SET)
          #line= "%s %d %s"%(ttcmi_hns,newcorde, csps_applied_new); f.write(line)
          #f.close()
          if fineshift != "None":
            f= open(fn,"r"); line= f.readline(); f.close()
            mylog.logm("Clock shift in db before SET: %s"%line)
            corde_shift= "%d"%(-csps/10)
            arg= (corde_shift,)
            res= pydim.dic_cmnd_service("TTCMI/CORDE_SET", arg, "C")
            mylog.logm("dim TTCMI/CORDE_SET "+corde_shift, 1)
            #time.sleep(1)
      else:
          mylog.logm("bad $dbctp/clockshift 3rd number (last_applied) missing(%s %d)."%\
            (ttcmi_hns, corde_10ps))
  else:
    mylog.logm("Measured clock shift: %dps left unchaged (%s) "%(csps,str(fineshift)))
  # always apply DLL_RESYNC:
  if force != None:
    mylog.logm("DLL_RESYNC not done (force option).")
  else:
    arg=("none",)
    #res= pydim.dic_cmnd_service("TTCMI/DLL_RESYNC", arg, "C")
    mylog.logm("DLL_RESYNC not started...")   # CJI
def checkShift():
  cshift= getShift()
  mylog.logm("checkShift: after 10 secs:"+ cshift)
def callback_bmold(bm):
  #print "callback_bmold: '%s' (%s)" % (bm, type(bm))
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
  mylog.logm("callback_bmold: "+bmname)
def callback_fsn(fnum_name):
  fsn= rmzero(fnum_name)   # number or filling sheme name
  #if fsn!="" :mylog.logm("FSN:%s:"%fsn)
  #print "callback_fsn: '%s' (type:%s)" % (fsn, type(fnum_name))
def callback_bm(ecsbm):
  #print "callback_bm: '%s' (%s)" % (p2, type(p2))
  #WEB.miclock= rmzero(now) ; WEB.save()
  bmname= rmzero(ecsbm)
  #print "callback_bm: '%s' (%s)" % (bmname, type(bmname))
  bm= bm2clocknames[bmname]
  if bm2clock.has_key(bm):
    i01= bm2clock[bm][0]
    if i01==0:
      expclock= "LOCAL"
    else:
      expclock= "BEAM1"
    bmname= bm2clock[bm][1]
  else:
    expclock= "?" ; bmname="???"
  if bmname == WEB.lastbmname:
    #mylog.logm("No change in BM, no action")
    return
  prev_bmname= WEB.lastbmname
  WEB.lastbmname= bmname
  ## 
  #mylog.logm("callback_bm: "+bmname)
  #if (prev_bmname=="RAMP") or (bmname=="FLAT TOP"):
  if (bmname=="PREPARE RAMP") or (bmname=="RAMP"):
    sys.path.append(os.path.join(os.environ['VMECFDIR'],"filling"))
    import getfsdip
    reload(getfsdip)
    mylog.logm("getfsdip.py act...")
    getfsdip.main("act")
  cshift= getShift()
  if WEB.miclock==expclock:
    if bmname=="SQUEEZE":
      if cshift!='old':
        checkandsave(cshift, bmname)   # fine shift
        # it should certainly be less then 100ps after adjustment:
        t= threading.Timer(10.0, checkShift)
        t.start()
      else:
        arg=("none",)
        #res= pydim.dic_cmnd_service("TTCMI/DLL_RESYNC", arg, "C")
        #mylog.logm("DLL_RESYNC after clock shift started...")
        mylog.logm("DLL_RESYNC after clock shift NOT started...")   # CJI
    mylog.logm("BEAM MODE:%s, clock: %s OK, shift:%s"%(bmname, expclock, cshift))
    if bmname=="RAMP_NOSCOPE":   # never do this (no scope)
      if os.environ['VMESITE']=='ALICE':
        import sctel
        reload(sctel)
        mininf= "INF"
        tn= sctel.TN()
        tn.setPersitence(mininf)
        tn.close()
        mylog.logm("alidcsaux008 scope persitence: "+mininf)
  else:
    #if (bm==13) or (bm==4): # BEAM DUMP/INJECTION SETUP BEAM adjust clock
    #if expclock != "LOCAL":
    #  checkandsave(cshift)   # not called from 29.6.
    #mylog.logm( "BEAM MODE:%s, clock %s not correct. miclock mode:%s shift:%s"%\
    #  (bmname, WEB.miclock,WEB.clockchangemode, cshift) )
    #print "%s bad clock:%s for beam mode:%s(%d) clock_change_mode:%s"%\
    #  (ltim, WEB.miclock, bmname,bm, WEB.clockchangemode)
    if (bm>=5) or (bm<=12):
      if WEB.clockchangemode=="AUTO":
        wf='f'
      else:
        wf='w'
      mylog.infolog( "BEAM MODE:%s, clock %s not correct. miclock mode:%s"%\
      (bmname, WEB.miclock,WEB.clockchangemode), level=wf )
    if WEB.clockchangemode=='AUTO_NEVERCHANGE':  # it is on lhcint now to change th clock
      # change clock
      mylog.logm("changing clock to %s. Wait 3 half-minutes please..."%(expclock))
      WEB.newclock= expclock; WEB.save()
      res= pydim.dic_cmnd_service("TTCMI/MICLOCK_SET", (expclock,), "C")
      mylog.logm("cmnd sent: TTCMI/MICLOCK_SET")
      mininf=""
      if expclock=="LOCAL":
        mininf= "MIN"
      else:
        if bmname=="RAMP":
           mininf= "INF"
      #if mininf!="":
      if mininf=="NOSCOPE":   # never do tis (no scope)
        if os.environ['VMESITE']=='ALICE':
          import sctel
          reload(sctel)
          tn= sctel.TN()
          mylog.logm("setting scope persitence: "+mininf)
          tn.setPersitence(mininf)
          tn.close()
          #mylog.logm("alidcsaux008 scope persitence: "+mininf)
      ##mylog.logm("NOT CHANGED ##")
  mylog.flush()

def cbtran(now):
  nownz= rmzero(now)
  WEB.transition= nownz ; WEB.save()
  mylog.logm("TTCMI/MICLOCK_TRANSITION: %s."%(nownz)) #,type(now)
def main():
  global WEB,mylog
  if not pydim.dis_get_dns_node():
    print "Please set the environment variable DIM_DNS_NODE (aldaqecs)"
    sys.exit(1)
  #signal.signal(signal.SIGKILL, signal_handler)
  #signal.signal(signal.SIGUSR1, signal_handler)
  for bmix in bm2clock.keys():
    bmnamx= bm2clock[bmix][1]
    bm2clocknames[bmnamx]= bmix
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
    sys.exit(1)
  mylog= pylog.Pylog("miclock","ttyYES")
  pid= str(os.getpid())
  mylog.logm("my pid:"+ pid+ " MICLOCKID:"+ MICLOCKID)
  f= open(MICLOCKID, "w"); f.write(pid+'\n'); f.close()
  # authenticate:
  if os.environ['USER']!="trigger" and os.environ['USER']!="oerjan":
    print "Warning: not trigger account:",os.environ['USER']
  ##mylog.logm("## vesion -i.e. miclock_shift.py")
  mylog.logm("miclock.py started...")
  time.sleep(2)   # 1sec was enough
  WEB=web()
  res = pydim.dic_info_service("TTCMI/MICLOCK", "C", callback1)
  restran = pydim.dic_info_service("TTCMI/MICLOCK_TRANSITION", "C", cbtran)
  # next line after res service (i.e. current clock retrieved already)
  resbmold = pydim.dic_info_service("CTPDIM/BEAMMODE", "L:1", callback_bmold)
  if os.environ['VMESITE']=='ALICE':
    maid = pydim.dic_info_service("ALICE/LHC/TTCMI/CLOCK_MODE", "C", callback_manauto)
    # following returns '' between fills
    resbm = pydim.dic_info_service("ALICEDAQ_LHCBeamMode", "C:100", callback_bm)
    # following commented, when not available (between fills) message:
    # DIM Wrapper: src/dimmodule.cpp:1588 :: dic_ino_service_dummy: ERROR: Could not get new data to update service
    #resfn = pydim.dic_info_service("ALICEDAQ_LHCFillNumber", "C:100", callback_fsn)
  # ALICEDAQ_LHCFillNumber not available after dump (availablebe after INJECTION PROBE...)
    resfsn= pydim.dic_info_service("ALICEDAQ_LHCFillingSchemeName", "C:100", callback_fsn)
    #print "res...:", resbm, res, restran
    if not res or not restran or not resbm:
      mylog.logm("Error registering with info_services"%d(resbm, res, restran))
      sys.exit(1)
  while True:
    #time.sleep(10)
    #man/auto     -change operation mode (manual or automatic) now:%s
    #             auto is forbidden from 28.4.2015
    try:
      #a= raw_input(  enter BEAM1 BEAM2 REF LOCAL man auto (now:%s)
      a= raw_input("""
   enter:
   BEAM1       	-change the ALICE clock to BEAM1
   LOCAL       	-change the ALICE clock to LOCAL
   getshift    	-display current clock shift
   reset       	-reset current clock shift to 0
   q            -quit this script
""")
      #%WEB.clockchangemode)
    except:
      a='q'
      mylog.logm("exception:"+str(sys.exc_info()[0]))
    if string.find("getshift",a)==0: a="getshift"
    if (a!='q') and (a!='') and \
      (a!='BEAM1') and (a!='BEAM2') and (a!='LOCAL') and \
      (a!='getshift') and (a!='reset') and \
      (a!='REF') and (a!='man') and (a!='auto') and (a!='show') :
      mylog.logm('bad input:%s'%a) ; continue
    if a=='q': break
    if a=='': continue
    if a=='auto':
      mylog.logm("Attempt to go to auto... Forbidden, no action")
      #WEB.clockchangemode='auto'
      #WEB.save()
    elif a=='man':
      WEB.clockchangemode='MANUAL'
      WEB.save()
    elif a=='show':
      WEB.show()
    elif (a=='getshift'):
      cshift= getShift()
      if cshift != "old":
        mylog.logm("Clock shift: %s ns."%cshift)
      else:
        mylog.logm("Clock shift not measured (too old).")
    elif a=='reset':
      cshift= getShift()
      if cshift != "old":
        mylog.logm("Clock shift (%s ns) reset..."%cshift)
        checkandsave(cshift,"fineyes", force='yes')
      else:
        mylog.logm("Clock shift measurement is too old, reset not done")
    else:
      mylog.logm("Wait 3 half-minutes till MICLOCK_TRANSITION is 0. Switching to "+a+" ...");
      ##mylog.logm("not supported... ##, i.e. miclock_shift debug version")
      ##continue
      WEB.newclock= a; WEB.save()
      arg= (a,)
      res= pydim.dic_cmnd_service("TTCMI/MICLOCK_SET", arg, "C")
      mylog.logm("TTCMI/MICLOCK_SET "+a, 1)
      mylog.flush()
      time.sleep(1)
  ##os.remove(MICLOCKID)
  os.remove(MICLOCKID)
  #pydim.dic_release_service(resbm)    -Segmentation fault when 'q'
  #pydim.dic_release_service(res)
  #pydim.dic_release_service(restran)
  mylog.close()

if __name__=="__main__":
  main()

