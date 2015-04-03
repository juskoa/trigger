#!/usr/bin/python
from Tkinter import *
#import os, sys,string
import myw

class VmeReg:
 def prtbut():
  print 'prtbut'
 def main():
  f=Tk()
  f.title("try mult regs")
  f1= myw.MywEntry(f, label="reg1", defvalue=' ', width=8,
    helptext='abc\ndef')
  f2= myw.MywButton(f, label='read',cmd=prtbut)
  f3= myw.MywMenu81632(f, items=( ('a1','v1'),('a2','v2') ))
  f4= myw.MywxMenu(f, items=( ('aa1','v1'),('aa2','v2') ))
  f.mainloop()
  
if __name__ == "__main__":
    main()

