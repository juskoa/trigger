#!/usr/bin/python
import sys,string
def dif32(first, second):
  #f= eval('0x'+ first) ; s= eval('0x'+ second)   # hexa
  f= eval(first) ; s= eval(second)   # dec
  if s>=f:
    dif= s-f
  else:
    dif= s + (0xffffffff-f) + 1
  #print second ,' - ', first, ' = ', dif
  return dif
def proc_cgtime(fn):
  """ print rates between 2 consecutive counters readings. 
  fn: data file with hexa/dec numbers
  """
  f= open(fn,"r")
  first= True ; sum={}
  for line in f.readlines():
    if line[0]=='#': continue
    lsplit= string.split(line)
    # date time cg runx1..5 l0time count1 count2
    # 0    1    2  3 4 5 67 8      9
    clg= lsplit[2]
    ct= [lsplit[9], lsplit[8]]   # i.e. (count,l0time)  -l0time in 0.4secs
    if first:
      prevct= ct; first= False; continue
    dif= map(dif32, prevct, ct)
    #print prevct, ct, dif
    prevct= ct
    if sum.has_key(clg):
      sum[clg]= sum[clg] + int(dif[1])   # total time
    else:
      sum[clg]= int(dif[1])   # total time
    rate= dif[0]*1000000/(dif[1]*0.4)
    #rate40= dif[0]/(dif[1]*0.4/40)
    #print lsplit[4], rate40
    print "cg:", lsplit[2], " t:", dif[1]*0.4/1000000, " count:", dif[0],\
      rate, "hz"
  f.close()
  for clg in sum.keys():
    print "clg:%s total active time [s]:%10.3f"%(clg, sum[clg]*0.4/1000000.)
def proc_12(fn):
  """ print rates between 2 consecutive counters readings. 
  fn: data file with hexa/dec numbers
  """
  f= open(fn,"r")
  first= True
  for line in f.readlines():
    if line[0]=='#': continue
    lsplit= string.split(line)
    # date time cg runx1..5 l0time count1 count2
    #ct= lsplit[2:4]   # i.e. (count, l0time)  -l0time in 0.4secs
    # date time count l0time
    ct= [lsplit[9], lsplit[8]]   # i.e. (count,l0time)  -l0time in 0.4secs
    if first:
      prevct= ct; first= False; continue
    dif= map(dif32, prevct, ct)
    #print prevct, ct, dif
    prevct= ct
    rate= dif[0]/(dif[1]*0.4)
    #rate40= dif[0]/(dif[1]*0.4/40)
    #print lsplit[4], rate40
    print "cg:", lsplit[2], " t:", dif[1]*0.4/1000000, " count:", dif[0],\
      rate
  f.close()
def proc_example(fn):
  f= open(fn,"r")
  for line in f.readlines():
    print string.split(line)[2]
  f.close()
def main():
  if len(sys.argv) < 2:
    print """
1. check/modify proc_12()
2. give the name of data file obtained through linux/analyse 
"""
    sys.exit(0)
  fn= sys.argv[1];
  #proc_example(fn)
  #proc_12(fn)
  proc_cgtime(fn)
if __name__ == "__main__":
    main()
