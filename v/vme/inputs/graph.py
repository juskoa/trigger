#File: graph.py

from Tkinter import *

#--------------------------------------------------------------
class Body:
 """ Basic class for graph

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
 """
 def __init__(self,parent=None,xsize=800,ysize=600,
              x0=None,y0=None,xgraph=None,ygraph=None ):
  Canvas.__init__(self,parent,width=xsize,height=ysize)
  xoffset=xsize*0.1
  yoffset=ysize*0.1
  xsizewin=xsize-2.*xoffset
  ysizewin=ysize-2.*yoffset
  # 2 frames: win frame;grf frame
  # x0,y0 origin of grf frame in grf frame
  # (xoffset,yoffset_ysizewin) origin of grf frame in win frame
  self.xoff=xoffset
  self.yoff=yoffset
  self.x0=x0
  self.y0=y0
  self.x0win=xoffset
  self.y0win=yoffset+ysizewin
  self.xscal=xsizewin/xgraph
  self.yscal=ysizewin/ygraph
 def grf2win(self,bod=None):
  if bod:
   bodwin0 =( self.x0win +  (bod[0] - self.x0)*self.xscal)  
   bodwin1 =( self.y0win -  (bod[1] - self.y0)*self.yscal)
  return (bodwin0,bodwin1) 
 def create_rect(self,b1,b2):
  b1w=self.grf2win(b1)
  b2w=self.grf2win(b2)
  self.create_rectangle(b1w,b2w,fill="red")
 def create_box(self,b1):
  b1w=self.grf2win(b1(self,b1):
  b1w=self.grf2win(b1w[1]-self.yoff/10.) # 10 -size of symbol
  b2w=(b1w[0]+self.xoff/10.,b1w[1]+self.yoff/10.)
  self.create_rectangle(b1p,b2w,fill="red")
 def create_l(self,b1,b2):
  b1w=self.grf2win(b1)
  b2w=self.grf2win(b2)
  self.create_line(b1w,b2w)
 def create_txt(self,b1,text=None):
  b1w=self.grf2win(b1)
  self.create_text(b1w,text=text)
class Graph(Body,Mycanvas):
 """
 """
 def __init__(self,parent,xy=None,x0=None,y0=None,xgraph=None,ygraph=None,
                          nxtick=10,nytick=10):
  Body.__init__(self,xy=xy)
  if(xgraph == None):
    xgraph = self.getmaxx()-self.getminx()
    print "xgraph=",xgraph
  if(ygraph == None):
    ygraph = self.getmaxy()-self.getminy()
  if(x0 == None):
    x0 = self.getminx()
  if(y0 == None):
    y0 = self.getminy()
  Mycanvas.__init__(self,parent,x0=x0,y0=y0,xgraph=xgraph,ygraph=ygraph)
  self.axis(x0,y0,xgraph,ygraph)
  self.xticks(x0,y0,xgraph,ygraph,nxtick)
  self.yticks(x0,y0,xgraph,ygraph,nytick)
 def axis(self,x0,y0,xgraph,ygraph):
  self.create_l((x0,y0),(x0+xgraph,y0))
  self.create_l((x0+xgraph,y0),(x0+xgraph,y0+ygraph))
  self.create_l((x0+xgraph,y0+ygraph),(x0,y0+ygraph))
  self.create_l((x0,y0+ygraph),(x0,y0))
 def xticks(self,x0,y0,xgraph,ygraph,nxtick):
  dielik=xgraph/nxtick
  for i in range(x0,x0+xgraph+dielik,dielik):
   b1=(i,y0)
   b2=(i,y0-ygraph/50.)   # tick length
   self.create_l(b1,b2)
   b1=(i,y0-ygraph/20.)   # number size
   self.create_txt(b1,text=`i`)
 def yticks(self,x0,y0,xgraph,ygraph,nytick):
  dielik=ygraph/nytick
  for i in range(y0,y0+ygraph+dielik,dielik):
   b1=(x0,i)
   b2=(x0-xgraph/50.,i)   # tick length
   self.create_l(b1,b2)
   b1=(x0-xgraph/20.,i)   # number size
   self.create_txt(b1,text=`i`)   
 def plot(self,xy):
  for i in xy:
   self.create_box(i)
################################################################
#----------------------------------------------------------------
def callback(event):
 canvas=event.widget
 x=canvas.canvasx(event.x)
 y=canvas.canvasy(event.y)
 print canvas.find_closest(x,y)
 print 'canx,cany= ',x,y
 print 'winx,winy= ',event.x,event.y

#------------------------------------------------------------------
if __name__ == "__main__":
  root=Tk()
#  e=ErrorDialog(root)
  xy=[(10,10),(20,20),(30,30),(15,40)]
  a=Body()
  c=Graph(root,xy,x0=0,y0=0,xgraph=32,ygraph=150,nxtick=8)
  #c=Graph(root,xy)
  c.plot(xy)
  c.pack()
#  r=MywRadio2(root)
  root.mainloop()
