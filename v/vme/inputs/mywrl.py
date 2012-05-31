#File: rl.py

from Tkinter import *
from myw import *
from math import *
#--------------------------------------------------------------
class Body:
 """ 
    Basic class for graph.

    xy - tuple of input values.
    Methods:
    minmax - calculates minimum in x=minx; maximum in x =maxx;
             minimum in y =miny; maximum in y =maxy;
    getminx - return minx
    getminy - return miny
    getmaxx - return maxx
    getmaxy - return maxy
 """
 def __init__(self,xy=None):
  self.minx,self.miny,self.maxx,self.maxy=self.minmax(xy)
  self.xy=xy
 def minmax(self,xy):
  minx,miny,maxx,maxy=None,None,None,None
  if xy:
   minx,miny,maxx,maxy=1.e23,1.e23,-1.e23,-1.e23
   for i in xy:
     if minx > i[0]: minx=i[0]
     if miny > i[1]: miny=i[1]
     if maxx < i[0]: maxx=i[0]
     if maxy < i[1]: maxy=i[1]
  return minx,miny,maxx,maxy
 def getminx(self):
  return self.minx 
 def getmaxx(self):
  return self.maxx
 def getminy(self):
  return self.miny
 def getmaxy(self):
  return self.maxy  
class Mycanvas(Canvas):
 """
    Basic class for graph. It comes from canvas and defines
    similar methods like canvas: create_l,create_box,create_txt ...
    The Mycanvas uses as input graph coordinates.

    Two frames are used:
     -win frame: frame of the window
     -graph frame: frame of the graph

    xsize,ysize - window size on screen. (I am not sure in which units)
    The origin = (0,0) is in upper left corner.
    This defines so called win frame.

    The other frame is graph frame, i.e. frame of points we want to plot.
    (x0,y0) - the point in graph frame which we want to plot
              in lower left corner of the window
    xgraph,ygraph - graph range in x and y which is plotted
    Methods:
             grf2win
             create_rect,create_l,create_box,create_txt.
 """
 def __init__(self,parent,x0,y0,xgraph,ygraph,xsize=600.,ysize=400.):
  Canvas.__init__(self,parent,width=xsize,height=ysize)

#  xoffset - distance between the edge of the window and xaxis of the graph
#  yoffset - disrance between the edge of the window and yaxis
  xoffset=xsize*0.1
  yoffset=ysize*0.1
# xsizewin - size of the graph inside the window
# ysizewin
  xsizewin=xsize-2.*xoffset
  ysizewin=ysize-2.*yoffset
  
  self.xoff=xoffset
  self.yoff=yoffset
  self.x0=float(x0)
  self.y0=float(y0)
# x0win - coordinates of the lower left corner of the graph in win frame
# y0win
  self.x0win=float(xoffset)
  self.y0win=float(yoffset+ysizewin)
# xscal - scaling between window and graph frame
# yscal
  self.xscal=xsizewin/xgraph
  self.yscal=ysizewin/ygraph
 def grf2win(self,bod=None):
  """
     Transform the graph coordinates to window coordinates
  """
  #print 'bod:',bod
  if bod:
   bodwin0 =( self.x0win +  (float(bod[0]) - self.x0)*self.xscal)  
   bodwin1 =( self.y0win -  (float(bod[1]) - self.y0)*self.yscal)
  return (bodwin0,bodwin1) 
 def create_rect(self,b1,b2,colour):
  """
     Create rectangle, input in graph coordinates.
     Rectangle plotted at lower left corner.
  """
  b1w=self.grf2win(b1)
  b2w=self.grf2win(b2)
  self.create_rectangle(b1w,b2w,fill=colour)
 def create_box(self,b1,colour):
  """
     Create box, input in graph coordinates.
     Rectangle plotted at center.
  """ 
  b1w=self.grf2win(b1)
  b1p=(b1w[0]-self.xoff/20.,b1w[1]-self.yoff/20.) # 10 -size of symbol
  b2w=(b1w[0]+self.xoff/20.,b1w[1]+self.yoff/20.)
  self.create_rectangle(b1p,b2w,fill=colour)
 def create_l(self,b1,b2):
  """
     Create line, input in graph coordinates.
  """
  b1w=self.grf2win(b1)
  b2w=self.grf2win(b2)
  self.create_line(b1w,b2w)
 def create_txt(self,b1,text=None):
  """
     Create text, input in graph coordinates.
  """
  b1w=self.grf2win(b1)
  item=self.create_text(b1w,text=text)
  return item
