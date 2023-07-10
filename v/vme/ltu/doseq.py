#!/usr/bin/env python
"""doseq.py
Prepare special .seq files
"""
from __future__ import print_function
#import os.path, os, string, sys, glob
from builtins import str
from builtins import range
from builtins import object
import os.path, os
#SNAME=['ZERO', 'L0  ', 'L2A ', 'L2R ', 'CPP ', 'CL0 ', 'CL2A', 'CL2R']
class dofile(object):
  def __init__(self, slmfile):
    #if os.access(slmfile, os.R_OK) == 0:
    #  self.error(slmfile+" doesn't exist")
    #  return
    #sf= open(slmfile, 'w')
    beg= """generated
1
0000000
0000000000010110"""
    print(beg)
    l=1
    for i in range(255):
      lin=''
      for i in range(7,-1,-1):
        #c= "%c"%((l&(1<<i))>>i)
        c= (l&(1<<i))>>i
        #print ":",c
        cs= str(c)
        #c= "%c"%((l&(1<<i))>>i)
        lin= lin+cs
      print(lin+"00000000") 
      #print "%s00000000"%(lin)
      l=l+1
class hex2seq(object):
  def __init__(self):
    pass
def main():
  import sys
  if len(sys.argv) < 2:
    print("""
Cretae on stdout .seq file:
Usage: doseq.py spec|input
       spec  -special file (looking for problem with SLM)
       input -take input in as hexa, 16 bytes per line
""")
    #a= disslm("all.slm")
    #print a.getlist()
  else:
    if sys.argv[1]=='spec':
      a= dofile(sys.argv[1])
    if sys.argv[1]=='input':
      a= hex2seq()

if __name__ == "__main__":
  main()

