#!/usr/bin/env python
from tkinter import *
import myw

def fdestroy(event):
  print("fdestroy:",event)
def checklabel(arg1=None):
  print("here checklabel. arg1:", arg1)
def main():
  f = Tk()
  f.bind("<Destroy>", fdestroy)
  f.title("Myw examples")
  entriesFrame=myw.MywFrame(f, side=LEFT); 
  entriesFrame.bind("<Destroy>", myw.efdestroy)
  entriesFrame.config(bg="blue")
  lb= myw.MywEntry(entriesFrame,label="MywEntry with help/cmd:",
    side=TOP, defvalue='abc1',cmdlabel=checklabel,helptext="abc\n\
  2 riadok\ntreti\n\
  stvrty\n\
piaty")
  f.mainloop()

if __name__ == "__main__":
    main()

