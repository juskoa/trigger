#!/usr/bin/python
"""
export PYTHONPATH=$PYTHONPATH:$VMECFDIR/TRG_DBED
cd pydim ; ./dimserver.py
Operation:
waits following messages on CTPRCFG/RCFG:
1.
rcfg partName NNN clu1 clu2 ... clu6 cl1 ... cl50 0xA 0xB 0xC 0xD NewLine
-> creates $VMEWORKDIR/WORK/RCFG/rNNN.rcfg
   and its copy in $CLRFS/alidcsvme001/home/alie/trigger/v/vme/WORK/RCFG/
   (this copy is used by ctp_proxy for DAQlogbook + it allows SOD generation)
Notes:
partname starts with ALICE   (real runs). If not, debugging case -
i.e. .debug file will be created (not .rcfg file)
2.
rcfgdel partName NNN
-> delete $VMEWORKDIR/WORK/RCFG/rNNN.rcfg (check if exists before)
rcfgdel ALL 0xc606
-> move $VMEWORKDIR/WORK/RCFG/*.rcfg to delmeh/
3.
smaqmv file_name_in_WORK_directory
-> move filename to trigger@alidcscom027:SMAQ/data/
4.
todo:
5.
pcfg partName 
-> create .pcfg file in $VMECFDIR/CFG/ctp/pardefs/
   - scp to alidcscom001:/tmp/
Note: server.c:Docmd() dwnloads fresh copy of .partition
      file from ACT if available
6.
todo:
- update classname/N in DAQdb in time of .rcfg file creation
  done: 4.5.2009 (see pydim/server.c too)
- create ctp_alignment file along with .rcfg file
  done: 5.10.2009: directly in ctp_proxy.c

create ctp_config file (with each run?)
- always when ctp_config created, ctp_proxy should be restarted
"""
#import os, os.path
#PYTHONPATH=os.environ['VMEBDIR']+':'+\
#  os.path.join(os.environ['VMECFDIR'],'TRG_DBED')
import os, os.path, string, sys, time, shutil, parted, pylog, threading, subprocess
#import miclock
#miclock.mylog= pylog.Pylog("pydim_shift")
mylog= pylog.Pylog(info="info")
VERSION= "7"
def tasc():
  lt= time.localtime()
  ltim= "%2.2d.%2.2d.%4.4d %2.2d:%2.2d:%2.2d"%(lt[2], lt[1], lt[0], lt[3], lt[4], lt[5])
  return ltim

# not supported in run2:
#def checkShift():
#  cshift= miclock.getShift()
#  print tasc()+" resetclock: check after 5secs:"+ cshift

class CNSO:
  def __init__(self, starts):
    self.starts= starts
  def getpos(self, typeix, ordnum):
    # typeix: 0-8 for l[0-2]inp*, l[0-2]classB*, l[0-2]classA*
    return self.starts[startpos]+ordnum-1

