#!/usr/bin/python
from Tkinter import *
import string
import myw,trgglobs

COLOR_SHARED="#cc66cc"
class sdg1:
  def __init__(self,sdgcls, name):
    self.sdgcls= sdgcls
    self.name= name
    self.e= myw.MywEntry(sdgcls.entriesfr,label=name,side=TOP,
      width=6,defvalue=sdgcls.sdgs[name], bind="lr",
      cmdlabel=self.update,
      helptext="""downscaling factor in %
0<dsf<100 """)
    #def destroyEntry(self):
  def update(self,enval=None):
    #print "update:",enval
    if enval != self.sdgcls.sdgs[self.name]:
      self.sdgcls[self.name]= string.strip(enval)
class TrgSDG:
  err="n% (where 0<n<100) expected"
  def __init__(self):
    self.sdgs={}   # "name":3.4%
    self.firstclass={}   # 1..NCLS   (i.e. int),  0: if not used
    self.tlw=None  # not displayed
  def find(self, name):
    if self.sdgs.has_key(name):
      return self.sdgs[name]
    return None
  def setl0prsdg(self, name, firstc):
    self.firstclass[name]= firstc
  def getl0prsdg(self, name):
    if self.sdgs.has_key(name):
      return self.firstclass[name]
    return None
  def add(self, name, dsf):
    #print "syncdg adding:",name,dsf,self.sdgs
    #import pdb ; pdb.set_trace()
    if self.sdgs.has_key(name):
      return "%s already defined"%name
    elif len(name)>23:   # see limit in ctplib/Tparttion.c
      return "%s too long:%d cahrs (max length: 23)"%(name,len(name))
    else:
      if len(self.sdgs)>=trgglobs.NCLS:
        return "too many definitions of sync. downscaling groups"
    if dsf=="": 
      return TrgSDG.err
    if dsf[-1]!="%": 
      return TrgSDG.err+ ' instead of:'+dsf+' in SDG named:'+name
    try:
      fn= float(dsf[:-1])
    except:
      return TrgSDG.err
    if (fn<0.) or (fn>100.):
      return TrgSDG.err+ " but n="+ dsf[:-1]
    #print "syncdg2 adding:",name,dsf,self.sdgs
    self.sdgs[name]= dsf
    self.firstclass[name]= 0
    #print "syncdg added:",name,dsf
    if self.tlw!=None:
      self.entries.append(sdg1(self, name))
    return None
  def show(self):
    if self.tlw!=None:
      myw.RiseToplevel(self.tlw); return
    self.tlw= myw.NewToplevel("SDG", self.hide, bg= COLOR_SHARED) 
    # new sdg:
    self.addfr= myw.MywFrame(self.tlw,side=TOP)
    addbut= myw.MywButton(self.addfr, label="Add", side=RIGHT, cmd=self.addsdg)
    self.sdgname= myw.MywEntry(self.addfr,label="", defvalue="nm",
      width=8, bind="lr", helptext="SDG name")
    self.dsf= myw.MywEntry(self.addfr, label="dsf:",width=6, bind="lr",
      helptext="Downscaling factor in %, 0<dsf<100. E.g. 23.3%")
    self.entries=[]
    self.entriesfr= myw.MywFrame(self.tlw,side=BOTTOM)
    for n in self.sdgs.keys():
      print("%s: %s"%(n, self.sdgs[n]))
      self.entries.append(sdg1(self, n))
  def addsdg(self, event=None):
    #print "addsdg:", self.sdgname.getEntry(), self.dsf.getEntry()
    rc= self.add(string.strip(self.sdgname.getEntry()), 
      string.strip(self.dsf.getEntry()))
    if rc!=None:
      print("Error:"+rc)
  def hide(self, event=None):
    #print "hide:", event
    self.tlw.destroy()
    self.tlw=None
    # +destroy:
    #self.entries[]
    #self.entriesfr 
    # addbut sdgname dsf addfr
  def save(self,outfile=None, SDG=""):
    # save: will save only used SDG groups 
    # (i.e. parted.TrgPartition.findfirst() has to be invoked before
    # to get self.firstclass dictionary updated
    from myw  import frommsL0pr
    for n in self.sdgs.keys():
      hexarep= "0x%x"%int(frommsL0pr('', self.sdgs[n]))
      if outfile==None:
        print("%s: %s/%s 1.class:%d"%
          (n, self.sdgs[n], hexarep, self.firstclass[n]))
      else:
        if self.firstclass[n]!=0:  
          if SDG=="":   # .partition file, i.e. symvalue
            outfile.write("%s%s %s\n"%(SDG, n, self.sdgs[n]))
          else:
            outfile.write("%s%s %s\n"%(SDG, n, hexarep))
def main():
  f= Tk()
  #fr= myw.MywFrame(f,side=BOTTOM)
  a= TrgSDG()
  a.show()
  f.mainloop()
  a.save()

if __name__ == "__main__":
    main()

