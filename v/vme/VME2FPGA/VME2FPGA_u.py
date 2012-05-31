#file VME2FPGA_u.py
#from Tkinter import *
from myw import *
from mywrl import *
#-----------------------------------------------------
def MonitLoad(vb):
  class st:
   def __init__(self,vb):
    self.vb=vb
    self.tl=Toplevel(vb.master)
    self.tl.title("VME2FPGA")
##########################################################
    self.f1=MywFrame(self.tl,side=TOP)

    self.c1=Graph(self.f1,x0=0.,y0=0.,
           xgraph=100.,nxtick=10,ygraph=2.,nytick=2)
    #self.c1.plot(xy)
    self.c1.xlabel(text='Time')
    self.c1.ylabel(text='Signals')
    self.c1.pack()
    f2=MywFrame(self.tl)
    b0=MywButton(f2,label='Cancel',cmd=self.tl.destroy,side=LEFT,
        helptext='Close the window without accepteng the value.')
    b2=MywButton(f2,label="Measure",cmd=self.measure,side=LEFT,
        helptext="Measure points again.")       
    b3=MywButton(f2,label='Save plot',cmd=self.save,side=LEFT,
        helptext="Save plot in directory $VMEWORKDIR/WORK") 
    self.b4=MywEntry(f2,label="Ntimes",side=LEFT,defvalue="100",
        helptext="How many times the signals are measured. Going above 1000 is slow.")
    self.b5=MywEntry(f2,label="T scale",side=LEFT,defvalue="400000",
        helptext="Time scale:0 - minimum time step about 1 us; 400000 - timestep about 2.35 ms  ")
############################################################################       
   def checkRYBU(self):
    """
       Check if FPGA is ready.
    """
    cmd="GetStatusFPGA()"
    output=self.vb.io.execute(cmd,log="no",applout="<>")
    #print "output=",output,' ',output[0]
    if (output[0] != '0'):
       MywError(errmsg="FPGA is not READY, status="+output[0])
       self.tl.destroy()
       return 0
    return 1
   def save(self):
    """
       Save the postscript file of the plot to WORK directory.
    """
    fn=os.path.join(os.environ['VMEWORKDIR'],
        "WORK","FPGAmon.ps")
    rc=self.c1.postscript(file=fn)
    if rc is not '':
     MywError(errmsg="Directory WORK does not exist.")
     print "rc=",rc,len(rc)
   def measure(self):
    """
      Measure and plot points again.
    """
    if(self.c1):
      self.c1.destroy()
      self.c1=None
    if(self.checkRYBU()):
     ntimes=self.b4.getEntry();
     tscale=self.b5.getEntry();
     cmd="MonitorLoad("+ntimes+","+tscale+")"
     #output=self.vb.io.execute(cmd,log="out",applout="<>")
     output=self.vb.io.execute(cmd,log="no",applout="<>")
     xmax=float(ntimes)
     xtick=10.
     if(xmax < 10.): xtick=xmax
     self.c1=Graph(self.f1,x0=0.,y0=0.,xgraph=xmax,nxtick=xtick,
                   ygraph=13.,nytick=13.)
     
     xy=self.xy(output,1)
     self.c1.plot(xy)
     self.c1.create_txt((xmax/20.,1.5),"RY/BU")

     xy=self.xy(output,2)
     self.c1.plot(xy,col="blue") 
     self.c1.create_txt((xmax/15.,3.5),"nSTATUS")

     xy=self.xy(output,3)
     self.c1.plot(xy)
     self.c1.create_txt((xmax/12.,5.5),"CONF_DONE")

     xy=self.xy(output,4)
     self.c1.plot(xy,col="blue")
     self.c1.create_txt((xmax/12.,7.5),"INIT_DONE")

     xy=self.xy(output,5)
     self.c1.plot(xy)
     self.c1.create_txt((xmax/15.,9.5),"nCONFIG")

     xy=self.xy(output,6)
     self.c1.plot(xy,col="blue")
     self.c1.create_txt((xmax/20.,11.5),"DCLK")

     time=self.timeofmeasurement(output)*1000./xmax
     timeunit="1 unit = " +str(time)+"usecs"
     self.c1.create_txt((xmax/2.,0.5),timeunit)

     self.c1.xlabel(text='Time')
     self.c1.ylabel(text='Signals')
     self.c1.pack()
   def timeofmeasurement(self,output):
    """
       Returns the time of the measurement as the last parameter
       in output
    """
    ll=len(output)
    return float(output[ll-1])
   def xy(self,output,signal):
    """
       Transforms self.vb.io output (list) to 2-tuples.
       2 tuples corresponds to (x,y)=(time,0 or 1).
       There are 6 signals: 1=RY/BU ....
       Converts strings to floats.
    """

    #print output
    ll=len(output)-1
    if ll:
      xy=[]
      for i in range(0,ll,7):
        xynow=(float(output[i]),float(output[i+signal])+2.*signal-1.)
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
    for i in range(0.,32.,1.): #i is never 32
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
