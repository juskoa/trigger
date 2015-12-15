#!/usr/bin/python
# dec 2015: see vistar.png for explanation, why we use
#           DEFLUMI=1.0 and 1.0E27 dividing in Pb-Pb
import string,types

DEFLUMI=1.0   # for Pb: 1.0, for p-p it was 3.0 (and 1E30)
lumi= DEFLUMI
lumi_source=None   # "dim" or "default". None: preproc.getlumi() never called

def getlumi():
  """ set 2 globals: lumi + lumi_source
  lumi: float number, luminosity in hz/ub
  lumi_source: how lumi was acquired
     dim:     dim
     default: dim was not available, or dip published number <1.0
     None:    not initialized (i.e. getlumi not called)
  """
  global lumi,lumi_source
  import pydim
  rc=None
  try:
    #Enabled 13.11.2015 and changed 1.0E30 -> 1.0E27 for HI run
    #rclumi = pydim.dic_sync_info_service("IR_MONITOR/CTP/Luminosity", ("F",), 2)
    #rclumi= None
    # In run1 next line invoked 'TypeError: argument list must be a tuple' stdout line
    # when dim service not available.
    rclumi = pydim.dic_sync_info_service("IR_MONITOR/CTP/Luminosity", "F", 2)
    if rclumi!=None:
      lumidim= rclumi[3]
      if lumidim>1.0:
        lumi_source= "dim"
        lumi=lumidim/1.0E27    # hz/ub (was 1.E30 before 13.11.2015)
      else:
        lumi_source= "default"
        lumi= DEFLUMI
    else:
      lumi_source= "default"
      lumi= DEFLUMI
      print "preproc.getlumi: IR_MONITOR/CTP/Luminosity not available, default:%f taken."%lumi
  except:
    print "Except: in dic_sync_info_service(IR_MONITOR/CTP/Luminosity)"
    lumi_source= "default"
    lumi= DEFLUMI

#getlumi()  invoked form parted.py after importing preproc

class symbols:
  def __init__(self):
    # self.typ[key]: "REPL" or "FIXLUM" or FIXLOSS or FIXPOWER
    # self.symbols[key]: string or [deflum_float, a_float, b_float]
    self.typ={} ; self.symbols={}
  def add(self, key, value):
    #if self.symbols.has_key(key):
    self.typ[key]= "REPL"
    #else:
    #  self.typ[key]= None
    self.symbols[key]= value
  def add_FIXLL(self, key, value, fixll):
    """ Input: key: symbolic name
    value: 'floatnumber' or 'n floatnumb' in case of FIXPOWER
    fixll: FIXLUM or FIXLOSS or FIXPOWER (npower needed)
    rc: None -> ok   rc: string -> Error
    """
    global lumi
    ao= string.split(string.strip(value))  # original string
    npowertxt= None
    if len(ao) > 1:
      inm= ao[0]; ao[0]= ao[1]; ao[1]= inm
      npowertxt= ao[1]
    try:
      a= float(ao[0])
    except:
      return "Incorrect %s line:%s"%(fixll,value)
    if lumi==None:
      return "lumi (service IR_MONITOR/CTP/Luminosity) not available"
    dfn1= a/lumi
    if fixll=="FIXLUM":
      dfn= dfn1
    elif fixll=="FIXLOSS":
      dfn= dfn1*dfn1
    elif fixll=="FIXPOWER":
      if npowertxt == None:
        return "Expected: 'FIXPOWER N float' in line:%s"%(fixll+" "+value)
      try:
        npower= int(npowertxt)
      except:
        return "Expected: 'FIXPOWER int float' in line:%s"%(fixll+" "+value)
      dfn= dfn1
      for n in range(npower-1):
        dfn= dfn*dfn1
    if dfn<0.000001: df= "0%"
    elif dfn>0.99999: df= "100%"
    else: df= "%.7f"%(dfn*100) + '%'   # g: can return 1.1e-05 !, better f
    self.typ[key]= fixll
    self.symbols[key]= df
    return None   # i.e. ok
  def add_lindf(self, key, value):
    global lumi
    ao= string.split(string.strip(value))  # original strings
    try:
      if ao[0]=="act":
        deflum= DEFLUMIACT
      else:
        deflum= float(ao[0])
      a= float(ao[1])
      b= float(ao[2])
    except:
      return "Incorrect LINDF line:%s"%value
    if lumi==None: lumi= deflum
    dfn= a + b*(lumi/deflum)
    if dfn<0.000001: df= "0%"
    elif dfn>99.99999: df= "100%"
    else: df= str(dfn) + '%'
    self.typ[key]= "LINDF"
    self.symbols[key]= df
    return None   # ok
  def get(self, key):
    if self.symbols.has_key(key):
      return self.symbols[key]
    return None
    
def main():
  print "main..."
if __name__ == "__main__":
    main()
