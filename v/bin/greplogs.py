#!/usr/bin/env python
import string,os.path,trigdb

def searchltu(str):
  ltus= trigdb.TrgLTUS()
  for ltu in ltus.ltus:
    #lognp= ltu.getLogName26()
    lognp="/data/ClientLocalRootFs/%s/home/alice/trigger/v/%s/WORK/LTU-%s.log"%\
      (ltu.vmecpu, ltu.name.lower(), ltu.name)
    if not os.path.exists(lognp):
      #print "LTU-%s.log does not exist:"%ltu.name
      continue
    print lognp
    delay="?"; f= open(lognp,"r")
    while True:
      line= f.readline()
      if line=='':
        print "eof"
        break
      line= line.strip()
      #print line,":"
      if line[:13]=="BC_DELAY_ADD:": delay=line[13:15]
      if line[:11]=="FineDelay1:": break
    print line, "BC_DEALY_ADD:",delay
def main():
  import sys
  if len(sys.argv) < 2:
    print """
greplogs.py string
"""
    return
  str=sys.argv[1]
  searchltu(str)  

if __name__ == "__main__":
  main()
