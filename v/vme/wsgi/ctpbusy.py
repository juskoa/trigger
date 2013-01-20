import cgi,os
import cgitb; cgitb.enable()
#import cgitb; cgitb.enable(display=0, logdir="/tmp")
def prtenv(environ):
  env_names = environ.keys()
  env_names.sort()
  view_env = "\n".join(
      ["%s=%s" % (name, environ[name]) for name in env_names]
  )
  #
  form = cgi.FieldStorage(fp=environ['wsgi.input'], 
                      environ=environ)
  myvar= "PATH_INFO:%s REMOTE_HOST:%s"%(environ["PATH_INFO"], environ["REMOTE_HOST"])
  view_form = "\n".join(
    ["%s=%s" % (name, form[name].value) for name in form.keys()]
  )
  #
  return """
<html>
  <head>
    <title>Hello World!</title>
  </head>
  <body>
    <img src="helloworld.jpg" align="right"/>
    <b>PATH_INFO and REMOTE_HOST:</b>
    <pre>%s</pre>
    <b>form variables:</b>
    <pre>%s</pre>
    <b>Check out my dynamic environment!</b>
    <pre>%s</pre>
  </body>
</html>
""" %(myvar, view_form, view_env)

def application(environ, start_response):
  env_names = environ.keys()
  form = cgi.FieldStorage(fp=environ['wsgi.input'], 
                      environ=environ)
  hostname= environ["HOSTNAME"]
  myvar= "PATH_INFO:%s REMOTE_HOST:%s"%\
    (environ["PATH_INFO"], environ["REMOTE_HOST"])
  resp="???"
  if (environ["REMOTE_HOST"]=="pcalicebhm11.cern.ch") or \
     (environ["REMOTE_HOST"]=="aldaqacr07.cern.ch"):
    if environ["PATH_INFO"]=="/":
      resp= "form definititon with CTP controls here"
    elif environ["PATH_INFO"]=="/setbsy":
      resp= " changing busy calculation..."
      os.system("pkill -SIGUSR1 readctpc")
    elif environ["PATH_INFO"]=="/getfsdip":
      os.system("$VMECFDIR/filling/getfsdip.py act")
      # return getfsdip.log? or rc ?
    elif environ["PATH_INFO"]=="/prtenv":
      #resp= None
      body= prtenv(environ)
    else:
      resp= "uknown action:"+environ["PATH_INFO"]
  else:
    resp= "required action can be done only from trigger desktop machine"
  #
  if resp==None:
    body= prtenv(environ)
  else:
    body= html %(resp,hostname, hostname)
  #
  start_response("200 OK", 
                   [("Content-type", "text/html"),
                    ("Content-length", str(len(body))),
                    ])
    
  return [body,]

html = """
<html>
  <head>
    <title>CTP control </title>
  </head>
  <body>
    <!--
    <img src="helloworld.jpg" align="right"/>
    -->
    <h2>%s</h2>
    <A href="http://%s/htmls/busyL0.html">Back to BUSY status screen</A> <br>
    <A href="http://%s/htmls/bsyhelp.html">help</A>
  </body>
</html>
"""
