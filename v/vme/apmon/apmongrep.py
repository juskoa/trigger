#!/usr/bin/python
import sys,string,time
def getbt(lin):
  sest= "busyTime:"
  ix=lin.find(sest)
  if ix>0:
    #bt= int(lin[ix+ len(sest)]) 
    bsti= string.split(lin[ix:],':')
    bt= int(bsti[2])
  else:
    bt= None
  return bt
def getlimit(lin):
  sest= "DET("
  ix=lin.find(sest)
  if ix>0:
    #blstr= lin[ix:].split(" ")[0]
    det= lin[ix+4:ix+7]
  else:
    det="???"
  #print det
  if det == "CPV": return 480
  sest= "busyLimit:"
  ix=lin.find(sest)
  if ix>0:
    #bt= int(lin[ix+ len(sest)]) 
    blstr= lin[ix:].split(" ")[0]
    bsti= string.split(blstr,':')
    bt= int(bsti[2])
  else:
    bt= None
  return bt
def grfile(fn):
  lsf= open(fn); ixl= 0
  print "file:",fn
  for lin in lsf.readlines():
    lin= lin.strip()
    ixl= ixl+1
    #print ixl,":"
    #if ixl>50: break
    bt= getbt(lin)
    busylimit= getlimit(lin)
    if bt>busylimit:
      print lin.strip()
    #pl= string.split(lin)
    #
  lsf.close()
def main(argv):
  if len(argv) < 2:
    print """
  apmongrep.py file
   
"""
    return
  grfile(argv[1])

if __name__ == "__main__":
  main(sys.argv)
