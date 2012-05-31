import shmpyext
  def perRead(self):
    print "perRead"
    #thread= threading.Thread(self.historygram,arg=(1))
    self.history= threading.Thread(target=self.historygram)
    self.stophistoygram=None
    self.history.start()
  def historygram(self, first=None):
    if shmpyext.startstopfw(1) != 1:
      myw.errorprint(self,"Cannot open shm")
      return
    fifo= open("/tmp/dataready","r")
    #for n in range(10):
    while 1:
      #time.sleep(1)
      line= fifo.readline()
      if self.stophistorygram:
        n=shmpyext.startstopfw(0)
        print "RC from stopping fifo:",n
        break
      if line=='': break
      print "hg:", line[:-1]
    print "hg: finishing"
  def allreadshm(self):
    for i in range(len(self.regs)):
      addr= self.regs[i][1]
      val=shmpyext.getcnt(i)
      print "allreadshm:", addr,val
    #vals=shmpyext.getcnts("get..",(1,2,3))
    #print "allreadshm2:",vals

