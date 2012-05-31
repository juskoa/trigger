#!/usr/bin/python
# Server program

import socket,time

# Set the socket parameters
host = "localhost"
port = 9931
buf = 1024
addr = (host,port)

def main():
  global addr,buf
  # Create socket and bind to address
  UDPSock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
  UDPSock.settimeout(130)   # None disables timeout
  UDPSock.bind(addr)
  # Receive messages
  print "waiting for udp messsages on port %d..."%port
  while 1:
    try:
      data,addr = UDPSock.recvfrom(buf)
    #data,addr = UDPSock.recvfrom(buf,socket.MSG_DONTWAIT)
    # socket.error: (11, 'Resource temporarily unavailable')
    except:
      print "except"
      data="timeout"
    if not data:
      print "Client has exited!"
      #break
      #time.sleep(1)
    else:
      print "\nReceived message type:",type(data),"'", data,"'"
  UDPSock.close() # Close socket

if __name__ == "__main__":
    main()