def main():
  if hasattr(sys,'version_info'):
    if sys.version_info >= (2, 3):
      # sick off the new hex() warnings, and no time to digest what the
      # impact will be!
      import warnings
      warnings.filterwarnings("ignore", category=FutureWarning, append=1)
  #dimserver= os.popen("./dimserver.exe","r")
  copyit=False ; strict= None
  if os.environ.has_key('VMESITE'):
    pitsrc= os.path.join( os.environ['VMEWORKDIR'],"WORK","RCFG")
    if os.environ['VMESITE'] == 'ALICE':
      copyit=True ; acchost='trigger@alidcsvme001'
      strict= "strict"
      #pitdes= os.path.join( os.environ['CLRFS'],"alidcsvme001/home/alice/trigger/v/vme/WORK/RCFG")
    elif os.environ['VMESITE'] == 'SERVER':
      copyit=True ; acchost='trigger@altri1'
      strict= "strict"
    elif os.environ['VMESITE'] == 'SERVER2':
      copyit=True ; acchost='trigger@altri2'
      strict= "strict"
      #pitdes= os.path.join( os.environ['CLRFS'], "alidcsvme001/home/alice/trigger/v/vme/WORK/RCFG")
    elif os.environ['VMESITE'] == 'PRIVATE':
      copyit=True ; acchost='localhost'
      strict= None
  executable= os.path.join( os.environ['VMECFDIR'],"pydim","linux_s","server")
  #io= popen2.popen2(executable+" CTPRCFG RCFG",1) #0- unbuffered, 1-line buffered
  print "ver: %s Popen %s..."%(VERSION,executable)
  p= subprocess.Popen(string.split(executable+" CTPRCFG RCFG"), bufsize=1,
    stdin=subprocess.PIPE, stdout=subprocess.PIPE, close_fds=True)
  io= (p.stdout, p.stdin)
  #try:
  line= io[0].readline()   #ignore 1st line: DIM server:ctprcfg cmd:rcfg
  print "%s 1st line from ./dimserver:\n%s"%(tasc(), line)
  if line[:10]!='DIM server':
    print "'DIM server' expected, trying 2nd line..."
    line= io[0].readline()   #ignore 1st line: DIM server:ctprcfg cmd:rcfg
    print "%s 2nd line from ./dimserver:\n%s"%(tasc(), line)
    if line[:10]!='DIM server':
      line= io[0].readline()   #ignore 2nd line: DIM server:ctprcfg cmd:rcfg
      print "%s 3rd line from ./dimserver:\n%s"%(tasc(), line)
      if line[:10]!='DIM server':
        print "'DIM server' expected, exiting..."
        io[1].write("quit\n");
        io[0].close() ; io[1].close
        return
  #cshift= miclock.getShift()
  #miclock.checkandsave(cshift,"fineyes")
  #print "current clock shift:"+cshift+" [saved in db+corde]"
  #pts: active paritions, i.e. added: with pcfg and removed after rcfg finished or
  # later -when part content to be kept till rcfgdel
  pts={}
  while(1):
    line= io[0].readline()
    if line=='':
      print "empty line received (server.c crash ?), closing..." ; break
    if line[-1]=='\n':
      print "%s received:%s"%(tasc(),line[:-1])
    else:
      print "%s received:%s"%(tasc(),line)
    if line=='\n':
      print "empty line received (NL), ignored..." ; continue
    if line=='stop\n':
      print "stop received, closing..." 
      break
    # line (sent from ctp_proxy): 
    # partname runnumber 1 2 3 4 5 6 c1 c2 ... c50
    cmd= string.split(line)
    if len(cmd)<1:
      print "strange line received:'%s', closing..."%line ; break
    if cmd[0]=='getinpdets':
      print line
      # process trigger db and update service
    elif cmd[0]=='rcfg' or cmd[0]=='pcfg':
      if len(cmd)<3:
        print "Short cmd ignored:",cmd
        continue
      partname= cmd[1]
      runnumber= cmd[2]
      if cmd[0]=='pcfg':
        #note: .partition file was downloaded directly in server.c
        # from ACT if present!
        part= parted.TrgPartition(partname, strict=strict)
        part.prt()
        print "%s part.loaderrors:"%tasc(),part.loaderrors
        print "%s part.loadwarnings:"%tasc(),part.loadwarnings
        fname= partname+".pcfg"
        if part.loadwarnings!='':
          mylog.infolog(part.loadwarnings, level='w', partition=partname)
        if part.loaderrors=='':
          part.savepcfg(wdir=parted.WORKDIR)   # without 'rcfg '
          pts[partname]= part   # store for rcfg phase only if no load errors
          # following seems ok even before savercfg (to be checked):
          inpdets= part.prtInputDetectors(hexs="yes")    # ret: effout inpdets
          print "after savepcfg indets effectively out:", inpdets, runnumber
          io[1].write("indets %s %s\n"%(runnumber, inpdets))
        else:
          f= open( os.path.join( parted.WORKDIR,fname), "w")
          f.write("Errors:\n") ; f.write(part.loaderrors) ; f.close()
        print "%s %s saved,"%(tasc(), fname)
        if copyit:
          #rcpath= os.path.join( os.environ['VMECFDIR'],"CFG","ctp","pardefs",fname)
          #take file created in LOAD time:
          rcpath= os.path.join( parted.WORKDIR,fname)
          cmd="scp -B -2 %s %s:/tmp/%s"% (rcpath, acchost, fname)
          print "%s rcscp..."%(tasc())
          rcscp=os.system(cmd)
          pcfgn= "q%s.pcfg"%runnumber
          pcfgdes= os.path.join( os.environ['VMEWORKDIR'],"WORK","PCFG")
          shutil.copyfile(rcpath,os.path.join(pcfgdes,pcfgn))
          print "%s rcscp:%d from %s archived in PCFG/%s"%(tasc(), rcscp, cmd, pcfgn)
        else:
          print "not in the pit neither lab"
          #print "not done:",os.path.join(pitsrc,fname),'->',os.path.join(pitdes,fname)
      else:
        part= pts[partname]    # rcfg phase, just restore partition from pcfg time
        part.savercfg(line[5:]) 
        fname="r"+runnumber+".rcfg"
        print "%s %s saved,"%(tasc(), fname)
        usedinps= part.prtinputs()
        print "usedinps:",usedinps,";"
        io[1].write("inpupd %s %s\n"%(runnumber, usedinps))
        print "effectively out:"+ part.filteredout
        # before the copy, ctp_proxy is waiting for, update
        # triggerClassNames in DAQdb
        print "%s now update DAQlogbook... "%(tasc())
        lout="class %s"%runnumber
        for clsn in part.activeclasses.keys():
          clname= part.activeclasses[clsn][0]
          clg   = part.activeclasses[clsn][1]
          clgtim= part.activeclasses[clsn][2]
          dscint= part.activeclasses[clsn][3]
          #lout="%s %s %s"%(lout, clsn, part.activeclasses[clsn])
          lout="%s %s %s %s %s %s"%(lout, clsn, clg,clgtim,dscint,clname)
        lout=lout+'\n'
        print "class RUN# CLS# clg clgtim downsc CLSNAME CLS# clg ..."
        print lout[:-1]
        io[1].write(lout); #io[1].write("blabla\n");
        if copyit:
          #here we should wait for end of whole DAQupdate! (io[0].read...)
          # instead, let's queue the copy request through the same LIFO cahnnel:
          rcpath= os.path.join(pitsrc,fname)
          #shutil.copyfile(rcpath,os.path.join(pitdes,fname))
          # in addition, copy it once more time through scp into /tmp
          if os.path.exists(rcpath):                                                        
            cmd="cmd scp -B -2 %s %s:/tmp/%s\n"% (rcpath, acchost, fname)
            #print "%s rcscp..."%(tasc())
            #rcscp=os.system(cmd); print "%s rcscp:%d from %s"%(tasc(), rcscp, cmd)
            print "%s cmd:%s"%(tasc(), cmd)
            io[1].write(cmd);
          else:
            print "File %s does not exist"%rcpath
        else:
          print "not in the pit neither lab"
          #print "not done:",os.path.join(pitsrc,fname),'->',os.path.join(pitdes,fname)
        del pts[partname]
      sys.stdout.flush()
      part=None
    elif cmd[0]=='rcfgdel':
      if len(cmd)==3:   #rcfgdel partname NNN or ALL 0xc606 or reload 0xc606
        #if (cmd[2]=='0') and (cmd[1]=='ALL'):   #rcfgdel ALL 0, i.e. ctpproxy restarted
        if cmd[1]=='ALL':
          #rcfgdel ALL 0xc606, i.e. ctpproxy restarted
          #rcfgdel reload   -in case there are no paritions in ctpproxy (needed to update BC masks)
          import glob
          FPGAVERSION= cmd[2]
          #os.putenv('FPGAVERSION', FPGAVERSION)  nebavi (copy of environ)
          os.environ['FPGAVERSION']= FPGAVERSION
          print "%s reloading parted (ctpproxy restarted) FPGA:%s..."%\
            (tasc(), FPGAVERSION)
          #import parted   # nepomohlo
          reload(parted)
          parted.initparted()
          if len(pts)!=0: 
            print "ERROR partition list not empty:", pts.keys()
          pts= {}
          os.chdir("RCFG")
          rnames= glob.glob('*.rcfg')
          if len(rnames)>0:
            mvcmd= "mv *.rcfg delmeh/"
            print "delmeh:", str(rnames)
            os.system(mvcmd)
          os.chdir("../")
        elif cmd[1]=='reload':
            print "%s reloading parted (reload request from ctpproxy)..."%(tasc())
            #import parted   # nepomohlo
            reload(parted)
            parted.initparted()
        else:
          rn= parted.dorcfgname(cmd[2], "rcfg")
          if os.path.exists(rn):
            #os.remove(rn)
            mvcmd= "mv %s RCFG/delme/"%rn 
            print "rcfgdel:", mvcmd
            os.system(mvcmd)
          else:
            print "rcfgdel:%s does not exist:"%rn
      else:
        print "Command:%s (no action in py code)"%(line[:-1])
    elif cmd[0]=='smaqmv' and len(cmd)==2:   #smaq filename in WORK directory
      smaqname= "/data/ClientLocalRootFs/alidcsvme001/home/alice/trigger/v/vme/WORK/" + cmd[1] 
      cmd="scp -B -2 %s trigger@alidcscom027:SMAQ/data/"%smaqname
      rcscp=os.system(cmd)
      os.remove(smaqname)
      print "rcscp:%d from %s"%(rcscp, cmd)
    elif cmd[0]=='INFO' or cmd[0]=='ERROR':
      #print line
      pass
    elif cmd[0]=='ERROR':
      print line
      pass
    elif cmd[0]=='resetclock':
      # adjust clock shift: correct any shift
      # resetclock is called from ctp_proxy, when PHYSICS_1 started.
      # 3.6.2015: removed from ctp_proxy( i.e. should not be called)
      print "resetclock: not supported in run2 yet"
      resetclock="""
      cshift= miclock.getShift()
      if cshift != "old":
        try:
          cshift_ps= eval(cshift)*1000
        except:
          #mylog.infolog(part.loadwarnings, level='w', partition=partname)
          print "ERROR miclock.getShift():bad clock shift:"+ cshift
        else:
          if abs(cshift_ps) >= 100.0:
            print "current clock shift:"+cshift+" [saving in db+corde]"
            mylog.infolog("Clock shift %sns resetting to 0..."%cshift, level='i')
            miclock.checkandsave(cshift,"fineyes", force="yes")
            t= threading.Timer(5.0, checkShift)
            t.start()
          else:
            print "current clock shift (not adjusted):"+cshift
            mylog.infolog("Clock shift %sns ok, (<100 ps)"%cshift, level='i')
"""
    else:
      print "Bad command:%s"%(line)
    sys.stdout.flush()
  #except:
  #print "dimserver.py except:",sys.exc_info()[0]
  io[1].write("quit\n");
  io[0].close() ; io[1].close
  
if __name__ == "__main__":
    main()

