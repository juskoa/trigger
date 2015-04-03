#!/usr/bin/python
import os,os.path,sys,string,time
def goover():
  import glob
  #os.chdir(self.basedir)
  globpat= 'ctp_proxy120[3-5]*.log'
  globpat= 'ctp_proxy1[23]*.log'
  rnames= glob.glob(globpat)
  rnames.sort()
  #print rnames, len(rnames)
  nlogs=0
  for fn in rnames:
    #print fn
    lsf= open(fn,'r')
    runs={}
    for lin in lsf.readlines():
      pl= string.split(lin)
      if len(pl)>=6 and pl[2]=='Run' and pl[5]=='xcounters.':
        run= pl[3]
        if run=='0:': continue
        #print(lin)
        if pl[4]== 'starting':
          runs[run]= '1'
        elif pl[4]== 'stopping':
          del runs[run]
        else:
          print(fn,'error:',lin)
    if len(runs) >0:
      print fn,':',runs.keys()
    lsf.close; nlogs= nlogs+1
  print("log files pattern %s, processed:%d"%(globpat,nlogs))

def main(argv):
  if len(argv) < 1:
    print """
  overfiles.py suffix
    -print out all files with names ending suffix
"""
    return
  goover()

if __name__ == "__main__":
    main(sys.argv)
