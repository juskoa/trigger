#file inputs_u.py
from mywrl import *
#import ROOT
import ssmbrowser,myw,types
import os,os.path
from time import gmtime, strftime
COLOR_SSMC='#ffffcc'
COLOR_normal='#dcdcdc'
COLOR_error='red'
COLOR_change='#aaaaaa'
global allinp
allinp=0
global rareradio
rareradio=StringVar()
def SSMbrowser(vb):
  ssmbrowser.main(vb)
def INPUTS(vb):
  global allinp
  if allinp:
     print "Input window already opened."
  else:
     allinps=None
     Detectors2Choose(vb,allinps)
class Detectors2Choose:
 """
 """
 def __init__(self,vb,allinps):
  self.vb=vb
  self.detectors=[]
  self.window=Toplevel(vb.master)
  dets=' '
  self.dets=myw.MywEntry(self.window, side=LEFT, width=30,defvalue=dets,
      expandentry='no',label='Detectors', 
      helptext="List of detectors separated by commas")
  myw.MywButton(self.window,side=LEFT,label='OK',
           cmd=self.getDetectors,helptext='Values read')
  myw.MywButton(self.window,'All',cmd=self.all)
  self.window.grab_set()
  self.window.protocol("WM_DELETE_WINDOW", self.window.destroy)
  self.window.wait_window(self.window)
 def getDetectors(self):
  inp=self.dets.getEntry()
  for i in inp.split(','):
      det=i.replace(" ","")
      if det=="":continue
      self.detectors.append(det)
  print "Detectors: ",self.detectors
  self.window.destroy()
  allinps=AllInputs(self.vb,0,self.detectors)
  allinp=1
 def all(self):
  self.detectors.append("ALL")
  print "Detectors: ",self.detectors
  self.window.destroy()
  allinps=AllInputs(self.vb,0,self.detectors)
  allinp=1
