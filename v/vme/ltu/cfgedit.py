#!/usr/bin/env python
"""
usage:
  import cfgedit
  curdefs= cfgedit.Variables(vmeb=vb)
  curdefs.show()

commend line (debug):
cd $VMEWORKDIR
$VMECFDIR/ltu/cfgedit.py 

"""
from __future__ import print_function
from future import standard_library
standard_library.install_aliases()
from builtins import str
from builtins import range
from builtins import object
import sys, os.path
import myw
from tkinter import *
vb=None

def err(errmsg, warning=None):
  global vb
  if warning!=None:
    errwarn="Warning:"
  else:
    errwarn="ERROR:"
  if vb:
    vb.io.write(errwarn+errmsg+"\n")
  else:
    print(errwarn, errmsg)

class Comment(object):
  """ Comment line starts with '#' 
  """
  def __init__(self, text, error=None):
    self.text=text
    self.error=error
  def prt(self):
    print(self.text)
  def line(self):
    if self.error: text= "#ERROR: "+self.text
    else: text=self.text
    return text
  def readhw(self):
    err("Comment.readhw: should not be called!")
    pass
  def show(self, tw):
    if self.error: tag="ERROR"
    else: tag= None
    if self.text=='\n': return
    tw.insert("end", str(self.text), tag)
class Variable(object):
  """ if variable name starts with '!', it is
  SYSTEM variable, not visible in defaults editor -i.e.
  we do not want users to control it, but processed in readltuttcdb()
  """
  def __init__(self, name, text, defvalue=None, curvalue=None, hwupdate="yes"):
    if name[0]=='!':
      self.name= name[1:]
      self.type='system'
    else:
      self.name= name           # name field. If starting '!': SYSTEM variable
      self.type=None
    self.defvalue= defvalue   # value from Database
    self.curvalue= curvalue
    self.hwupdate= hwupdate   #None: variable does not exist in Memory (can't be updated)
    if text==None: text='\n'  # comment field
    self.text= text
  def isSYSTEM(self):
    if self.type=='system':
      return True
    else:
      return False
  def prt(self):
    print(self.name+"="+str(self.defvalue), str(self.curvalue))
  def line(self):
    rs= str(self.name) + '\t' + str(self.curvalue) + '\t' + str(self.text)
    return rs
  def show(self, tw):
    if self.isSYSTEM():   # do not show SYSTEM variables
      if self.defvalue != self.curvalue: 
        msg="System:%s memory:%s != db:%s"%(self.name, 
          self.curvalue,self.defvalue)
        err(msg,"warn")
    else:
      tw.insert("end", str(self.name) + '\t')
      if self.defvalue != self.curvalue: 
        tag="DIF"
      else: 
        tag= None
      tw.insert("end", str(self.curvalue) + '\t', tag)
      tw.insert("end", str(self.text))
  def readhw(self):
    global vb
    self.setcv(None)
    if vb:
      # get it from (shared) memory:
      #nvr=string.split(vb.io.execute("setOption(\"%s\", \"printvalue\")"%(self.name), log="NO"))[0]
      nvr=vb.io.execute("setOption(\"%s\", \"printvalue\")"%(self.name), log="NO").split()[0]
      #print "vb: not None nvr:%s:"%nvr
      #self.setcv(string.rstrip(nvr))
      self.setcv(nvr.rstrip())
    else:
      if not os.path.exists(Variables.HWNAME):
        err(Variables.HWNAME+" does not exist")
        return
      f= open(Variables.HWNAME);
      while 1:
        l= f.readline()
        if l=='': break
        #nvr= string.split(l,None,1)
        nvr= l.split(None,1)
        if nvr[0]==self.name: self.setcv(nvr[1].rstrip())
      f.close()
  def writehw(self):
    global vb
    if vb:
      cval= self.curvalue.strip('"')
      rc=vb.io.execute("setOption(\"%s\", \"%s\")"%\
        (self.name, cval),log="NO",applout="<>")
      #print "writehw: not None rc:",rc,type(rc)
      if rc[0] != '0':
        err(self.name+" incorrect, not updated")
        #vb.io.write('ERROR: '+self.name+" incorrect, not updated\n")
    else:
      err("Variable.writehw() should not be called with vb=None")
  def setcv(self, curvalue):
    self.curvalue= curvalue
