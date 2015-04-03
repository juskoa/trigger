#!/usr/bin/python
import sys,string
orbitl=3564
def c_sync(fdf):
  n= 100./fdf
  hwval= int(round((100-n)*0x1fffff/100))
  print " request           in class definition"
  print "%7.7fx           L0pr=%f%%   or %d=0x%x"%(fdf, n, hwval, hwval)
def r_sync(fdf):
  n= int(0x7fffffff/fdf)
  realdf= 0x7fffffff/float(n)
  print " request   n     real downscaling"
  print "%7.2fx   %d =      %7.2f"%(fdf, n, realdf)
def b_sync(fdf):
  df= int(fdf)
  ndf= df%orbitl   # normalized (0..3563)
  wn= df/orbitl    # integer part
  # 3564= 3**4 * 2*2 * 11
  upper= bottom= ndf
  print " request   n     real downscaling"
  points=0 ; bottom= bottom-1
  while (points<1 and bottom>1):
    if (bottom%2 != 0) and (bottom%3 != 0) and (bottom%11 != 0):
      upperx= bottom + wn*orbitl
      print "%7.2fx   %d =      %7.2f"%(fdf, upperx-1, upperx)
      points= points+1
    bottom= bottom-1
  points=0
  while (points<2):
    if (upper%2 != 0) and (upper%3 != 0) and (upper%11 != 0):
      upperx= upper + wn*orbitl
      print "%7.2fx   %d =      %7.2f"%(df, upperx-1, upperx)
      points= points+1
    upper= upper+1
def reverse(dsf):
  hwbits= eval(dsf)
  if hwbits==0x1fffff:
    fperc=0 ; ntxt="oo x"
  else:
    fperc= 100.-100.*hwbits/0x1fffff
    ntxt= ("%.8g x")%(100./fperc)
  print "dsf:%d = 0x%x   L0pr=%.8f%%   n:%s"%(hwbits,hwbits, fperc,ntxt)
def printhelp():
  print """
b factor       BC (_B) synchronous downscaling using BC1/2 generator
r factor       random (_R) synchronous downscaling using RND1/2 generator
c factor       class random downscaling (not in sync with other class)
x dsf          reverse calculation from class downscale hw-value -> factor
q              ->to quit
where:
factor  =how many times. E.g. 20 means 'downscale 20 times' or to 5%
dsf     =21 bits downscaling value (hex or dec) encoded in class register
"""

def main():
  printhelp()
  while 1:
    line=raw_input(":")
    if line[0]=='q':break
    linest= string.split(line)
    if linest[0]=='b':
      b_sync(float(linest[1]))
    elif linest[0]=='r':
      r_sync(float(linest[1]))
    elif linest[0]=='c':
      c_sync(float(linest[1]))
    elif linest[0]=='x':
      reverse(linest[1])
    else:
      printhelp()
if __name__ == "__main__":
    main()