class AllInputs:
 """
 """
 def __init__(self,vb,id,detectors):
  self.vb=vb
  self.id=id
  self.tl=None
  self.detectors=detectors
  dbinputs=self.readVCTPINPUTS()
  self.Detectors=self.readVLTUS()
  if(self.Detectors == None): return
  if(dbinputs==None): pass
  orbit={'name':'ORBIT','number':'-','detector':'machine','signature':'none','level':'-'}
  self.numofinpperdet(dbinputs)
  #print self.Detectors
  dbinputs.append(orbit) 
  self.tl=Toplevel(vb.master)
  self.tl.title("L0,L1 and L2 inputs")
  self.addinp=[];   
  self.inputsframe= myw.MywFrame(self.tl, side=TOP,relief=FLAT, bg=COLOR_SSMC) 
  header= myw.MywLabel(self.inputsframe, side=TOP, anchor='w',
      helptext=mainhelp, label=
      ' Input  Level          Name                     Det                          Action                  Signature      Status     Activity  Edge Delay   Phase   Last Rare') 
  self.inputs=[]
  self.rare=StringVar()
  self.last=StringVar()
  for i in dbinputs:
    print "Adding: ", i
    self.inputs.append(Input(vb,i,self.inputsframe,'INDB',self))
  self.oinpheader=self.showotherinpsframe()
  print "Checking inputs ...."
  self.checkallinputs()
  print "Checking inputs finished."
  self.showalleditactions()
  self.showallactions()
 #
 def setRare(self,value):
  self.rare=value
 def getRare(self):
  return self.rare
 def setLast(self,value):
  self.last=value
 def getLast(self):
  return self.last
 def showotherinpsframe(self):
  oinpframe=myw.MywFrame(self.tl, side=TOP,relief=FLAT, bg=COLOR_SSMC)
  oinpheader= myw.MywLabel(oinpframe, side=TOP, anchor='w',label=' ',
              helptext='Active inputs not in the main input list menu')
  return oinpheader
 #
 def showallactions(self):
  helpsynchro="""
      Synchronise inputs chosen by Input button. 
      The phases of all chosen inputs should be measured before.
      The Edges of the inputs necessary for correct synchronisation are calculated.
      If the calculated input's edge differs from actual one (in hardware),
      the edge window goes green.
      (Each click of 'Synchronisation' reset the colour.)
      The user should change the edge herself by Action in the input. 
  """
  helpauto="""
      The autocorrelation functiions of chosen inputs. 
      One graph per input is opened.
      Provides indication of the correlated background, noise.
  """
  helpalign="""
      Aligns the chosen inputs. 
      The following windows depending on chosen inputs are opened:
      - one window for alignment of L0 inputs if any, one graph per correlation,
        for N inputs N-1 correlation are produced
      - one window for L1 inputs, if any, same as L0
      - one window for L2 inputs, if any, same as L0
      - one window for L0-L1 correlation, only one input from L0 level and
        one input from L1 level is chosen (the first in the list at the moment,
        will be changed so that user can chose these inputs)
      - one window for L0-L1 correlation if necessary
      - one window for L1-L2 correlation if necessary
  """
  helpactivity="""
      Activity = number of nonzero BC in snapshot memory
      Activity of all inputs is scanned.
      If inputs with nonzero activity not in the input menu list found, they
      are listed bellow the main input list menu as 'Other inputs with activity'
  """
  helpclear="""
      During the autocorrelation or alignment the snapshot memory (ssm) is recorded and saved in computer
      memory. The ssm data are accumulating from all readings. They can be removed by ClearData button.
      Clearing is necessary when the measurement conditions are changed.
  """
  cntrlframe=myw.MywFrame(self.tl, side=TOP,relief=FLAT, bg=COLOR_SSMC) 
  #cheader=myw.MywLabel(cntrlframe,side=TOP,anchor='w',
  #        label='                           All Inputs Actions') 
  checkbut=myw.MywButton(cntrlframe,side=LEFT,label='Check Activity',
           cmd=self.checkallinputs,helptext=helpactivity)
  synchrbut=myw.MywButton(cntrlframe,side=LEFT,label='Synchronise',
            cmd=self.synchronise,helptext=helpsynchro)
  clearbut =myw.MywButton(cntrlframe,side=LEFT,label='ClearData',cmd=self.cleardata,helptext=helpclear)
  autobut =myw.MywButton(cntrlframe,side=LEFT,label='AutoCorrelate',cmd=self.autocor,helptext=helpauto)
  alignbut=myw.MywButton(cntrlframe,side=LEFT,label='Align',cmd=self.align,helptext=helpalign)
 #
 def showalleditactions(self):
  cntrlframe=myw.MywFrame(self.tl, side=TOP,relief=FLAT, bg=COLOR_SSMC) 
  #cheader=myw.MywLabel(cntrlframe,side=TOP,anchor='w',
  #        label='                           Edit:') 
  delbut=myw.MywButton(cntrlframe,side=LEFT,label='Delete Inputs',
           cmd=self.delete,helptext='Delte inputs chosen by Input button')
  modbut=myw.MywButton(cntrlframe,side=LEFT,label='Modify Inputs',
           cmd=self.modify,helptext='Modify inputs chosen by Input button')
  addbut=myw.MywButton(cntrlframe,side=LEFT,label='Add Input',
           cmd=self.addinput,helptext='Adds new input')
  printbut=myw.MywButton(cntrlframe,side=LEFT,label='Print',
           cmd=self.printinps,helptext='Print for debugging ')
  quitbut=myw.MywButton(cntrlframe,side=LEFT,label='Quit',
           cmd=self.quit,helptext=' ')
 #
 def cleardata(self):
   """
       Clear ssm data.
   """
   self.vb.io.execute("resetINPUTS()",log="yes",applout="<>")
 #
 def align(self):
   """
     Align all selected inputs
     H0 - new level for Hardware L0 = L0 output mode f
   """
   flag=0
   input=None
   level=None
   board=None
   ainps={'L0':[],'L1':[],'L2':[],'H0':[]} 
   for i in self.inputs:
     if(i.inputnumber.var.get() == 1):
       if i.inpnumall == rareradio:
           input=i.inpnum
           level=i.level
           board=i.board
           print 'Rare chosen:',level,' ',input
       ainps[i.level].append(i.inpnum)
       flag=flag+1
   #print 'ainps:',ainps  
   if flag < 2 :
     print "Align: less then 2 inputs chosen. " 
     return
   if input==None:
     cmd="setRareFlag(0,0,0)"
   else:
     mode='0'
     if level == 'H0': mode = '1'
     cmd="setRareFlag("+board+','+input+','+mode+")"
   print "seting rare: ",cmd
   output=self.vb.io.execute(cmd,log="yes",applout="<>") 
   self.align=Corel(self.vb,ainps)
   self.align.croscor()
 #
 def autocor(self):
   """
     Calculate autocorrelation of  all selected inputs
     H0 - new level for Hardware L0 = L0 output mode f
   """
   flag=0
   input=None
   level=None
   board=None
   ainps={'L0':[],'L1':[],'L2':[],'H0':[]} 
   for i in self.inputs:
     print i.inputnumber.var.get()
     if(i.inputnumber.var.get() == 1):
       print i.inpnumall," ",rareradio.get()
       if i.inpnumall == rareradio.get():
           input=i.inpnum
           level=i.level
           board=i.board
           print 'Rare chosen:',level,input,i.board
       ainps[i.level].append(i.inpnum)
       flag=1
   #print 'ainps:',ainps  
   if flag == 0:
     print "Autocorrelation: No inputs chosen. " 
     return
   # set rare flag in c for board,mode and input
   if input==None:
     cmd="setRareFlag(0,0,0)"
   else:
     mode='0'
     if level == 'H0': mode = '1'
     cmd="setRareFlag("+board+','+input+','+mode+")"
   output=self.vb.io.execute(cmd,log="yes",applout="<>") 
   self.auto=Corel(self.vb,ainps)
   self.auto.autocor()
 #
 def synchronise(self):
   """
    Calculate phases of all selected inputs. Calclulate p/n triggering
   """
   ainps=[]
   last=None
   for i in self.inputs:
     i.inputedge.entry['bg']=COLOR_normal       
     i.inputedge.setEntry(i.edge)       
     if(i.inputnumber.var.get() == 1):
        ainps.append(i)
        if i.inpnum == self.allinputs.getLast():  last=i
   if len(ainps) < 1:
    print 'No inputs chosen for synchro.' 
    return
   if last == None:
     print 'No last input chosen for synchronisation.'
     return
   print 'last=',last.name
   flag=0
   for i in ainps:
     #print i.phase
     if i.phase == '?':
        flag=1 
        print 'The phase of input ',i.name,i.inpnum,'is not measured.'
   if flag: return 
   last.phi='P'
   D = (int(last.phase)+5) % 25
   #here set bcdelay
   cmd="setbcdelay("+str(D)+")"
   output=self.vb.io.execute(cmd,log="yes",applout="<>")
   for i in ainps:
      if i==last: continue
      phi= (int(i.phase)-D) % 25
      if (phi<6) or (phi>18): i.phi='N'
      else: i.phi='P'
      print i.name,i.phi,i.edge
      if i.phi != i.edge: 
         i.inputedge.entry['bg']='green'
         i.inputedge.setEntry(i.phi)       
 #
 def printinps(self):
   for i in self.inputs:
     i.printme()
 #
 def addinput(self):
   """
   """
   print "adding input"
   self.addinp.append(AddModInput(self.inputs,self.vb,self.inputsframe,None,self))
 #
 def delete(self):
  """
  """
  delinps=[]
  for i in self.inputs:
    if(i.inputnumber.var.get() == 1):
      print "Deleting input ",i.inpnum,i.name
      i.destroy()
      delinps.append(i)
      #self.inputs.remove(i)
  for i in delinps:
      self.inputs.remove(i)
 #
 def modify(self):
  """
  """
  for i in self.inputs:
    if(i.inputnumber.var.get() == 1):
      self.addinp.append(AddModInput(self.inputs,self.vb,self.inputsframe,i,self))
 #
 def findinput(self,inpnum,board):
  """
  """
  for i in self.inputs:
    if i.inpnum == inpnum and i.board == board:
       return self.inputs.index(i)
  return None
 #
 def quit(self):
  """
  """
  global allinp
  for i in self.addinp:
    i.destroy()
    del i
  self.tl.destroy()
  allinp=0
  # tu by bolo treba znicit aj vsetky inputy
 #
 def checkallinputs(self):
  """
  """
  activeinputs=self.getactiveinputs()
  self.checksignature(activeinputs)
  self.showsignatureM()
  self.showactiveinputs(self.inputs,activeinputs)
  #self.showerrors(self.inputs)
 #
 def checksignature(self,activeinputs):
  """
    Try to detect signatures for all active inputs
  """
  for i in ['1','2','3']:
     inps=''
     start=1
     for j in activeinputs[i]:
       inps=inps+(int(j[0])-start)*'0'+'1'
       start=int(j[0])+1
     #print 'inps= ',inps
     if inps != '': 
       cmd="FindSignatures("+i+","+'"'+inps+'"'+")"
       output=self.vb.io.execute(cmd,log="out",applout="<>")
       #print i,'output=',output,len(output)
       for j in range(0,len(output)-1,2):
         k=self.findinput(output[j],i)
         if k != None:
           print 'checksignature: ',j,output[j],k,self.inputs[k].name
           self.inputs[k].signatureM=output[j+1]
 #
 def showsignatureM(self):
  """
  """
  for i in self.inputs:
    if i.name=='ORBIT':continue
    if i.signatureM: 
       i.inputsignatureM['text']=i.signatureM
    else:
       i.inputsignatureM['text']='?'
 # 
 def showactiveinputs(self,inputs,activeinputs):
  """
      Show active inputs: if input is in list show activity,
      if not create new unknown input with different colour
  """
  for i in inputs:
    i.inputactivity['text']='0'
    i.activity='0'
    for k in activeinputs[i.mode]:
        #print k[0],i.inpnum
        if k[0]==i.inpnum:
           i.inputactivity['text']=k[1]
           i.activity=k[1]
           activeinputs[i.mode].remove(k)
  #print 'activeinputs after:',activeinputs
  # dealing with active inputs not in DB
  otherinps={'L0':[],'L1':[],'L2':[]} 
  for i in ['1','2','3']:
    if activeinputs[i] != []:
       for k in activeinputs[i]:
           level='L'+str(int(i)-1)
           otherinps[level].append(k[0])  
  #print otherinps
  text=''
  for i in ['L0','L1','L2']:
      if otherinps[i] != []:
         text=text+i+':'
         for k in otherinps[i]:
             text=text+' '+k+' ' 
  if(text==''):
    text='Other inputs with activity: None'
  else:
    text='Other inputs with activity: '+text  
  #print text
  self.oinpheader['text']=text
  #print 'activeinputs after 2:',activeinputs
 #
 def showerrors(self,inputs):
  """
      Highlights:
      all statuses in error are under getstatus control
      1.) actvity==0 and not error status
      2.) activity!=0 and error status
  """
  for i in inputs:
    if i.name == 'ORBIT': continue
    #print i.name,i.status,i.activity
    error=(i.status in i.statuses and i.activity == '0') 
    error= error or (i.status in i.statuserrors and i.activity != '0')          
    if error:
       #i.inputstatus.configure(highlightbackground=COLOR_error)
       #i.inputstatus.entry['bg']=COLOR_error
       #i.inputactivity['bg']=COLOR_error
       pass
    else:
       #i.inputstatus['bg']=COLOR_normal
       i.inputactivity['bg']=COLOR_normal
    error= (i.signature != i.signatureM) and (i.status in i.statuses)
    if error:
       i.inputsignatureM['bg']=COLOR_error
    else:
       i.inputsignatureM['bg']=COLOR_normal
 #
 def getactiveinputs(self):
  """
      Get active inputs: take snapshot memory and 
      check if any signal present
      Also BUSY board is added due to the orbit, otherwise is not necessary.
  """
  boards=['0','1','2','3']
  activeinputs={'0':[],'1':[],'2':[],'3':[],'1o':[]}
  for i in boards:
    cmd="checkInputsActivity("+i+")"
    #output=self.vb.io.execute(cmd,log="out",applout="<>")
    output=self.vb.io.execute(cmd,log="no",applout="<>")
    #print i,' output= ',output
    for j in range(0,len(output)-1,2):
        activeinputs[i].append([output[j],output[j+1]])
  #print 'active inputs: ',activeinputs
  CTPinternalsignals=0 
  if CTPinternalsignals:
    cmd="checkInputsActivityRB()"
    output=self.vb.io.execute(cmd,log="no",applout="<>")
    #print i,' RB output= ',output
    for j in range(0,len(output)-1,2):
      activeinputs['1o'].append([output[j],output[j+1]])
    #print 'active inputs after RB: ',activeinputs 
  return activeinputs   
 #
 def numofinpperdet(self,inputs):
  """
      Count number of inputs per detector. Needed to know for
      communication with dim detector server.
  """
  for i in self.Detectors:
      name=i['name']
      n=0
      for j in inputs:
        #print j['detector']
        if j['detector'] == name: n=n+1
      #print name,n
      i['numofinps']=str(n)
  for i in inputs:
      i['numofinps']='0'
      for j in self.Detectors:
          if i['detector'] == j['name']:
              i['numofinps']=j['numofinps']
              break;
  #print inputs
 def readVLTUS(self):
  """
      Read file $VMECFDIR/CFG/ctp/DB/VALID.LTUS
      Info about detectors. 
      Only detector names extracted, can be modified, if something else 
      necessary
  """ 
  fname= os.environ['VMECFDIR'] +"/CFG/ctp/DB/VALID.LTUS"
  try:
    database=open(fname,"r") 
  except IOError:
    print "Cannot open ",fname
    return None
  else:
    print "File ",fname," open successfuly."
  #print "database= ",database
  lines=database.readlines()
  database.close() 
  #print lines,len(lines) 
  Detectors=[] 
  for i in lines:
    for j in i:
       if j == ' ': continue
       else: break
    if j=='#': continue
    items=i.split('=')
    detector={}
    detector['name']=items[0]
    Detectors.append(detector)
  #print Detectors  
  #print '-----------------------------'  
  return Detectors 
 def readVCTPINPUTS(self):
  """
     Read file in $VMECFDIR/CFG/ctp/DB/VALID.CTPINPUTS
  """ 
  fname= os.environ['VMECFDIR'] +"/CFG/ctp/DB/VALID.CTPINPUTS"
  try:
    database=open(fname,"r") 
  except IOError:
    print "Cannot open ",fname
    return None
  else:
    print "File ",fname," open successfuly."
  #print "database= ",database
  lines=database.readlines()
  database.close()  
  #print lines,len(lines) 
  dbinputs=[]
  count=0
  #print "look for me if you want different inputs range..."
  for i in lines:
   if(i[0] == 'l' and i[1] == '0'): continue
   if(i[0] != '#'):
    items=string.split(i)
    #print 'items= ',items,len(items)
    if(len(items)<6):
      print "Error parsing database, not enough items in line:"
      print items
      continue
      #return None
    count=count+1
    #if count<6 or count>11  : continue
    #if count>10 and count<24 : continue
    #if count<16: continue
    #if count > 4 and count < 15: continue
    #if items[3] != '1': continue
    #if items[2] != "EMCAL": continue
    #if (items[2] != "SPD") and (items[2] != "T0"): continue
    flag=1
    for i in self.detectors:
        if items[2].find(i)>=0 or i.find("ALL")>=0:
          flag=0;
          break
    if flag: continue
    db={}
    db['number']=items[5]
    db['numberDIM']=items[6]
    db['level']='L'+items[3]
    db['name']=items[0]
    db['detector']=items[2]
    db['signature']=items[4]
    dbinputs.append(db)
    #print "Adding: ", db
  return dbinputs
 #
 def readdatabase2(self):
  """
    Read file in ADCI/DB directory. Can be chabged to database later.
  """
  fname="/home/alice/rl/v/vme/ADCI/DB/INPUTS.txt"
  try:
    database=open(fname,"r") 
  except IOError:
    print "Cannot open ",fname
    return None
  else:
    print "File ",fname," open successfuly."
  #print "database= ",database
  lines=database.readlines()
  database.close()  
  #print lines,len(lines) 
  dbinputs=[]
  for i in lines:
   if(i[0] != '#'):
    items=string.split(i)
    #print 'items= ',items,len(items)
    if(len(items)<6):
      print "Error parsing database, not enough items in line:"
      print items
      return None
    db={}
    db['number']=items[0]
    db['numberDIM']=items[1]
    db['level']=items[2]
    db['name']=items[3]
    db['detector']=items[4]
    db['signature']=items[5]
    dbinputs.append(db)
  return dbinputs