class Graph(Body,Mycanvas):
 """
    Creates a root window with plot of inputb points.
    Input: xy - tuple of plotted points.
    Methods:
     axis - draw rectangle of axis.
     xticks,yticks - draw ticks and numbers
     xlabel,ylabel - draw labels
     plot - draw the input points.
     log scale allowed:xgraphl,ygraphl , ...
 """
 def __init__(self,parent,xy=None,x0=None,y0=None,xgraph=None,ygraph=None,
                          nxtick=10.,nytick=10.,xlog='no',ylog='no',xsize=600,ysize=400):
   """
      x0,y0,xgraph,ygraph should be all defined or all not defined
   """
   xyl=[]
   if ylog=='yes':
    for i in xy:
      xyl.append((i[0],log10(i[1])))
   self.xlog=xlog
   self.ylog=ylog
   self.body=Body(xyl)
   Body.__init__(self,xy=xy)
   if(xgraph == None):
     xgraph = self.getmaxx()-self.getminx()
     ygraph = self.getmaxy()-self.getminy()
     x0 = self.getminx()
     y0 = self.getminy()
     xgraphl=xgraph
     ygraphl=ygraph
     x0l=x0
     y0l=y0
     if ylog=='yes':
       xgraphl = self.body.getmaxx()-self.body.getminx()
       ygraphl = self.body.getmaxy()-self.body.getminy()
       x0l = self.body.getminx()
       y0l = self.body.getminy()
   else:     
     xgraphl = xgraph
     ygraphl = ygraph
     x0l=x0
     y0l=y0
     if ylog == 'yes':
       ygraphl = log10(y0+ygraph)-log10(y0)
       y0l=log10(y0)
   if(ygraphl == 0.): ygraphl=1.
   self.xg=xgraphl
   self.yg=ygraphl
   self.y0=y0l
   self.x0=x0l
   print 'x0l,y0l,xgraphl,yraphl',x0l,y0l,xgraphl,ygraphl
   print 'x0,y0,xgraph,yraph',x0,y0,xgraph,ygraph
   if nxtick == 10. and xgraph > 10.0:
      dielik=int(xgraph/10.)
      nxtick = xgraph/dielik
   print 'nxtick,nytick',nxtick,nytick
   Mycanvas.__init__(self,parent,x0=x0l,y0=y0l,xgraph=xgraphl,ygraph=ygraphl,xsize=xsize,ysize=ysize)
   self.axis(x0l,y0l,xgraphl,ygraphl)
   self.xticks(x0,y0,xgraph,ygraph,x0l,y0l,xgraphl,ygraphl,nxtick)
   self.yticks(x0,y0,xgraph,ygraph,nytick)
 def axis(self,x0,y0,xgraph,ygraph):
  """
     Draw the rectangle of axis, i.e. box where points are plotted.
     No change for log
  """
  self.create_l((x0,y0),(x0+xgraph,y0))
  self.create_l((x0+xgraph,y0),(x0+xgraph,y0+ygraph))
  self.create_l((x0+xgraph,y0+ygraph),(x0,y0+ygraph))
  self.create_l((x0,y0+ygraph),(x0,y0))
 def xticks(self,x0,y0,xgraph,ygraph,x0l,y0l,xgraphl,ygraphl,nxtick):
  """
     Draw the ticks and numbers. Tick length and number size fixed.
     No change for log
  """
  i=x0
  dielik=xgraph/nxtick
  #print 'dielik',dielik
  while(i<(x0+xgraph+dielik/2.)):
   b1=(i,y0l)
   b2=(i,y0l-ygraphl/50.)   # tick length
   #print 'b1,b2',b1,b2
   self.create_l(b1,b2)
   b1=(i,y0l-ygraphl/20.)   # number size
   j= float(int(i*10))/10.
   self.create_txt(b1,text=str(j))
   i=i+dielik
 def yticks(self,x0,y0,xgraph,ygraph,nytick):
  """
    Draw the ticks and numbers. Tick length and number size fixed.
    Inputs in nonlog scale
  """
  dielik=ygraph/nytick
  #for i in range(y0,y0+ygraph+dielik,dielik):
  i=y0
  while(i<(y0+ygraph+dielik/2.)):
   if self.ylog =='yes':
    b1=(x0,log10(i))
    b2=(x0-xgraph/50.,log10(i))   # tick length
    self.create_l(b1,b2)
    b1=(x0-xgraph/20.,log10(i))   # number size
    j= float(int(i*10))/10.
    self.create_txt(b1,text=str(j))
   else:
    b1=(x0,i)
    b2=(x0-xgraph/50.,i)   # tick length
    self.create_l(b1,b2)
    b1=(x0-xgraph/20.,i)   # number size
    j= float(int(i*10))/10.
    self.create_txt(b1,text=str(j))
   i=i+dielik
 def plot(self,xy,colour):
  """
     Plot the points.
  """
  if xy:
   for i in xy:
    if self.ylog =='yes': j=(i[0],log10(i[1]))
    else: j = i
    self.create_box(j,colour)
 def xlabel(self,text=None,width=None):
   """
      Draw xlabel. 
   """
   b1=(self.x0+self.xg,self.y0-self.yg/13.)
   item=self.create_txt(b1,text=text)
   self.itemconfig(item,anchor=NE)
   self.itemconfig(item,font="Times 12 bold")
   #if width: self.itemconfig(item,width=width)
 def ylabel(self,text=None,width=None):
   """
      Draw y-label.
   """
   b1=(self.x0-self.xg/13.,self.y0+self.yg)
   item=self.create_txt(b1,text=text)
   self.itemconfig(item,anchor=NE)
   self.itemconfig(item,font="Times 12 bold")
   self.itemconfig(item,width=1.)
 def title(self, text=None):
   b1=(self.x0+self.xg-self.xg/13.,self.y0+self.yg+self.yg/13)
   item=self.create_txt(b1,text=text)
