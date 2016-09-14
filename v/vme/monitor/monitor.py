#!/usr/bin/python
import os,sys,string,subprocess,time,types,signal,socket,smtplib,pylog
from threading import Thread

UDP_TIMEOUT=70
ALARMhddtemp=35   # if >=, issue sms
OKhddtemp=33      # as soon it goes back to this temp., sms: 'ok temp'
log=None
quit=None
pidpath= os.path.join(os.environ['VMEWORKDIR'], "WORK","monitor.pid")
pitlab="lab"
if string.find(os.environ["HOSTNAME"],"alidcscom")==0:
  pitlab="pit"
def signal_handler(signal, stack):
  global quit
  log.logm("signal:%d received, quitting monitor.py..."%signal)
  log.logm("anyhow, waiting till udp timeout elapses...")
  log.logm("rm %s..."%pidpath)
  log.flush()
  quit="quit"
  #time.sleep(2)
  os.remove(pidpath)
  #sys.exit(8)

def send_mail(text, subject='', to='41754112090@mail2sms.cern.ch'):
  """
  Usage:
  send_mail('This is a test', 'subj')
  """
  sender="Anton.Jusko@cern.ch"   #must, or error: 5.1.7 Invalid address
  headers = "From: %s\r\nTo: %s\r\nSubject: %s\r\n\r\n" % (sender, to, subject)
  message = headers + text
  mailServer = smtplib.SMTP("cernmx.cern.ch")
  mailServer.sendmail(sender, to, message)
  mailServer.quit()

def gcalib_onfunc(inm):
  """ inm: gcalib 18.08.2011 09:57:50  TOF 92.917 4.903  MUON_TRG 88.031 0.017  
  T0 0.000  0.000  ZDC 0.000  0.000  EMCAL 0.000  0.000
  rc:
  [None] -cannot decide
  [ON, msg] - check all detectors receiving L2a if they get also PP
  [HUNG, msg] - at least one detector receives L2a, but not PP
  """
  rates= string.split(inm)
  outmsg="" ; rc0= Daemon.ON ; checked= checked_0= 0
  for ix in range(3,len(rates)-3,3):   
    checked= checked+1
    if rates[ix+1]!= "0.000":   # L2a rate
      outmsg= outmsg + "%s:%s/%s "%(rates[ix], rates[ix+1], rates[ix+2])
      if rates[ix+2] == "0.000":   # PP rate
        rc0= Daemon.HUNG       # at least one detector not getting PP
    else:
      checked_0= checked_0+1
  if checked == checked_0: rc0= None
  return [rc0, outmsg]

def miclock_onfunc(inm):
  """ inm: miclock clock beammode
  rc:
  [ON, msg] -ok
"""
  outmsg="" ; rc0= Daemon.ON
  inms= string.split(inm)
  ix=inm.find(' ')
  if ix > 0:
    msg= inm[(ix+1):]
  else:
    msg="noclock nobeammode"
  return [rc0, msg]

class Udp(Thread):
  def __init__(self, daemons):
    Thread.__init__(self)
    self.daemons= daemons
    host = "localhost"
    port = 9931
    addr = (host,port)
    self.UDPSock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    self.UDPSock.settimeout(UDP_TIMEOUT)   # None disables timeout
    self.UDPSock.bind(addr)
    log.logm("starting udp thread...")
    self.start()
  def run(self):
    while 1:
      if quit: 
        self.UDPSock.settimeout(None)   # None disables timeout
        # previous line probably not to be there (error msg when monitor stopped)
        self.close()
        break
      data= self.waitudp()
      if data=="":
        # when no udp packet received in UDP_TIMEOUT (should not happen with
        # stable monitoring of more udp-processes)
        #log.logm("udp timeout")  
        continue
      #log.logm("udp:"+data)
      dname= string.split(data)[0]
      if self.daemons.has_key(dname):
        self.daemons[dname].regudp(data)
      else:
        log.logm("unknown udp:"+data)
  def waitudp(self):
    try:
      data,addr = self.UDPSock.recvfrom(1024)
    except:
      print sys.exc_info()[0]   # this produces: <class 'socket.timeout'>
      #log.logm("waitudp except")
      data=""
    return data
  def close(self):
    #self.UDPSock.shutdown(socket.SHUT_RD) 
    # socket.error: (107, 'Transport endpoint is not connected')
    self.UDPSock.close() 

