#!/usr/bin/python
"""
"""
from Tkinter import *
import myw,parted

class doMainWin:
  def __init__(self,master):
    self.master= master
    self.frame= myw.MywFrame(master)   # load/save buttons frame

def main():
  f= Tk()
  doMainWin(f)
  f.mainloop()

if __name__ == "__main__":
    main()