########################################################################
class MywRadio2(Radiobutton,MywHelp):
  # Radiobutton with help
  def __init__(self,master,side=LEFT,text="Radio2",variable=None,value=None,helptext=None,command=None):
      Radiobutton.__init__(self,master,text=text,variable=variable,value=value,command=command)
      Radiobutton.config(self,indicatoron=3)
      Radiobutton.pack(self,side=side)
      if helptext: MywHelp.__init__(self,master, helptext)

class MywRadioMany:
  def __init__(self, master=None,
    text="txt",variable="0",value="0",
    items=(("text1","1"),("text2","2")),
    helptext=None,side=LEFT):
    self.radf=Frame(master, borderwidth=1,relief=RAISED)
    self.radf.pack(fill='y',expand='yes',side=side)
    self.radf.rb1=MywRadio2(self.radf,text=text,variable=variable,value=value,helptext=helptext)
    self.radf.rb1.config(indicatoron=0)
    self.radf.rb1.pack()
#
    self.posval= StringVar(); self.posval.set(items[0][1])
    self.radf.choices=[]
    for ch in items:
      self.radf.choices.append( Radiobutton(self.radf,
        text=ch[0], variable=self.posval, value=ch[1]))
      self.radf.choices[-1].pack(fill='x',expand='yes')

  def getEntry(self):
    #print "radio:", self.posval.get()
    return(self.posval.get())
  def setEntry(self,var=' '):
    self.posval.set(var)
#####################################################################

class MywButtonRadio(Button, MywHelp):
  """
  label -used as positional parameter sometimes (2nd one)
  myw button modified to have two states  
  """
  def __init__(self, master, label='label',  
    bg=None,side=None,anchor=None,helptext=None,expand='no',width=2):
    Button.__init__(self,master,text=label, command=self.setstate, 
            bg=bg,width=width,state=ACTIVE,relief=SUNKEN)
    abg=Button.cget(self,'activebackground'); pbg=Button.cget(self,'background')
    #print "MywButton:",abg,pbg
    Button.pack(self,side=side, expand=expand, fill='x', anchor=anchor)
    #if funcdescr and funcdescr[VmeBoard.NFUSAGE]:
    #  MywHelp.__init__(self,master,funcdescr[VmeBoard.NFUSAGE])
    if helptext: MywHelp.__init__(self,master, helptext)
  def setstate(self):
    state=self.cget('state')
    #print ACTIVE,NORMAL
    print "state= ",state
    if state == NORMAL:
       Button.configure(self,relief=SUNKEN)
       Button.configure(self,state=ACTIVE)
    elif state == ACTIVE:
       Button.configure(self,relief=RAISED)
       Button.configure(self,state=NORMAL)
  def disbale(self):
     print "disbale button"
class MywCheckButton(Checkbutton,MywHelp): 
   def __init__(self, master,label,side,labellength=2,helptext=None):
        spaces=labellength-len(label)
        #print 'spaces=',spaces
        if spaces > 0: label=spaces*' '+label
        self.var = IntVar()
        Checkbutton.__init__(self,master, text=label,width=2,
                        variable=self.var, command=self.cb)
        Checkbutton.pack(self,side=side,expand='no',fill='x')
        if helptext: MywHelp.__init__(self,master, helptext)
   def cb(self):
        pass
        #print "variable is", self.var.get()
####################################################################
if __name__ == "__main__":
  root=Tk()
#  e=ErrorDialog(root)
  #print 'log10=',log10(10)
  #xy=[(-5.,10.),(-3.,20.),(0.,30.),(6.,40.)]
  xy=[(-5,10),(-3,20),(0,30),(6,40)]
  #a=Body()
  #c=Graph(root,xy,x0=0,y0=1,xgraph=32,ygraph=150,nxtick=8,nytick=6,ylog='yes')
  c=Graph(root,xy,ylog='no')
  #c=Graph(root,x0=0,y0=0,xgraph=32,ygraph=128)
  c.plot(xy,"#eeff55")
  c.ylabel(text='lalal')
  c.xlabel(text='baba')
  c.pack()
#  r=MywRadio2(root)
  root.mainloop()

