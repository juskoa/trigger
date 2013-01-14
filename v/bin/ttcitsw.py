#!/usr/bin/python
import string,serial,time, trigdb
def osc(str):
  """Open/Send/Close string str to serial port
  return: switch response (including final \r)
  """
  #ser = serial.Serial('/dev/ttyS2', 9600, bytesize=serial.EIGHTBITS,parity=serial.PARITY_ODD,stopbits=serial.STOPBITS_ONE,\
  ser = serial.Serial('/dev/ttyS2', 9600, \
    timeout=1,writeTimeout=1)
  if str=='break':
    ser.sendBreak(); time.sleep(1)
  else:
    ser.write(str+"\r")
  r= ser.read(20)
  print "cmd:%s\\r response:(%d):%r"%(str, len(r),r)
  ser.close()
  return r
def main():
  import sys
  if len(sys.argv) < 2:
    print """

Switch on/off: http://alidcsaux021
ttcitsw.py i       -show current connection
ttcitsw.py r       -reset TTCit optical switch (*RST command)
ttcitsw.py N       -connect channel N (1..24) to TTCit
ttcitsw.py detname -connect LTU detname to TTCit

Notes:
chmod a+rwx /dev/ttyS2
ls -l /dev/ttyS2
crwxrwxrwx 1 root uucp 4, 66 Feb 17  2012 /dev/ttyS2
[alidcsvme007] /home/alice/trigger/v/vme/WORK > ttcitsw.py sdd
Connecting: sdd (chan:23) to TTCit switch...
cmd:SWITCH:23\r response:(6):'23,OK\r'
[alidcsvme007] /home/alice/trigger/v/vme/WORK > ttcitsw.py i
cmd:SWITCH?\r response:(3):'23\r'
TTCIT switch channel:23 = ltu:SDD

"""
  else:
    cmd=sys.argv[1]
    # i -show info   1..24 -set channel   r -reset
    if cmd=='i':
      r=osc("SWITCH?")
      if len(r)>0:
        chan=string.split(r)[0]
      else:
        chan='EMPTY'
      if chan=='ERR1' or chan=='EMPTY':
        print "bad response from switch:", chan
      else:
        ls= trigdb.TrgLTUS()
        ltu= ls.findTTCITSW(chan)
        if ltu: ltuname= ltu.name
        else: ltuname=None
        print "TTCIT switch channel:%s = ltu:%s"%(chan,ltuname)
    elif cmd=='r': 
      osc("*RST")
    elif cmd.isdigit():
      if (int(cmd)>=1) and (int(cmd)<=24):
        osc("SWITCH:"+cmd)
      else:
        print "Bad channel %s (1..24 allowed)"%cmd
    else:
      ls= trigdb.TrgLTUS()
      ltuchan= ls.getTTCITSW(string.upper(cmd))
      if ltuchan==None:
        print "Bad command: %s (r i 1 2 ... 24 expected)"%cmd
        osc(cmd)
      else:
        print 'Connecting: %s (chan:%s) to TTCit switch...'%(cmd,ltuchan)
        osc("SWITCH:"+ltuchan)

if __name__ == "__main__":
  main()

#print ser.portstr
#ser.write("\r")
#ser.write("*IDN?\r")
#ser.write("SWITCH:6\r")
#ser.write("SWITCH?\r")
#ser.write("MAX#?\r")
#ser.write("*RST\r")
#ser.write("*RST\x0d")
#serwr(ser, "*RST\r")
#print "sleep(1)..." 
#time.sleep(1)
#print "bytes in receive buffer:",ser.inWaiting()
#print ser.getCTS(),ser.getDSR(), ser.getRI(),ser.getCD()
#ser.readline()

