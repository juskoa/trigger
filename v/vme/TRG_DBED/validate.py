#!/usr/bin/python
"""
Usage: validate.py partname
Stdout: list of input detectors (1 line)
        ' '\n -no input detectors
        'Errors:\n... -error message (integrity of files broken)
RC: 0: ok
    8: error message printed to stdout
Output files:
/tmp/validate.log
/tmp/partname.pcfg 
"""
import os.path, sys
WORKDIR= "/tmp"   # "./"
def prterr(errlines):
  #print os.path.join( WORKDIR,partname+".pcfg")
  #f= open( os.path.join( WORKDIR,partname+".pcfg"), "w")
  #f.write("Errors:\n") ; f.write(errlines)  ; f.close()
  print "Errors:"
  print errlines
  sys.exit(8)
def main():
  if hasattr(sys,'version_info'):
    if sys.version_info >= (2, 3):
      # sick off the new hex() warnings, and no time to digest what the
      # impact will be!
      import warnings
      warnings.filterwarnings("ignore", category=FutureWarning, append=1)
  #reload(parted)
  if len(sys.argv)<2:
    prterr("1 parameter missing: partition name")
  saveout= sys.stdout
  logfile= open(os.path.join( WORKDIR,"validate.log"), "w")
  sys.stdout= logfile
  import parted
  partname= sys.argv[1]   #"beam" "erp"
  part= parted.TrgPartition(partname)
  part.prt()
  if part.loaderrors=="":
    errs= part.savepcfg(wdir=WORKDIR)   # without 'rcfg '
    sys.stdout= saveout ; logfile.close()
    if errs!="":
      prterr(errs)
    else:
      part.prtInputDetectors()
  else:
    sys.stdout= saveout ; logfile.close()
    prterr(part.loaderrors)
 
if __name__ == "__main__":
    main()

