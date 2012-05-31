# LVDS tester, 17.2.2007
from Tkinter import *
#import tkFileDialog
import os, os.path, glob, string
import myw,counters
class SOFTLEDS:
  SWLEDS=(("R","VME read"), ("W","WME write"), ("A","SCOPE-A"),
          ("B","SCOPE-B"),
          (".","""'continuous update' indication (i.e. there is 
SW VME read every half second if this label is moving)"""))
  #ooo=("-","\\","|","/")
  ooo=("\\","/")
  def __init__(self, vb, frame):
    self.vb=vb
    self.ooo=0
    if frame==None:
      self.f1=myw.NewToplevel("Soft LEDS")
    else:
      self.f1= frame
    self.labels=[]
    for ix in range(5):
      self.labels.append(myw.MywLabel(self.f1, SOFTLEDS.SWLEDS[ix][0],
        SOFTLEDS.SWLEDS[ix][1], side=LEFT))
    self.update()
  def update(self):
    self.sls= self.vb.io.execute("getSWLEDS()", log="NO")   # "1011"
    #print self.sls
    for ix in range(4):
      if self.sls[ix]=='1':
        self.labels[ix].setColor("red")
      else:
        self.labels[ix].setColor("white")
    self.labels[4].label(SOFTLEDS.ooo[self.ooo])
    self.ooo= self.ooo+1
    if self.ooo >= len(SOFTLEDS.ooo): self.ooo=0
    self.afterid=self.f1.after(500, self.update)

#--------------------------- Scope outputs selection:
def ScopeAB(vb):
  class st:
    def __init__(self, vb):
      self.vb=vb
      self.tl=myw.NewToplevel("Oscilloscope signals selection")
      #self.f1= myw.MywFrame(self.tl,side=TOP)
      #self.f12= myw.MywFrame(self.f1,relief=FLAT,side=BOTTOM)
      ab=self.vb.io.execute("vmeopr32(SCOPE_SELECT)")[:-1]
      defa= eval(ab)&0x1f
      defb= (eval(ab)&0x3e0)>>5
      itemsA=itemsB=(
        ("BC (delayed)","0",self.selectAB),
        ("BCin (no delay)","1",self.selectAB),
        ("pattern (L0)","2",self.selectAB),
        ("synchronised pattern (delay:0)","3",self.selectAB),
        ("delayed_pattern1","4",self.selectAB),
        ("delayed_pattern2","5",self.selectAB),
        ("ADCin cable1 1","6",self.selectAB),
        ("ADCin cable2","7",self.selectAB),
        ("ADCin pattern","8",self.selectAB),
        ("Cable1 in (BUSY1)","9",self.selectAB),
        ("Cable1 in (BUSY2)","10",self.selectAB),
        ("Error1","11",self.selectAB),
        ("Error2","12",self.selectAB),
        ("Error1 latched","13",self.selectAB),
        ("Error2 latched","14",self.selectAB),
        ("Sync. cable1 in","15",self.selectAB),
        ("Sync. cable2 in","16",self.selectAB),
        ("ADCin_selected","17",self.selectAB), 
        ("Sequence strobe","18",self.selectAB),
        ("GND","19",self.selectAB), ("GND","20",self.selectAB),
        ("GND","21",self.selectAB), ("GND","22",self.selectAB),
        ("GND","23",self.selectAB), ("GND","24",self.selectAB),
        ("GND","25",self.selectAB), ("GND","26",self.selectAB),
        ("GND","27",self.selectAB), ("GND","28",self.selectAB),
        ("VME Read","29",self.selectAB),
        ("VME Write","30",self.selectAB),
        ("VME STROBE","31",self.selectAB))
      self.selA= myw.MywxMenu(self.tl, label='A:',
        helptext="""Scope A output """,
        defaultinx=defa, side=TOP, items=itemsA)
      self.selB= myw.MywxMenu(self.tl, label='B:',
        helptext="""Scope B output """,
        defaultinx=defb, side=TOP, items=itemsB)
      f1= myw.MywFrame(self.tl,side=TOP)
      SOFTLEDS(self.vb, f1)
    def selectAB(self):
      #print "selectAB:",self.selA.getEntry(),self.selB.getEntry()
      self.vb.io.execute("setAB("+ self.selA.getEntry()+","+
        self.selB.getEntry()+")", "out","no")
  return(st(vb))
def Counters(vb):
  st=counters.VMEcnts(counters.LVDSTcnts) 
  return

def ADC_Scan(vb):
  class st:
   def __init__(self,vb):
    import mywrl
    self.vb=vb
    self.tl=myw.NewToplevel("ADC")
