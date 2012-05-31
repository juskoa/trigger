#!/usr/bin/python
import string,pydim_ex, trigdb

def measureFOLTU(ctpdim, ltuname):
  """ Input: ctpdim  -has to be connected
             ltuname -name of the ltu to be measured
  return:
  None:  error
  (ltuphase, fophase)    tuple of 2 integers
  """
  print "--------------------------- %s connecting..."%ltuname
  ltu= pydim_ex.DimServerLtu(ltuname)   # connect
  # measure FO phase. Actually, it is enough to measure only once
  # -4 measurements for 4 connectors should be equal
  if ltu.tag==0: return None   # connect error
  answer=ctpdim.execDO("TOGGLE 1 "+ltuname.upper())
  answer=ltu.execDO("setglobalmode()\n")
  answer=ltu.execDO("getbcphase(0)\n")
  #print "%s:FO phase>%s<"%(ltu.ltuname,answer)
  fophase= int(answer)
  #   restore normal conditions (no toggle, stdalone):
  answer=ctpdim.execDO("TOGGLE 0 "+ltuname.upper())
  answer=ltu.execDO("setstdalonemode()\n")
  #answer=ctpdim.execDO("W 1\n")
  # measure ltu phase:
  answer=ltu.execDO("getbcphase(1)\n")
  #print "%s:phase>%s<"%(ltu.ltuname,answer)
  ltuphase= int(answer)
  ltu=None   # disconnect
  return (ltuphase, fophase)

class fout:
  def __init__(self):
    self.phases= [None, None, None, None]
  def prt(self):
    return str(self.phases)

def main():
  print "--------------------------- CTPDIM connecting..."
  ctp= pydim_ex.DimServerCtp()
  if ctp.tag==0:
    ctp.errprt("exiting...") ; return 
  phases={}
  fos_phases=[]
  for ix in (1,2,3,4,5,6):
    fos_phases.append(fout())
  #measure busy L0/1/2 INT:
  answer=ctp.execDO("CHECKPHASES")
  print "ctp:BUSY L0/1/2 INT phases:>%s<"%answer
  #
  #ltu_phase= measureFOLTU(ctp, "hmpid")
  #return
  ps= string.split(answer);
  if len(ps) != 5:
    ctp.errprt("Bad ctp answer:%s"%answer) ; return 
  phases['busy']= int(ps[0]); phases['l0']= int(ps[1])
  phases['l1']= int(ps[2]); phases['l2']= int(ps[3])
  phases['int']= int(ps[4])
  ltus= trigdb.readVALIDLTUS()
  for ltu in ltus:
    if ltu.fo!=0:   #connected LTU
      ltulower= ltu.name.lower()
      ps= measureFOLTU(ctp, ltulower)
      if ps==None: continue
      phases[ltulower]= ps[0] 
      #fo:
      fos_phases[ltu.fo-1].phases[ltu.focon-1]= ps[1]
  #for ix in range(100):
  #  answer=proxy.execDO("vmeopr32(VERSION_ADD)\n")
  #close connection:
  ctp=None;
  print "CTP and LTU boards (except FOs) phases:"
  print phases
  print "CTP FO boards phases (each FO connector measured independently with its LTU):"
  for ix in (1,2,3,4,5,6):
    print ix,':',fos_phases[ix-1].prt()

if __name__=="__main__":
  main()

