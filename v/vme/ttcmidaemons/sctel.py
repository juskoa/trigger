#!/usr/bin/python
import sys,telnetlib,time,string,os
class TN:
  scpi="SCPI> "
  def __init__(self):
    try:
      self.tn = telnetlib.Telnet('alidcsaux009.cern.ch', 5024)    # ,timout_insec from 2.6
      # will be: alictpclockmon
      #time.sleep(2)
      self.tn.write("\n")
      #time.sleep(1)
      rep=self.tn.read_until(self.scpi)
    except:
      print "ERROR:cannot open telnet to alidcsaux009"
      #print sys.exc_info()
      rep=""
    print "sctel.prompt1:%s"%rep
    self.prompt1=rep
    # here we should check prompt and set self.tn to None if error
  def close(self):
    # did not close: "", "\004", "\004\n", "003" 
    time.sleep(1)
    try:
      self.tn.write("\003\n")    # ok (closes connection)
      self.tn.close()
    except:
      print "ERROR:cannot writeclose"
      #print sys.exc_info()
    self.prompt1=""
  def readPrompt(self):
    try:
      rep= self.tn.read_until(self.scpi)
    except:
      print "ERROR:readPrompt"
      #print sys.exc_info()
      rep=""
    return rep
  def cmdS(self, cmd):
    try:
      self.tn.write(cmd)
    except:
      print "ERROR:cannot write cmdS:",cmd
      #print sys.exc_info()
      return ""
    rep= self.readPrompt()
    return rep
  def cmdF(self, cmd):
    try:
      self.tn.write(cmd)
    except:
      print "ERROR:cannot write cmdF:",cmd
      #print sys.exc_info()
      return ""
    rep= self.readPrompt()
    if rep=="": return rep
    fnum= string.strip(string.split(rep,'\n')[0])   # return first line
    #print 'cmdF['+rep+']('+fnum+')'
    return fnum
  def setPersitence(self,mininf):
    rep= self.cmdS(":DISPLAY:PERSISTENCE %s\n"%mininf)
  def measure(self):
    #while True:
    #rep= self.cmdS(":SYSTEM:TIME?\n")
    #print rep
    #ti= string.replace(rep[:8],',',' ')
    #print 'ti['+rep+']('+ti+')'
    ti= "%.3f"%time.time()   #epoch
    fo3= self.cmdF(":MEASURE:DELTATIME? CHANNEL2,CHANNEL4\n")
    fo3rep=0
    while string.find(fo3,":MEAS")>=0:
      fo3rep= fo3rep+1
      if fo3rep>=3:
        fo3=""
        break
      print "fo3 once more %d time"%fo3rep
      fo3= self.cmdF(":MEASURE:DELTATIME? CHANNEL2,CHANNEL4\n")
    #print "=",ti, fo3,"="
    corde= self.cmdF(":MEASURE:DELTATIME? CHANNEL2,CHANNEL3\n")
    fo2= self.cmdF(":MEASURE:DELTATIME? CHANNEL2,CHANNEL1\n")
    if (fo3=="") or (corde=="") or (fo2==""):
      return "%s"%ti
    else:
      return "%s %s %s %s"%(ti, fo3, corde, fo2)
  
def main():
  #lt= time.localtime()
  #ltim= "%2d.%2d. %2d:%2d"%(lt[2], lt[1], lt[3], lt[4])
  # "%2.2d%2.2d%2.2d%2.2d%2.2d"%(lt[0]-2000,lt[1],lt[2],lt[3],lt[4])
  #today= "%4.4d-%2.2d-%2.2d"%(lt[0],lt[1],lt[2])
  #tfile= "osc-"+today
  #FOUT= open(tfile,"a")
  #rep= tn.cmdS(":SYSTEM:TIME?\n")
  #print "file:", tfile   #,"rep1:",rep
  if os.environ['VMESITE']!='ALICE':
    print "sctel.py: not ALICE environment. argv:", sys.argv
    return
  tn=None
  if (len(sys.argv)>1) and \
    ((sys.argv[1]=="M") or (sys.argv[1]=="INF") or (sys.argv[1]=="MIN")\
      or (sys.argv[1]=="MININF") ):
    tn= TN()
  if (len(sys.argv)>1) and ((sys.argv[1]=="INF") or (sys.argv[1]=="MIN")):
    tn.setPersitence(sys.argv[1])
  elif (len(sys.argv)>1) and (sys.argv[1]=="MININF"):
    tn.setPersitence("MIN")
    time.sleep(1)
    tn.setPersitence("INF")
  elif (len(sys.argv)>1) and (sys.argv[1]=="M"):
    if tn.prompt1=="": 
      sys.exit(8)
    for ix in range(3):
      time.sleep(2)       # usually we leave 10secs between measurements
      print tn.measure()
  else:
    print """
INF -switch persistence to infinite
MIN -switch persistence to minimum
M   -measure (3 lines) -i.e. show on stdout Epoch_time fo3 corde co2

DRS4 notes:
edge1 = corde (fo1) - rf2ttc(fo2)
edge2 = fo2-fo3 (both rf2ttc, used for debuging)
edge3 = nothing
value1 = sigma of edge1
value2 = sigma of edge2

"""
  if tn!=None:
    tn.close()
 
if __name__ == "__main__":
  main()