##########################################################
    if(self.checkbcclock()):
     self.f1=myw.MywFrame(self.tl,side=TOP)
     cmd="rndtest(1)"
     output=self.vb.io.execute(cmd,log="no",applout="<>")
     xy=self.xy(output)
     #print 'xy=',xy
     max=self.finddelay(xy)

     self.c1=mywrl.Graph(self.f1,x0=0.,y0=0.,
           xgraph=32.,nxtick=8,ygraph=130.,nytick=10)
     self.c1.plot(xy)
     self.c1.xlabel(text='Delay [ns]')
     self.c1.ylabel(text='ADC')
     self.c1.pack()
     f2=myw.MywFrame(self.tl)
     b0=myw.MywButton(f2,label='Cancel',cmd=self.tl.destroy,side=LEFT,
        helptext='Close the window without accepting the value.')
     b2=myw.MywButton(f2,label="Measure",cmd=self.measure,side=LEFT,
        helptext="Measure points again. N scans will be measured (if diffrent from 0)")
     self.en=myw.MywEntry(f2,label="DELAY:",defvalue=str(max))
     self.butN=myw.MywEntry(f2,label="N:",defvalue="0",
        helptext="Number of scans (delay:0->31). 0:scan with 100 random values")
     b1=myw.MywButton(f2,label="OK",cmd=self.ok,side=LEFT,
       helptext="Close the window accepting the value")
     b3=myw.MywButton(f2,label='Save plot',cmd=self.save,side=LEFT,
        helptext="Save plot in directory $VMECFDIR/WORK") 
############################################################################     
   def checkbcclock(self):
    """
       Check id BC clock is present and ready.
    """
    cmd="getbcstatus()"
    output=self.vb.io.execute(cmd,log="no",applout="<>")
    #print "output=",output,' ',output[0]
    if (output[0] != '0x2'):
       myw.MywError(errmsg="BC clock is not present, staus="+output[0])
       self.tl.destroy()
       return 0
    return 1
   def save(self):
    """
       Save the postscript file of the plot to WORK directory.
    """
    fn=os.path.join(os.environ['VMECFDIR'],
        "WORK","adc.ps")
    rc=self.c1.postscript(file=fn)
    if rc is not '':
     myw.MywError(errmsg="Directory WORK does not exist.")
     print "rc=",rc,len(rc)
   def measure(self):
    """
      Measure and plot points again.
    """
    import mywrl
    if(self.c1):
      self.c1.destroy()
      self.c1=None
    if(self.checkbcclock()):
     cmd="rndtest("+ self.butN.getEntry()+ ")"
     output=self.vb.io.execute(cmd,log="no",applout="<>")
     xy=self.xy(output)
     max=self.finddelay(xy)
     self.c1=mywrl.Graph(self.f1,x0=0.,y0=0.,xgraph=32.,nxtick=8,
                   ygraph=130.,nytick=10)
     self.c1.plot(xy)
     self.c1.xlabel(text='Delay [ns]')
     self.c1.ylabel(text='ADC')
     self.c1.pack()
     self.en.setEntry(str(max))
   def ok(self):
    """
       Accept delay value and destroy ADC window.
    """
    delay=self.en.getEntry()
    cmd="setbcdelay("+delay+")"
    self.vb.io.execute(cmd)
    #print 'Value accepted ',delay
    self.setbcdefault(delay)
    self.tl.destroy()
   def setbcdefault(self, bcd):
     mn= os.path.join(os.environ['VMECFDIR'],"CFG","ltu","init.mac")
     mf= open(mn); lines= mf.readlines(); mf.close()
     i=0
     for l in lines:
       funcsp= string.split(l,"(")
       #print ":",funcsp
       if funcsp[0]=="setbcdelay":
         lines[i]= funcsp[0]+"("+bcd+")\n"
         #print i,":",lines[i]
       i=i+1
     #print lines
     mf= open(mn,'w'); mf.writelines(lines); mf.close()
     self.vb.io.thds[0].write("BC delay "+bcd+" Stored in CFG/ltu/init.mac file\n")
   def xy(self,output):
    """
       Transforms self.vb.io output (list) to 2-tuples.

       2 tuples corresponds to (x,y)=(delay,ADC).There
       may be many points with the same x.
       Convert dtrings to floats.
    """
    #output[-2] -average in micsec waiting for PLL unlock
    #output[-1] -average in micsec waiting for PLL lock
    self.vb.io.write("average unlock wait:%s average total lock wait:%s\n"
      %(output[-2],output[-1]))
    ll=len(output)-2
    if ll:
      xy=[]
      for i in range(0,ll,2):
        xynow=(float(output[i]),float(output[i+1]))
        flag=1
        for j in xy:
          if (j == xynow):
           flag=0
        if flag:
          xy.append(xynow)
    else:
       xy=None
    return xy
   def finddelay(self,xy):
     """
        Try to find delay from measured data.
        This version works only with 2 mirror curves.
     """
     if xy:
      listx,listy=self.sort(xy)
      #print 'listx=',listx
      #print 'listy=',listy
      numof0,numof1,numof2=self.evaluate(listx,listy)
      #print "numof0,numof1,numof2 ",numof0,numof1,numof2
      delay=None
      if(numof2 > 18):
        delay=self.find2(listx,listy)
        print "delay=",delay
      return delay
     else:
      return None
   def sort(self,xy):
     """
        1.) Find x  2.) For every x find all y
     """
     xy.sort()
     #print xy
     x0=xy[0][0]   # x of first tuple
     listy=[]      # list of list of y values for given x
     listx=[]      # list of x values
     ll=[]
     for i in xy:
       if(i[0] == x0):      # change of x
         ll.append(i[1])
       else:
         listy.append(ll)
         listx.append(x0)
         ll=[]
         ll.append(i[1])
         x0=i[0]
     listy.append(ll)
     listx.append(x0)
     return listx,listy
   def evaluate(self,listx,listy):
    """
       Find numberof delays with 0,1 and more than 1 entries:
       numof0,numof1,numof2. This will be used for the decision
       how to find delay.
    """
    numof0,numof1,numof2=0,0,0
    for i in range(0,32,1): #i is never 32
      if listx.count(i):
        j=listx.index(i)
        ll=len(listy[j])
        if ll == 1: numof1=numof1+1
        else: numof2=numof2+1
      else:
        numof0=numof0+1
    return numof0,numof1,numof2
   def find2(self,listx,listy):
     j,gmax,jmax=0,0,0
     for i in listy:
       numofy=len(i)    # number of points for given delay
       _min,_max=1.e23,-1.e23
       if numofy>1:
          for l in i:
            if(_min>l): _min=l
            if(_max<l): _max=l
   #       print listx[j],_max-_min
   #    elif numofy == 1:
   #       if( (i[0] == 0) | (i[0] >128)): 
   #	     _max,_min=129,0 

       if( (_max-_min) > gmax):
           gmax=_max-_min
           jmax=j
       j=j+1
     return jmax
  st(vb)    
  return None
