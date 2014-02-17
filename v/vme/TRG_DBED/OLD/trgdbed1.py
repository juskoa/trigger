#!/usr/bin/python
"""
obsolete (see trgdbed -which load everything from files)
"""
from Tkinter import *
import sys
sys.path.append("/home/aj/ALICE/epmp2/backup/v/vmeb")
import myw
class TrgInput:
  def __init__(self, name):
    self.name=name
class TrgLTU:
  def __init__(self, name):
    self.name=name
  def show(self,master):
    ltubut= myw.MywButton(master, label=self.name, side= RIGHT)
class TrgDescriptor:
  def __init__(self, name, inp=[]):
    self.name= name
    self.inputs= inp
  def show(self,master):
    tdbut= myw.MywButton(master, label=self.name, 
      side=RIGHT, cmd=self.prtInputs)
  def prtInputs(self):
    inps=""
    for inp in self.inputs:
      inps= inps+" "+inp.name
    print self.name,":",inps
class TrgCluster:
  def __init__(self, tds=[], outdets=[]):
    self.tds= tds
    self.outdets= outdets
  def show(self,master):
    clfr= myw.MywFrame(master)
    clusterLabel= myw.MywLabel(clfr,label="Cluster:")
    tdsfr= myw.MywFrame(clfr)
    outfr= myw.MywFrame(clfr)
    tdsl= myw.MywLabel(tdsfr,label=" TDS:", side=LEFT)
    for td in self.tds:
      td.show(tdsfr)
    outl= myw.MywLabel(outfr,label="LTUS:", side=LEFT)
    for outd in self.outdets:
      outd.show(outfr)
class TrgPartition:
  def __init__(self, name, clusters=[]):
    self.name= name
    self.clusters= clusters
    self.status="stopped"
    self.plabel=None   # i.e. 'not shown'
  def show(self,master):
    self.pfr= myw.MywFrame(master)
    self.plabel= myw.MywxMenu(self.pfr,label="Partition: "+self.name,
      items=[["start","start",self.cmd],
        ["stop","stop",self.cmd]],
      side=TOP, defaultinx=0)
    #self.plabel.allowed([1,0])
    pclfr= myw.MywFrame(self.pfr)
    for cl in self.clusters:
      cl.show(pclfr)
  def cmd(self):
    actinx= self.plabel.getIndex()
    print "actinx:",actinx

def main():
  """
  Trigger input names:
  L0:  T0, TRDpre, V0mb,...
  L1: ZDC1_l1  (i.e. _l1 suffix for all)
  """
  T0=TrgInput("T0")
  V0mb= TrgInput("V0mb")
  V0sc= TrgInput("V0sc")
  V0ce= TrgInput("V0ce")
  TRDpre= TrgInput("TRDpre")
  ZDC1_l1= TrgInput("ZDC1_l1")
  ZDC2_l1= TrgInput("ZDC2_l1")
  ZDC3_l1= TrgInput("ZDC3_l1")
  #
  HMPID= TrgLTU("HMPID")
  TPC= TrgLTU("TPC")
  TRD= TrgLTU("TRD")
  PIXEL= TrgLTU("PIXEL")
  #
  mb= TrgDescriptor("mb", [T0, V0mb, TRDpre, ZDC1_l1])
  sc= TrgDescriptor("sc", [T0, V0sc, TRDpre, ZDC2_l1])
  ce= TrgDescriptor("ce", [T0, V0ce, TRDpre, ZDC3_l1])
  #
  cl1= TrgCluster(tds=[mb,sc], outdets=[HMPID,TPC])
  cl2= TrgCluster(tds=[sc], outdets=[PIXEL])
  cl3= TrgCluster(tds=[ce], outdets=[TRD])
  P1= TrgPartition("P1",clusters=[cl1, cl2])
  P2= TrgPartition("P2",clusters=[cl3])
  f= Tk()
  P1.show(f)
  P2.show(f)
  f.mainloop()
  
if __name__ == "__main__":
    main()