#-------------------------------------------------------------------------
#     AddModInput
#-------------------------------------------------------------------------
class AddModInput:
 """
    Interactive window for 
      - adding new input: number,level,name
      - modifying existing input
 """
 def __init__(self,inputs,vb,inputsframe,input,allinputs):
   self.inputs=inputs
   self.input=input
   self.vb=vb
   self.allinputs=allinputs
   self.inputsframe=inputsframe
   self.tl=Toplevel()
   if input:
     self.tl.title("Modify Input "+input.name)
     numdef=input.inpnum
     levdef=input.level
     namedef=input.name
     detdef=input.detector
     dimdef=input.inpnumDIM
     sigdef=input.signature
   else:
     self.tl.title("Input Input")   
     numdef=" "
     levdef=" "
     namedef=" "
     detdef=" "
     dimdef=" "
     sigdef=" "
   fr=myw.MywFrame(self.tl, side=TOP,relief=FLAT, bg=COLOR_SSMC)   
   self.num = myw.MywEntry(fr, side=LEFT, width=3,defvalue=numdef,
      expandentry='no',label='Input', 
      helptext="Input number: 1-24 for [L0,L1];1-12 for L2")
   self.level = myw.MywEntry(fr, side=LEFT, width=3,defvalue=levdef,
      expandentry='no',label='Level', helptext="Trigger level=[L0,L1,L2]")
   self.name = myw.MywEntry(fr, side=LEFT, width=3,defvalue=namedef,
      expandentry='no',label='Inp name', helptext="Input name")
   self.det = myw.MywEntry(fr, side=LEFT, width=3,defvalue=detdef,
      expandentry='no',label='Detector', helptext="Detector name")
   self.numDIM = myw.MywEntry(fr, side=LEFT, width=3,defvalue=dimdef,
      expandentry='no',label='DIM number', 
      helptext="DIM number: input position in DIM status string")
   self.sig = myw.MywEntry(fr, side=LEFT, width=3,defvalue=sigdef,
      expandentry='no',label='Signature', 
      helptext="Number 1-119, input signature")
   okbut=myw.MywButton(fr,side=LEFT,label='OK',
           cmd=self.ok,helptext='Values read and added to inputs')
   cancel=myw.MywButton(fr,side=LEFT,label='Cancel',
           cmd=self.cancel,helptext='Close window')
 def destroy(self):
   self.tl.destroy() 
 def ok(self):
   input={}
   num=self.num.getEntry()
   num=num.strip()
   if isnumber(num,'Input')==None: return
   input['number']=num
   level=self.level.getEntry()
   level=level.strip()
   input['level']=level
   name=self.name.getEntry()
   name=name.strip()
   input['name']=name
   detector=self.det.getEntry()
   detector=detector.strip()
   input['detector']=detector
   numberDIM=self.numDIM.getEntry()
   number=numberDIM.strip()
   if isnumber(number,'DIMnumber ')==None: return
   input['numberDIM']=numberDIM
   det=self.det.getEntry()
   det=det.strip()
   input['detector']=det
   sig=self.sig.getEntry()
   sig=sig.strip()
   if isnumber(sig,'Signature ')==None: return
   input['signature']=sig
   #print input
   if self.checkinput(input):
    if self.input: self.update(input)
    else: 
     for i in self.inputs:
      if num == i.inpnum and level == i.level:
         print "Input "+ level+" "+num+" already exists"
         return 0
     self.inputs.append(Input(self.vb,input,self.inputsframe,'NOTINDB',self.allinputs))
   #self.tl.destroy()
 def update(self,input):
    self.input.inpnum   =input['number']
    self.input.level    =input['level']
    self.input.name     =input['name']
    self.input.detector =input['detector']
    self.input.inpnumDIM=input['numberDIM']
    self.input.signature=input['signature']
    self.input.board=str(int(self.input.level[1])+1)
    self.input.updateshow()
 def checkinput(self,input):
   level=input['level']
   if level not in ['L0','L1','L2']:
     #mywerror
     print "Wrong level ",level
     MywError("Wrong level: "+level)
     return 0
   num=input['number']
   valid = []
   for i in range(1,13,1):
     valid.append(str(i))
   if level in ['L2'] and num not in valid:
     MywError("Wrong input: "+num)
     print "Wrong input ",num  
     return 0
   sig=input['signature']
   if int(sig) < 1 or int(sig) > 119:
      print "Wrong signature"
      MywError('Wrong signature: '+sig)
      return 0
   return 1
 def cancel(self):
   self.tl.destroy()
