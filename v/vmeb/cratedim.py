#!/usr/bin/python
from Tkinter import *
import os,sys,string,time
import myw

def main():
  """
  retcode: 0: ok   4:parameter error   8:server not running
  """
  if hasattr(sys,'version_info'):
    if sys.version_info >= (2, 3):
      # sick off the new hex() warnings, and no time to digest what the
      # impact will be!
      import warnings
      warnings.filterwarnings("ignore", category=FutureWarning, append=1)
  if len(sys.argv) < 2:
    print """Usage: cratedim.py detname
detname   -detector name, i.e. one of: """
    print myw.DimLTUservers.keys()
    return 4
  if os.environ.has_key('VMECFDIR'):
    cfdir= os.environ['VMECFDIR']
  else:
    cfdir= '/opt/ltuclient/vme'
  #print "VMECFDIR:",cfdir
  board= sys.argv[1]
  if myw.DimLTUservers.has_key(board):
    if not os.environ.has_key('VMEWORKDIR'):
      print "VMEWORKDIR env. variable not defined"
      return 4
    #else:
    #  wdup1=os.path.join(os.environ['VMEWORKDIR'],"../")
    #  os.chdir(wdup1)
    wdir=os.environ['VMEWORKDIR']
    if os.access(wdir, os.W_OK) == 0:
      # create  working directory (first time call for this detector):
      print "creating VMEWORKDIR:",os.environ['VMEWORKDIR'],'...'
      os.makedirs(wdir)
      os.chdir(wdir)
      cmd="cp -a "+ os.path.join(cfdir,"WORKIMAGE","*") + " ."
      os.system(cmd)
    else:
      os.chdir(wdir)
    #print "VMEWORKDIR:",os.environ['VMEWORKDIR']
    f=Tk()
    f.title("ltu client")
    f1= myw.getCrate(f)
    vmeboard= myw.VmeBoard(f1, boardName=board)
    f1.addBoard(vmeboard)
    syspadd= os.path.join(cfdir,"ltu")
    sys.path.append(syspadd)   # to find user gui routines
  else:
    print "Known detectors:",myw.DimLTUservers.keys()
    return 4
  if myw.vbexec==None:   # nbi LTU could open it
    #print 'opening log window'
    try:
      vmeboard.openCmd(); 
      #print "cratedim.openCmdio:",vmeboard.io.thds[0].io
      if vmeboard.io.thds[0].io:
        myw.vbinit(vmeboard)
      else:
        #f.after(2000, sys.exit(4))
        return 8
    except:
      print "exception:",sys.exc_info()[0]
      return 8
  f.mainloop()
  
if __name__ == "__main__":
  retcode= main()
  sys.exit(retcode)

