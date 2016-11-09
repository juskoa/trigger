#!/usr/bin/python
import os,os.path,time,subprocess,string,types
def iopipe(cmd, grep=''):
   #print "popen2("+cmd+")"
   #iop= popen2.popen2(cmd, 1) #0- unbuffered, 1-line buffered
   if type(cmd)==types.ListType:
     cmdl= cmd
   else:
     cmdl= string.split(cmd)
   p= subprocess.Popen(cmdl, bufsize=1,
     stdin=subprocess.PIPE, stdout=subprocess.PIPE, close_fds=True)
   iop= (p.stdout, p.stdin)
   line=''
   if iop:
     #print "pylog.iopipe ioplen:",len(iop)    (2)
     if grep=='1':
       line= iop[0].readline()
     elif grep!='':   # return 1st line starting with given string
       while True:
         line= iop[0].readline()
         if line =='': break
         if string.find(line, grep)==0:
           break
     else:            # return array of stdout lines
       line= iop[0].readlines()
     iop[0].close()
     iop[1].close()
   return line

class Pylog:
  """Usage:
  lg= pylog.Pylog("file_name")  only file
  lg= pylog.Pylog("file_name", "tty")  file+[stdout]
  lg= pylog.Pylog("file_name", info="info")  file+[infoLogger]
  lg.logm("message")    write to file + [stdout,infoLogger]
  lg.logm("message", 1)    write only file
  lg.logm("message", 2)    write only tty (if default is ON)
  lg.flush()
  msgtime= lg.gettimenow()
  lg.logm("message", ltime=msgtime)    use msgtime instead of current time
  """
  def __init__(self,lname=None, tty=None, info=None):
    self.lname=lname   # logname (in VMEWORKDIR or current directory)
    self.tty=tty       # stdout
    self.info=info     # infoLogger
    self.logf=None
    if os.environ.has_key('VMEWORKDIR'):
      wd= os.path.join(os.environ['VMEWORKDIR'], "WORK")
    else:
      wd= os.getcwd()
    if self.lname:
      self.pname= os.path.join(wd, lname)
      self.open()
  def close(self):
    if self.logf: self.logf.close()
  def flush(self):
    if self.logf: 
      self.logf.flush()
      #os.fsync()
  def open(self):
    if self.lname==None: return
    if os.path.exists(self.pname+".log"):
      lt= time.localtime()
      #ltim= "%2d.%2d. %2d:%2d"%(lt[2], lt[1], lt[3], lt[4])
      newn= self.pname+"%2.2d%2.2d%2.2d%2.2d%2.2d"%(lt[0]-2000,lt[1],lt[2],lt[3],lt[4])
      mvcmd= "mv %s.log %s.log"%(self.pname, newn)
      os.system(mvcmd)
    self.logf= open(self.pname+".log","w")  
  def gettimenow(self, secs=None):
    lt= time.localtime()
    if secs!=None:
      ltim= "%2.2d.%2.2d. %2.2d:%2.2d:%2d "%(lt[2], lt[1], lt[3], lt[4], lt[5])
    else:
      ltim= "%2.2d.%2.2d. %2.2d:%2.2d "%(lt[2], lt[1], lt[3], lt[4])
    return ltim
  def logm(self,msg,log2=3, ltime=None):
    """log2:   1: only file   2: only tty   3: both
    ltime: if present use it instead of the current one
    """
    if ltime==None:
      ltime= self.gettimenow()
    tmsg= ltime+msg
    if self.tty and (log2>=2): print tmsg
    if self.logf and (log2!=2): self.logf.write(tmsg+'\n')
  def infolog(self, msg, level='i', partition=""):
    """
    level: i(nfo) w(arning) e(rror) f(atal)
    partition: partition name
"""
    if self.info: 
      if partition!="":
        part= "-p "+partition
      else:
        part=""
      #iopipe("sh -c '/opt/infoLogger/log -%s %s'"%(level, msg))
      #iopipe("sh -l -c '/opt/infoLogger/log -%s %s'"%(level, msg))
      #os.system("/opt/infoLogger/log -%s %s"%(level, msg))
      #iopipe("export DATE_INFOLOGGER_SYSTEM=TRG ; /opt/infoLogger/log -%s %s"%(level, msg))
      #iopipe("/opt/infoLogger/log -%s %s"%(level, msg))
      #iopipe("infologger.sh -%s '%s'"%(level, msg))
      #iopipe("infologger.sh -%s '%s'"%(level, msg))
      # seems not ok from getfsdip.py  with cat (ok from cmdline )
      #iopipe("cat /dev/null | /opt/infoLogger/log -f CTP -l OPS %s -s %s '%s'"%\
      iopipe(['/opt/infoLogger/log', '-f', 'CTP', '-l', 'OPS', part, '-s', level, '%s'%msg])
    self.logm("%s:%s: %s"%(level, partition, msg))
def main(argv):
  if len(argv) < 2:
    print """
"""
    help(Pylog)

if __name__ == "__main__":
  import sys
  main(sys.argv)
