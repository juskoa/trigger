#!/usr/bin/python
import string,types

DEFLUMI=3.0
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
    #rclumi = pydim.dic_sync_info_service("IR_MONITOR/CTP/Luminosity", ("F",), 2)
    # next line invokes 'TypeError: argument list must be a tuple' stdout line
    # when dim service not available.
    rclumi = pydim.dic_sync_info_service("IR_MONITOR/CTP/Luminosity", "F", 2)
    if rclumi!=None:
      lumidim= rclumi[3]
      if lumidim>1.0:
        lumi_source= "dim"
        lumi=lumidim/1.0E30   # hz/ub
      else:
        lumi_source= "default"
        lumi= DEFLUMI
    else:
      lumi_source= "default"
      lumi= DEFLUMI
  except:
    print "Except: in dic_sync_info_service(IR_MONITOR/CTP/Luminosity)"
    lumi_source= "default"
    lumi= DEFLUMI

#getlumi()  invoked form parted.py after importing preproc

class symbols:
  def __init__(self):
    # self.typ[key]: "REPL" or "FIXLUM" or FIXLOSS
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
    value: float number
    fixll: FIXLUM or FIXLOSS
    rc: None -> ok   rc: string -> Error
    """
    global lumi
    ao= string.split(string.strip(value))  # original string
    af=[]                                 # floats
    try:
      a= float(ao[0])
    except:
      return "Incorrect %s line:%s"%(fixll,value)
    if lumi==None:
      return "lumi (service IR_MONITOR/CTP/Luminosity) not available"
    dfn= a/lumi
    if fixll=="FIXLOSS":
      dfn= dfn*dfn
    if dfn<0.000001: df= "0%"
    elif dfn>0.99999: df= "100%"
    else: df= "%.6f"%(dfn*100) + '%'
    self.typ[key]= fixll
    self.symbols[key]= df
    return None   # i.e. ok
  def add_lindf(self, key, value):
    global lumi
    ao= string.split(string.strip(value))  # original strings
    af=[]                                 # floats
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
