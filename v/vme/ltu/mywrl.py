#from __future__ import division
#from __future__ import print_function
#File: rl.py

#from future import standard_library
#standard_library.install_aliases()
from builtins import range
from past.utils import old_div
from builtins import object
from tkinter import *
from myw import *

#--------------------------------------------------------------
class Body(object):
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
 def __init__(self,parent,xsize=600,ysize=400,
              x0=None,y0=None,xgraph=None,ygraph=None ):
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
  self.x0=x0
  self.y0=y0
# x0win - coordinates of the lower left corner of the graph in win frame
# y0win
  self.x0win=xoffset
  self.y0win=yoffset+ysizewin
# xscal - scaling between window and graph frame
# yscal
  self.xscal=old_div(xsizewin,xgraph)
  self.yscal=old_div(ysizewin,ygraph)
 def grf2win(self,bod=None):
  """
     Transform the graph coordinates to window coordinates
  """
  if bod:
   bodwin0 =( self.x0win +  (bod[0] - self.x0)*self.xscal)  
   bodwin1 =( self.y0win -  (bod[1] - self.y0)*self.yscal)
  return (bodwin0,bodwin1) 
 def create_rect(self,b1,b2):
  """
     Create rectangle, input in graph coordinates.
     Rectangle plotted at lower left corner.
  """
  b1w=self.grf2win(b1)
  b2w=self.grf2win(b2)
  self.create_rectangle(b1w,b2w,fill="red")
 def create_box(self,b1):
  """
     Create box, input in graph coordinates.
     Rectangle plotted at center.
  """ 
  b1w=self.grf2win(b1)
  b1p=(b1w[0]-self.xoff/20.,b1w[1]-self.yoff/20.) # 10 -size of symbol
  b2w=(b1w[0]+self.xoff/20.,b1w[1]+self.yoff/20.)
  self.create_rectangle(b1p,b2w,fill="red")
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
 """
 def __init__(self,parent,xy=None,x0=None,y0=None,xgraph=None,ygraph=None,
                          nxtick=10,nytick=10):
   #if xy:
   Body.__init__(self,xy=xy)
   if(xgraph == None):
    xgraph = self.getmaxx()-self.getminx()
    print("xgraph=",xgraph)
   if(ygraph == None):
    ygraph = self.getmaxy()-self.getminy()
   if(x0 == None):
    x0 = self.getminx()
   if(y0 == None):
    y0 = self.getminy()
   self.xg=xgraph
   self.yg=ygraph
   Mycanvas.__init__(self,parent,x0=x0,y0=y0,xgraph=xgraph,ygraph=ygraph)
   self.axis(x0,y0,xgraph,ygraph)
   self.xticks(x0,y0,xgraph,ygraph,nxtick)
   self.yticks(x0,y0,xgraph,ygraph,nytick)
 def axis(self,x0,y0,xgraph,ygraph):
  """
     Draw the rectangle of axis, i.e. box where points are plotted.
  """
  self.create_l((x0,y0),(x0+xgraph,y0))
  self.create_l((x0+xgraph,y0),(x0+xgraph,y0+ygraph))
  self.create_l((x0+xgraph,y0+ygraph),(x0,y0+ygraph))
  self.create_l((x0,y0+ygraph),(x0,y0))
 def xticks(self,x0,y0,xgraph,ygraph,nxtick):
  """
     Draw the ticks and numbers. Tick length and number size fixed.
  """
  dielik=old_div(xgraph,nxtick)
  for i in range(x0,x0+xgraph+dielik,dielik):
   b1=(i,y0)
   b2=(i,y0-ygraph/50.)   # tick length
   self.create_l(b1,b2)
   b1=(i,y0-ygraph/20.)   # number size
   self.create_txt(b1,text=repr(i))
 def yticks(self,x0,y0,xgraph,ygraph,nytick):
  """
    Draw the ticks and numbers. Tick length and number size fixed.
  """
  dielik=old_div(ygraph,nytick)
  #print "yticks:",y0,ygraph,dielik
  for i in range(int(y0),int(y0+ygraph+dielik),int(dielik)):
   b1=(x0,i)
   b2=(x0-xgraph/50.,i)   # tick length
   self.create_l(b1,b2)
   b1=(x0-xgraph/20.,i)   # number size
   self.create_txt(b1,text=repr(i))   
 def plot(self,xy):
  """
     Plot the points.
  """
  if xy:
   for i in xy:
    self.create_box(i)
 def xlabel(self,text=None,width=None):
   """
      Draw xlabel. 
   """
   b1=(self.xg,0-self.yg/20.-self.yg/50.)
   item=self.create_txt(b1,text=text)
   self.itemconfig(item,anchor=NE)
   self.itemconfig(item,font="Times 12 bold")
   #if width: self.itemconfig(item,width=width)
 def ylabel(self,text=None,width=None):
   """
      Draw y-label.
   """
   b1=(0-self.xg/20.-self.xg/50.,self.yg)
   item=self.create_txt(b1,text=text)
   self.itemconfig(item,anchor=NE)
   self.itemconfig(item,font="Times 12 bold")
   self.itemconfig(item,width=1.)

########################################################################
class MywRadio2(Radiobutton,MywHelp):
  # Radiobutton with help
  def __init__(self,master,text="Radio2",variable=None,
      value=None,helptext=None):
      Radiobutton.__init__(self,master,text=text,variable=variable,value=value)
      Radiobutton.pack(self)
      if helptext: MywHelp.__init__(self,master, helptext)

class MywRadioMany(object):
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

####################################################################
if __name__ == "__main__":
  root=Tk()
#  e=ErrorDialog(root)
  xy=[(10,10),(20,20),(30,30),(15,40)]
  a=Body()
  #c=Graph(root,xy,x0=0,y0=0,xgraph=32,ygraph=150,nxtick=8)
  #c=Graph(root,xy)
  c=Graph(root,x0=0,y0=0,xgraph=32,ygraph=128)
  c.plot(xy)
  c.pack()
#  r=MywRadio2(root)
  root.mainloop()

