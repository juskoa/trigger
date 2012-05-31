#!/usr/bin/python
#on ACT client i.e. machine where this script is called by:
# ssh trigger@alidcscom026 'ctpproxy.py actrestart' >/tmp/ctp_actrestart.log
# -check $?
#
#1. [ ssh-keygen -t rsa ]
#2.cat ~/.ssh/id_rsa.pub | ssh trigger@alidcscom026 'cat - >>.ssh/authorized_keys'

import os,sys,string,popen2,time
class iopipe:
  def __init__(self, cmd, tilltext=None):
    #os.chdir(os.environ['VMEWORKDIR'])
    #clientcmd= os.path.join(os.getenv('VMECFDIR'),'ltudim/linux/ltuclient')+' '
    self.cmd= cmd
    print "popen2("+cmd+")"
    self.iop= popen2.popen2(cmd, 1) #0- unbuffered, 1-line buffered
    #print "iopipe.iop:",self.iop
    self.getout(tilltext)
    #self.iop[1].write('\n')
    #self.iop[1].close()
    #print "iopipe:",self.outlines
    #['PID xxx ltuclient:2.1 (popen already active, clients:3)']
  def close(self):
    #print "closing"
    #self.cmd('q')
    self.iop[0].close()
    self.iop[1].close()
  def getout(self, tilltext=None):
    """
    tilltext: None or any text: read all
              ""  : ignore the output completely
    """
    #self.iop[1].close()
    iline=0; self.outlines=[]
    if tilltext!=None:
      if tilltext=="":
        return ""
    while(1):
      line= self.iop[0].readline()
      #if line ==':\n': break  #don't take last '\n:\n'
      #print "%s:"%self.cmd,line
      if line =='':
        #line="pipe from cmdline interface closed/broken"
        #outlines.append(line)
        break
      #print line[:-1]
      self.outlines.append(line[:-1])
      iline=iline+1
      if iline>100: 
        self.outlines.append("ERROR: too many lines")
        break
    #print self.outlines
  def check(self, text):
    """ find text in self.out list, return index or -1 if not found
    """
    #print "check:lines:",len(self.outlines)
    for ix in range(len(self.outlines)):
      ix2= string.find(self.outlines[ix], text)
      if ix2==0: return ix
    return -1
  def cmd(self, cmd):
    out=None
    self.iop[1].write(cmd+'\n')
    #print self.nbcmd+':'+cmd
    #if cmd!='q': out= self.getout()
    return out

