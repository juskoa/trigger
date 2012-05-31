#!/usr/bin/python
"""
"""
from Tkinter import *
import myw,parted

class doMainWin:
  def __init__(self,master):
    self.master= master
    self.actparts=[]   # list of active partitions (instances)
    self.frparts= myw.MywFrame(master) 
    myw.MywLabel(self.frparts,"Active partitions")
    # probably, button displaying Toplevel window allowing
    # - the 'examination' of the partition to be added 
    # - the display of resulting configuration before its load into CTP
    # is a good choice
    myw.MywHButtons(self.frparts,[["Activate",self.activate],
      ["Deactivate", self.deactivate]],
      helptext="""
Make a partition active or remove
it from the list of active partitions
""")
    #self.frresources= myw.MywFrame(master) 
    #self.frtrigger= myw.MywFrame(master) 
  def activate(self):
    # find out all (in database) and strip active ones:
    pns=[]
    for x in parted.getNames():
      act=0
      for ax in self.actparts:
        if x==ax.name:act=1; break
      if act==0: pns.append(x)
    if len(pns)>0:
      print 'pns:',pns,'appending:',pns[0]
      self.actparts.append(parted.TrgPartition(pns[0]))
      self.actparts[-1].show(self.frparts)
    else:
      print 'empty pns, nothing activated'

  def deactivate(self):
    if len(self.actparts)>0:
      print "Deactivating:"
      self.actparts[0].prt()
      self.actparts[0].hide()
      del(self.actparts[0])
    else:
      print 'no active partititions, nothing deactivated'
def main():
  f= Tk()
  doMainWin(f)
  f.mainloop()

if __name__ == "__main__":
    main()