#------------------------------------------------------------------------
class Input:
 """
   Class for creation an Input panel in synchronisation window menu
   ORBIT in the context of synchronisation is treated as another input.
   (To pass vb is necessary for having access to c functions) 
 """
 def __init__(self,vb,dbinput,toplevel,dborigin,allinputs):
  self.vb=vb
  self.allinputs=allinputs
  self.statuserrors=['DIM Error','BUG','None','FE error']
  self.statuses=['Toggle','Random','Normal','Signature']
  self.toplevel=toplevel
  self.dborigin=dborigin
  self.signatureM=None
  self.name=dbinput['name']
  self.edge='?' 
  self.status='None'
  self.phase='?'
  self.phi='100'
  self.numofinps=None
  self.inpnumall='O'
  if self.name == 'ORBIT':
    self.inpnum='0'  
    self.inpnumDIM='0'  
    self.level='-'
    self.detector='LHC'
    self.signature='-'
    self.board='0'
    self.mode='0'   # = board , default = inmon mode
    self.delay='-'
    self.activity=None
    self.actions=["Get Status","Get Edge","Measure Phase"]
  else:
    self.inpnum=dbinput['number']
    self.inpnumDIM=dbinput['numberDIM']
    self.detector=dbinput['detector']
    self.level=dbinput['level']
    self.board=str(int(self.level[1])+1)
    if self.detector == 'CTP':
       self.mode='1o'   
       self.level='H0'
    else:
       self.mode=self.board   # = board, default = inmon mode 
    self.signature=dbinput['signature']
    self.activity="Unknown"
    self.delay='?'
    self.numofinps=dbinput['numofinps']
    if self.level=='L0': self.inpnumall='L0'+self.inpnum
    elif self.level=='L1':self.inpnumall='L1'+self.inpnum
    elif self.level=='L2':self.inpnumall='L2'+self.inpnum
    #self.actions=["Get Status (DIM)","Get Edge/Delay","Set Status (DIM)","Measure Phase","Check Signature","Set Delay"]
    self.actions=["Get Status (DIM)","Set Status (DIM)","Check Signature","Get Edge/Delay","Set Edge","Set Delay","Measure Phase"]
  #print dbinput
  self.show()
  if self.name == 'ORBIT': self.inputsignatureM['text']='-'
 def printme(self):
   """
      Print class variables
   """
   p='inpnum: '+self.inpnum+' name:'+self.name+' inpnumDIM:'+self.inpnumDIM
   p=p+' level:'+self.level+' detector:'+self.detector
   p=p+' signature:'+self.signature+' board:'+self.board
   #print p
 def action(self,mywinst,ix):
   """
      Input actions: Get Status (DIM), ....
   """
   om=self.bactions.getEntry()
   print "action",ix,om
   if om == "Get Status (DIM)":
    self.getstatus()
   elif om == "Measure Phase":
    #Measure(self.vb,self.board,self.inpnum,self.name)
    Measure(self)
   elif om == "Get Edge/Delay":
     self.getedge()
   elif om == "Set Status (DIM)":
     self.setstatus(None)
   elif om == "Check Signature":
     self.checksignature()
   elif om == "Get Status":
     self.getstatus()
   elif om == "Set Delay":
     self.setdelay()
   elif om == "Set Edge":
     self.setedge()
 def show(self):
   """
      Show the input panels
   """
   actionshelp="""
      Get Status (DIM): Toggle,Signature,Normal,Random,DIM Error
      Set Status (DIM): Toggle,Signature,Normal,Random
      Check Signature:
      Get Edge/Delay
      Set Edge
      Set Delay
      Measure Phase
      ------------------------------------------------------------------------------------------------
     Check signature results:
     Headers:    number of headers found
     FHeaders:   number of headers attemted but failed
     Signatures: number of ssm signatures equaled to input signature
     CmplCntsS:  number of cases when ssm signature == ~(compl ssm signature) 
     CmplCntsI:  number of cases when (compl ssm signature) = ~(input signature)
     CheckDist:  number of changes of distance between two subsequent headers
       Typical situations.
--------------------------------------------------------------------
Channel  Headers FHeaders Signatures CmplCntsS CmplCntsI CheckDist
    16        0        0         0         0         0         0 
No headers found, nothing in ssm channel.
---------------------------------------------------------------------
Channel  Headers FHeaders Signatures CmplCntsS CmplCntsI CheckDist
    16        0    11760         0         0         0         0 
No headers found, ssm channel has some nonzero bits.
-------------------------------------------------------------------
Channel  Headers FHeaders Signatures CmplCntsS CmplCntsI CheckDist
    19     1048        0         0         0         0         0 
Headers ok.
-------------------------------------------------------------------
Channel  Headers FHeaders Signatures CmplCntsS CmplCntsI CheckDist
    19     1048     1048      1048         0         0         0 
Complements are not ok, also some spurious signalbetween headers. 
--------------------------------------------------------------------
Channel  Headers FHeaders Signatures CmplCntsS CmplCntsI CheckDist
    19     1047        0      1047         0         0         0 
Headers and signatures ok, complements wrong
---------------------------------------------------------------------
Channel  Headers FHeaders Signatures CmplCntsS CmplCntsI CheckDist
    19     1047     1047      1047      1047      1047         0 
Looks like you have some spurious signal between signatures.
---------------------------------------------------------------------
Channel  Headers FHeaders Signatures CmplCntsS CmplCntsI CheckDist
    19     1048        0         0      1048         0         0 
You are almost there,
looks like signature in ssm is different from one you input
but otherwise everything ok.
---------------------------------------------------------------------
Channel  Headers FHeaders Signatures CmplCntsS CmplCntsI CheckDist
    19     1048        0      1048      1048      1048         0 
Everything OK!


   """
   fr1= myw.MywFrame(self.toplevel, side=TOP,relief=FLAT, bg=COLOR_SSMC)
   self.fr1=fr1 
   self.inputnumber = MywCheckButton(fr1, side=LEFT, label=self.inpnum, 
                      helptext="Input number")
   self.inputlevel = myw.MywLabel(fr1, side=LEFT, label=self.level,
      width=4, expand='no',fill='y',borderwidth=1, 
      helptext="Input trigger level")
   self.inputname = myw.MywLabel(fr1, side=LEFT, label=self.name,
      width=16, expand='no',fill='y', borderwidth=1,helptext="Input name")
   self.inputdet = myw.MywLabel(fr1, side=LEFT, label=self.detector,
      width=16, expand='no',fill='y', helptext="Detector name")
   self.bactions= myw.MywxMenu(fr1, side=LEFT, width=20, defaultinx=0,
      label='', items=self.actions,cmd=self.action,helptext=actionshelp)
   self.inputsignatureDB= myw.MywLabel(fr1, side=LEFT, label=self.signature,
      width=5, expand='no',fill='y',borderwidth=1, 
      helptext="Signature of the input from DB")
   self.inputsignatureM= myw.MywLabel(fr1, side=LEFT, label='?',
      width=5, expand='no',fill='y',borderwidth=1, 
      helptext="Signature of the input as Measured")
   self.inputstatus= myw.MywEntry(fr1, side=LEFT, width=8,defvalue="None",
      expandentry='no',label='', helptext="DIM status")
   self.inputactivity= myw.MywLabel(fr1, side=LEFT, width=8,
      expand='no',fill='y',label='', helptext="Activity status: number of nonzero bins in SSM")
   self.inputedge= myw.MywEntry(fr1, side=LEFT, width=3,defvalue="?",
      expandentry='no',label='', helptext="P=positive,N=negative")
   self.inputdelay= myw.MywEntry(fr1, side=LEFT, width=5,defvalue=self.delay,
      expandentry='no',label='', helptext="Delay of given input")
   self.inputphase= myw.MywLabel(fr1, side=LEFT, label=self.phase,
      width=8, expand='no',fill='y',borderwidth=1, helptext="Phase of the input")
   # when I do allinputs functions these would be removed ?
   self.inputlast=MywRadio2(fr1,text='',variable=self.allinputs.getLast(),value=self.inpnum)
   self.inputrare=MywRadio2(fr1,text='',variable=rareradio,value=self.inpnumall,command=self.tellme)
   self.getstatus()
   self.getedge()
   self.phi=self.edge
   #self.checksignature()
 def tellme(self):
  print "Selected input: ",rareradio.get()
 def updateshow(self):
  self.inputnumber['text']=self.inpnum  
  self.inputlevel['text']=self.level  
  self.inputname['text']=self.name  
  self.inputsignatureDB['text']=self.signature  
 def destroy(self):
  self.fr1.destroy()
 def status2word(self,status):
  """
     Translate one character status from DIM to normal word,e.g T=Toggle
  """
  if status == 'T':
     return "Toggle"
  elif status == 'S':
     return "Signature"
  elif status == 'R':
     return "Random"
  elif status == 'N':
     return "Normal"
  elif status == 'E':
     return "FE Error"
  elif status == 'X':
     return "NO TIN"
  else:
     print "Unknown status !"
     return "BUG"
 def getstatus(self):
  """
     Get DIM status of the input:
         - check DIM connection, if no connection =>  DIM Error
         - set input to send signature
         - check signature, if not ok => Sig Error
         - get status: T,S,R, N or E; E= FE error
     There will be also all inputs in one function, maybe
  """
  if(self.name == 'ORBIT'):
    cmd="getorbitstatus()"
    output=self.vb.io.execute(cmd,log="out",applout="<>")
    #print 'orbitstatus=',output
    self.status=output[0]
  else:
    ninps=self.numofinps
    if self.detector =='SPD': ninps='10' 
    cmd="getDetInputStatus("+'"'+self.detector+'",'+ninps+")"
    output=self.vb.io.execute(cmd,log="out",applout="<>")
    #print 'Detector input status:',output,len(output)
    if(output != []):
      numDIM=int(self.inpnumDIM)-1
      try:
        status=str(output[0][numDIM])
      except IndexError:
        print "Looks like DIM number wrong:",numDIM
        self.status="DIM Error"
      else:      
        self.status=self.status2word(status) 
        #print 'self.status, status=',self.status,status
    else:
      self.status="DIM Error"
  self.inputstatus.setEntry(self.status)
  error=self.status in self.statuserrors
  if error:
     self.inputstatus.entry['bg']=COLOR_error
  else:
     self.inputstatus.entry['bg']=COLOR_normal
 def setstatus(self,statusin):
  """
     Sets the input status via DIM
  """
  if self.status in self.statuserrors:
    print "Action forbidden."
    self.inputstatus.setEntry(self.status)
    return 
  if statusin == None: status=self.inputstatus.getEntry()
  else: status=statusin
  if status not in self.statuses:
    print "Unknown status:",status
    #self.inputstatus.setEntry(self.status)
    return
  option=status[0]
  cmd="setStatus("+'"'+self.detector+'"'+","+self.inpnumDIM +","+"'"+option+"'"+")"
  output=self.vb.io.execute(cmd,log="out",applout="<>")
  #self.getstatus()
 def getedge(self):
  """
      Get the edge definition for given input. Edge deffinition is in
      SYNCAL word (Pedja nomenclature)=SYNCH_ADD (Anton nomenclature).
      /ApplicatioInput/Orbit difference is in C.
  """
  cmd="getEdge("+self.board+","+self.inpnum+")"
  output=self.vb.io.execute(cmd,log="out",applout="<>")
  #print 'edge= ',output
  self.edge=output[0]
  self.inputedge.setEntry(self.edge)
  if self.board != '0': 
     self.delay=output[1] 
     self.inputdelay.setEntry(self.delay)
 def setdelay(self):
  """
      Set delay according to value in get entry
  """
  delay=self.inputdelay.getEntry()
  cmd="setDelay("+self.board+','+self.inpnum+','+delay+')'
  self.vb.io.execute(cmd,log="out",applout="<>")
 def setedge(self):
  edgeA=self.inputedge.getEntry()
  if edgeA == 'P': edge='0'
  elif edgeA == 'N': edge='1'
  else:
     print 'Unknown edge ',edgeA,' no action.'
     return
  cmd="setEdge("+self.board+','+self.inpnum+','+edge+')'
  self.vb.io.execute(cmd,log="out",applout="<>")
  self.inputedge.entry['bg']=COLOR_normal
  self.edge=edgeA       
 def checksignature(self):
  """
    If the status is not error check signature
  """
  if(self.name=='ORBIT'): return
  cmd="CheckSignature("+self.board+","+self.signature+","+self.inpnum+")"
  output=self.vb.io.execute(cmd,log="out",applout="<>")
  #print output
