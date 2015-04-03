""" Publisher simplest example
Start:
http://localhost/CNTWEB/hello/say
http://localhost/CNTWEB/hello.py/say
http://localhost/CNTWEB/hello/say?what=something
http://localhost/CNTWEB/hello.py/say?what=something

http://localhost/CNTWEB/hello.py   ->defaults to hello/index
http://localhost/CNTWEB/hello   ->defaults to hello/index
 """
ncalls=100
def say(req, what="NOTHING"):
  return "I am saying %s" % what
def bsyform(req):
  keys=""
  keys= str(req.form.keys())
  for bs in req.form.keys():
    keys= keys+ bs + "<br>"
  return keys
def index(req):
  global ncalls
  ncalls= ncalls+1
  html="""<html><head>
<meta HTTP-EQUIV="Refresh" CONTENT=300>
</head>
ncalls:%d
</html>"""%(ncalls)
  return html
class notcall:
  def __init__(self):
    return "notcall here..."
