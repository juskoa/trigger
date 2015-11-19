#!/usr/bin/python
#on ACT client i.e. machine where this script is called by:
# ssh trigger@alidcscom026 'ctpproxy.py actrestart' >/tmp/ctp_actrestart.log
# -check $?
#
#1. [ ssh-keygen -t rsa ]
#2.cat ~/.ssh/id_rsa.pub | ssh trigger@alidcscom026 'cat - >>.ssh/authorized_keys'

import os,sys,string,datetime
#
VMEWORK=os.environ['VMEWORKDIR']
DELTA=None
OFFSET=None
#
############################################################################################### 
def openFile(filename):
  """
      Input file.
  """
  try:
   infile=open(filename,"r")
  except IOError:
   print "Cannot open file ",filename
   return None
  else:
   print "File " ,filename," opened succesfully"
   lines=infile.readlines()
   infile.close()
   return lines
def run():
  log='orbitddl2'
  dirwork=VMEWORK+'/WORK/'
  logfile=dirwork+log+'.log'
  dircf=os.environ['VMECFDIR']
  # save old log
  if os.path.isfile(logfile):
   time=str(datetime.datetime.now())
   items=time.split()
   tt=items[1][0:8]
   tt=tt.replace(':','_')
   logsave=dirwork+log+items[0]+'_'+tt+'.log'
   os.rename(logfile,logsave)
  runcmd=dircf+'/ctp++/findLMOrbitOff.e>'+logfile
  os.system(runcmd)
def anal():
  """
     Analyse output of findLM0OrbitOff
  """
  global DELTA,OFFSET
  log='orbitddl2'
  dirwork=VMEWORK+'/WORK/'
  logfile=dirwork+log+'.log'
  lines=openFile(logfile)
  if lines:
   for i in lines:
     if i.find('Warning') >=0:
      print i.strip()
     if i.find('ERROR') >= 0:
      print i.strip()
      return 1;
     elif i.find('DELTA') >=0 :
      items = i.split()
      try:
        num=int(items[1],16)
      except ValueError:
        print 'ValueError:',items[1]
        return 1
      else:
        DELTA=str(num)
        if num==0:
         print "Everything ok"
         return 1
        else:
         print "DELTA != 0 !"
         print i.strip()
	 if num==0xffffffff:
          print 'DELTA not found. Probably no INTS, check log and CTP config'
	  return 1
     elif i.find('OFFSET') >= 0:
      items = i.split()
      try:
        num=int(items[1],16)
      except ValueError:
        print 'ValueError:',items[1]
        return 1
      else:
        OFFSET=str(num)
        print i.strip()
     else: continue
   return 0
  else: return 1
def set():
  delta=int(DELTA,16)
  if delta == 0:
    print 'Nothing to do. Measured delta=0'
    return 
  dircf=os.environ['VMECFDIR']
  runcmd=dircf+'/ctp++/findLMOrbitOff.e '+DELTA+' '+OFFSET + ' > /dev/null'
  print 'runcmd: ',runcmd
  os.system(runcmd)
  print 'Print offset set, next DELTA should be 0. Please, check with run and anal' 
def configctp():
  dircf=os.environ['VMECFDIR']
  runcmd=dircf+'/ctp++/findLMOrbitOff.e '+'3'
  print 'runcmd: ',runcmd
  os.system(runcmd)

def main():
  import sys
  rc=0
  if len(sys.argv) < 2:
    print """
Usage:
logs are in $VMEWORKDIR/WORK

orbitddl2.py run 
expects
-INT1 or INT2 to be defined via input 3
-some signal faster than 1 in 27milisec on input 3
takes 
-l0 and int ssm
-compares l0 and int IR to find orbit offsets

orbitddl2.py anal 
- reads orbitddl2.log in WORK dir
- parses measured offset


orbitddl2.py analset
- runs anal()
- sets measured offset to hw


orbitddl2.py configctp
- set swicth
-select interaction
- starts random
"""
    rc=8
  else:
   if sys.argv[1] == 'run':
    run()
   elif sys.argv[1] == 'anal':
    anal()
   elif sys.argv[1] == 'analset':
    if anal()==0: 
      if DELTA==None or OFFSET==None: 
       print 'No action: DELTA: ',DELTA,' OFFSET: ',OFFSET
       return
      set()
   elif sys.argv[1] == 'configctp':
      configctp() 
   else:
    print "nothing to do"
    rc=0
if __name__ == "__main__":
  rc=main()
  sys.exit(rc)
