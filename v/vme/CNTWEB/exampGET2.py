#!/usr/bin/python
#from cgi import escape
#from urllib import unquote
from mod_python import util

# The Publisher passes the Request object to the function
atable={'__getattribute__':1, 'status':1, 'args':1, 'bytes_sent':1, \
  'filename':1}

def copypart(req):
  inpdata= util.FieldStorage(req)
  inptxt= inpdata.get("name","empty")
  inptxt= inptxt +"="+ inpdata.get("physics_1","empty")
  return inptxt
  #return showreq(req)
def form1(req):
  lastargs= req.__getattribute__("args")
  #kwa= act(**lastargs)
  kwa= copypart(req)
  return """
<form action="chp" method="GET">
  partition ?
  <input type="text" name="name" />
  <SELECT NAME="physics_1" SIZE="3" TITLE="blabla">
  <OPTION SELECTED>_noV0
  <OPTION >_noV0_no0LSR
  </SELECT>
  <input type="submit" />
  last args:%s:%s:
</form>"""%(lastargs, kwa)

def showreq(req):
   s = """
<h1> req object attribs: </h1>
<table cellspacing="0" cellpadding="0">%s</table>
"""
   attribs = ''
   # Loop over the Request object attributes
   for attrib in dir(req):
     #attribs += '<tr><td>%s</td><td>%s</td></tr>'
     #attribs %= (attrib, escape(unquote(str(req.__getattribute__(attrib)))))
     #attribs %= (attrib, str(req.__getattribute__(attrib)))
     try:
       if atable.has_key(attrib):
         value= str(req.__getattribute__(attrib))
       else:
         value='-'
     except:
       #value= sys.exc_info()[0]
       value='caught'
     attribs += '<tr><td>%s</td><td>%s</td></tr>'%(attrib, value)
   #return "showreq"
   return s % (attribs)
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

  htm=htm+ showreq(req)
  htm=htm+"""
</body></html>
"""
  return htm
  #return showreq(req)

if __name__ == "__main__":
  print "main:",index()
  #main()

