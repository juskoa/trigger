#!/usr/bin/env python
#from __future__ import print_function
from builtins import object
import os,os.path,time,subprocess,types
def iopipe(cmd, grep=''):
   #print "popen2("+cmd+")"
   #iop= popen2.popen2(cmd, 1) #0- unbuffered, 1-line buffered
   if type(cmd)==list:
     cmdl= cmd
   else:
     #cmdl= string.split(cmd)
     cmdl= cmd.split()
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
         #if string.find(line, grep)==0:
         if line.find(grep)==0:
           break
     else:            # return array of stdout lines
       line= iop[0].readlines()
     iop[0].close()
     iop[1].close()
   return line

infologgerpath= "/opt/o2-InfoLogger/bin/o2-infologger-log"
if os.path.exists(infologgerpath):
  infologger= infologgerpath
else:
  infologger= None
def infoLogger(msg,facility="", level='Info'):
  if infologger:
    if facility!="":
      ofacility= "-oFacility={}".format(facility)
    os.system('{} -s {} -oSystem=TRG {} {}'.format(infologger, level, ofacility, msg))

class Pylog(object):
  """Usage:
    lg= Pylog(fname, tty="tty", info="info")  # stdout+infoLogger
    lg.infolog(argv[1],"CLOCK","D")    # Facility CLOCK,  Debug Info Warning Error Fatal
    lg.logm(argv[1])                   # only stdout
    to be tested:
    lg.logm("message", 1)    write only file
    lg.logm("message", 2)    write only tty (if default is ON)
    lg.flush()
    msgtime= lg.gettimenow()
    lg.logm("message", ltime=msgtime)    use msgtime instead of current time
  """
  def __init__(self,lname=None,tty=None,info=None,wdir=None):
    self.lname=lname   # logname (in CTP3_WORK_DIRECTORY or current directory)
    self.tty=tty       # stdout
    self.info=info     # infoLogger
    self.logf=None
    if wdir!=None:
      wd= wdir
    elif 'CTP3_WORK_DIRECTORY' in os.environ:
      wd= os.environ['CTP3_WORK_DIRECTORY']
    //if 'VMEWORKDIR' in os.environ:
    //  wd= os.path.join(os.environ['VMEWORKDIR'], "WORK")
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
    if False: # os.path.exists(self.pname+".log"):
      lt= time.localtime()
      #ltim= "%2d.%2d. %2d:%2d"%(lt[2], lt[1], lt[3], lt[4])
      newn= self.pname+"%2.2d%2.2d%2.2d%2.2d%2.2d"%(lt[0]-2000,lt[1],lt[2],lt[3],lt[4])
      mvcmd= "mv %s.log %s.log"%(self.pname, newn)
      os.system(mvcmd)
    self.logf= open(self.pname+".log","w")    # a+  if he same file...
  def gettimenow(self, secs=True):
    lt= time.localtime()
    if secs!=None:
      ltim= "%2.2d.%2.2d.%2.2d %2.2d:%2.2d:%2.2d "%(lt[2], lt[1], lt[0], lt[3], lt[4], lt[5])
    else:
      ltim= "%2.2d.%2.2d.%2.2d %2.2d:%2.2d "%(lt[2], lt[1], lt[0], lt[3], lt[4])
    return ltim
  def logm(self,msg,log2=None, ltime=None):
    """log2:   1: only file   2: only tty   3: both
    ltime: if present use it instead of the current one
    """
    if ltime==None: ltime= self.gettimenow()
    tmsg= ltime+msg
    if log2==None:
      log2= 0  if self.tty and (log2>=2): print(tmsg)
      if self.tty: log2= log2 | 0x2
      if self.logf: log2= log2 | 0x1
    if log2 & 0x2: print(tmsg)
    if log2 & 1:
      self.logf.write(tmsg+'\n')
      self.flush()
  def infolog(self, msg, facility="", level='Info'):
    """
    facility: use only one of these: CLOCK
    level: i(nfo) w(arning) e(rror) f(atal)
"""
    infoLogger(msg,facility,level)
    self.logm("{} {} {}".format(level, facility, msg))
def main(argv):
  if len(argv) < 2:
    print("""
""")
    help(Pylog)
  else:
    lg= Pylog(tty="tty", info="info")  # stdout+infoLogger
    lg.infolog(argv[1],"CLOCK","D")    # Facility CLOCK,  Debug Info Warning Error Fatal
    lg.logm(argv[1])                   # only stdout
    
if __name__ == "__main__":
  import sys
  main(sys.argv)