#-----------------------------------------------------
class Measure:
   """
       Measures the phase of the input with ADC for L0,L1 and L2 board
       Measures the phase of the ORBIT with edge mechanism for BUSY board
   """
   #def __init__(self,vb,board,input,name):
   def __init__(self,input):
    #print "Measuring ",board,input
    self.vb=input.vb
    self.board=input.board
    self.input=input.inpnum
    self.name=input.name
    self.inp=input
    self.inputphase=input.inputphase
    self.tl=Toplevel(self.vb.master)
    if(self.board != '0'):
      self.tl.title("ADC "+self.name)
      self.ytitle='ADC'
      self.cmd="measurephase("+self.board+","+self.input+")"
    else:
      self.tl.title("ORBIT phase (EDGE)")
      self.ytitle='EDGE %'
      self.cmd="measureedge()"
##########################################################
    boardhelp="""
0 = BUSY board: measures phase of the input ORBIT
    wrt to BC clock by edge mechanism (see busy board doc)     
1 = L0 board: measures the phases of the input signal
    wrt to BC closck
2 = L1 board
3 = l2 board"""
    inputhelp="""
(1-24) = input number to scan
  0    = ground
  25   = Vcc ~ 255
  26   = 20 MHz ~ 126
  27   = ADC test input ~  /|/"""         
