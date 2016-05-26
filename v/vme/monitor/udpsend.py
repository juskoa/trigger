#!/usr/bin/python
import sys,socket,string,os
#todo: thread sending 1/min a message

def main():
  if (len(sys.argv)<=1):
    print """
./udpsend.py daemon_name
"""
    return
  daname= sys.argv[1]
  host = "localhost"
  port = 9931
  addr = (host,port)
  sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
  numb= 0
  while(1):
    terinp= raw_input("""
enter:
n -next message
t -start thread sending messages 1/min
q -quit
""")
    if terinp == "q":
      break
    elif terinp == "n":
      message="%s data %d for %s"%(daname, numb, daname)
      print "sending: %s"%message
      sent= sock.sendto(message, addr)
      numb= numb+1
    elif terinp == "t":
      print "not done yet"
    else: continue
  sock.close()
  
if __name__ == "__main__":
  main()