#------------------------------------------------------- LVDST 
def LVDSTfun(vb):
  class st:
   def __init__(self,vb):
    import mywrl
    print "Here i am"
    self.vb=vb
  st(vb)    
  return None
#------------------------------------------------------- LVDST 2
def Window(vb):
  class st:
   def __init__(self,vb):
    import mywrl
    print "LVDST"
    self.vb=vb
#---------------
    self.tl=myw.NewToplevel("LVDST")
##########################################################
    if(self.checkbcclock()):
     self.f1=myw.MywFrame(self.tl,side=TOP)
     #cmd="rndtest(1)"
###########################################################################
     cmd="Find_Window(0,31,1,0,1,'s')"
     output=self.vb.io.execute(cmd,log="no",applout="<>")
     xy=self.xy(output)
     print 'xy=',xy
     #max=self.finddelay(xy)
     self.c1=mywrl.Graph(self.f1,x0=0.,y0=0.,
           xgraph=32.,nxtick=8,ygraph=20.,nytick=10)
     self.c1.plot(xy)
     self.c1.xlabel(text='Delay [ns]')
     self.c1.ylabel(text='errors')
     self.c1.pack()
     f2=myw.MywFrame(self.tl)
     b0=myw.MywButton(f2,label='Cancel',cmd=self.tl.destroy,side=LEFT,
        helptext='Close the window without accepting the value.')
     b1=myw.MywButton(f2,label="OK",cmd=self.ok,side=LEFT,
       helptext="Close the window accepting the value")
     b3=myw.MywButton(f2,label='Save plot',cmd=self.save,side=LEFT,
        helptext="Save plot in directory $VMECFDIR/WORK") 