##########################################################
    if(self.checkbcclock(0)):
     self.f1=MywFrame(self.tl,side=TOP)
     self.c1=Graph(self.f1,x0=0.,y0=0.,
           xgraph=32.,nxtick=8,ygraph=260.,nytick=13)
     self.c1.xlabel(text='Delay [ns]')
     self.c1.ylabel(text=self.ytitle)
     self.c1.pack()
     f2=MywFrame(self.tl)
     b0=MywButton(f2,label='Cancel',cmd=self.tl.destroy,side=LEFT,
        helptext='Close the window without accepteng the value.')
     b2=MywButton(f2,label="Measure",cmd=self.measure,side=LEFT,
        helptext="""Measure points again.
Edge: choose negative (for delay:0) if unstability is found around delay 0.
      choose positive if 5 < unstability < 20.
""")       
     self.en=MywEntry(f2,label="DELAY:",helptext=""""List of delay candidates, plaese, choose one of them. 
Edge: choose negative (for delay:0) if unstability is found around delay 0.
      choose positive if 5 < unstability < 20.
""",defvalue="None")
     b1=MywButton(f2,label="OK",cmd=self.ok,side=LEFT,
       helptext="Close the window accepting the value")
     b3=MywButton(f2,label='Save plot',cmd=self.save,side=LEFT,
        helptext="Save plot in directory $VMECFDIR/WORK")
     self.measure() 
