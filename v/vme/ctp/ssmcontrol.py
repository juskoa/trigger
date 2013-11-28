#!/usr/bin/python
'''
SSM operation (read/write record/generate/stop). 17.11.2005 -1st version
18.11. 
prepare global ssm.mode name (_nomode is last resort) for ssmbrowser
always at the time of reading SSMstatus word.
30.11.
ssmbrowser works only through sms -it should never touch VME
(like ssmcontrol), because there is test-type SSM. Or, if necessary,
it should differentiate between real and non-real SSM.
31.3.2006
key preparation for finding fname in fixsmsmode now correct 
'''
from Tkinter import *
import os, os.path, string, time
cfdir= os.environ['VMECFDIR']
bdir= os.environ['VMEBDIR']
os.chdir(cfdir)
sys.path.append(bdir); sys.path.append(cfdir+'/ctp')
import myw

COLOR_SSMC='#ffffcc'
vb=None
ssmctlframe=None
ssmctl=None

def ssmcend(ev=None):
  global vb, ssmctlframe
  #print "ssmcend:",vb
  vb=None; ssmctlframe=None; ssmctl=None
def findCPosition(cname):
  import trigdb
  cnames= trigdb.TrgCNAMES()
  return cnames.getRelPosition(cname)

class SSM:
  """Represents 1 SSM (on 1 board). 3 types
  of the board: ctp, ltu in ctp crate, ltu outside of ctp crate (todo)
  """
  MODOPEW=35
  LTUbits=[("mode",1),("op1",1),("BUSY",0),("n/a",1),("FP->SSM",0)]
  CTPbits=[("mode",1),("op1",1),("op2",1),("InOut",0),
        "CS1","CS2",("EnaOut",0),("EnaIn",0),("BUSY",0)]
  modoperation= ["Read", "Write", 
      "Recording\nAFTER (27ms)",
      "Recording\nBEFORE (cont.)",
      "Generate (1pass)",
      "Generate (cont.)",
      "Bad mode/op:6",
      "Bad mode/op:7"
      ]
  CTPactions= [["Read","Read"], ["Write","Write"], 
      ["INMON: record AFTER-27ms (1pass)+READ","R0x00a"],
      ["OUTMON: record AFTER-27ms (1pass)+READ","R0x002"],
      ["INMON: record AFTER-27ms (1pass)","0x00a"],
      ["OUTMON: record AFTER-27ms (1pass)","0x002"],
      ["INMON: record BEFORE -(continuous)","0x00b"],
      ["OUTMON: record BEFORE -(continuous)","0x003"],
      ["INGEN: Generate (1pass)","0x20c"],
      ["OUTGEN: Generate (1pass)","0x104"],
      ["INGEN: Generate (continuous)","0x20d"],
      ["OUTGEN: Generate (continuous)","0x105"],
      ["Stop (gen. or rec.)","Stop"],
      ["Conditional stop","Condition"]
      ]
  LTUactions= [["Read","Read"], ["Write","Write"], 
    ["FP->SSM: record AFTER-27ms (1pass)","0x2"],
    ["FP->SSM: record BEFORE -(continuous)","0x3"],
    ["Stop (gen. or rec.)","Stop"]
    ]
  modenames= {"ltu":"ltu_im", "ltuFP":"ltu_i1",
    # bnameio][mg]cs1cs2:fname
    "busyom00":"busy_outmon",  
    "busyim00":"busy_inmon",  
    "busyog00":"busy_outgen",  
    "busyig00":"busy_ingen",  
    "l0ig00":"l0_ingen",  
    "l0im00":"l0_inmon",  
    "l0om00":"l0_outmon",  
    "l0og00":"l0_outgen",
    "l1ig00":"l1_ingen",  
    "l1og00":"l1_outgen",  
    "l1im00":"l1_inmon",  
    "l1om00":"l1_outmon",  
    "l2im00":"l2_inmon",  
    "l2om00":"l2_outmon",  
    "l2om01":"l2_pf",  
    "l2ig00":"l2_ingen",  
    "l2og00":"l2_outgen",  
    "foom00":"fo_inmonl0",  
    "foom01":"fo_inmonl1",  
    "foom10":"fo_inmonl2",  
    "foig00":"fo_igl0l1",  
    "foig01":"fo_igl2",  
    "foog00":"fo_outgen",
    "intim00":"int_inmon",  
    "intim01":"int_i2c",  
    "intom00":"int_ddldat",  
    "intom01":"int_ddllog"  
     }  
  def __init__(self,name,smsix):
    self.name=name
    self.smsix=smsix   # position in sms[] global array
    if name[:3]=='ltu':
      self.ltuctpbits= SSM.LTUbits
      self.actions= SSM.LTUactions
    else:
      self.ltuctpbits= SSM.CTPbits
      self.actions= SSM.CTPactions
    self.status=0xffff
  def show(self, frs):
    #print "SSM.show:",self.name
    fr1= myw.MywFrame(frs, side=TOP,relief=FLAT, bg=COLOR_SSMC)
    lheader= myw.MywLabel(fr1, side=LEFT, label=self.name,
      width=10, expand='no',fill='y', helptext="Symbolic name of the board")
    self.bactions= myw.MywxMenu(fr1, side=LEFT, width=SSM.MODOPEW, defaultinx=0,
      label='', items=self.actions,cmd=self.action,helptext=
      SSMcontrol.helpaction)
    self.lname= myw.MywEntry(fr1, side=LEFT, width=8,defvalue="0",
      expandentry='no',label='', helptext="SSMstatus word after last operation")
    self.modoper= myw.MywxMenu(fr1, side=LEFT, width=14, defaultinx=0,
      label='', items=SSM.modoperation,helptext="""
Symbolic meaning of SSMstatus[2..0] bits.
""")
    self.modoper.disable()
    self.lstatus= myw.MywBits(fr1, side=LEFT, defval= self.status,
      label= '', bits= self.ltuctpbits, cmd=self.modcs,helptext="""
Symbolic meaning of SSMstatus[7..2] bits.
Set CS1 CS2 flags before starting GEN/MON operation. 
Certainly for fo* operations:
CS2 CS1  command      ssmsigs_file
------------------------------------
0   0   fo OUTMON     fo_inmonl0.sig
0   1   fo OUTMON     fo_inmonl1.sig
1   0   fo OUTMON     fo_inmonl2.sig
""")
    self.readssmst()
  def action(self,mywxinst,ix):
    global ssmctl
    om= self.bactions.getEntry()
    #print "action:",ix, om
    if om=='Read':
      intstr= vb.io.execute("readSSM(%d)"%(self.smsix))
    elif om=='Write':
      intstr= vb.io.execute("writeSSM(%d)"%(self.smsix))
    elif om=='Stop':
      intstr= vb.io.execute("stopSSM(%d)"%(self.smsix))
    elif om=='Condition':
      posentry= string.strip(ssmctl.bcntpos.getEntry())
      if not posentry.isdigit():
        pos= findCPosition(posentry)
        vb.io.write("%s rel. position:%s\n"%(posentry,pos))
      else:
        pos= posentry
      if pos==None:
        vb.io.write("rel. position not valid, cond. stop not executed\n")
      else:
        intstr= vb.io.execute("condstopSSM(%d,%s,%s, 0)"%\
          (self.smsix+20, pos, ssmctl.btimeout.getEntry()))
        # board 21: stop L0 + L1
    else:
      waitread= False
      if om[:3]== "R0x":
        # special case: "R..." -> 27ms IN/OUT mon + WAIT 27ms + READ
        om= om[1:] ; waitread= True
      om=eval(om) | (self.status&0x30)
      hexstr= vb.io.execute("setomSSM(%d,0x%x)"%(self.smsix,om),
        applout="<>")[0]
      if hexstr != '0':
        vb.io.write(hexstr+' -from setomSSM, action not started\n')
      else:
        hexstr= vb.io.execute("startSSM1(%d)"%(self.smsix))
        if waitread:
          self.readssmst()
          time.sleep(0.030)
          intstr= vb.io.execute("readSSM(%d)"%(self.smsix))
    self.readssmst()
  def modcs(self):
    self.status= self.lstatus.getEntry()
    self.lname.setEntry(hex(self.status))
  def readssmst(self):
    hexstr= vb.io.execute("getswSSM(%d)"%self.smsix,applout="<>")[0]
    self.lname.setEntry(hexstr)
    self.status= eval(hexstr)
    if self.name[:3]=='ltu':
      iix=self.status&0x3
    else:
      iix=self.status&0x7
    self.modoper.setEntry(iix)
    self.lstatus.setEntry(self.status)
    self.fixsmsmode()
    return hexstr
  def fixsmsmode(self):
    """
    Update sms[].mode always at the time of reading SSMstatus word,
    only for known modes (probably for all modes different from 
    read/write/stop).
    op/mo board/CSbit1 CSbit2   sms[].mode      -> see SSM.modenames
    INMON ltu/FP         ltu_i1
          ltu/notFP      ltu_im
          fo/00          fo_inmonl0
          fo/01          
          fo/10
          l0/00 
    INGEN fo/00 fo_igl0l1
          fo/01 fo_igl2
          l0/00 l0_ingen
    OUTMON l0/00 l0_outmon
    OUTGEN fo/00 fo_outgen
           l0/00 l0_outgen
    """
    # preapre key: ctpboard[io][mg][01][01]   or
    #              ltu[FP]
    key=''
    iomg=''
    if self.name[0:3]=='ltu': 
      if (self.status & 0x3)>=2:
        if self.status & 0x10:
          key= 'ltuFP'
        else:
          key= 'ltu'
    else:
      if self.name[0:2]=='fo': key='fo'
      else: key= self.name
      sce= self.status & 0xce   # EnaIn EnaOut InOut Operation[1..0] 0
      if   sce==0x0a: iomg='im'
      elif sce==0x02: iomg='om'
      elif sce==0x8c: iomg='ig'
      elif sce==0x44: iomg='og'
      #else: iomg=''
      key= key+iomg
      cs= self.status & 0x30    # CS2 CS1
      if   cs==0: key= key+'00' 
      if   cs==0x10: key= key+'01' 
      if   cs==0x20: key= key+'10' 
      if   cs==0x30: key= key+'11' 
    #print "fixsmsmode:",self.name, "status:", hex(self.status),"key:",key
    if SSM.modenames.has_key(key):
      vb.io.execute('setsmssw(%d,"%s")'%(self.smsix, SSM.modenames[key]))
    else:
      if iomg != '':
        #errmsg="ssmcontrol.py: key %s missing in SSM.modenames"%key
        errmsg="ssmcontrol.py: mode %s doesn't exist"%key
        vb.io.write(errmsg)
        print errmsg
