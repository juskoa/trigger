#!/usr/bin/python
#import myw, txtproc, trigdb
import os,sys,trigdb
def main():
  #if len(sys.argv)>=2:
  #  rname=sys.argv[1]
  #else:
  #  print "give rXXXXX.rcfg in VMEWORKDIR/WORK"
  #  return
  basedir= os.path.join(trigdb.TRGWORKDIR, "RCFG","y2009tillJul16")
  rf= trigdb.TrgRcfg(basedir)
  #rf.addrcfg("r75544.rcfg"); rf.addrcfg("r75545.rcfg")
  rf.addrcfgall()
  rf.prtrcfg()
if __name__ == "__main__":
  main()
