#!/usr/bin/python
import string,types

DEFLUMACT=1.0
lumi=None

def getlumi():
  """ rc: 
  float number: luminosity in hz/ub
  None if luminosity DIM service not available, or lumi<1 hz/ub
  """
  import pydim
  rc=None
  try:
    rclumi = pydim.dic_sync_info_service("IR_MONITOR/CTP/Luminosity", ("F",), 2)
    if rclumi!=None:
      lumi= rclumi[3]
      if lumi>1.0:
        rc=lumi/1.0E30   # hz/ub
  except:
    print "Except: in dic_sync_info_service(IR_MONITOR/CTP/Luminosity)"
  return rc

lumi= getlumi()
if lumi==None: lumi= DEFLUMACT

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
    dfn= a/lumi
    if fixll=="FIXLOSS":
      dfn= dfn*dfn
    if dfn<0.000001: df= "0%"
    elif dfn>99.99999: df= "100%"
    else: df= str(dfn*100) + '%'
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
