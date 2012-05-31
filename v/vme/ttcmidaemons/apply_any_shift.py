#!/usr/bin/python
import miclock,pylog
miclock.mylog= pylog.Pylog("apply_any_shift","ttyYES")

def main():
  cshift= miclock.getShift()
  if cshift != "old":
    miclock.checkandsave(cshift,"fineyes", force='yes')
    miclock.mylog.logm("current shift measured:"+cshift+" saved in db+corde")
  else:
    miclock.mylog.logm("current shift measured:"+cshift)

if __name__=="__main__":
  main()