class Variables(object):
  DBNAME="CFG/ltu/ltuttc.cfg"
  HWNAME="CFG/ltu/ltuttc.hw"
  def __init__(self, cfgname=DBNAME, vmeb=None):
    global vb
    vb= vmeb
    self.cfgname= cfgname
    self.f1=None    # not shown in the text widget
    self.fillFromDefaults()
    self.lastAction=None
    self.readhw()
    self.lastAction=1   #loaded from memory
    #self.prt()
  def myopen(self, rw="r"):
    global vb
    if vb!=None:
      if rw=="r":
        if vb.boardName in myw.DimLTUservers:
          vb.io.execute('getfile("'+self.cfgname+'")')
    self.f= open(self.cfgname, rw);
  def myclose(self, rw="r"):
    global vb
    if self.f:
      self.f.close()
      if vb!=None:
        if rw=="w":
          if vb.boardName in myw.DimLTUservers:
            vb.io.execute('putfile("'+self.cfgname+'")')
  def mygetline(self):
    if self.f:
      #l=string.lstrip(self.f.readline(),' \t')   #not NL
      l=self.f.readline().lstrip(' \t')   #not NL
    else:
      l=None
    return l
  def procLine(self, line):
    nvr=[] ; name=value=None ; rest='\n'
    try:
      nvr= line.split(None,2)
    except:
      #print "except:",name,":",value,":",rest,":"
      err("except:"+str(nvr))
    if len(nvr)>0: name= nvr[0]
    #if len(nvr)>1: value= string.strip(nvr[1],'"')
    if len(nvr)>1: value= nvr[1]
    if len(nvr)>2: rest= nvr[2]
    return name,value,rest
  def updateFromText(self, destination):
    """ 2 cases:
    1. "HW"
       updateFromText before writing to memory:
       - only variables marked 'updated' are updated in HW, 
         warning issued for 'not updated (i.e. not given in widget)'
         variables
    2. "DDB"
       updateFromText before writing DefaultsDB:
       - DefaultsDB is updated from default values, warning
         'varname value used for defaultsdb' issued
    Operation:
    A. create lines2 (from text), check variables for errors:
    - unknown variable
    - var. not defined, original definition used (see 2 cases above)
    B. lines= lines2
    """
    if (destination != "DDB") and (destination != "HW"):
      err("Internal: updateFromText(%s)",str(destination))
      return
    lines2=[]
    #f.write(self.textview.get("1.0","end"))
    endix= self.textview.index("end")
    ilineend= eval(endix.split('.')[0])
    #print "end:", endix, ilineend
    for iline in range(ilineend)[1:]:
      #l= string.rstrip(self.textview.get("%d.0"%iline, "%d.end"%iline))+'\n'
      l= self.textview.get("%d.0"%iline, "%d.end"%iline).rstrip()+'\n'
      #print "l:",l,":"
      if l[0]=='#' or l[0]=='\n':
        lines2.append(Comment(l)) ; continue
      name,value,rest= self.procLine(l)
      if rest==None: rest=''
      #check if exists
      ixvar= self.findVariable(name)
      #print "fv:",name,var
      if ixvar!=None:
        var= self.lines[ixvar]
        #if var.isSYSTEM():
        #  sname= '!'+var.name   # cannot happen (no SYSTEM in widget)
        #else:
        #  sname= var.name
        if destination=="DDB":
          defval= value; curval= var.curvalue;
        else:     # "HW"
          defval= var.defvalue; curval= value;
        lines2.append(Variable(name, rest,defvalue=defval, curvalue=curval))
        #print "added:",sname
        del self.lines[ixvar]
      else:
        if value==None: value=''
        lines2.append(Comment(name+' '+str(value)+' '+str(rest), error=1))
        err("-"+str(name)+"- -no such variable in DefaultsDB, line ignored")
    #print "selflines:",self.lines
    #check if all variables given:
    for misvar in self.lines:
      if isinstance(misvar, Comment): continue
      if not misvar.isSYSTEM():
        if destination=="DDB":
          err("Missing: "+ misvar.name+" defvalue:"+misvar.defvalue+
            "used for DefaultsDB")
        else:     # "HW"
          err("Variable "+ misvar.name+" missing, it is not updated in memory")
        lines2.append(Variable(misvar.name, misvar.text,defvalue=misvar.defvalue, 
          curvalue=misvar.curvalue, hwupdate=None))
      else:
        #print "info: SYSTEM var:%s (destination:%s)"%(misvar.name, destination)
        # always update memory if SYSTEM variable. Reason:
        # if changed in Database manually, this is the way, how to 
        # copy its value to memory:
        lines2.append(Variable('!'+misvar.name, misvar.text,defvalue=misvar.defvalue, 
          curvalue=misvar.curvalue, hwupdate='yes'))
    self.lines= lines2
    #print "updateFromText done:"; self.prt()
  def fillFromDefaults(self):
    """ Input: database file.
    Operation:
    Fill self.lines: list of objects, where object
    is either Variable or Comment
    """
    self.lines=[]
    self.myopen("r")
    while 1:
      l= self.mygetline()
      if l=='': break
      if l[0]=='#' or l[0]=='\n':
        self.lines.append(Comment(l)) ; continue
      name,value,rest= self.procLine(l)
      #print "fillFromDefauls:",name,value,rest
      self.lines.append(Variable(name, rest, defvalue=value))
    self.myclose()
  def readhw(self):
    for var in self.lines:
       if isinstance(var, Comment): continue
       var.readhw()
  def setcv(self, varname, value):
    """Set current value in variable varname"""
    ixvar= self.findVariable(varname)
    if ixvar != None:
      var= self.lines[ixvar]
      var.setcv(value)
    else:
      err(varname+" -unknown parameter")
  def findVariable(self, varname):
    for i in range(len(self.lines)):
      if isinstance(self.lines[i], Comment): continue
      #print "findVariable:"+varname+":"+self.lines[i].name+":"
      if self.lines[i].name == varname:
        return i
      else: continue
    return None
  def writehw(self):
    self.updateFromText("HW")
    if vb!=None:
      for var in self.lines:
        if isinstance(var, Comment): continue
        var.writehw()
    else:
      f= open(Variables.HWNAME,"w");
      for i in range(len(self.lines)):
        var= self.lines[i]
        if isinstance(var, Comment): continue
        if var.hwupdate==None: 
          err("%s -no hwupdate"%var.name)
        f.write("%s %s\n"%(var.name, var.curvalue))
      f.close()
    self.reDisplay(0)
  def saveAsDefaults(self):
    """1. text->curvalue  (for ALL)
    2. curvalue -> defvalue  (for ALL)
    3. write to defaults file  (for ALL)
    """
    self.updateFromText("DDB")
    for i in range(len(self.lines)):
      v= self.lines[i]; 
      if isinstance(v, Comment): continue
      v.defvalue= v.curvalue
    self.myopen("w");
    for i in range(len(self.lines)):
      self.f.write(self.lines[i].line())
    self.myclose("w")
    self.reDisplay(2)
  def copyFromDefaults(self):
    """
    1. fill defaults from database file
    2. defvalue -> curvalue  (for ALL)
    3. write to text  (for ALL)
    """
    self.fillFromDefaults()
    for i in range(len(self.lines)):
      v= self.lines[i]; 
      if isinstance(v, Comment): continue
      v.curvalue= v.defvalue
    self.reDisplay(3)
  def reDisplay(self, button):
    self.textview.delete("1.0","end");
    for i in range(len(self.lines)):
      self.lines[i].show(self.textview)
    self.setLastAction(button)
  def copyFromMemory(self):
    self.readhw()
    self.reDisplay(1)
  def prt(self):
    nvar= len(self.lines)
    print("----------------------------------------Variables:%d"%nvar)
    for i in range(nvar):
      self.lines[i].prt()
  def setLastAction(self, button=None):
    if button!=None: self.lastAction=button
    if self.lastAction==None: return
    if self.f1:
      for ix in range(4):
        if ix==self.lastAction: 
          self.actions.buttons[ix].setColor("yellow")
        else:
          self.actions.buttons[ix].resetColor()
  def show(self):
    if self.f1: return
    self.f1=myw.NewToplevel("Defaults editor:"+self.cfgname, self.quit)
    self.f11= myw.MywFrame(self.f1,side=TOP)   #editor frame
    self.f12= myw.MywFrame(self.f1,relief=FLAT,side=BOTTOM)
    self.actions= myw.MywHButtons(self.f12, buttons= [
      ("Save in memory",self.writehw),
      ("Load from memory",self.copyFromMemory),
      ("Save in database",self.saveAsDefaults),
      ("Load from database",self.copyFromDefaults),
      ("Quit",self.quit)], helptext=
"""
Save in memory   -save values in memory (i.e. Defaults Database is left untouched)
                  Hardware registers are not loaded - this will be done
                  by pressing LTUinit/TTCinit buttons, or before run starts
Load from memory -show current values (held in shared memory) in editor window
Save in database -save values in Defaults database (i.e. they
                  will be loaded after next power up). ATTENTION:
                  CURRENT MEMORY VALUES HAVE TO BE DISPLAYED (I.E.
                  LAST ACTION WAS 'Save' or 'Load from memory')
                  BEFORE PRESSING THIS BUTTON -by other words:
                  Only 'memorised' values can be saved in database.
Load from database -show Defaults Database values in editor window

Recently activated button is colored in yellow.
"""
    )
    self.textview   = Text(self.f11, bg = "#ccffff")
    scrollview = Scrollbar(self.f11, command = self.textview.yview)
    self.textview.pack(side = "left")
    scrollview.pack(side = "right", fill = "y")
    myw.MywHelp(self.f11, """
The text area showing values from either Database or shared (CPU's) memory. 
Each line consists of 2 or 3 fields:
Name      -the name of variable
Value     -the value of variable (the current value in memory or default value in database)
          Possible values:
          Identifier, e.g. YES NO SW ...: 
              consists of alphanumeric chars and underscores
          Numbers, e.g. 123 34.5: whole or float numbers 
          Strings: enclosed in " ", if value contains
              special chars ( . / : ; ...)

#comment  -comment (usually explains the possible choices of values)

The Value is displayed in YELLOW if there is the difference between 
the 'database' and 'in shared memory' values.

Notes about available parameters:
l2aseq "L2a.seq" corresponds to (for both run1/2 formats):
L2A 0x800000000000 0x800000000000 0x0 roc=7 Restart Last ErrProne 
i.e. class 48 is in class pattern.

""" , self.textview)
    self.textview.configure(yscrollcommand = scrollview.set)
    self.textview.tag_config("DIF", background="yellow")
    self.textview.tag_config("ERROR", foreground="red", background="white")
    #self.textview.delete("1.0", "end")
    for i in range(len(self.lines)):
      #self.textview.insert("end", self.lines[i].line())
      self.lines[i].show(self.textview)
    self.setLastAction()
  def saveMemory(self):
    pass
  def quit(self, ev=None):
    #sys.exit()
    self.f1.destroy()
    self.f1=None
    #print "quit...",ev
      
def fdestroy(event):
  print("fdestroy:",event)
def main():
  f = Tk()
  f.bind("<Destroy>", fdestroy)
  f.title("Editor example")
  v=Variables()
  v.show()
  f.mainloop()

if __name__ == "__main__":
    main()

