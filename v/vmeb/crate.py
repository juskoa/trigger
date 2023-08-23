#!/usr/bin/env python
#standard_library.install_aliases()
#from builtins import str
from tkinter import *
import os, sys, traceback
import myw

def DimServerOn(basead):
  """
[altri1] /home/alice/aj/v/vmeb > ps -C ltuserver o user,args
USER     COMMAND
aj       /home/alice/aj/v/vme/ltudim/linux/ltuserver hmpid 0x811000
  """
  if basead=='': basead="0x810000"
  lsf= os.popen("ps -C ltu_proxy o user,args","r")
  #ls=lsf.readlines()
  #print ls
  rc=None
  while(1):
    lsf.readline()
    line= lsf.readline().rstrip()
    if line==None or line=='': break
    line= line.split()
    #print line
    if len(line)==4:
      if line[3][-8:]==basead:
        rc="ltu %s is allocated for %s DIM server running under login %s"%(basead, line[2], line[0])
        break
  return(rc)

def main():
  if hasattr(sys,'version_info'):
    if sys.version_info >= (2, 3):
      # sick off the new hex() warnings, and no time to digest what the
      # impact will be!
      import warnings
      warnings.filterwarnings("ignore", category=FutureWarning, append=1)
  f=Tk()
  f.title("VME crate")
  cfdir= os.environ['VMECFDIR']
  os.chdir(os.environ['VMEWORKDIR'])
  #vme boards:
  #f1= myw.VmeCrate(f)
  f1= myw.getCrate(f)
  if len(sys.argv) < 2 or (len(sys.argv)==2 and sys.argv[1]=="nbi"):
    print("""Usage: crate.py [nbi] boardName[=base]
boardName   -board name 
base        -base address (default taken from /*BOARD line in board.cf file)
nbi         -if parameter nbi supplied, the board won't be initialised
             i.e. boardInit() won't be called at the time of the start of
             the command line interface
""")
    return
    #bnbase=[]; initboard="nbi"; args= ["ltu=0x812000"]
  else:
    bnbase=[]
    if sys.argv[1]=="nbi":
      initboard= "nbi"
      args= sys.argv[2:]
    else:
      initboard= "yes"
      args= sys.argv[1:]
  for p in args:
    if p == "nbi": initboard= "nbi" ; continue
    if len(bnbase)==1:
      print("Only 1 board allowed")
      break
    bnbase.append(p.split('='))
    board= bnbase[-1]
    #print 'p:',p, board
    if len(board) == 2:         # base address given too
      basead= board[1].strip()
    else:       #default base addr.
      basead=''
    #print board,basead
    if board[0] in myw.DimLTUservers:     #over DIM
      try:
        vmeboard= myw.VmeBoard(f1, boardName=board[0])
      except:
        print("overDIM except:",sys.exc_info()[0])
        sys.exit(8)
      else:
        f1.addBoard(vmeboard)
        syspadd= os.path.join(cfdir,"ltu")
    else:                                       #direct VME
      # Prevent direct VME, if DIM server is on:
      #rtxt=DimServerOn(basead)
      rtxt=None
      if rtxt:
        print(rtxt,"\n")
        print("Try: vmecrate YourDetectorName          from the same login!\n")
        return #sys.exit(0)
      #try:
      vmeboard= myw.VmeBoard(f1, boardName=board[0], baseAddr=basead, initboard=initboard)
      #except:
      #  print("crate.py:", sys.exc_info())
      #  print("crate.py:tb:", traceback.print_stack())
      #  print("""Board name ['ltu', 'ttcvi',...], or detector name """ +str(list(myw.DimLTUservers.keys())) + " expected.")
      #  sys.exit(8)
      #else:
      f1.addBoard(vmeboard)
      syspadd= os.path.join(cfdir, board[0])
    if vmeboard.errmsg!=None:
      print(vmeboard.errmsg)
      sys.exit(8)
    sys.path.append(syspadd)   # to find user gui routines
    #print "crate.py: myw.vbexec:", myw.vbexec
    if myw.vbexec==None:   # nbi LTU could open it
      try:
        vmeboard.openCmd(); myw.vbinit(vmeboard)
      except:
        print(sys.exc_info()[0])
        sys.exit(0)
    #print "crate.py2: myw.vbexec:", myw.vbexec
    if board[0]=='ctp': 
      import ctp_u,ctpcfg
      if not ctp_u.ctpmem: 
        ctp_u.ctpmem=ctpcfg.Ctpconfig(vmeboard)
        if len(ctp_u.ctpmem.pfbs)==0:
          sys.exit(0)
    elif board[0]=='ctpt': 
      import ctpt_u,ctpcfg
      if not ctpt_u.ctpmem: 
        ctpt_u.ctpmem=ctpcfg.Ctpconfig(vmeboard)
        if len(ctpt_u.ctpmem.pfbs)==0:
          sys.exit(0)
    #if board[0:3]!='ctp' and board[0]!='ltu': 
    # CTP common was necessary only during IO CTP testing, commented
    #  sys.path.append(cfdir+'/CTPcommon')
  # f.destroy nie celkom OK (nerobi poriadok)
  #f1.pack()
  # whole crate actions:
  #f2= Frame()
  #okbut= MywButton(f2, label='OK', cmd= f.destroy,side="bottom")
  #f2.pack()
  f.mainloop()
  
if __name__ == "__main__":
    main()

