#!/usr/bin/python
import os,os.path,sys,string,time

def main(argv):
  if len(argv) < 2:
    print """
  hx2d.py filename
"""
    return
  fn=argv[1]
  f= open(fn,"r")
  for line in f.readlines():
    if line[0]=='#': 
      print line[:-1]
      continue
    ar= string.split(line)
    oline= ar[0]+' '+ar[1]
    for hx in ar[2:]:
      oline= oline+' '+"%10d"%(eval("0x"+hx))
    print oline
if __name__ == "__main__":
    main(sys.argv)
