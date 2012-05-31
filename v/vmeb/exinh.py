#!/usr/bin/python
from Tkinter import *
import string
import myw, cmdlint

class labeledMenu(myw.MywFrame, myw.MywMenu, myw.MywEntry):
  def __init__(self,vb, items=()):
    myw.MywFrame.__init__(self,vb)
    #myw.MywEntry.__init__(self, myw.MywFrame.Frame, label="Counting:",
    #myw.MywEntry.__init__(self, myw.MywFrame, label="Counting:",
    #myw.MywEntry.__init__(self, vb, label="Counting:",
    myw.MywEntry.__init__(self, Frame, label="Counting:",
      helptext="hlptx", defvalue='',side='left')
    cur=1
    myw.MywMenu.__init__(self, vb, label="",
      helptext="hlptexxt", side= 'right',
      items=(("events","0"),("orbits","1")),
      defaultinx=cur)
  def getEntry(self):
    #print 'labeledMenu:',self.MywMenu.getEntry()
    #print 'labeledMenu:',self.myw.MywMenu.getEntry()
    print 'labeledMenu:',myw.MywMenu.getEntry(self)
 
def main():
  f=Tk()
  labmen=labeledMenu(f)
  #geb= myw.MywButton(f, text='print',cmd=labmen.MywMenu.getEntry)
  geb= myw.MywButton(f, text='print',cmd=labmen.getEntry)
  f.mainloop()
if __name__ == "__main__":
    main()
