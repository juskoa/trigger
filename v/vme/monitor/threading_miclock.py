#!/usr/bin/python

import threading,socket
import time

class myThread (threading.Thread):
  def __init__(self, threadID, name, delay):
    threading.Thread.__init__(self)
    self.threadID = threadID
    self.name = name
    self.delay = delay
    self.exitflag= 0
  def run(self):
    print "Starting " + self.name
    self.udp2monitor()
    print "Exiting " + self.name
  def stop(self):
      self.exitflag= 1
      self.join()
  def udp2monitor(self):
    host = "localhost"
    port = 9931
    addr = (host,port)
    sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    udpcount=0
    while True:
      #if exitFlag:
      if self.exitflag:
          break
      time.sleep(self.delay)
      message="miclock data %d"%(udpcount)
      sentrc= sock.sendto(message, addr)
      udpcount= udpcount+1
      print "%s: %s sentrc:" % (time.ctime(time.time()), message), sentrc

# Create new threads
thread1 = myThread(1, "Thread-1", 2)
thread2 = myThread(2, "Thread-2", 30)

# Start new Threads
#thread1.start()
thread2.start()
print "thread 2 started..."

# print "Exiting Main Thread, threads are still active... (pkill -SIGUSR1 threading_ex.py)"
while True:
  terinp= raw_input("""
enter:
p   -show threads
t 1 -start thread 1
s 1 -stop thread 1
q   -quit
""")
  sl= terinp.split()
  if terinp == "q":
    break
  elif terinp == "p":
    print "activeCount:", threading.activeCount(), "currentThread:",threading.currentThread()
    #print "enumerate:", threading.enumerate()
    print "1:", thread1.getName(), thread1.isAlive()
    print "2:", thread2.getName(), thread2.isAlive()
  elif terinp[0] == "t":
    print "starting ", sl[1], "..."
    if sl[1]=='1': thread1.start()
    if sl[1]=='2': thread2.start()
  elif terinp[0] == "s":
    print "exitFlag to 1 for ", sl[1], "..."
    print "joining... ", sl[1], "..."
    # join() does not help -anyhow: raise RuntimeError("thread already started")
    # after reinitialisation (myThread(1,...) is ok
    if sl[1]=='1': 
      #thread1.exitflag= 1
      #thread1.join()
      thread1.stop()
      thread1 = myThread(1, "Thread-1", 1)
    if sl[1]=='2': 
      thread2.stop()
      thread2 = myThread(2, "Thread-2", 4)
  else: pass

