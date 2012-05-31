#!/usr/bin/python
"""
9.3. ctp.py archived in ctp1.py
- Activate/Deactivate buttons replaced by MywMenuList button
- CTP class added
28.7. parted now initialised in main()
 2. 4.2006 -1 partition can be loaded only ONCE
"""
from Tkinter import *
import myw,parted

def GetResources(parts):
  """
  Go through the list of partitions in parts.
  RC:tuple of:
     -the list of allocated resources (total)
     -string form of the all. resources
  """
  allrs=[0,0,0,0]
  allrsrefs=[[],[],[],[]]
  print "partitions:",parts
  for par in parts:
    prs=par.getRR()
    #print par.name   #,':',prs
    for i in range(len(allrs)):
      for res1 in prs[i]:
        if parted.findRR(allrsrefs[i], res1): continue
        allrsrefs[i].append(res1)
        allrs[i]=allrs[i]+1
    print "all resources:", par.name,allrs
  txt="Classes:%d P/Fs:%d \n Clusters:%d LTUs:%d"%(allrs[0],
    allrs[1],allrs[2],allrs[3])
  return allrs,txt
class CTP:
  def __init__(self):
    self.maxresources=[50,6,4,24]   #simplified (will be more complex) 
    self.partitions=[]
    allr,self.rsrcsAss=GetResources(self.partitions)
  def load(self, partitions):
    if self.checkResources(partitions):
      self.partitions=[]
      pns2load=''
      for ix in range(len(partitions)):
        pns2load= pns2load+partitions[ix].name+' '
        #self.assignResources(partitions[ix])
        self.partitions.append(partitions[ix]) 
      allr,self.rsrcsAss=GetResources(self.partitions)
      print "loaded:", pns2load
    else:
      print "CTP configuration was not reloaded"
  def checkResources(self, ps):
    allrs,allrstxt=GetResources(ps)
    rc=1
    for i in range(len(self.maxresources)):
      if allrs[i]>self.maxresources[i]:
        rc=0
    return rc
  def assignResources(self, p):
    pass
ctp=CTP()

class DoMainWin:
  def __init__(self,master):
    self.master= master
    self.actparts=[]   # list of active partitions (instances)
    self.frparts= myw.MywFrame(master)   # active part. frame
    myw.MywLabel(self.frparts,"Active partitions")
    # probably, button displaying Toplevel window allowing
    # - the 'examination' of the partition to be added 
    # - the display of resulting configuration before its load into CTP
    # is a good choice
    #
    self.frresources= myw.MywFrame(master) 
    self.frtrigger= myw.MywFrame(master) 
    self.pns=[]
    for x in parted.getNames():
      self.pns.append(x)
    self.adpb=myw.MywMenuList(self.frresources,
      label="Add/Remove \npartition",side=LEFT,
      cmd=self.modap, items=self.pns, helptext="""
Make a partition active or remove
it from the list of active partitions
""")
    rs,rstxt=GetResources(self.actparts)
    self.rsrcsl=myw.MywLabel(self.frresources,rstxt, 
      bg=parted.COLOR_OK, side=LEFT,helptext=
""" CTP resources necessary for loading shown partitions
""")
    myw.MywButton(self.frtrigger, label="Load CTP",side=LEFT,
      cmd=self.loadctp,helptext="""
Load current 'active partitions' into CTP.
Operation:
- create ctp.cfg file
- load it into CTP
""")
    self.frl=myw.MywLabel(self.frtrigger,ctp.rsrcsAss, 
      bg=parted.COLOR_WARN, side=LEFT,helptext=
""" Allocated CTP resources. This field becomes green, if 
loaded configuration is equivalent to the shown one
""")
  def loadctp(self):
    ctp.load(self.actparts)
    self.frl.configure(text=ctp.rsrcsAss)
    self.frl.configure(bg=parted.COLOR_OK)
  def modap(self, mli, ix):
    print "modap:", ix,':',mli.getEntry(ix)
    rc=0
    if mli.getEntry(ix):
      rc=self.activate(ix)
    else:
      rc=self.deactivate(ix)
    rs,rstxt=GetResources(self.actparts)
    self.rsrcsl.configure(text=rstxt)
    if rc:
      self.frl.configure(bg=parted.COLOR_WARN)
  def activate(self,ix):
    newpartition=parted.TrgPartition(self.pns[ix])
    self.actparts.append(newpartition)
    if ctp.checkResources(self.actparts):
      #print 'adding:',self.pns[ix]
      self.actparts[-1].show(self.frparts,name='yes')
      rc=1
    else:
      print 'Not enough resources to add:',self.pns[ix]
      del(self.actparts[-1])
      #self.updateADPB()
      self.adpb.setEntry(ix, 0)   # reset button back to 'not active'
      rc=0
    return rc
  def deactivate(self, ix):
    rc=0
    #print "deactivate:ix",ix, self.pns
    #print "deactivate:actpars", self.actparts
    for ixa in range(len(self.actparts)):
      if self.actparts[ixa].name==self.pns[ix]:
        #print "Removing:",self.pns[ix]
        #self.actparts[ix].prt()
        self.actparts[ixa].hide()
        del(self.actparts[ixa])
        rc=1
        break     #break -partition is unique (can't be loaded 2x)
    if rc==0:
      print self.pns[ix],'not found in active partititions'
    return rc

def main():
  parted.TDLTUS.initTDS()
  f= Tk()
  DoMainWin(f)
  f.mainloop()

if __name__ == "__main__":
    main()

