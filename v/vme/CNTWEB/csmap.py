#!/usr/bin/python
#from cgi import escape
#from urllib import unquote
import sys,subprocess,string,cntwebenv
from mod_python import util
#sys.path.append(cntwebenv.VMECFDIR+"filling")
#import lhc2ctp
import popen2
def cmdex(cmd, str1=None):
  iop=  popen2.popen2(cmd, 1) #0- unbuffered, 1-line buffered
  #p= subprocess.Popen(string.split(cmd), bufsize=1,
  #  stdin=subprocess.PIPE, stdout=subprocess.PIPE, close_fds=True)
  #iop= (p.stdout, p.stdin)
  if str1:
    lines= iop[0].read()
  else:
    lines= iop[0].readlines()
  return lines
def form1(req):
  cspath= cntwebenv.dbctp+"COLLISIONS.SCHEDULE"
  #otx= cmdex("ls -ld "+cspath)
  csf= open(cspath,"r"); csname= csf.readline().rstrip(); csf.close()
  getmapcmd= "cd /tmp ; cp "+cspath+" "+csname+".alice"
  #cmdex(cmd)
  getmapcmd= getmapcmd +";"+ cntwebenv.VMECFDIR+"filling/lhc2ctp.py " + csname + ".alice 3"
  csmap= cmdex(getmapcmd, str1="yes")
  return """
<pre>
<!--
%s<hr>
-->
%s
</pre>"""%(getmapcmd, csmap)

def index(req):
  htm= """\
<html><head>
<style type="text/css">
td {padding:0.2em 0.5em;border:1px solid black;}
table {border-collapse:collapse;}
</style>
</head><body>
"""
  htm=htm+form1(req)
  #htm=htm+ showreq(req)
  htm=htm+"""
</body></html>
"""
  return htm

if __name__ == "__main__":
  print "main:",index()
  #main()

