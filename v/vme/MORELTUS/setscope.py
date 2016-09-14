#!/usr/bin/python
import os,sys, subprocess
#sys.path.append(os.environ['VMEBDIR'])

class iopipe:
  def __init__(self, nbcmd):
    self.nbcmd=nbcmd
    print "Popen("+nbcmd+")"
    p= subprocess.Popen(string.split(nbcmd), bufsize=1,
      stdin=subprocess.PIPE, stdout=subprocess.PIPE, close_fds=True)
    self.iop= (p.stdout, p.stdin)
    self.printstdout()
  def printstdout(self):
    iline=0
    while(1):
      line= self.iop[0].readline()
      if line ==':\n':   #don't take last '\n:\n'
        break
      if line =='':
        line="pipe from cmdline interface closed/broken\n"
        print line
        break
      print line[:-1]
      iline=iline+1
      if iline>6: break
  def cmd(self, cmd):
    self.iop[1].write(cmd+'\n')
    #print self.nbcmd+':'+cmd
    if cmd!='q':
      self.printstdout()
def emptyfun(out):
  print 'emptyfun:',out

def main():
  #cstdal= cmdlin2.cmdlint(cmd='ltu/ltu.exe -noboardInit 0x816000')
  #c=iopipe('ltu/ltu.exe nbi 0x816000')
  set6=[('setAB(18,23)', 'L0->A'),
        ('setAB(3,23)', 'L1->A'),
        ('setAB(4,6)', 'L2Strobe->A L1Data->B'),
        ('setAB(4,3)', 'Prepulse->B'),
        ('setAB(4,7)', 'L2Data->B')]
  os.chdir(os.environ['VMECFDIR'])
  cs=iopipe('ltu/ltu.exe -noboardInit 0x816000')
  cg=iopipe('ltu/ltu.exe -noboardInit 0x810000')
  cs.cmd('?'); cg.cmd('blabla')
  while(1):
    for ix in range(len(set6)):
      il=raw_input('press ENTER (or q ENTER:')
      if il=='q':
        cs.cmd('q'); cg.cmd('q')
        break;
      cs.cmd(set6[ix][0]); cg.cmd(set6[ix][0])
      print "Now: ", set6[ix][1]
    if il=='q':
      break;
    
if __name__ == "__main__":
    main()

