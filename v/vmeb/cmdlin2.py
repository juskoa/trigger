#!/usr/bin/python
#9.11.2002 threading/Lock added (i.e. 
#          - if more commands for 1 exe than thay are queued
#          - if more boards (more exes), they run parallely
#4.1.2003  -scrollbar added for cmdlint log area
#5.1.      -logfile option added
#19.2.2004 -outfilter added, 'wait' for thread completion added
#6.5. 2004 -rewritten for 'paralle multiple pipes'
#13.5.2004 - parallel thread1,2... windows are iconified when
#            initialised 
from Tkinter import *
import os, popen2, sys, string, signal, time
from threading import *

#sys.path.append("/home/juskoa/ALICE/vmeb")
import myw

##class myThread(Thread):
##  def __init__(self, target, args=(), name=None):
##    # we need a tuple here
##    if type(args)<>type((1,)):
##      args = (args,)
##    Thread.__init__(self, target=target, name=name, args=args)
##    self._uptime = time.ctime(time.time())
##    self.start()
##  def getUptime(self):
##    return self._uptime

class ioWindow:
  def write(self, text, tag=None):
    #print "text",text,tag
    self.w.insert(END, text, tag); self.w.see(END)
    self.logit(text)
  def getOutput(self, wout='out', applout='out'):
    """ Process the output lines until:
    ':\n' -> end of the command execution or 
    ''    -> pipe broken
    MSG2ALL message\n:\n   -don't pass this message. 
                            Only print it to log window!
    wout:
    out   -> output written to log window
    NO    -> even cmd output (in red) will be suppressed
    no (any) ->output to log window suppresed, but MSG2ALL messsage
    applout:
    out   -> getOutput returns the output
    <>    -> output is filtered, only <> items returned in the list
    no (any) getOutput returns None
    """
    lastOutput=''; self.lineNumber=0; msg2all=0; self.ignoreall=0
    try:
      while 1:
        line= self.io[0].readline()
        #print '>'+str(self.ixthds)+'>',line,'<<'
        #print "cmdlint:",line,"eol"
        ##if buf and buf!="wait":
        if line ==':\n':   #don't take last '\n:\n'
          if msg2all==1:
            msg2all=0
            continue  #now process the app output
          if self.ignoreall==1:
            self.ignore2all=0
            #continue  #now process the app output
          break
        if line[0:17]=="Server restarted.":
          self.ignoreall=0
        if self.ignoreall==1: 
          self.write("ignored:"+line+"\n","MSG"); 
          continue
        if msg2all==1:
          self.write(line+"\n","MSG"); 
        if line[0:9]=='failed...' or line[0:6]=='ERROR:':
          self.write(line+"\n","MSG"); 
          self.ignoreall=1
          continue       
        if line[0:8]=='MSG2ALL ':
          self.write("                        Message from operator:\n","MSG");
          self.write(line[8:]+"\n","MSG"); 
          msg2all=1
          continue       
        if wout =="out":
          #nebavi if line[-1]=='\r':
          #  self.w.delete("%d.0" %(self.lineNumber),END)
          #else:
          # problem: we cannot get it line by line -
          # fflush(stdout) in application doesn't solve it...
          #ladr=str(self.lineNumber)+':'
          #self.w.insert(END, ladr+line); self.w.see(END)
          self.w.insert(END, line); self.w.see(END)
          self.logit(line)
        if applout =="out" or applout=="<>":
          lastOutput=lastOutput+line
          self.lineNumber= self.lineNumber+1
        if line =='':
          line="pipe from cmdline interface closed/broken (server down)\n"
          self.w.insert(END, line); self.w.see(END)
          break
    except:
      print 'problem processing pipe output'
      etype,evalue,etrace= sys.exc_info()
      print 'exc_value:',evalue
    if applout =="out":
      return lastOutput
    elif applout=="<>":
      return self.outfilter(lastOutput)
    else:
      return None
  ##def getlastOutput(self,filter='no'):
  ##  #print "cmdlint.getlastOutput:",self.lastOutput,"---"
  ##  if self.threadcmd.isAlive():
  ##    #print 'waiting for the thread completion...'
  ##    self.threadcmd.join()   # wait for the completion of the thread
  ##  if filter=='<>':
  ##    return self.outfilter(self.lastOutput)
  ##  else:
  ##    return self.lastOutput
  def outfilter(self,text):
    out=[];i=0
    #print 'outfilter:',text
    for s in text:
        if(s=='>'):
          if(len(out) == i):print "Wrong syntax ><";break;
          i=i+1
        if(len(out) != i ):
          if(s == '<'):print "Wrong syntax <<";break;
          out[i]=out[i]+s
        if(s=='<'): out.append('')
    #print "out=",out," i=",i
    return out
  def bindexecute(self,event):
    self.execStart()
  def execStart(self):
    cmdtoex= self.cmd.getEntry()
    self.cmd.setEntry("")
    self.executet(cmdtoex,None,'out',None)
  def plock(self, lockunlock):
    self.busyp= lockunlock   # 0- not locked, 1- locked
  def pwrite(self, lines):
    self.io[1].write(lines)
  def waitcmdw(self):
    ix=0
    while 1:
      if self.busy2!= 0:
        time.sleep(0.1)
        ix= ix+1
        if ix>9:
          print 'cmdlint: too long wait in waitcmdw'
      else: break
  def executet(self, cmd=None, ff=None, log='out', applout='out'): 
    #if ff==None:
    #  print 'ERROR: executet(cmd,ff,...)  ff-has to be supplied!'
    #  return
    #print "cmdlin2.executet.cmd:",cmd
    self.cli.lockexe.acquire()
    #print "cmdlin2.executet.cmdNOTHREAD:",cmd
    # no thread:
    self.threadedexe(cmd,log,ff,applout) ; return
    ##self.busy2=1
    ##self.threadcmd= myThread(self.threadedexe, args=(cmd,log,ff,applout))
    ##if ff==None:
    ##  self.threadcmd.join()   # wait for the completion of the thread
  def threadedexe(self, cmdte, buf, ff, applout):
    self.busy=1 #;print "busy-1"
    self.cli.lockexe.release()
    #print "ioWindow.threadedexe:",cmdte,buf,ff,applout
    if self.io == None:
      print "Error in cmdlin2.threadedexe: can't execute ",cmdte
      #raise "Error: can't execute: "+cmdte
      return
    if buf!="NO":  #don't even log the command
      #print "cmdlin2:threadedexe:",cmdte
      self.write(cmdte+"\n", "INPUT");
    self.logit(":"+cmdte+"\n")
    #CCTOPEN
    try:
      #self.io[1].write("cctopen()\n"); self.getOutput(buf,applout)
      self.io[1].write(cmdte+"\n")
    except:
      print 'cmdlint:cannot write to cmdline interface'
      etype,evalue,etrace= sys.exc_info()
      print 'exc_value:',evalue
    self.busy2=0
    if ff:
      ff(self.getOutput(buf,applout))
    else:
      self.lstOutput= self.getOutput(buf,applout)
    #CCTCLOSE
    #self.io[1].write("cctclose()\n"); self.getOutput(buf,applout)
    self.busy=0 #;print "busy-0"
  def bindstop(self,event):         # x (win. manager) button
    #print "ioWindow.bindstop:",event
    self.stop("wm")
  def stop(self,XorQuit=None):                   # quit button (or destory -x)
    """ stop and close pipe, close logfile, close window
    if last window closed, return back Black text in VmeBoard 
    """
    #print "ioWindowstop() XorQuit:", XorQuit
    try:
      if self.io != None:
        if self.busy==1:   # pipe active
          self.kill(signal.SIGKILL);
          time.sleep(1.5)  # wait 1.5 secs (quit variable tested...)
        else:
          self.io[1].write("q\n")
        if sys.platform != "win32":
          #linux, cygwin:
          os.waitpid(-1, os.WNOHANG)
        self.io[0].close; self.io[1].close;
        self.io=None
      else:
        pass
    except:
      print "Error: problems when closing popen pipe"
    if self.lofi != None:
      self.lofi.close()
      self.lofi=None
    if XorQuit!="wm":
      self.tlio.destroy()
    self.cli.delthds(self.ixthds)
  def kill(self, signal=signal.SIGUSR1):
    #print 'isAlive:',self.threadcmd.isAlive(),':'
    #print 'pid:',self.pidexe,':',self.lastOutput
    os.kill(self.pidexe, signal)
  def logit(self,ttl):
    if self.ixthds!=0: return   # no log for parallel windows
    lfn=self.logfile.getEntry()
    if lfn != None and len(lfn)>0:
     if lfn[0] != " " and lfn != "None":
      if self.lofi == None:
        self.lofi= open(lfn,"a")
      self.lofi.write(ttl)
    else:
      if self.lofi != None:
        self.lofi.close()
        self.lofi=None
  def __init__(self, cli,ixthds):
    """
    cli    -the parent cmdlint class
    ixthds -pointer to thds[] to myself (0 -special 1st thread)
    """
    #print "ioWindow:",cli.cfdir
    #iow self.cfdir=os.environ['VMECFDIR']
    self.cli=cli
    self.ixthds=ixthds
    self.busy=1
    self.plock(0)
    self.lofi=None   # logifle object
    self.lastOutput=''; self.lineNumber=0
    self.lstOutput='nictunieje'
    self.tlio= myw.NewToplevel('%s:%s' % (cli.board.boardName,cli.board.baseAddr))
    if ixthds!=0: 
      # if it is transient, it is not possible to iconify
      self.tlio.iconify()
    #-----------------------------------------------------controls:
    self.tp= Frame(self.tlio)
    self.tp.pack(side="top")
    self.tp.bind("<Destroy>", self.bindstop)
    self.cmd= myw.MywEntry(master=self.tp, label="cmd",defvalue=" ",width=44)
    self.cmd.entry.bind('<Key-Return>', self.bindexecute)
    #
    # ako da sa urobit callback s parametrom:
    #cmdh= lambda s=self,x='11':s.execute(x)
    #self.exbut= myw.MywButton(self.tp, "start", cmd=cmdh, side="left")
    self.exbut= myw.MywButton(self.tp, "start", cmd=self.execStart, side="left")
    self.ok= myw.MywButton(self.tp, "kill", cmd=self.kill, side='left')
    if ixthds==0:
      myw.MywButton(self.tp, "quit", cmd=self.stop, side='left')
      self.logfile= myw.MywEntry(master=self.tp, label="Log:",defvalue="None",
        width=10,helptext="Log file name")
    #-----------------------------------------------------log area:
    self.loga= Frame(self.tlio)
    self.loga.pack(side=BOTTOM)
    scrollbar = Scrollbar(self.loga)
    self.w=Text(self.loga, yscrollcommand=scrollbar.set); 
    self.w.pack(side=LEFT)
    scrollbar.config(command=self.w.yview)
    scrollbar.pack(side=RIGHT,fill='y')
    self.w.tag_config("INPUT", foreground="red")
    self.w.tag_config("MSG", foreground="red", background="white")
    # or:
    #import tkFont
    #tkFont.families()
    #ifont= tkFont.Font(family="Courier", size="10",weight="bold")
    #w.tag_config("INPUT", foreground="red", font=ifont)
    # to disable keyboard-write:
    # w.cget("state")
    #w.config(state="disabled") ; w.config(state="normal")
    #---------------------------------- popen2 way
    self.io=None
    #print "ioWindow.init: cmdlin2:cmd:",cli.cmd
    exep=string.split(cli.cmd,None,1)[0]
    #print "cmdlin2:exep:",exep
    if not os.path.exists(exep):
      #print "Where is ",exep,"?"
      raise "Where is "+exep+"?"
      #raise self.cli, "Where is "+exep+"?"
    else:
      nbcmd= cli.cmd
      if ixthds!=0 or cli.board.initboard=="nbi":   
        # noboardInit for paralel tasks! 
        nbcmd= nbcmd+" -noboardInit"
      #iow self.lockexe= Lock()
      #von self.lockexe.acquire()
      if (sys.platform == "win32"):
        print "popen2..."
        self.io= popen2.popen2(nbcmd, -1)
        #self.io= os.popen2(nbcmd, -1)
      else:    # linux, cygwin:
        self.io= popen2.popen2(nbcmd, 1) #0- unbuffered, 1-line buffered
        #nop
      #print "cmdlin2:",nbcmd,":",os.getcwd(),":"
      #print "cmdlin2:self.io:",self.io
      #pidline=string.split(self.getOutput('out','out'),'\n',1)[0]
      sgot=self.getOutput('out','out')
      #print "cmdlin2.sgot:",sgot,'<'
      pidline=string.split(sgot,'\n',1)[0]
      #print ":",pidline,":"
      #pidline="PID 234"
      self.pidexe=None
      #print "pidline:",pidline
      ##clm,pid,restofline= string.split(pidline, None, 2) 
      clmpid= string.split(pidline, None, 2) 
      if len(clmpid) < 2:
        print "can't open ioWindow interface (instead PID got:",clmpid,"):"
        print sgot
        self.io[1].close(); self.io[0].close(); self.io=None
      elif clmpid[0] != "PID":
        print "can't open ioWindow interface (instead :PID got:",clmpid[0],"):"
        print sgot
        self.io[1].close(); self.io[0].close(); self.io=None
      else:
        try:
          ipid= int(clmpid[1])
        except:
          print "cannot open ioWindow interface(instead pid got:",clmpid[1],")"
          self.io[1].close(); self.io[0].close(); self.io=None
        else:
          self.pidexe=ipid
    self.busy=0
      #von self.lockexe.release()

