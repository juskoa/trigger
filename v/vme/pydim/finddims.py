#!/usr/bin/python
import string,pydim, trigdb

def ltuon(ltuname):
  print "--------------------------- %s connecting..."%ltuname
  ltu= pydim.DimServerLtu(ltuname)   # connect
  if ltu.tag==0:
    print "ltuproxy: %s down"%ltuname
    return 0
  else:
    print "ltuproxy: %s ok"%ltuname
    return 1

def main():
  print "--------------------------- CTPDIM connecting..."
  ctp= pydim.DimServerCtp()
  if ctp.tag==0:
    ctp.errprt("exiting...") ; return 
  else:
    print "ctpdims ok"
  ctp=None;
  ltus= trigdb.readVALIDLTUS()
  for ltu in ltus:
    if ltu.fo!=0:   #connected LTU
      ltulower= ltu.name.lower()
      ltuon(ltulower)

if __name__=="__main__":
  main()