class Daemon:
  # return codes from do_check()
  ON=0
  IDLE=1
  OFF=2
  HUNG=3
  # states
  DOWN=0
  RESTARTED=1
  OK=2
  MAXSMS=1   # max. sms message to be sent when daemon died
  def __init__(self,name, scb="scb", onfunc=None, autor='y'):
    """
    scb: 
    "scb": startClients.bash way check (i.e. name used 
          with startClietns.bash to check status/start)
    "udp" check last udp message arrived from this Daemon (name still
          used for status/start with startClients.bash)
    "hddtemp" just execute self.hddtemp.
          ["OFF"] in case onfunc could not run correctly
          do_check returns: 
            [self.ON, msg_temp]    -temperature ok
            [self.HUNG, msg_temp]  -high temperature (msg sent)
            [self.OFF]             -cannot measure

    onfunc: execute with each UDP message (scb="udp")
            onfunc() should return: [None], [ON,msg] or [HUNG]

    autor: 'y' automatic restart
           'n' action done (i.e. mail), but not restarted
    """
    self.name= name
    self.autor= autor
    self.scb= scb
    if scb=="hddtemp":
      self.autor='n'
      self.hightempstate= False
    self.onfunc= onfunc 
    self.state= None   # ok, restarted, down
    self.d1= self.d2= None    # 'dates' of last 2 actions (d2: older)
    self.a1= self.a2= None    # last 2 'actions' done
    self.last_state= None
    self.udplast= None        # or [time,data]
    self.sms_sent= 0
    rc= self.do_check()
    if (rc[0]==Daemon.OFF) or (rc[0]==Daemon.HUNG):
      self.state= Daemon.DOWN
      self.logm('down')
    if (rc[0]==Daemon.IDLE):
      self.state= Daemon.OK
      self.logm('idle')
    if (rc[0]==Daemon.ON):
      self.state= Daemon.OK
      self.logm('on, '+rc[1])
    # read .last2 file
  def do_check(self):
    """ self.scb: defines a way how the status is checked:
    "scb" (StartClients.Bash way) -run localy (this script
          is supposed to run on server) startClients.bash
    "udp" check last udp message arrived from this Daemon
    "hddtemp" check temp against ALARMhddtemp/OKhddtemp.
          Not ready for 835, but:
[aj@pcalicebhm11 py]$ hddtemp /dev/sda
/dev/sda: ST3250310AS: 37oC
                         \xc2
    rc: ON: on, msg
        IDLE: idle (e.g.: gcalib on, but no calib. triggers needed)
        OFF: off (= not running)
        HUNG:
    """
    rc=[None]
    if self.scb=="hddtemp":
      rline= self.iopipe("hddtemp /dev/hda",lookfor="/dev/hda")
      if rline:
        #self.logm(rline)
        tempC= string.split(rline)[-1]
        temp= int(tempC[:-3])
        if temp>=ALARMhddtemp:
          msg="temp alarm:%d >= %d C"%(temp,ALARMhddtemp)
          if not self.hightempstate:
            self.hightempstate= True
            self.logm_mail(msg)
            #rc= [self.ON, msg]
        elif temp>OKhddtemp:
          msg="temp high:%d > %d C"%(temp,OKhddtemp)
          #rc= [self.ON, msg]   # state: OK all the time
          #rc= [self.HUNG, msg]
        else:
          msg= "temp ok:%d C"%temp
          if self.hightempstate:
            self.hightempstate= False
            self.logm_mail(msg)
          #rc= [self.ON, msg]
        rc= [self.ON, msg]
      else:
        rc= [self.OFF]
    elif self.udplast:
      difsec= time.time()- self.udplast[0]
      if difsec<UDP_TIMEOUT: # we got fresh udp message
        # let's decide from msg content if ok or not
        if self.onfunc:
          rc= self.onfunc(self.udplast[1])
          # should return: [None], [ON,msg] or [HUNG]
        else:
          self.logm("self.onfunc not defined")
      else:
        rc= [self.OFF, "no udp %d secs"%difsec]
    #else:
    #  rc= [None]
    if (rc[0]==None) and (self.scb=="scb"): 
      # we could not decide from udp, check at least status if possible,
      # i.e. 'scb':
      self.getpid()
      if self.pid!=None:
        rc= [self.IDLE]
      else:
        rc= [self.OFF]
    return rc
  def do_stop(self):
    pass
  def do_start(self):
    self.iopipe("startClients.bash "+self.name+" start")
    self.logm("started")
  def do_restart(self):
    """ 
    kill + start
    """
    rc= self.iopipe("startClients.bash "+self.name+" kill","killing:")
    self.logm(rc)
    time.sleep(1)
    self.do_start()
    return 
  def set_state(self, newstat):
    self.state= newstat
    if newstat==Daemon.OK: self.sms_sent=0
  def logm(self, msg, state=None):
    """ 
    log msg ONLY WHEN CHANGED. Keep last 2 messages in 
    self.a1/2 and their times in self.d1/2
    """
    ltime= log.gettimenow()
    if msg != self.a1:
      if ((state!=None) and (self.last_state != state)) or \
         (state==None): # log msg only if state changed, and the log required
        log.logm(self.name+': '+str(msg), ltime= ltime)
        log.flush()
      self.a2= self.a1 ; self.d2= self.d1
      self.a1= msg ; self.d1= ltime
      #print "logm:",self.name, msg
      self.last_state= state
  def logm_mail(self, msg):
    global pitlab
    msg2= pitlab+' '+self.name + ': ' + msg
    if self.sms_sent<self.MAXSMS:   # send max. MAXSMS messages for a daemon
      self.sms_sent= self.sms_sent+1
      self.logm(msg) #self.logm("mail:"+msg)
      if self.sms_sent==self.MAXSMS:
        msg2= msg2+". SMSs DISABLED (max. %d)"%self.MAXSMS
      send_mail(msg2)
      if self.name=='xcounters':
        send_mail("DOWN:xcounters",to='41754111902@mail2sms.cern.ch')
  def flush(self):
    if self.a2!=None: log.logm(self.name+': '+self.a2, ltime= self.d2)
    if self.a1!=None: log.logm(self.name+': '+self.a1, ltime= self.d1)
  def do_html(self):
    lin= "%8s | %s | %s:%24s | %s:%24s\n"%(self.name, self.autor, 
      str(self.d1), str(self.a1), str(self.d2), str(self.a2))
    return lin
  def getpid(self):
    self.pid= None
    pidline= self.iopipe("startClients.bash "+self.name+" status", "pid:")
    #self.logm("getpid:pidline:"+pidline+":")
    if pidline:
      pid= string.split(pidline)[1]
      self.pid= pid
  def iopipe(self, cmd, lookfor=None):
    #rcode= self.iopipe(os.WEXITSTATUS(os.system("startClients.bash "+self.name+" status")))
    #iop= popen2.popen2(cmd, 1)
    p= subprocess.Popen(string.split(cmd), bufsize=1,
      stdin=subprocess.PIPE, stdout=subprocess.PIPE, close_fds=True)
    iop= (p.stdout, p.stdin)
    rc=None
    while(1):
      line= iop[0].readline()
      #self.logm("iopipe:"+line+":")
      if line =='':
        break
      if lookfor:
        ix= string.find(line, lookfor)
        if ix>=0: rc= line[ix:]   # remaining test from 'pid:' returned
    iop[0].close()
    iop[1].close()
    return rc
  def regudp(self, udpdata):
    self.udplast= [time.time(),udpdata]

