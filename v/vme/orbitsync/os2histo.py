#!/usr/bin/python
import string
def main():
  maxdif= 0x5f49cb
  hist={}
  f= open("os2.sorted","r")
  for line in f.readlines():
    if line[0] == "C": continue
    if line[0] == "M": continue
    if line[0] == "\n": continue
    if line[0] == "#": continue
    s= string.split(line[:-1])
    us1= eval(s[2])
    odif= eval(s[1])
    us= (maxdif - odif)*89.1 + us1
    if hist.has_key(us): 
      hist[us]= hist[us]+1
    else:
      hist[us]= 1
  allus= hist.keys(); allus.sort()
  for us in allus:
    print "%f %d"%(us, hist[us])

if __name__ == "__main__":
    main()

