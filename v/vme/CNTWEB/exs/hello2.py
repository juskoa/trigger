#!/usr/bin/python
from cgi import escape
from urllib import unquote

# The Publisher passes the Request object to the function
atable={'__getattribute__':1, 'status':1, 'args':1, 'bytes_sent':1, \
  'filename':1,'hostname':1}

def showreq(req):
   s = """\
<html><head>
<style type="text/css">
td {padding:0.2em 0.5em;border:1px solid black;}
table {border-collapse:collapse;}
</style>
</head><body>
<h1> req object attribs (exs): </h1>
%s
<table cellspacing="0" cellpadding="0">%s</table>
</body></html>
"""
   attribs = ''
   # Loop over the Request object attributes
   for attrib in dir(req):
     attribs += '<tr><td>%s</td><td>%s</td></tr>'
     attribs %= (attrib, escape(unquote(str(req.__getattribute__(attrib)))))
     #attribs %= (attrib, str(req.__getattribute__(attrib)))
     if False:
       try:
         if atable.has_key(attrib):
           value= str(req.__getattribute__(attrib))
         else:
           value='-'
       except:
         #value= sys.exc_info()[0]
         value='caught'
       attribs += '<tr><td>%s</td><td>%s</td></tr>'%(attrib, value)
   remhost = "<hr>"+ req.get_remote_host()
   #return "showreq"
   return s % (remhost, attribs)
def index(req=None):
  return showreq(req)
  #return "index here"

if __name__ == "__main__":
  print "main:",index()
  #main()