def main():
  global log
  if os.path.exists(pidpath):
    pidf=open(pidpath,"r"); pid= pidf.readline().rstrip(); pidf.close()
  else:
    pid=None
  if (len(sys.argv)>1) and ((sys.argv[1]=='stop') or (sys.argv[1]=='restart') ):
    if pid: 
      print "stopping %s..."%pid
      os.kill(int(pid), signal.SIGHUP)
      #os.remove(pidpath)
    else:
      print "monitor.py not active"
    if sys.argv[1]=='stop': return
    else: time.sleep(1) ; pid=None
  if pid!=None:
    print "monitor.py running. kill -s SIGHUP %s ; rm %s or use stop"%(pid,pidpath)
    return
  else:
    if (len(sys.argv)>1) and (sys.argv[1][-5:]=='start'):
      print "starting..."
    else:
      print """
nohup ./monitor.py start >$VMEWORKDIR/WORK/monitor.nohup &
monitor.py stop
"""
      return
  mypid= str(os.getpid())
  pidf=open(pidpath,"w"); pid= pidf.write(mypid); pidf.close()
  log=pylog.Pylog("monitor") #, tty="tty")
  log.logm("mypid: "+mypid+ " ALARMhddtemp: %d"%ALARMhddtemp)
  htmlfn= os.path.join(os.environ['VMEWORKDIR'], "WORK","monitor.html")
  signal.signal(signal.SIGUSR1, signal_handler)  # 10
  signal.signal(signal.SIGHUP, signal_handler)   # 1  kill -s SIGHUP mypid
  signal.signal(signal.SIGINT, signal_handler)   # 2  CTRL-C
  # all monitorable:
  #allds={"udpmon":Daemon("udpmon"), 
  #  "gcalib":Daemon("gcalib", onfunc=gcalib_onfunc),
  #  "pydim":Daemon("pydim"), "html":Daemon("html")}
  # in lab:
  #allds={"udpmon":Daemon("udpmon"), 
  #  "gcalib":Daemon("gcalib"),
  #  "ctpwsgi":Daemon("ctpwsgi"), "pydim":Daemon("pydim", autor='n'),
  #  "ttcmidim":Daemon("ttcmidim"), "html":Daemon("html"),
  #  "DiskTemp":Daemon("DiskTemp",scb="hddtemp")}
  #
  # allds={"udpmon":Daemon("udpmon"), 
  # "pydim":Daemon("pydim", autor='n'),
  #  "DiskTemp":Daemon("DiskTemp",scb="hddtemp")}
  # p2:
  if os.environ['VMESITE'] == "ALICE":
    allds={"xcounters":Daemon("xcounters", autor="n"), 
      "ctpwsgi":Daemon("ctpwsgi"), 
      "ttcmidim":Daemon("ttcmidim"), "html":Daemon("html"),
      "miclock":Daemon("miclock", scb="udp", onfunc=miclock_onfunc, autor="n"),
      "gcalib":Daemon("gcalib")}
    log.logm("Udp used.")
    udpmsg=Udp(allds)   
  else:
    # bhm10:
    # allds={"gcalib":Daemon("gcalib"), "ctpdim":Daemon("ctpdim")}
    # adls: ttcmidim problematic (runs on altri2 which is on bhm10)
    allds={"xcounters":Daemon("xcounters", autor="n"), 
      "ctpwsgi":Daemon("ctpwsgi"), 
      "html":Daemon("html"),
      "miclock":Daemon("miclock", scb="udp", onfunc=miclock_onfunc, autor="n"),
      "gcalib":Daemon("gcalib")}
    # test in lab:
    log.logm("Udp used...")
    udpmsg=Udp(allds)   
  lin=""
  for dm in allds.keys():
    lin=lin+dm+" "
  log.logm("monitored daemons: "+lin)
  while 1:
    for dmName in allds:
      dm= allds[dmName]
      # 2 sources:
      # status: up, down
      # lastlog: ok, smssent, down
      rc= dm.do_check()
      if rc[0]==Daemon.OFF:
        if dm.state != Daemon.RESTARTED:   # was DOWN or OK
          if dm.autor=='y':
            dm.do_start()
            dm.set_state(Daemon.RESTARTED)
            dm.logm_mail("DOWN/OK->OFF. restarted")
          else:
            dm.set_state(Daemon.DOWN)
            dm.logm_mail("DOWN/OK->OFF. not restarted (autrest:NO)")
        else:
          dm.logm_mail("DOWN/OK->OFF cannot restart")
      elif rc[0]==Daemon.HUNG:
        dm.logm("hung "+rc[1])
        if (dm.state == Daemon.OK) or (dm.state == Daemon.DOWN):
          if dm.autor=='y':
            dm.do_restart()
            dm.set_state(Daemon.RESTARTED)
          else:
            dm.logm_mail("DOWN/OK->HUNG. not restarted (autrest:NO)")
        elif dm.state == Daemon.RESTARTED:
          dm.logm_mail("RESTARTED->HUNG cannot restart")
      elif rc[0]==Daemon.IDLE:
        dm.set_state(Daemon.OK)
        dm.logm("idle")
      elif rc[0]==Daemon.ON:
        if dm.state!= Daemon.OK:
          dm.set_state(Daemon.OK)
          dm.logm("ok   "+rc[1], Daemon.OK)
      else:
        dm.logmsg= str(rc)
      if quit: 
        log.logm("quitting daemon loop")
        break
    htm= ""
    htmlf=open(htmlfn,"w")
    htmlf.write("Status at %s\n"%log.gettimenow())
    for dmName in allds:
      dm= allds[dmName]
      htm= dm.do_html()
      htmlf.write(htm)
    htmlf.close()
    log.flush()
    time.sleep(30)   # was 5 before 11.7.2013
    if quit:
      #for dm in allds: no need (a1,a2 are logged in the time of their creation)
      #  dm.flush()
      log.logm("quit. pid: %s..."%mypid)
      log.flush()
      #udpmsg.close()
      time.sleep(1)   #
      #os.remove(pidpath)
      break
if __name__ == "__main__":
  main()
