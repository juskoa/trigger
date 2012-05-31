#!/usr/bin/python

#from urllib import unquote
#import cgi
import string, lhc2ctp
from mod_python import util

#import cgitb; cgitb.enable(display=0, logdir="/tmp")

# The Publisher passes the Request object to the function
atable={'__getattribute__':1, 'status':1, 'args':1, 'bytes_sent':1, \
  'filename':1, 'document_root':1, 'user':1, 'hostname':1,
  'headers_in':1, 'headers_out':1}
cfg=None
fsname='???'

def HtmlEditPrint(fn,**args):
  """
  fn           -name of an html file to be printed to stdout
  a1='valuea1' -#a1# is replaced by 'valuea1' before print
  FCa1='fn'    -#FCa1# is replaced by content of the file fn
                or by '' if fn==''
  ! Most 1 replacement in 1 line !
  """
  #print fn,':',args
  prn=""
  htmlf=open(fn)
  for line in htmlf.read().splitlines():
    i1=-1
    for par in args.keys():
      parh='#'+par+'#'
      i1=string.find(line, parh)
      if i1!=-1:
        if par[0:2]=='FC':
          if args[par]=='':
            prn= prn+'\n'+ string.replace(line, parh, args[par], 1)
          else:
            i2=i1+len(parh)
            if line[:i1]: prn= prn+'\n'+ line[:i1]
            trginf=open(args[par])
            for l in trginf.read().splitlines(): 
              if l: prn= prn+'\n'+ l
            trginf.close()
            #print parh,i1,i2,':',args[par]
            prn= prn+'\n'+ line[i2:]
        else: 
          prn= prn+'\n'+ string.replace(line, parh, args[par], 1)
        break
    if i1==-1:
      prn= prn+'\n'+ line
  htmlf.close()
  return prn
def showreq(req):
   s = """\
<html><head>
<style type="text/css">
td {padding:0.2em 0.5em;border:1px solid black;}
table {border-collapse:collapse;}
</style>
</head><body>
<h1> req object attribs: </h1>
<table cellspacing="0" cellpadding="0">%s</table>
</body></html>
"""
   attribs = ''
   # Loop over the Request object attributes
   for attrib in dir(req):
     #attribs += '<tr><td>%s</td><td>%s</td></tr>'
     #attribs %= (attrib, cgi.escape(unquote(str(req.__getattribute__(attrib)))))
     #attribs %= (attrib, str(req.__getattribute__(attrib)))
     try:
       if atable.has_key(attrib):
       #if True:
         value= str(req.__getattribute__(attrib))
       else:
         value='-'
     except:
       #value= sys.exc_info()[0]
       value='caught'
     attribs += '<tr><td>%s</td><td>%s</td></tr>'%(attrib, value)
   attribs += '<tr><td>req.interpreter</td><td>%s</td></tr>'%(str(dir(req.interpreter)))
   #return "showreq"
   return s % (attribs)
def index(req=None):
  return showreq(req)
# #return "index here"
def genpage(lhc="", xls="", masks="", fsname="", parasitic=""):
  #xls="copy/paste text from .xls file", masks="", fsname=""):
  html= HtmlEditPrint("/var/www/html/CNTWEB/fill1noend.html",
    lhcinp=lhc, xlsinp=xls, bcmasks=masks, fsname=fsname, parasitic=parasitic)
  return html
def translate(req):
  global fsname
  #toto nebavi: form= util.FieldStorage(req,keep_blank_values=1)
  lhc= req.form.getfirst("lhc","lhc not defined")
  xls= req.form.getfirst("xls","xls not defined")
  if xls=="xls not defined" and lhc=="lhc not defined":
    html= genpage()
  else:
    #req.form["maskout"]= xlsinp
    lhc= req.form.getfirst("lhc","lhc not defined")
    lhcinp= req.form.getfirst("lhcinp","lhcinp not defined")
    xlsinp= req.form.getfirst("xlsinp","xlsinp not defined")
    #bcmasks= req.form.getfirst("bcmasks","...")
    if lhc=="lhc not defined":
      xls= req.form.getfirst("xls","xls not defined")
      if xls=="xls not defined":
        # initial page
        html= genpage("error","error","error")       
      else:   # xls->VALID.BCMASKS
        fs= lhc2ctp.FilScheme("", xlsinp)
        bcmasks= fs.getMasks()
        html= genpage(lhcinp, xlsinp, bcmasks, fsname)
    else:   # lhc-> xls
      fsname= req.form.getfirst("fsname","")
      xlsinp= lhc2ctp.bu2bcstr(lhcinp, fsname)
      html= genpage(lhcinp, xlsinp, fsname=fsname)
  #html=html+'<hr>'
  dirreq= req.form.keys()
  #html=html+str(dirreq)+"""
  html=html+"""
</body>
</html>
"""
  return html

if __name__ == "__main__":
  print "main:",index()
  #main()

