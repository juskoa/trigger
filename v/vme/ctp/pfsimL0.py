#!/usr/bin/python
import os,string
ORBITL=3564
MAXBC=40000000
class LIP:
  def __init__(self, lip):
    self.l0,self.ir,self.pf= lip[0],lip[1],lip[2]
    self.l0bc_first=1 ; self.ir_first=1
  def overlaps(self):
    printed=0; n=0; self.pfhit=0
    while 1:
      l0bc= n*(self.l0+1)+self.l0bc_first
      if l0bc>MAXBC: break
      rc=self.checkpf(l0bc)
      if rc>0 and printed<20:
        # +(self.l0+1): becasue 1st IR and L0 are in the same BC
        print("pf at bc:%d IR at:%d"%(l0bc, rc))
        printed= printed+1
      n= n+1
    ratio= 100.0*self.pfhit/n
    secs= MAXBC/40000000.0
    print("%6.2fs simulated. %8.4f%% killed:%d out of %d L0s:"%\
     (secs,ratio,self.pfhit,n))
  def checkpf(self, l0bc):
    """ kill +-self.pf BCs around l0bc. l0bc: 1..oo
    rc: bc of IR killing this or 0 if not killing
    """
    irs=0
    # nearest lower IR:
    # killing current triger:
    if ((l0bc-self.ir_first) % (self.ir+1))==0:   # simple (i.e. L0==IR
      self.pfhit= self.pfhit+1
      irs= l0bc
      return irs
    for ipf in range(1,self.pf+1):
      # only theory (cannot kill future):
      #if (((l0bc - self.ir_first + ipf) % (self.ir+1))==0):
      #  irs= l0bc+ipf
      # killing past:
      if (((l0bc - self.ir_first - ipf) % (self.ir+1))==0):
        irs= l0bc-ipf
      if irs !=0:
        self.pfhit= self.pfhit+1
        return irs
    return irs

def main():
  print("""
L0 BC: 1..3564 (number in orbit).
L0 IR PF (BCs between L0 or IR, PF: +-BCs)

When PF entered here is n-1 (n == bcs in WritePFuser), 
the calculated killed/allL0s ratio is very close to the real one 
measured with L0board counters. Why?
""")
  while 1:
    line=raw_input(">")
    if line[0]=='q':break
    print ">%s<"%line
    lip= map(int,string.split(line))
    print "L0 IR PF:", lip
    case1= LIP(lip)
    case1.overlaps()
if __name__ == "__main__":
    main()

