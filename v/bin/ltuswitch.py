#!/usr/bin/python
"""
This script, if to be used by users (simple authentication
through their login name) should use setuid-proc.c (setuid already set when
called), or to be started from pydim server.
I.e. now, it can be started only from trigger@26 by trigger shifter.
"""
import os, pwd, sys, trigdb

l0all= trigdb.TrgL0INPUTS()
ltuswitch= trigdb.TrgSwitchLTU()
def show():
  print "\nltu <- CTPin_signal"
  print "--------------------"
  unconnected={}
  for k in trigdb.TrgSwitchLTU.ALL.keys():
    unconnected[k]= ltuswitch.ALL[k]
  for ent in ltuswitch.ents:
    if ent[0][0]=='#': continue
    sout= ent[4]
    #print "sout:",sout
    user= ltuswitch.ALL[sout]
    del unconnected[sout]
    #user= trigdb.TrgSwitchLTU.ALL[sout]
    ix= l0all.findixof(0, ent[3])
    #print "l0ent[%s]:"%ent[3], ix,":"
    #print l0all.ents[ix]
    signame="%s:%s"%(l0all.ents[ix][1],l0all.ents[ix][3])
    print "%s <- %s"%(user, signame)
    #print "not connected:",unconnected
def showltu(user):
  line= ltuswitch.getltu(user)
  if line!=None:
    sin= line[3]
    ix= l0all.findixof(0, sin)
    signame="%s:%s"%(l0all.ents[ix][1],l0all.ents[ix][3])
    print "%s <- %s"%(user, signame)
  else:
    print "%s <- not connected"%user
def connect(user, signal):
  line= ltuswitch.getltu(user)
  if line==None:
    print "%s not connected"%user
    return
  if len(line)==0:
    sin= None
    print "%s not connected yet"%user
  else:
    sin= line[3]   # curent connection to this switch input
    print "%s connected now:%s"%(user,sin)
  ix= l0all.findixof(3, signal)
  if ix==None:
    print "Unknown signal name:%s"%signal
    return
  newcable= l0all.ents[ix][2]
  newctpname= l0all.ents[ix][3]
  newsin= l0all.ents[ix][0]
  print "new:", newcable, newctpname, newsin
  rc= ltuswitch.setltu(user, newcable, newctpname, newsin)
  if rc==None:
    print "LTU.SWITCH neither LTU switch not updated"
  else:
    ltuswitch.writetable()
    trigdb.pexecute("ssh -q -2 trigger@alidcsvme004 'loadswitch ltu'")
  #signame="%s:%s"%(l0all.ents[ix][1],l0all.ents[ix][3])
  
def prtusage(trglast):
    print """Usage:
ltuswitch L0input %s
  -connect L0input (e.g. 0ASL) to your LTU, i.e. to TPC LTU if
   this script was started from tpc@alidcscom026

ltuswitch show 
  -show current connections of all LTUs

ltuswitch showall
  -show all the possible signals your LTU can be connected to
"""%trglast

def main():
  user= pwd.getpwuid(os.getuid())[0]
  ltuname= user
  npars= len(sys.argv)
  #user='hmpid'
  if user=='trigger':
    # last par. is LTU (or v0fee) to be connected
    ltuname= sys.argv[-1]
    prthlp="ltuname" 
    npars= npars-1
  else:
    prthlp=""
  if npars<2:
    if npars<1:
      prtusage(prthlp)
      print "ul:", user, ltuname
      return
    if user!='trigger':
      showltu(ltuname)
    else:
      show()
      ltuswitch.prtall()
    if sys.argv[1]=='showall':
      l0all.prtnames()
    elif sys.argv[1]=='show':
      show()
  else:
    if ltuswitch.ltu2so(ltuname) != None:
      #showltu(user)
      connect(ltuname, sys.argv[1])
      showltu(ltuname)
    else:
      print "Bad detector:%s. Only these LTUs are connected to SWITCH:"%ltuname
      show()
if __name__ == "__main__":
    main()

