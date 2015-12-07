#!/usr/bin/python
from Tkinter import *
import os.path, sys
from operator import itemgetter
import myw
INPUT=['sin','detector','nameweb','namectp','eq','signature','dimnum','edge','delay','configured','deltamin','deltamax']
class Inputs:
 def __init__(self):
  """
  """
  self.ctpinputs=None
  self.prefix="../CFG/ctp/DB/"
 ##############################################################################################
 def check(self):
  if self.readctpinputs("ctpinputs.cfg")==1 : return 1 
  #self.printctpinputs()
  ret=self.checkInSw()
  if ret: return ret
  ret=self.checkL0LM()
  return ret
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
 def printctpinputs(self):
  j=0
  print "ctpinputs=============================================="
  for i in self.ctpinputs: 
   print j,' ',i
   j+=1
 ##############################################################################################
 def readctpinputs(self,filename):
  """
  """
  file=self.prefix+filename
  lines = self.openFile(file)
  if lines==None: 
   print "Cannot open: ",file
   return 1
  self.ctpinputs = self.parsectpinputs(lines)
  if self.ctpinputs: return 0
  else: return 1
 ##############################################################################################
 def parsectpinputs(self,lines):
  """
  """
  valctpinps=[]
  for line in lines:
    line=line.strip(' ')
    line=line.strip('\tab')
    items=line.split(" ")
    #print items
    if(items[0][0]=="#"): continue
    if(items[0][0]=="l"): continue  # function ignoring for the moment
    iteminp=[]
    for i in items:
      if i=='': continue
      if i[0]=='#': break
      iteminp.append(i.strip('\n'))
      #print iteminp,' ',len(iteminp)
    nitems=len(iteminp)
    if nitems < 4:
      print "Incorrect line (too short):"
      print line
      return None;
    if iteminp[3] == 'M': 
      if nitems != 10:
       print "Incorrect M level line (nitems=",nitems," != 10):"
       print line
       return None
    else:
      if nitems != 17:
       print "Incorrect L0/L1/L2 line (nitems=",nitems," != 17):"
       print line
       return None
    if len(iteminp) >= 10:
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
      #input['deltamin']=iteminp[10]
      #input['deltamax']=iteminp[11]
      valctpinps.append(input)
  #print input
  return sorted(valctpinps,key=itemgetter("namectp")) 
 ###############################################################################################
 def checkInSw(self):
  """
   check for configured inputs if 2 inputs connected to same input number
   check for configured inputs if 2 inputs conneteced to same switch number
  """
  inps=self.ctpinputs
  leninp = len(inps)
  for i in range(leninp):
   if inps[i]['configured'] == '0': continue
   if inps[i]['inpnum'] == '0': continue
   for j in range(i+1,leninp):
     if inps[j]['configured'] == '0': continue
     if inps[j]['inpnum'] == '0': continue
     if inps[i]['level'] != inps[j]['level']: continue
     if inps[i]['inpnum'] == inps[j]['inpnum']:
      print 'ERROR: inputs with same In : ---------------------------------------'
      print inps[i]   
      print inps[j]   
      return 1
     if inps[i]['configured'] == inps[j]['configured']:
      print 'ERROR: inputs with same Sw : ---------------------------------------'
      print inps[i]   
      print inps[j]   
      return 1
  return 0
 ###############################################################################################
 def checkL0LM(self):
  """
   check if every LM input has same L0 input and diff of delays is 15
  """
  inps=self.ctpinputs
  leninp = len(inps)
  ret=0
  for i in range(leninp):
   if inps[i]['configured'] == '0': continue
   if inps[i]['inpnum'] == '0': continue
   if inps[i]['level'] != 'M': continue
   flag=1
   for j in range(leninp):
     if inps[j]['configured'] == '0': continue
     if inps[j]['inpnum'] == '0': continue
     if inps[j]['level'] != '0': continue
     if inps[i]['inpnum'] == inps[j]['inpnum']:
      #print i,' ',j
      #print inps[i]   
      #print inps[j]   
      flag=0
      if inps[i]['namectp'] != inps[j]['namectp']: ret=1
      if inps[i]['detector'] != inps[j]['detector']: ret+=10
      if inps[i]['configured'] != inps[j]['configured']: ret+=100
      if ret: print 'ERROR: different L0 and LM inputs: ', ret
      delay= int(inps[j]['delay']) - int(inps[i]['delay'])
      if delay != 15:
       ret += 1000
       print 'ERROR: wrong delays for L0/Lm inputs'
      if ret:
       print inps[i]   
       print inps[j]   
       return ret
   if flag:
     print 'ERROR: LM input not found in L0 inputs:'
     print inps[i]
     return 10000
  return 0
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
################################################################################  
def main():
 print sys.argv
 #if len(sys.argv) == 2:
 if 1:
   inps=Inputs()
   ret=inps.check()
   print 'ret=', ret
   return ret
 elif len(sys.argv) == 1:
   # edit and archive - not finished 
   # instead in inputs_u.py at every measuremtn plot is saved
   inps=Inputs()
   f=Tk()
   editor=InputPhasesEditor(f,inps)
   f.mainloop()
 else:
   print "Wrong number of arguments." 
if __name__ == "__main__":
    main()
