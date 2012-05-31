""" Publisher simplest example
Start:
http://localhost/CNTWEB/hello/say
http://localhost/CNTWEB/hello.py/say
http://localhost/CNTWEB/hello.py/say?what=something

http://localhost/CNTWEB/hello   ->defaults to hello/index
 """

def say(req, what="NOTHING"):
  return "I am saying %s" % what
def index(req):
  return "index ok"
#def __auth__(req, user, passwd):
  #print "%s %s"%(user,passwd)
  #return 0
  return 1
class notcall:
  def __init__(self):
    return "notcall here..."