def main():
  import sys
  rc=0
  if len(sys.argv) < 2:
    print """

Usage:
ctpproxy.py actrestart
rc:
 1: internal error (syntax in this script)
 4: can't stop ctpproxy (global run active?), leaving it on
 5: can't download from ACT, ctpproxy left off
 6: not executed on alitri, nothing done
 7: can't start ctpproxy, after ctpproxy stop and ACT download
 8: actrestart expected, nothing done
 9: CTP switch not loaded, ctpproxy left off
10: can't update CTPRCFG/CS service
11: can't update CTPRCFG/INT1,2 services
"""
    rc=8
  else:
    #if os.environ.get('VMESITE')==None:
    iop= iopipe("hostname")
    hostname= iop.outlines[0]
    print "HOSTNAME:",os.environ.get('HOSTNAME'),"-%s-"%hostname
    #if os.environ.get('VMESITE')=="SERVER" or os.environ.get('VMESITE')==None:
    if hostname=="pcalicebhm05":
      os.environ["VMECFDIR"]="/data/ClientCommonRootFs/usr/local/trigger/vd/vme"
      vmectp="altri1"
      vmeswitch="trigger@altri2"
      #os.environ["ACT_DB"]= "daq:daq@pcald30/ACT"
    #elif hostname=="alidcscom026":
    #  os.environ["VMECFDIR"]="/data/ClientCommonRootFs/usr/local/trigger/v/vme"
    #  vmectp="alidcsvme001"
    #  vmeswitch="trigger@alidcsvme004"
    #  os.environ["ACT_DB"]= "daq:daq@aldaqdb/ACT"
    elif hostname=="alidcscom188":
      os.environ["VMECFDIR"]="/data/dl/root/usr/local/trigger/v/vme"
      vmectp="alidcsvme001"
      vmeswitch="trigger@alidcsvme004"
      #os.environ["ACT_DB"]= "daq:daq@aldaqdb/ACT"
    else:
      rc= 6
      print "host:",hostname, "rc:",rc
      return rc 
    #os.environ["LD_LIBRARY_PATH"]= os.environ["LD_LIBRARY_PATH"]+"
    dbctp= os.path.join(os.environ.get('VMECFDIR'), "CFG/ctp/DB")
    os.environ["dbctp"]= os.path.join(os.environ.get('VMECFDIR'), "CFG/ctp/DB")
    print "dbctp:", dbctp
    print "VMESITE:",os.environ.get('VMESITE')
    print "VMECFDIR:",os.environ.get('VMECFDIR')
    sys.stdout.flush()
    cmd=sys.argv[1]
    if cmd=='actrestart':
      iop= iopipe("ssh -2 -q %s ctpproxy.sh stop"%vmectp,"")
      time.sleep(2)
      iop= iopipe("ssh -2 -q %s ctpproxy.sh status"%vmectp)
      print iop.outlines
      if iop.check("TRIGGER::CTP not running")>=0:
        #iop= iopipe("...ctp_proxy/linux/act.exe")
        #iop= iopipe("startClients.bash ctpproxy actstart","nout")
        #actdownload=os.path.join(os.environ.get('VMECFDIR'), "ctp_proxy/linux/act.exe")
        #iop= iopipe(actdownload)
        #print iop.outlines
        #if iop.check("CTP config files downloaded from ACT.")>=0:
        os.chdir(os.path.join(os.environ.get('VMECFDIR'), "switchgui"))
        #iop= iopipe("./switched.py load")
        #print iop.outlines
        sys.path.append(os.environ.get('VMECFDIR')+'/switchgui/')
        import switched
        rc= switched.main("actload")
        if rc==0:
          iop= iopipe("ssh -2 -q %s '$VMECFDIR/../bin/loadswitch ctp'"%vmeswitch)
          ixl= iop.check("CTP.SWITCH: connected")
          if ixl>=0:
            print iop.outlines[ixl]
            iop= iopipe("cd $VMECFDIR/pydim ; linux/client CTPRCFG/RCFG intupdate")
            if iop.check("Callback: OK")>=0:
              iop= iopipe("ssh -2 -q %s ctpproxy.sh start"%vmectp,"")
              time.sleep(2)
              iop= iopipe("ssh -2 -q %s ctpproxy.sh status"%vmectp)
              if iop.check("TRIGGER::CTP running.")>=0:
                f= open(os.path.join(dbctp,"FillingScheme"),"r") ; 
                lin1= f.readline() ; f.close()
                if lin1[:7]=="bcmasks":
                  iop= iopipe("colschedule.bash update")
                  if iop.check("Callback: OK")>=0:
                    rc=0
                  else:
                    print iop.outlines
                    rc=10   # can't update CTPRCFG/CS service
                else:
                  #print "FillingScheme != BCMASK. -> CTPRCFG/CS not updated"
                  print "FillingScheme == auto, reading DIP service..."
                  sys.path.append(os.path.join(os.environ['VMECFDIR'],"filling"))
                  import getfsdip
                  rc=getfsdip.main("act")
                  if rc==0:
                    print "New masks prepared and CTPRCFG/CS DIM service updated"
                  else:
                    print "CTP masks not updated, filling scheme not ready through DIP"
                  # if DIP not available, nothing should happen, that's why we force rc to 0
                  rc=0
              else:
                print iop.outlines
                rc=7   # can't start ctpproxy
            else:
              print iop.outlines
              rc=11   # can't update CTPRCFG/INT1,2 services
          else:
            rc=9   # can't load CTP switch
        else:
          rc=5   # can't download from ACT
      else:
        rc=4     # can't stop ctpproxy
    #elif cmd=='stop': 
    #  iop= iopipe("startClients.bash ctpproxy stop")
    #  print iop.outlines
    #elif cmd=='status': 
    #  iop= iopipe("startClients.bash ctpproxy status")
    #  print iop.outlines
    elif cmd=='stop':
      #ok iop= iopipe("ssh -2 -f %s ctpproxy.sh stop </dev/null 2>&1"%vmectp)
      iop= iopipe("ssh -2 %s ctpproxy.sh stop"%vmectp)
      #pass
      #iop.getout()
      #print iop.outlines
    elif cmd=='start':
      iop= iopipe("ssh -2 -q %s ctpproxy.sh stop"%vmectp) ; time.sleep(2)
      #iop= iopipe("nohup ssh -2 -f -q %s ctpproxy.sh start &"%vmectp,"")
      iop= iopipe("ssh -2 -f -q %s ctpproxy.sh start"%vmectp,"")
      iop.close()
      #os.system("nohup ssh -2 -f -q %s ctpproxy.sh start &"%vmectp) ; time.sleep(2)
      #
      #rc= spawn("/usr/bin/ssh", "-2", "-f", "-q", vmectp, "ctpproxy.sh", "start") ; time.sleep(2)
      #print "spawn rc:",rc
      # not start, rc: None
      iop= iopipe("ssh -2 -q %s ctpproxy.sh status"%vmectp)
    else:
      print 'actrestart expected...'
      rc=8
  print "rc:",rc
  return rc
if __name__ == "__main__":
  rc=main()
  sys.exit(rc)
