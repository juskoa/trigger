#!/usr/bin/python
import sys,string
def bu2bc(beam,buc):
  shifts=(345, 3018)
  bc= ((buc+10)/10 + shifts[beam-1])%3564
  return bc
def bu2bcfile(fn):
  f= open(fn,"r")
  for line in f.readlines():
    if line[0] == "": continue
    if line[0] == "\n": continue
    if line[0] == "#": continue
    ll= string.split(line)
    beam= int(ll[0]) ; buc= int(ll[1])
    bc=bu2bc(beam, buc)
    print beam,bc
  f.close()
def main():
  if len(sys.argv)<2:
    print """Usage:
buc2bc.py beam bucket
where:
beam: 1(blue,A) or 2(red,C)
bucket: 1..35640
or buc2bc.py file_name
"""
    sys.exit()
  if len(sys.argv)<3:
    bu2bcfile(sys.argv[1])
  else:   
    beam= int(sys.argv[1])
    buc= int(sys.argv[2])
    bc=bu2bc(beam, buc)
    print "%d -> %d (beam:%d)"%(buc, bc, beam)

if __name__ == "__main__":
    main()

