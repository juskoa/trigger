#!/usr/bin/python
import miclock,pylog,sys,types
miclock.mylog= pylog.Pylog("apply_any_shift","ttyYES")

def main():
  if len(sys.argv)==1:
    print """
Usage examples:
./apply_any_shift.py -0.3
   - shift clock by -0.3ns    (parameter has to be float number with decimal point)
./apply_any_shift.py get
   - get a current shift of the clock from PHASE_SHIFT_BPTX1 DIM service
     and apply it. If measurement is too old (> 3 minutes), clock is not shifted
"""
  elif sys.argv[1]=='get':
    cshift= miclock.getShift()
    if cshift != "old":
      miclock.checkandsave(cshift,"fineyes", force='yes')
      miclock.mylog.logm("current shift measured:"+cshift+" saved in db+corde")
    else:
      miclock.mylog.logm("current shift not changed (old, not saved in db)")
  elif type(eval(sys.argv[1])) == types.FloatType:
    cshift= sys.argv[1]
    miclock.checkandsave(cshift,"fineyes", force='yes')
    miclock.mylog.logm("shift:"+cshift+" saved in db + corde")
  else:
    print "bad argument:", sys.argv[1]

if __name__=="__main__":
  main()