############################################################################       
   def checkbcclock(self,board):
    """
       Check id BC clock is present and ready.
    """
    cmd="getbcstatus("+str(board)+")"
    output=self.vb.io.execute(cmd,log="out",applout="<>")
    #print "output=",output,' ',output[0]
    if (output[0] != '0x2'):
       MywError(errmsg="BC clock is not present, staus="+output[0])
       self.tl.destroy()
       return 0
    return 1
   def saveauto(self):
    """
        Save file for every succesfull measurement 
    """
    ss=ss=strftime("_%Y-%m-%d_%H:%M:%S", gmtime())
    fn=os.environ['VMEWORKDIR'] +"/WORK/phases/"+self.name+ss+".ps"
    rc=self.c1.postscript(file=fn)
    if rc is not '':
     MywError(errmsg="File "+fn+" cannot be created.")
     print "rc=",rc,len(rc)
    else:
     print "File ",fn, " saved."
   def save(self):
    """
       Save the postscript file of the plot to WORK directory.
    """
    fn=os.environ['VMEWORKDIR'] +"/WORK/"+"phase.ps"
    rc=self.c1.postscript(file=fn)
    if rc is not '':
     MywError(errmsg="File "+fn+" cannot be created.")
     print "rc=",rc,len(rc)
   def measure(self):
    """
      Measure and plot points again.
    """
    if(self.c1):
      self.c1.destroy()
      self.c1=None
    if(self.checkbcclock(0)):
     output=self.vb.io.execute(self.cmd,log="out",applout="<>")
     #print 'output=',output
     if output[len(output)-1] != '0':
        self.vb.io.write('Error in measurephase.c')
     xy=self.xy(output)
     max=self.finddelay(xy)
     self.c1=Graph(self.f1,x0=0.,y0=0.,xgraph=32.,nxtick=8,
                   ygraph=260.,nytick=13)
     self.c1.plot(xy,'red')
     self.c1.xlabel(text='Delay [ns]')
     self.c1.ylabel(text=self.ytitle)
     self.c1.pack()
     self.en.setEntry(str(max))
     self.c1.update_idletasks()
     self.saveauto()
   def ok(self):
    """
       Accept phase value and destroy ADC window.
    """
    phasein=self.en.getEntry().strip()
    plen=len(phasein)
    #print plen,phasein
    if plen > 4:
      print 'Choose one of the phases, probably the lower one'
      return
    change=0
    phase=''
    for i in phasein:
      if i.isdigit(): 
         phase=phase+i;
      else:
         change=change+1
         continue
    if change > 2:
       print 'Incorect input number:',phasein
       return 
    board=self.board
    print 'Value accepted ',phase
    self.tl.destroy()
    self.inp.phase=phase
    self.inputphase['text']=phase
   def xy(self,output):
    """
       Transforms self.vb.io output (list) to 2-tuples.

       2 tuples corresponds to (x,y)=(delay,ADC).There
       may be many points with the same x.
       Convert dtrings to floats.
    """
    ll=len(output)
    if ll:
      xy=[]
      for i in range(0,ll-2,2):
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
        Find all derivatives > Max = 30,
        Ideally Max ~ 80, but scattered data doeas not work
     """
     if xy:
      listx,listy=self.sort(xy)
      #print 'listx=',listx
      #print 'listy=',listy
      delay=[]
      n=len(listx)
      for i in range(1,n,1):
        der=(listy[i][0]-listy[i-1][0])/(listx[i]-listx[i-1])
        #print listx[i],listy[i][0],der
        if(abs(der)>30.):
          delay.append(int(listx[i]-1))
      #print "delay=",delay
      if (delay==[]): delay=None
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
mainhelp="""Inp:
Input number in CTP board (1-24 for L0,L1; 1-12 for L2)
Level:
Trigger level (L0,L1,L2) of the input
Name: 
Trigger input name
Action:
Get Status (DIM) - get input status (Random,Toggle,Signature) 
Get Edge - reads edge definition for given input. Edge deffinition is in
SYNCAL word (Pedja nomenclature)=SYNCH_ADD (Anton nomenclature).
Set Status (DIM)
Measure phase
Signature: 
Trigger input signature
Check Signature
Status: 
Status as givem by DIM FE server (TIN)
Edge:
P = input triggered by positive edge of BC
N = input triggered by negative edge of BC
Delay:
number of BCs for which input will be delayed 
Phase:
the phase of the input signal wrt to BC (always measured with positive edge)
To measure it choose 'Measure Phase' in the Action Menu of the input.
Last:
flags which input is considered to be last in synchronisation procedure
in order minimise latency (i.e. not to loose any BC)
There should be only one last
Rare:
flag to indicate if triggered snapshot taking should be used
There should be only one rare
"""
def isnumber(number,message):
  """
     For this exists standard python method ?
  """
  print "isnumber ", number
  try:
    num=int(number) 
  except ValueError:
    fulmes=message+' '+number+'is not number'
    print fulmes
    MywError(fulmes)
    return None
  else:
    return num
#
class Corel:
  """
     3 levels : L0,L1,l2 and H0(= hardware L0 level)
  """
  def __init__(self,vb,inputs):
    self.vb=vb
    self.inputs=inputs
    self.inptitle=' ' 
    l0,self.chansl0,self.l0anyinp=self.chans('L0')
    l1,self.chansl1,self.l1anyinp=self.chans('L1')
    l2,self.chansl2,self.l2anyinp=self.chans('L2')
    h0,self.chansh0,self.h0anyinp=self.chans('H0')
    print 'chans: ',self.chansl0,self.chansl1,self.chansl2,self.chansh0
    print 'anyinps:', self.l0anyinp,self.l1anyinp,self.l2anyinp
    self.l0=self.chansl0 != '0x0'
    self.l1=self.chansl1 != '0x0'
    self.l2=self.chansl2 != '0x0'
    self.h0=self.chansh0 != '0x0'
    self.l=self.l0+2*self.l1+4*self.l2
    #self.rb= l0 & 0xf000000   #RND/BC condition flag
    #print 'Corel rn flag: ',hex(self.rb)
    if self.l==0: return;
  def autocor(self):
     """
         Steering for autocorrelation function:
         - open one graph per input to one frame
         - finds which ssm to read
     """
     chans0='0'
     chans1='0'
     chans2='0'
     if self.h0:
       title='Autocorrelation H0 CTP internal signals'
       cmddata='takerbSSM('
       CorellateGroup(self.vb,title,'0',chans0,chans1,chans2,self.chansh0,'0','15',cmddata)
     l=self.l
     if l==0: 
       return
     if l==1:
       title ='Autocorrelation L0 inputs'
       chans0=self.chansl0
       cmddata='take1SSM(1,'
     if l==2:
       title ='Autocorrelation L1 inputs'
       chans1=self.chansl1
       cmddata='take1SSM(2,'        
     if l==4:
       title ='Autocorrelation L2 inputs'
       chans2=self.chansl2
       cmddata='take1SSM(3,'        
     if l==3: 
       title ='Autocorrelation L0 L1 inputs'
       chans0=self.chansl0
       chans1=self.chansl1
       cmddata='take2SSM(1,2,'        
     if l==5: 
       title ='Autocorrelation L0 L2 inputs'
       chans0=self.chansl0
       chans2=self.chansl2
       cmddata='take2SSM(1,3,'        
     if l==6: 
       title ='Autocorrelation L1 L2 inputs'
       chans1=self.chansl1
       chans2=self.chansl2
       cmddata='take2SSM(2,3,'        
     if l==7: 
       title ='Autocorrelation L0 L1 L2 inputs'
       chans0=self.chansl0
       chans1=self.chansl1
       chans2=self.chansl2
       cmddata='take3SSM(1,2,3,'        
     CorellateGroup(self.vb,title,'0',chans0,chans1,chans2,'0','0','15',cmddata)
  def croscor(self):
     """
        In the first call for alignment this should open following windows if corresponding
        inputs exists:
         - one window for L0 inputs
         - one window for L1 inputs
         - one window for L2 inputs
         - one window for H0 inputs
         - one window for L0-L1 inputs alignment choosing to show only
           Line 0 input with max activity and one L1 input with max activity
         - one window for L0-L2
         - one window for L1-L2
         - NOTHING for H0 - Lx at the moment  
     """
     l=self.l
     if len(self.inputs['L0'])>1: 
       title ='Align L0 inputs'
       chans0=self.chansl0
       cmddata='take1SSM(1,'
       CorellateGroup(self.vb,title,'1',chans0,'0','0','0','0','15',cmddata)
     if len(self.inputs['L1'])>1: 
       title ='Align L1 inputs'
       chans1=self.chansl1
       cmddata='take1SSM(2,'
       CorellateGroup(self.vb,title,'1','0',chans1,'0','0','0','15',cmddata)
     if len(self.inputs['L2'])>1: 
       title ='Align L2 inputs'
       chans2=self.chansl2
       cmddata='take1SSM(3,'
       CorellateGroup(self.vb,title,'1','0','0',chans2,'0','0','15',cmddata)
     if len(self.inputs['H0'])>1: 
       title ='Align H0 inputs'
       chansh=self.chansh0
       cmddata='takerbSSM('
       CorellateGroup(self.vb,title,'1','0','0','0',chansh,'0','15',cmddata)
     if l==3: 
       print "Correlatin L0-L1 ----------------"
       title ='Find L0-L1 inputs delay'
       chans0=self.l0anyinp
       chans1=self.l1anyinp
       cmddata='take2SSM(1,2,'
       CorellateGroup(self.vb,title,'1',chans0,chans1,'0','0','250','15',cmddata)
     if l==5: 
       title ='Find L0-L2 inputs delay'
       chans0=self.l0anyinp
       chans2=self.l1anyinp
       cmddata='take2SSM(1,3,'
       CorellateGroup(self.vb,title,'1',chans0,'0',chans2,'0','3520','15',cmddata)
     if l==6: 
       title ='Find L1-L2 inputs delay'
       chans1=self.l1anyinp
       chans2=self.l2anyinp
       cmddata='take2SSM(2,3,'
       CorellateGroup(self.vb,title,'1','0',chans1,chans2,'0','3312','15',cmddata)
     if l==7: 
       chans0=self.l0anyinp
       chans1=self.l1anyinp
       chans2=self.l2anyinp
       title ='Find L0-L1 inputs delay'
       cmddata='take2SSM(1,2,'
       CorellateGroup(self.vb,title,'1',chans0,chans1,'0','0','250','15',cmddata)
       title ='Find L1-L2 inputs delay'
       cmddata='take2SSM(2,3,'
       CorellateGroup(self.vb,title,'1','0',chans1,chans2,'0','3312','15',cmddata)
  def chans(self,level):
    """
    """
    l=0
    first=0
    anyinp=0
    shift=1
    if level == 'H0': shift=0
    for i in self.inputs[level]:
      l=l+(1<<(int(i)-shift))
      anyinp=(1<<(int(i)-shift))
      if first == 0: 
       self.inptitle=self.inptitle+level+' '+i
       first=1
      else: self.inptitle=self.inptitle+' '+i
    print level,' chan=',l,anyinp
    return l,str(hex(l)),str(hex(anyinp))
######################################################################################
class CorellateGroup:
  """
  """
  def __init__(self,vb,title,type,chansl0,chansl1,chansl2,chansh0,cordist,delta,cmddata):
     self.vb=vb
     self.cmddata=cmddata 
     self.tl=Toplevel()
     self.tl.title(title) 
     self.frame=MywFrame(self.tl,side=TOP)
     self.ntimes='1'
     self.type=type
     self.chansl0=chansl0
     self.chansl1=chansl1
     self.chansl2=chansl2
     self.chansh0=chansh0
     #self.rb=rb
     self.dir='1'
     self.cordist=cordist
     self.delta=delta
     self.measure()
     f2=MywFrame(self.tl) 
     b0=MywButton(f2,label='Cancel',cmd=self.tl.destroy,side=LEFT,
        helptext='Close the window.')
     b2=MywButton(f2,label="Update",cmd=self.update,side=LEFT,
        helptext="Measure points again.")       
     self.distEntry=MywEntry(f2,label="Dist",helptext="Autocoerel from (Dist,Dist+Delta",defvalue=cordist)
     self.deltaEntry=MywEntry(f2,label="Delta",helptext="Autocorrel from (Dist,Dist+Delta)",defvalue=delta)
     self.ntimesEntry=MywEntry(f2,label="Take",helptext="How many ssm per sample ?",defvalue="1")
     #b3=MywButton(f2,label='Save plot',cmd=self.save,side=LEFT,
     #   helptext="Save plot in directory $VMECFDIR/WORK")
  def measure(self):
     cmd=self.cmddata+self.ntimes+')'
     output=self.vb.io.execute(cmd,log="out",applout="<>")
     #if(self.rb):
     #output=self.vb.io.execute('takerbSSM('+self.ntimes+')',log='out',applout='<>')
     self.cmd='Correl('+self.type+','+self.chansl0+','+self.chansl1+','+self.chansl2+','+self.chansh0+','+self.cordist+','+self.delta+','+self.dir+')' 
     output=self.vb.io.execute(self.cmd,log="no",applout="<>")
     #print 'output:',output
     graphs,inputs=self.parseoutput(output)
     #print 'inputs',inputs
     #print len(graphs)
     self.gr=[]
     j=0
     for i in graphs:
       xy=self.xy(i)
       gr=Graph(self.frame,xy,xsize=600,ysize=200)
       gr.plot(xy,'red')
       gr.title(inputs[j])
       gr.pack()
       self.gr.append(gr)
       j=j+1
  def update(self):
    if self.gr != []:
       for gr in self.gr:
           gr.destroy()
           del gr
       #del self.gr[0:len(self.gr)]
    #print 'self.gr',self.gr
    dist1=self.distEntry.getEntry()
    delta=self.deltaEntry.getEntry()
    ntimes=self.ntimesEntry.getEntry()
    # this exercisse is due to the bug with negative argument
    #print 'dist0=',dist1[0]
    if dist1[0] == '-': 
     dir='0'
     dist=dist1.replace('-',' ')
     #print dist1
    else: 
     dir = '1'
     dist=dist1
    self.delta=delta
    self.dir=dir
    self.cordist=dist
    self.ntimes=ntimes
    self.measure();
  def i60tonorm(self,inp):
    """
        Transforms 60 inputs to normal notation e.h.:
        0-L0 1, 24-> L1 1
    """
    i=int(inp)
    if i<(24) : return (' L0 '+str(i+1))
    elif i<(24+32) : return (' H0 '+str(i+0-24))
    elif i<(24+32+24): return (' L1 '+str(i+1-24-32))
    else: return (' L2'+str(i+1-48-32))
  def parseoutput(self,output):
    """
    """
    graphs=[]
    graph=[]
    inputs=[]
    for i in output:
       if i[0] == 'i':
          j=i.split()
          i1=self.i60tonorm(j[1])
          i2=''
          if len(j)==3 :i2=self.i60tonorm(j[2])
          inp=i1+i2
          inputs.append(inp)
       elif(i == 'end'):
          graphs.append(graph)
          graph=[]
       else: graph.append(i)
    return graphs,inputs
  def xy(self,output):
    """
       Transforms self.vb.io output (list) to 2-tuples.

       2 tuples corresponds to (x,y)=(delay,ADC).There
       may be many points with the same x.
       Convert dtrings to floats.
       If you want to include the 1st bin uncomment the line below
    """
    ll=len(output)
    if ll:
      xy=[]
      for i in range(0,ll-2,2): # 1st bin in plot
      #for i in range(2,ll-2,2):    # 1st bin out
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

