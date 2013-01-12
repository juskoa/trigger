#!/usr/bin/python
import sys
#ORBITL=3564
nlstart=""

def beambucs(bucsfn, of):
  global nlstart
  if bucsfn=="b1":
    beam='A'
  else:
    beam='C'
  nlines=0
  infi= open(bucsfn, "r")
  for line in infi.readlines():
    if line[0] == "\n": continue
    of.write(nlstart+"%s %s"%(beam, line))
    nlines= nlines+1
    if line[-1]!="\n": 
      nlstart="\n"
    else:
      nlstart=""
  print "%s: %d"%(beam, nlines)
  infi.close()
  
def main():
  if len(sys.argv)==1:
    print """dodip.py fsname
expects 2 files in workng directory: b1 b2
"""
    sys.exit(4)
  dipf= open(sys.argv[1]+".dip", "w")
  beambucs("b1", dipf)
  beambucs("b2", dipf)
  dipf.close()
if __name__ == "__main__":
    main()
