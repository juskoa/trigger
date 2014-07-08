#!/usr/bin/python
# start: nohup ./ltucheck.py restartloop sms >WORK/ltucheck.log
import os,sys, popen2, string, time, smtplib
#sys.path.append(os.environ['VMEBDIR'])

MAXATTEMPTS= 6   # 6th attemt is last one
mailorsms='mail'
def mail(text, subject='', to='41764872090@mail2sms.cern.ch'):
  """
  Usage:
  mail('This is a test', 'subj')
  """
  if mailorsms!='sms':
    print "%s:%s"%(subject, text)
    return
  sender="Anton.Jusko@cern.ch"
  headers = "From: %s\r\nTo: %s\r\nSubject: %s\r\n\r\n" % (sender, to, subject)
  message = headers + text
  mailServer = smtplib.SMTP("cernmx.cern.ch")
  mailServer.sendmail(sender, to, message)
  mailServer.quit()

def getarg(narg):
  rc=None
  if len(sys.argv)>narg: rc= sys.argv[narg]
  return rc
class ttcparts:
  def __init__(self):
    #os.chdir(os.environ['VMEWORKDIR'])
    self.nrestarts=0
    self.ltus=[]
    self.allltunames=[]  # ["ltuname", restarts] 
    fpath= os.path.join(os.getenv('VMECFDIR'),'CFG/ctp/DB/ttcparts.cfg')
    f= open(fpath,"r"); lines= f.readlines(); f.close()
    for line in lines:
      L= string.split(line)
      if len(L)>=3:
        if line[0]!='#':
          self.ltus.append((L[0], L[1], string.strip(L[2])))
          self.allltunames.append([L[0],0])
          #print L[0], L[1], L[2]
  def start(self, n=-1):
    self.current=n
  def getnext(self):
    self.current= self.current+1
    if self.current<len(self.ltus):
      return self.ltus[self.current]
    else:
      return None
  def printversion(self, ltu=None, cmd='vmeopr32(LTUVERSION_ADD)'):
    if ltu==None:
      ltus= self.allltunames
    else:
      ltus=[[ltu,0]]
    for lturestarts in ltus:
      ltu=lturestarts[0]
      cs=iopipe(ltu)
      if cs.firstout[0][:3] == 'PID':
        fo= cs.firstout[0][-10:]
        version= cs.cmd(cmd)
      else:
        fo=cs.firstout
        version=''
      cs.close()
      print ltu,":",fo, version
    return
  def restart(self, ltu=None):
    tl= time.localtime()
    restime= "%d.%d %d:%d"%(tl[2], tl[1], tl[3], tl[4])
    if ltu==None:
      ltus= self.allltunames
    else:
      ltus=[[ltu,0]]
    lturestarted=""
    for lturestarts in ltus:
      if lturestarts[1]>MAXATTEMPTS: continue
      ltu= lturestarts[0]
      cs=iopipe(ltu)
      fo= cs.firstout[0][-10:]
      #version= cs.cmd('vmeopr32(LTUVERSION_ADD)')
      if fo[0:7] != 'clients':
        if lturestarts[1] >= MAXATTEMPTS:
          if lturestarts[1]==MAXATTEMPTS:
            rc=os.system("lturestart.bash "+ltu)
            lturestarted= lturestarted + "LAST"+ltu + ' '
            print restime, "LASTrestart:", ltu, rc
          else:
            # too many attempts to restart -shoudl not be even checked
            pass
        else:
          rc=os.system("lturestart.bash "+ltu)
          lturestarted= lturestarted + ltu + ' '
          print restime, "restart:", ltu, rc
        lturestarts[1]= lturestarts[1] + 1
      cs.close()
    if lturestarted != '':
      mail(lturestarted, restime+" RESTART:")
    self.nrestarts= self.nrestarts+1
    if (self.nrestarts % 12)==1: print restime, "restarts:",self.nrestarts
    return

class iopipe:
  def __init__(self, ltu):
    self.ltu=ltu
    os.chdir(os.environ['VMEWORKDIR'])
    clientcmd= os.path.join(os.getenv('VMECFDIR'),'ltudim/lin64/ltuclient')+' '
    #print "popen2("+clientcmd+ltu+")"
    self.iop= popen2.popen2(clientcmd+ltu, 1) #0- unbuffered, 1-line buffered
    #print "iopipe.iop:",self.iop
    self.firstout= self.getout()
    #print "iopipe:",self.firstout
    #['PID xxx ltuclient:2.1 (popen already active, clients:3)']
  def close(self):
    #print "closing", self.ltu
    self.cmd('q')
    self.iop[0].close()
    self.iop[1].close()
  def getout(self):
    iline=0; out=[]
    while(1):
      line= self.iop[0].readline()
      if line ==':\n':   #don't take last '\n:\n'
        break
      if line =='':
        line="pipe from cmdline interface closed/broken"
        out.append(line)
        break
      #print line[:-1]
      out.append(line[:-1])
      iline=iline+1
      if iline>6: 
        out.append("ERROR: too many lines")
        break
    return out
  def cmd(self, cmd):
    out=None
    self.iop[1].write(cmd+'\n')
    #print self.nbcmd+':'+cmd
    if cmd!='q':
      out= self.getout()
    return out
def emptyfun(out):
  print 'emptyfun:',out

def main():
  all=ttcparts()
  if len(sys.argv)<2:
    print """Usage:
ltucheck.py state
  -check state of all LTUs (stdalone/global)
ltucheck.py busy [ltu]
  -print average busy time (busy/L0) in micsecs. If ltu not given, 
   measure for all LTUs (all ltu_proxies have to be restarted after March 3rd)
ltucheck.py printversion
  -check each ltuproxy if capable to respond with standard message
ltucheck.py ltu ssd
  -like printversion, but only for ssd 
ltucheck.py mail 'any message'
  -sned sms to 162090
ltucheck.py ltudefaults
  -start 'printltuDefaults' for all LTUs
ltucheck.py restart[loop] [sms]  (loop: repeat in the loop every 5 min)
   sms: if given, sms sent in case of restart 
  -restart ltuproxy if not responding with standard message 'clients:...'
"""
  else:
    arg2=getarg(2)
    if sys.argv[1]=='restart':
      if arg2 == 'sms': mailorsms='sms'
      all.restart()
    elif sys.argv[1]=='restartloop':
      if arg2 == 'sms': mailorsms='sms'
      while(1):
        all.restart()
        time.sleep(300)
    elif sys.argv[1]=='printversion':
      all.printversion()
    elif sys.argv[1]=='state':
      print "1:stdalone   0: GLOBAL"
      all.printversion(cmd='getsgmode()')
    elif sys.argv[1]=='ltu':
      all.printversion(ltu=sys.argv[2])
    elif sys.argv[1]=='busy':
      if len(sys.argv)==3:
        all.printversion(ltu=sys.argv[2], cmd='measureBusy100ms()')
      else:
        all.printversion(cmd='measureBusy100ms()')
    elif sys.argv[1]=='mail':
      mail(sys.argv[2], subject="mailtest")
    elif sys.argv[1]=='ltudefaults':
      all.printversion(cmd='printltuDefaults()')
    else:
      print "What?"
    mail("ok")
if __name__ == "__main__":
    main()

