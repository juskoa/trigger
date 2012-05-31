#!/usr/bin/python
import pylog
import pydim
mylog= pylog.Pylog("dll_resync","ttyYES")

def main():
  arg=("none",)
  res= pydim.dic_cmnd_service("TTCMI/DLL_RESYNC", arg, "C")
  mylog.logm("dll_resync")

if __name__=="__main__":
  main()


