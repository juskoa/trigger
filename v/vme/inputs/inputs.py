#!/usr/bin/python
import os.path, sys
from operator import itemgetter
INPUT=['sin','detector','nameweb','namectp','eq','signature','dimnum','edge','delay','configured','deltamin','deltamax']
class Inputs:
 def __init__(self):
  """
  """
  self.l0inputs=None
  self.inpphases=None
  self.validctpinputs=None
 ##############################################################################################
 def checkVALIDCTPINPUTS(self):
  """
     Read VALID.CTPINPUTS,L0.INPUTS,INPUT.phases
     Checks consistency of VALID.CTPINPUTS against L0.INPUTS and INPUT.phases 
  """
  prefix="../CFG/ctp/DB/"
  if self.readL0inputs(prefix+"L0.INPUTS"): return
  if self.readPhases(prefix+"INPUT.phases"): return
  if self.readVALIDCTPINPUTS(prefix+"VALID.CTPINPUTS"): return
  l0inputs=self.l0inputs
  phases=self.inpphases
  notinL0INPUTS=[]
  noPhases=[]
  for input in self.validctpinputs:
    index=0
    while(index< len(l0inputs) and input['namectp'] != l0inputs[index]['namectp']): index+=1
    if index == len(l0inputs) and input['level']=='0':
      notinL0INPUTS.append(input['namectp'])
    else:
      # Compare inputs
      if input['level']=='0': self.Compare2Inputs(input,l0inputs[index])
      # check phase
      indexp=0
      while(indexp<len(phases)) and input['namectp'] != phases[indexp]['namectp']:indexp+=1
      if indexp ==  len(phases):
       noPhases.append(input)
      else:
       phase=int(phases[indexp]['phase'])
       #print input['namectp']," ",phase
       edge='1'
       if phase>=6 and phase <=19: edge='0'
       if input['edge'] != edge: 
          print "---> ",input['namectp'],' VALID.CTPINPUTS edge: ',input['edge'], ' edge from phases: ',edge
  if(len(notinL0INPUTS)): 
    print "---> L0 inputs in VALID.CTPINPUTS not found in L0.INPUTS:"
    print ",".join(notinL0INPUTS).replace(",", " ")
  if(len(noPhases)):
    print "---> VALID.CTPINPUTS with phase not in table INPUT.phase:"
    printlist=[]
    for i in noPhases: printlist.append(i['namectp'])
    print ",".join(printlist).replace(",", " ")
  return      
 ##############################################################################################
 def Compare2Inputs(self,inpvctp,inpl0):
  """
  """
  del inpvctp['inpnum'] 
  del inpvctp['level']
  del inpl0['nameweb']
  del inpl0['eq']
  del inpl0['sin']
  shareditems =  set(inpvctp.items()) & set(inpl0.items())
  if(len(shareditems)==9): return 0
  difference =  set(inpvctp.items()) ^ set(inpl0.items())
  print "---> Different inputs ",inpvctp['namectp'],' :'
  print difference
  return 1
 ##############################################################################################
 def readVALIDCTPINPUTS(self,filename):
  """
  """
  lines = self.openFile(filename)
  if lines==None: return 1
  self.validctpinputs = self.parseVALIDCTPINPUTS(lines)
  #self.printVALIDCTPINPUTS()
  return 0
 ##############################################################################################
 def printVALIDCTPINPUTS(self):
  for i in self.validctpinputs: print i
 ##############################################################################################
 def parseVALIDCTPINPUTS(self,lines):
  """
  """
  valctpinps=[]
  for line in lines:
    line=line.strip(' ')
    line=line.strip('\tab')
    items=line.split(" ")
    if(items[0][0]=="#"): continue
    iteminp=[]
    for i in items:
      if i=='': continue
      if i[0]=='#': break
      iteminp.append(i.strip('\n'))
    if len(iteminp) == 12:
      input={}
      input['namectp']=iteminp[0]
      input['detector']=iteminp[2]
      input['level']=iteminp[3]
      input['signature']=iteminp[4]
      input['inpnum']=iteminp[5]
      input['dimnum']=iteminp[6]
      input['configured']=iteminp[7]
      input['edge']=iteminp[8]
      input['delay']=iteminp[9]
      input['deltamin']=iteminp[10]
      input['deltamax']=iteminp[11]
      valctpinps.append(input)
  return sorted(valctpinps,key=itemgetter("namectp"))  
 ##############################################################################################
 def readPhases(self,filename):
  """
  """
  lines = self.openFile(filename)
  if lines==None: return 1
  self.inpphases = self.parseInputsPhases(lines)
  #self.printInpPhases()
  return 0
 #############################################################################################
 def printInpPhases(self):
  for i in self.inpphases: print i
 #############################################################################################
 def parseInputsPhases(self,lines):
  """
  """
  inpphases=[]
  for line in lines:
    line=line.strip(' ')
    line=line.strip('\tab')
    items=line.split(" ")
    if(items[0][0]=="#"): continue
    itemph=[]
    for i in items:
      if i=='': continue
      if i[0]=='#': break
      itemph.append(i.strip('\n'))
    if len(itemph) < 3:
       print "Warning: parseInputsPhases: wrong number of items ", len(items)
       print line
    phase={}   
    phase['namectp']=itemph[0]
    phase['signature']=itemph[1]
    iphase=itemph[2].split('/')
    phase['phase']=iphase[0]
    inpphases.append(phase)
  return sorted(inpphases,key=itemgetter("namectp"))
 #############################################################################################
 def readL0inputs(self,filename):
  """
     Reads L0 inputs from file and creates input dictionary
  """
  linesname=self.openFile(filename)
  if linesname==None: return 1
  inputs=self.ParseL0inputs(linesname)
  self.l0inputs=self.inputs2dictionary(inputs)
  #self.printInpsDict()
  return 0
 ##############################################################################################
 def printInpsDict(self):
  for i in self.l0inputs: print i
 ##############################################################################################
 def Compare2L0inputs(self,filename,sin):
  """
     Compares two L0.INPUTS files : filename and filename.act
  """
  filenameact=filename+".act"
  linesname=self.openFile(filename)
  if linesname==None: return
  linesnameact=self.openFile(filenameact)
  if linesnameact==None: return
  inputs=self.ParseL0inputs(linesname)
  inputsact=self.ParseL0inputs(linesnameact)
  self.Compare(inputs,inputsact,sin)
 ############################################################################################### 
 def Compare(self,inputs,inputsact,sin):
  """
     Compare inputs in list format
     sin=0 : compare inputs with same sin
     sin=3: compare innnnnnputs with same ctpname
  """
  for inp in inputs:
    for inpact in inputsact:
      if inp[sin] == inpact[sin]: self.compare(inp,inpact)
 def compare(self,inp,inpact):
  inum=0
  flag=0
  for i in inp:
   if i != inpact[inum]:
    print 'Input ',inp[0],' ',INPUT[inum],' ',inp[inum],' ',inpact[inum]
    flag=1
   inum +=1
  if flag:
   print inp
   print inpact
 ############################################################################################### 
 def openFile(self,filename):
  """
      Input file.
  """
  #filename="L0inputs/"+filename
  try:
   infile=open(filename,"r")
  except IOError:
   print "Cannot open file ",filename
   return None
  else:
   print "File " ,filename," opened succesfully"
   lines=infile.readlines()
   infile.close()
   return lines
 #########################################################################################  
 def inputs2dictionary(self,inputs):
  """
     Creates inputs dictionary from inputs as tokens created by ParseL0inputs
  """
  inpsdict=[]
  for i in inputs:
    input={}
    for j in range(12):input[INPUT[j]]=i[j]
    inpsdict.append(input)
  return sorted(inpsdict,key=itemgetter("namectp"))	
 ############################################################################################### 
 def ParseL0inputs(self,lines):
  """
     Input is tokenised line of file L0.inputs
  """
  inputs=[]
  for line in lines:
    line=line.strip(' ')
    line=line.strip('\tab')
    items=line.split(" ")
    if(items[0][0]=="#"): continue
    #print items
    itemnum=0
    input=[]
    for i in items:
      if i=='': continue
      if i[0]=='#': break
      input.append(i.strip('\n'))
    if input[0] == '0': continue  
    if(len(input)==1 and input[0]==''): continue
    if(len(input)!= 12):
      print "WARNING: wrong number of items: ",len(input)
      print input
      return None
    #print input
    inputs.append(input)
  if(len(inputs) != 50):
    print "Wrong number of inputs: ", len(inputs)
    return None
  return inputs
################################################################################  
def main():
 #print sys.argv
 #lif len(sys.argv) != 3:
 if 0:
   print 'Wrong number of argumenst'
 else:
   inps=Inputs()
   inps.checkVALIDCTPINPUTS()
   return
   inps.readL0inputs(sys.argv[1])
   inps.readPhases("INPUT.phases")
   inps.readVALIDCTPINPUTS("VALID.CTPINPUTS")
   #l0inps.Compare2L0inputs(sys.argv[1],int(sys.argv[2]))
if __name__ == "__main__":
    main()
