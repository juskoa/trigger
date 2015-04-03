import cgi
import cgitb; cgitb.enable()
#import cgitb; cgitb.enable(display=0, logdir="/tmp")
def application(environ, start_response):
    env_names = environ.keys()
    env_names.sort()
    view_env = "\n".join(
        ["%s=%s" % (name, environ[name]) for name in env_names]
    )
    form = cgi.FieldStorage(fp=environ['wsgi.input'], 
                        environ=environ)
    myvar= "PATH_INFO:%s REMOTE_HOST:%s"%(environ["PATH_INFO"], environ["REMOTE_HOST"])
    view_form = "\n".join(
        ["%s=%s" % (name, form[name].value) for name in form.keys()]
    )
    body = html %(myvar, view_form, view_env)
    start_response("200 OK", 
                   [("Content-type", "text/html"),
                    ("Content-length", str(len(body))),
                    ])
    
    return [body,]

html = """
<html>
  <head>
    <title>Hello World!</title>
  </head>
  <body>
    <!--
    <img src="helloworld.jpg" align="right"/>
    -->
    <b>Check out my dynamic environment!</b>
    <pre>%s</pre> <hr>
    <pre>%s</pre> <hr>
    <pre>%s</pre> <hr>
  </body>
</html>
"""