class SSMcontrol:
  helpaction= """Board: symbolic name of the board

Action: perform one of the following actions with SSM:
  Read:   read content of the SSM on the board to the buffer in
          computer memory, which can be than examined by SSMbrowser       
  Write:  write the content of the buffer in computer memory to
          SSM on the CTPboard. The buffer should be prepared beforehand
          by user application.
  INMON:  record INPUTS of the CTPboard in SSM
  OUTMON: record OUTPUTS of the CTPboard in SSM.
  INGEN:  use SSM as generator - the input signals of the CTPboard
          will be taken from its SSM.
  OUTGEN: use SSM as generator - the outputs of the CTPboard
          will be disconnected. The output signals will be
          taken from its SSM.
  Stop:   stop continuous monitoring or generating
  Conditional stop:
          - read and check for the change of any counter 
            (see Counter and Timeout entry fields below)
          - stop continuous monitoring as soon as the change is detected
  1pass:  means '1 pass through SSM (when recording or generating)',
          which is 1024*1024*25 ns = ~27 miliseconds
  continuous: record/generate continuously (until 'Stop' Action)

SSMstatus: SSMstatus word after last operation. 

Flags: symbolic meaning of SSMstatus bits. 
       1. field explains SSMstatus[2..0] bits -Operation code and mode bits.
       2. field displays the remaining bits of SSMstatus set to 1.
          CS1/CS2 flags can be set before performing GEN/MON commands
"""
  def __init__(self,vbcmdlin, tlfr):
    global vb
    vb=vbcmdlin
    self.tlfr=tlfr
    self.sms=[]   # list of all SSMs  [name, smsix]
    lines= string.split(vb.io.execute("gettableSSM()","no"),"\n")
    #lines=["busy nossm","l0 outgen", "ltu1 nossm","ltu2 notin"]
    #print "SSMcontrol:",lines,':'
    for ix in range(len(lines)):
      nm= string.split(lines[ix])
      #print "SSMcontrol nm:", nm
      if len(nm)==0: break
      if nm[1]=='notin': continue
      if nm[0]=='test': continue
      self.sms.append(SSM(nm[0], ix))
    self.show()
  def show(self):
    fr1= myw.MywFrame(self.tlfr, side=TOP,relief=FLAT, bg=COLOR_SSMC)
    lheader= myw.MywLabel(fr1, side=TOP, anchor='w',
      helptext=SSMcontrol.helpaction, label=
      'Board           Action                                                      SSMstatus    Flags')
    # expand='no',fill='y',
    fr2= myw.MywFrame(self.tlfr, side=TOP,relief=FLAT, bg=COLOR_SSMC)
    for ix in range(len(self.sms)):
      self.sms[ix].show(fr1)
    self.bcntpos= myw.MywEntry(fr2, side=LEFT,label="Counter", 
      bind='lr', cmdlabel=self.checkcname,
      helptext="""Relative position of the counter (starting from 0).
When 'Conditional stop' applied, this counter is read and tested for the change.
Relative position of the counter can be find in
trigger@alidcscom026:$dbctp/cnames.sorted2
The name of the counter (as seen in Counters widget) can be entered
too -its relative position will be displayed ctp log-window.

""")
    self.btimeout= myw.MywEntry(fr2, side=LEFT,label="Timeout",
      helptext="""Max. number of loops executed while waiting
for 'counter change' after 'Conditional stop' action.
1 loop takes ~ 2ms, i.e. 1000 loops corresponds to 2 seconds.
""")
    bcheckall= myw.MywButton(fr2,side=LEFT, label="check status",
      cmd=self.checkall,helptext="""Check the status of all SSMs and
upgrade corresponding status fields in this window.
""")
    bquit= myw.MywButton(fr2,side=LEFT, label="quit",
      cmd=self.quit,helptext="quit")
  def checkall(self):
    #print "checkall:",len(self.sms)
    for s in self.sms:
      s.readssmst()
  def quit(self):
    self.tlfr.destroy()
    ssmcend()
  def checkcname(self, val):
    val=val.strip()
    if val=='': return
    vb.io.write('checkcname:%s\n'%val)
    if not val.isdigit():
      pos= findCPosition(val)
      if pos==None:
        vb.io.write("bad counter name:%s\n"%val)
      else:
        vb.io.write("%s rel. position:%s\n"%(val,pos))

class vbio2:
  def __init__(self):
    pass
  def execute(self,cmd):
    print "vbio:", cmd
    return ["0"]
class vbio:
  def __init__(self,master):
    self.master=master
    self.io=vbio2()
def main(vb):
  global ssmctl,ssmctlframe
  #print "ssmccontrol.main:",vb
  if ssmctlframe:
    myw.RiseToplevel(ssmctlframe)
  else:
    ssmctlframe= myw.NewToplevel("SSM control", ssmcend)
    ssmctl=SSMcontrol(vb, ssmctlframe)

if __name__ == "__main__":
  master=Tk()
  vb= vbio(master)
  main(vb)
  master.mainloop()