############################################################################    
   def checkbcclock(self):
    """
       Check id BC clock is present and ready.
    """
    cmd="getbcstatus()"
    output=self.vb.io.execute(cmd,log="no",applout="<>")
    #print "output=",output,' ',output[0]
    if (output[0] != '0x2'):
       myw.MywError(errmsg="BC clock is not present, staus="+output[0])
       self.tl.destroy()
       return 0
    return 1
   def save(self):
    """
       Save the postscript file of the plot to WORK directory.
    """
    fn=os.path.join(os.environ['VMECFDIR'],
        "WORK","windowFinding.ps")
    rc=self.c1.postscript(file=fn)
    if rc is not '':
     myw.MywError(errmsg="Directory WORK does not exist.")
     print "rc=",rc,len(rc)
   def measure(self):
    """
      Measure and plot points again.
    """
    import mywrl
    if(self.c1):
      self.c1.destroy()
      self.c1=None
    if(self.checkbcclock()):
     cmd="rndtest("+ self.butN.getEntry()+ ")"
     output=self.vb.io.execute(cmd,log="no",applout="<>")
     xy=self.xy(output)
     max=self.finddelay(xy)
     self.c1=mywrl.Graph(self.f1,x0=0.,y0=0.,xgraph=32.,nxtick=8,
                   ygraph=130.,nytick=10)
     self.c1.plot(xy)
     self.c1.xlabel(text='Delay [ns]')
     self.c1.ylabel(text='ADC')
     self.c1.pack()
     self.en.setEntry(str(max))
   def ok(self):
    """
       Accept delay value and destroy ADC window.
    """
    delay=self.en.getEntry()
    cmd="setbcdelay("+delay+")"
    self.vb.io.execute(cmd)
    #print 'Value accepted ',delay
    self.setbcdefault(delay)
    self.tl.destroy()
   def setbcdefault(self, bcd):
     mn= os.path.join(os.environ['VMECFDIR'],"CFG","ltu","init.mac")
     mf= open(mn); lines= mf.readlines(); mf.close()
     i=0
     for l in lines:
       funcsp= string.split(l,"(")
       #print ":",funcsp
       if funcsp[0]=="setbcdelay":
         lines[i]= funcsp[0]+"("+bcd+")\n"
         #print i,":",lines[i]
       i=i+1
     #print lines
     mf= open(mn,'w'); mf.writelines(lines); mf.close()
     self.vb.io.thds[0].write("BC delay "+bcd+" Stored in CFG/ltu/init.mac file\n")
   def xy(self,output):
    """
       Transforms self.vb.io output (list) to 2-tuples.

       2 tuples corresponds to (x,y)=(delay,ADC).There
       may be many points with the same x.
       Convert dtrings to floats.
    """
    #output[-2] -average in micsec waiting for PLL unlock
    #output[-1] -average in micsec waiting for PLL lock
    self.vb.io.write("average unlock wait:%s average total lock wait:%s\n"
      %(output[-2],output[-1]))
    ll=len(output)-2
    if ll:
      xy=[]
      for i in range(0,ll,2):
        xynow=(float(output[i]),float(output[i+1]))
        flag=1
        for j in xy:
          if (j == xynow):
           flag=0
        if flag:
          xy.append(xynow)
    else:
       xy=None
    return xy
   def finddelay(self,xy):
     """
        Try to find delay from measured data.
        This version works only with 2 mirror curves.
     """
     if xy:
      listx,listy=self.sort(xy)
      #print 'listx=',listx
      #print 'listy=',listy
      numof0,numof1,numof2=self.evaluate(listx,listy)
      #print "numof0,numof1,numof2 ",numof0,numof1,numof2
      delay=None
      if(numof2 > 18):
        delay=self.find2(listx,listy)
        print "delay=",delay
      return delay
     else:
      return None
   def sort(self,xy):
     """
        1.) Find x  2.) For every x find all y
     """
     xy.sort()
     #print xy
     x0=xy[0][0]   # x of first tuple
     listy=[]      # list of list of y values for given x
     listx=[]      # list of x values
     ll=[]
     for i in xy:
       if(i[0] == x0):      # change of x
         ll.append(i[1])
       else:
         listy.append(ll)
         listx.append(x0)
         ll=[]
         ll.append(i[1])
         x0=i[0]
     listy.append(ll)
     listx.append(x0)
     return listx,listy
   def evaluate(self,listx,listy):
    """
       Find numberof delays with 0,1 and more than 1 entries:
       numof0,numof1,numof2. This will be used for the decision
       how to find delay.
    """
    numof0,numof1,numof2=0,0,0
    for i in range(0,32,1): #i is never 32
      if listx.count(i):
        j=listx.index(i)
        ll=len(listy[j])
        if ll == 1: numof1=numof1+1
        else: numof2=numof2+1
      else:
        numof0=numof0+1
    return numof0,numof1,numof2
   def find2(self,listx,listy):
     j,gmax,jmax=0,0,0
     for i in listy:
       numofy=len(i)    # number of points for given delay
       _min,_max=1.e23,-1.e23
       if numofy>1:
          for l in i:
            if(_min>l): _min=l
            if(_max<l): _max=l
   #       print listx[j],_max-_min
   #    elif numofy == 1:
   #       if( (i[0] == 0) | (i[0] >128)): 
   #	     _max,_min=129,0 

       if( (_max-_min) > gmax):
           gmax=_max-_min
           jmax=j
       j=j+1
     return jmax
#----------------
  st(vb)    
  return None

