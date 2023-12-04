#!/usr/bin/python
import serial,time   # trigdb (i.e. no need for 2->3) for trigdb
# see altri24:/home/trigger/git/trigger/v/vme/CFG/ctp/DB/ttcparts.cfg
swchans={"trd":7, "zdc":10, "emc":13, "tpc":14,"pmd":6,"aco":5, "sdd":23,
  "mch":22, "mtr":21, "daq":20, "ssd":9, "fmd":11,"t0":8, "hmp":16, "phs":17,
  "cpv":15, "ad":0, "spd":12, "tof":19, "v0":18}
def getTTCITSW(name):
  """return: ECS detector number for LTU name"""
  #for ltu in self.ltus:
  #  if ltu.name==name: return ltu.ttcitsw
  if name in swchans: return str(swchans[name])
  return None
def findTTCITSW(chan):
  """return: LTU object TRGLTU connected to chan channel of TTCit switch"""
  #for ltu in self.ltus:
  #  if  int(ltu.ttcitsw)== int(chan): return ltu
  for ltuname in ttcitswchans:
    if swchans[ltuname]==chan: return ltuname
  return None

def osc(str):
  """Open/Send/Close string str to serial port
  return: switch response (including final \r)
  """
  #ser = serial.Serial('/dev/ttyS2', 9600, bytesize=serial.EIGHTBITS,parity=serial.PARITY_ODD,stopbits=serial.STOPBITS_ONE,\
  ser = serial.Serial('/dev/ttyUSB0', 9600, \
    timeout=1,writeTimeout=1)
  if str=='break':
    ser.sendBreak(); time.sleep(1)
  else:
    print("writing:", str+"\r")
    #ser.write(str+"\r")
    strbytes= (str+"\r").encode('raw_unicode_escape') # (must be ASCII)
    ser.write(strbytes)
  rbytes= ser.read(20)
  r= rbytes.decode()
  print("cmd:%s\\r response:(%d):%r"%(str, len(r),r))
  ser.close()
  return r
def main():
  import sys
  if len(sys.argv) < 2:
    print("""

Switch on/off: http://alidcsaux021
ttcitsw.py i       -show current connection
ttcitsw.py r       -reset TTCit optical switch (*RST command)
ttcitsw.py N       -connect channel N (1..24) to TTCit
ttcitsw.py detname -connect LTU detname to TTCit

Notes (example of operation):
chmod a+rwx /dev/ttyS2
ls -l /dev/ttyS2
crwxrwxrwx 1 root uucp 4, 66 Feb 17  2012 /dev/ttyS2
[alidcsvme007] /home/alice/trigger/v/vme/WORK > ttcitsw.py sdd
Connecting: sdd (chan:23) to TTCit switch...
cmd:SWITCH:23\r response:(6):'23,OK\r'
[alidcsvme007] /home/alice/trigger/v/vme/WORK > ttcitsw.py i
cmd:SWITCH?\r response:(3):'23\r'
TTCIT switch channel:23 = ltu:SDD

""")
  else:
    cmd=sys.argv[1]
    # i -show info   1..24 -set channel   r -reset
    if cmd=='i':
      r=osc("SWITCH?")
      if len(r)>0:
        if len(r)>1:
          chan=r.split()[0]
        else:
          chan= r+'?'   # bad response
      else:
        chan='EMPTY'
      if chan=='ERR1' or chan=='EMPTY':
        print("bad response from switch:", chan)
      else:
        #ls= trigdb.TrgLTUS()
        ltu= findTTCITSW(chan)
        if ltu: ltuname= ltu   #ltu.name
        else: ltuname=None
        print("TTCIT switch channel:%s = ltu:%s"%(chan,ltuname))
    elif cmd=='r': 
      osc("*RST")
    elif cmd.isdigit():
      if (int(cmd)>=1) and (int(cmd)<=24):
        osc("SWITCH:"+cmd)
      else:
        print("Bad channel %s (1..24 allowed)"%cmd)
    else:
      #ls= trigdb.TrgLTUS()
      ltuchan= getTTCITSW(cmd)   #string.upper(cmd))
      if ltuchan==None:
        print("Bad command: %s (r i 1 2 ... 24 expected)"%cmd)
        osc(cmd)
      else:
        print('Connecting: %s (chan:%s) to TTCit switch...'%(cmd,ltuchan))
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