class cmdlint:
  """
  Usage:
  1. start:
  cli= cmdlint(command)
       - creates control window (for command input/output)
       - starts command through popen2
  This should be started only once. 
  In BOARD_u.py, user routine is called with just 1 parameter (vb) 
  which is the instance of corresponding Myw.VmeBoard class.
  vb.io points to cmdlint instance for the current board.

  2. command execution (programmatically):
  - cli.cmd.setEntry("input line"); cli.execute() 
  - cli.execute(command, log, applout, ff)  -execute command
    in separate thread (thread1,2,...) and after its completion 
    start ff in the same thread. 
    if ff==None (default)
      WAIT FOR THREAD COMPLETION (main thread is 'joined').
    else
      DO NOT WAIT FOR THREAD COMPLETION.
  
  log:      'out' (default) -appl. output is logged in window
            'no'            -appl. output is not logged in log window
            'NO'            -executed command not logged too
  applout:  'out' (default) -returns application output (stdout)
            '<>'  -returns ['s1','s2',...], s1,s2,... are
                   strings found in <> brackets in the output text 
            'no'  -returns None
  ff:    exit function, which gets one parameter: the text of
         the output (if applout == 'out') or list of strings
         found in <> brackets in the output (if applout=='<>') 
         If applout=='no', ff receives None
         RC: thread number (index into thds, 0..3)
  ff==None: execute command in thread0 (reserved for 'short lasting threads')
         In this case, command returns what is required by
         applout parameter

  3.cli.write('text to be written to the window') (always thread0)
  4.cli.stop()
    -sends 'q\n' 
    -closes i/o pipes, releases defunc proc
    """
  def __init__(self, cmd, board=None):
    self.cmd=cmd
    #print "cmdlin2:cmd:",cmd
    self.board=board
    self.lockexe= Lock()
    self.cfdir=os.environ['VMECFDIR']
    self.lofi=None   # logifle object
    #iow self.lastOutput=''; self.lineNumber=0
    self.thds=[None,None,None,None]   #allow 4 parallel pipes
    self.thds[0]= ioWindow(self,0)
    #print "cmdlin2:thds:",self.thds
    if self.thds[0].pidexe==None:
      self.thds[0].write("exiting...\n","MSG")
      #raise "cmdlint exiting"
    else:  
      self.thds[0].write("+3 parallel pipes allowed...\n")
  def delthds(self,ix):
    #print "delthds:",ix
    self.thds[ix]= None; 
    noio=1
    for t in self.thds:
      if t!=None: noio=0
    if noio==1:
      self.board.delio()
  def execute(self, cmd=None, log='out', applout='out',ff=None): 
    if ff==None:
      # thread0 request:
      if self.thds[0]==None:
        self.thds[0]= ioWindow(self,0)
      #print "cmdlin2:execute:", self.thds
      self.thds[0].executet(cmd,None,log,applout)
      if applout=='<>' or applout=='out':
        #print "cmdlin2:execute:", self.thds[0].lstOutput,':'
        return self.thds[0].lstOutput
      else:
        return None
    #find free thread:
    freethread=None
    ixunopened=0      # if there is unopen interface, here it can be opened
    ix=1
    self.lockexe.acquire()
    for t in self.thds[1:]:   # don't take thread0
      if t!= None:       # pipe opened
        if t.busy==0 and t.busyp==0:    # and not busy
          freethread=t
          break;         # so it can be used for command execution
      else:              # pipe not opened
        ixunopened=ix    # remember, probably we need it (if there is
      ix=ix+1            # not free one)
    self.lockexe.release()
    if freethread==None:
      #print "Free thread not found, preparing new one"
      if ixunopened!=0:
        freethread= self.thds[ixunopened]= ioWindow(self,ixunopened)
        ix= ixunopened
      else:
        errmsg="Attempt to open too many threads (>"+str(len(self.thds))+")"
        myw.MywError(errmsg)
    else:
      pass
      #print "free thread found (i.e. opened & not busy)"
    if freethread!=None:
      #if ff==None:
      #  print 'ERROR: executet(cmd,ff,...)  ff-has to be supplied!'
      #  return
      #iow self.threadcmd= myThread(self.threadedexe, args=(cmd,log,ff,applout))
      freethread.executet(cmd,ff,log,applout)
    return ix
  def write(self, text=''):
    self.thds[0].write(text)
  def stop(self):
    """Close all the opened windows"""
    #print "cmdlin2.stop:"
    for t in self.thds:
      if t!= None:     
        t.stop()

def emptyfun(out):
  print 'emptyfun:',out
  #pass
def main():
  f=Tk()
  if len(sys.argv)>1:
    c= cmdlint(cmd=sys.argv[1])
  else:
    print "Usage: cmdlin2.py executable_path pars"
    sys.exit(8)
  #print "c.io:",c.io,"c:",c
  if c.thds[0] !=None:
    #c.cmd.setEntry("blabla"); c.execute()
    c.execute("loop(8)",ff=emptyfun,applout='<>')
    c.execute("loop(10)")
    c.execute("loop(12)",ff=emptyfun,applout='<>')
    c.execute("loop(3)")
  f.mainloop()

if __name__ == "__main__":
    main()

