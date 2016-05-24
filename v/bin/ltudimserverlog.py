#!/usr/bin/python
import os,os.path,sys,string,time
def docmd(cmd):
  import subprocess
  sp= subprocess.Popen(string.split(cmd), bufsize=1,
    stdin=subprocess.PIPE, stdout=subprocess.PIPE, close_fds=True)
  lsf= (sp.stdout, sp.stdin)
  out=""
  while(1):
    line= string.rstrip(lsf[0].readline())
    # lsf[1].write...
    #print line
    out= out + line
    if line==None or line=='': break
  lsf[0].close
  lsf[1].close
  return out

def findbusy(fn, datum, timefrom, timeto):
  #print "file:", fn
  fi= open(fn); linen=0; linenok=0
  for line in fi.readlines():
    linen= linen+1
    if line== '\n': continue
    ll= string.split(line," ")
    if ll[0][:14]!="updateMONBUSY:": continue
#                   01234567890123
# updateMONBUSY:18.05.2016 09:10:07:oldbusy: 0.0000 newbusytime:0.0545 nclients:0
# 0                        1                 2      3                  4
#:0             1                   2       3                   4
    dat = string.split(ll[0],":")[1]
    tim = ll[1][:8]
    oldb= ll[2]
    newb= string.split(ll[3],":")[1]
    #print string.strip(line)
    #print "dat:", dat, tim, oldb, newb
    if (dat == datum) and (tim>= timefrom) and (tim<= timeto):
    #if (dat == datum):
      #print string.strip(line)
      print "%s %s %s -> %s"%(dat, tim, oldb, newb)
      linenok= linenok+1
    #if linenok>5: break
  fi.close()
  
def main(argv):
  if len(argv) < 6:
    print """
ltudimserverlog.py busy det date timefrom timeto
e.g.
ltudimserverlog.py busy ssd 18.05.2016 08:57:00 09:09:00

stdout:

"""
    return
  fn= string.split(docmd("cdvme "+argv[2]))[1]
  #print fn
  if len(fn) < 10:
    print "Bad detector:", argv[2]
    sys.exit(8)
  fn= os.path.join(fn, "ltudimserver.log")
  #fn= "ltudimserver.log"
  timeto= argv[5]
  print "file:", fn
  print argv[2]+":"
  findbusy(fn, argv[3], argv[4], timeto)

if __name__ == "__main__":
    main(sys.argv)
