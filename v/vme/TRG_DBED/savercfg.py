#!/usr/bin/python
import subprocess, os, os.path, string, sys, time, parted
def main():
  if hasattr(sys,'version_info'):
    if sys.version_info >= (2, 3):
      # sick off the new hex() warnings, and no time to digest what the
      # impact will be!
      import warnings
      warnings.filterwarnings("ignore", category=FutureWarning, append=1)
  #dimserver= os.popen("./dimserver.exe","r")
  inthepit=False
  if os.environ.has_key('CLRFS'):
    if os.environ['CLRFS'] == '/data/ClientLocalRootFs': 
      inthepit=True
        #make copy in $CLRFS/alidcsvme001/home/alie/trigger/v/vme/WORK/RCFG/
  pitsrc= os.path.join( os.environ['VMEWORKDIR'],"WORK","RCFG")
  pitdes= os.path.join( os.environ['CLRFS'],\
    "alidcsvme001/home/alice/trigger/v/vme/WORK/RCFG")
  f= open(fname,"r"); lines= f.readlines(); f.close()

  line=readline(

