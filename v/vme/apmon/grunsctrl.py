#!/bin/env python

import sys,time,string,types,pydim,random
import redis

def redis_test():
  r= redis.StrictRedis(host='adls', password="nasedatasutu.")
  #r.hset("gruns_dets","100","0x41")
  grs= r.hgetall("gruns_dets")
  print grs
  # disconnect how ?
  #r.exists("globalruns")
  #r.hkeys("gruns_dets")
  #r.hdel("gruns_dets","100000")
  # hset("run1", key, value)
  # hmset("run1", dict)
  #r.delete("globalruns")
class GRUNS:
  ALLDETS= 0x2dffff
  HMPIDPHOS= (1<<6) | (1<<7)   # HMP: 0x40  PHS: 0x80
  def __init__(self):
    self.rs= {}
    self.nextrun= 10
  def getnewrn(self):
    n= self.nextrun
    self.nextrun= self.nextrun+1
    return n
  def addrun(self, runi="", oneonly=False):
    if len(self.rs)>=6:
      print "6 active runs already:", self.rs
      return
    dets= 0
    for run in self.rs.keys():
      cdets= self.rs[run]
      if cdets & dets:
        print "Error: run %d dets:%x"%(run, cdets)
        return
      dets= dets | self.rs[run]
    # dets: detectors in all runs
    if dets==GRUNS.ALLDETS:
      print "No free detectors, i.e. no new run"
      return
    freedets= ~dets & GRUNS.ALLDETS
    freedets2= 0
    # just take ~half of free dets in new run:
    for idet in range(22):
      if idet==17: continue   # trg
      if idet==20: continue   # not assigned
      lastdet= 1<<idet
      if lastdet & freedets:
        #lastdet is free
        if (random.randint(0,1)) or (lastdet & self.HMPIDPHOS) or oneonly:
          # assign it if PHOS HMPID
          freedets2= freedets2 | lastdet
          lastdet= 0   # assigned
        if oneonly: break
      else:
        lastdet= 0
    if (freedets2==0) and (lastdet!=0): 
      # no det choosen (random), but there is at least 1 free det
      freedets2= lastdet   # the last free one
    if runi != "":
      newrn= int(runi)
    else:
      newrn= self.getnewrn()
    self.rs[newrn]= freedets2
    tim= int(time.time())
    dimmsg= "s %d %d %6x"%(tim, newrn, freedets2)
    print dimmsg
    self.updatedims(dimmsg)
  def delrun(self):
    if len(self.rs)==0:
      print "No active run"
      return
    rnix= random.randint(0,len(self.rs)-1)
    ix=0 ; dimmsg= None
    for rn in self.rs.keys():  
      if ix == rnix: 
        del self.rs[rn]
        tim= int(time.time())
        dimmsg= "c %d %d"%(tim, rn)
        print "Run %d deleted. dimmsg:"%rn, dimmsg
        break
      ix= ix+1
    if dimmsg:
      self.updatedims(dimmsg)
    else:
      print "Error dimmsg empty. rnix:%d, lenrs:%d"%(rnix, len(self.rs))
  def delall(self):
    for rn in self.rs.keys():  
      del self.rs[rn]
    tim= int(time.time())
    dimmsg= "c %d 0"%tim
    print "All runs deleted. dimmsg:", dimmsg
    self.updatedims(dimmsg)
  def prtall(self):
    print "Runs:", self.rs.keys(), map(hex,self.rs.values())
  def updatedims(self, msg):
    pydim.dic_cmnd_service("CTPRCFG/RCFG", ("grunsup "+msg+"\x00",),"C")
def automat(grs, secs, reps):
  a= grs.addrun
  d= grs.delrun
  D= grs.delall
  opts=[a, d, a, a, a,d,D]
  ix= 0
  D()   # start clean
  while True:
    act= random.randint(0,len(opts)-1)
    opts[act]()
    ix= ix+1
    if ix==reps: break
    time.sleep(secs)
def main():
  if not pydim.dis_get_dns_node():
    print "No DIM_DNS_node found env. variable defined"
    sys.exit(1)
  print "dns:",pydim.dis_get_dns_node()
  grs= GRUNS()
  while True:
    line= raw_input("a[dd] [runn] [t] d[elete]  D[elete all] Log Nolog r[andom automat] R[edis_test] p[rint]q[uit]>")
    if len(line)<1:break
    c= line[0]
    spl= string.split(line)
    arg1= "" ; arg2= False
    if len(spl)>=2: arg1= spl[1]
    if len(spl)>=3: arg2= True
    if c=='q': break
    if c=='a': grs.addrun(arg1, arg2)
    if c=='d': grs.delrun()
    if c=='D': grs.delall()
    if c=='p': grs.prtall()
    if c=='R': redis_test()
    if c=='L':
      grs.updatedims("L")
      print "Logging ON: v/vme/WORK/apmon4.log"
    if c=='N':
      grs.updatedims("N")
      print "Logging OFF: v/vme/WORK/apmon4.log"
    if c=='r':
      print " starting automatic random s/c..."
      automat(grs, 30, 20000)

if __name__ == "__main__":
    main()

