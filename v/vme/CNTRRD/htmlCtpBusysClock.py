#!/usr/bin/python
"""
Goal: on call (once per minute), update ctpbusys.html
Parameters:
1.line: busy1 busy2 ... busy24
2.line: FO1_L0 FO2_L0 ... FO24_L0
"""
import sys,os,string, time, glob, trigdb

RCFGDIR= os.path.join(os.getenv('VMEWORKDIR'), "WORK", "RCFG")
MONSCALWDIR= os.path.join(os.getenv('VMEWORKDIR'), "WORK", "MONSCAL")
colorOK="ff6666"
colorSLOWEST="magenta"
colorINTERR="blue"
MaxBusyBarHeightInPixs=100

ltusdb= trigdb.TrgLTUS()

def PrintError(fstr):
  print "Error:",fstr

class Cluster:
  def __init__(self, color):
    self.color= color
    self.clean()
  def clean(self):
    self.lastbusy=0        # 
    self.lastltu =None     # None:empty cluster (no LTU assigned)
    # or the name of slowest LTU in this cluster

Clusters=(Cluster("99ffff"), Cluster("66ff33"), Cluster("ff9933"), 
          Cluster("cccccc"), Cluster("ffff00"), Cluster("ccff66"))

class Ltu:
  def __init__(self, name, nh):
    """ name: ltu name (one of names in ltudb)
    nh: (cluster_name, hw_number_assigned_for_this_cluster)
    Note: ltu can be member of several clusters, at least 1
    """
    self.name=name
    self.detnum=ltusdb.getdetnum(name)
    self.clusters=[nh]     # [(clust_name, hw_clust_numberAS_INT),...]
    self.busy=None
    self.color=colorOK     # colorOK, colorSLOWEST
  def addCluster(self,nh):
    self.clusters.append(nh)
  def updateBusy(self, busy):
    self.busy= busy
class Partition:
  def __init__(self, name, run):
    self.name= name
    self.run= run
    self.myltus={}   # self.myltus['sdd'].clusters=(clus_name, clus_hwn)
    # self.partmax reset made in makeHtml always when html actualised
    #self.partmax=(0,'')   #('maxbusyInPartition','ltuname')
  def addCluster(self, ncl):
    """ ncl[0] -clust_name,   ncl[1]  -hw_clust_number
    ncl[2:] -ltus
    In Partition:
    - 1 LTU can belong to more clusters
    - only 1 hw_cluster assigned to any name
"""
    clname= ncl[0]
    clhwn= ncl[1]
    try:
      for ltu in ncl[2:]:
        if ltu=='EMCal': ltu='EMCAL'
        if ltu=='DAQ_TEST': ltu='DAQ'
        if self.myltus.has_key(ltu):
          self.myltus[ltu].addCluster((clname, int(clhwn)))
        else:
          self.myltus[ltu]= Ltu(ltu, (clname, int(clhwn)))
    except:
      PrintError("addCluster: clust_name hw_clust_number ltu1 ltu2 ...")
      print ncl
  def printpart(self):
    print "Partition: %s #%s"%(self.name, self.run)
    for ltun in self.myltus.keys():
      #if ltun=='DAQ_TEST': ltun='DAQ'   #aj
      clusters=''
      for cluster in self.myltus[ltun].clusters:
        clusters= clusters+ "%s:%d "%(cluster[0], cluster[1])
      print " ",ltun,"is in",len(self.myltus[ltun].clusters), \
        "clusters:",clusters," Busy:", self.myltus[ltun].busy
      #print " ",clusters
  def doltucltab(self):
    #prepare ltu cluster tab (rectangle of appr. color 
    # for each cluster detector is in)
    for ltun in self.myltus.keys():
      #if ltun=='DAQ_TEST': ltun='DAQ'   #aj
      lt=""
      ltu=self.myltus[ltun]
      for ix in range(len(Clusters)):
        #lt= lt+'<td bgcolor="%s" width=5><b>%d</b></td>\n'% \
        #  (Clusters[ix].color, ix+1)
        isin=0
        for ixx in range(len(ltu.clusters)):
          if ltu.clusters[ixx][1]== ix+1:
            lt= lt+'<td bgcolor="%s" height=10 width=5></td>\n'% \
              (Clusters[ix].color)
            isin=1
            break
        if isin==0:
          lt= lt+'<td bgcolor="ffffff" height=15 width=3></td>\n'
        ltu.ltucltab= lt
class ActivePartitions:
  def __init__(self):
    self.parts={}    # self.parts['runnumber']
    self.mesdti= ['1.1.2008','0:0:0', 'minute']  # [date, time, interval]:20.05.2008 23:09:34 minute
    #self.getRuns()
  def printparts(self):
    for pn in self.parts.keys():
      self.parts[pn].printpart()
  def getRuns(self):
    """ rc: None -no need to recreate .html file
    "yes": .html file has to be recreated
    """
    newhtml=None    # we do not need to recreate .html file
    names= os.listdir(RCFGDIR)
    for ix in range(len(names)-1,-1,-1):
      if (len(string.split(names[ix],'.'))>1) and \
          (string.split(names[ix],'.')[1]=='rcfg'):
        continue
      else:
        del names[ix]
    runsinmem= self.parts.keys()
    # after going through all files in RCFG, runsinmem
    # should contain numbers to be deleted from self.parts
    print "getRuns:",names, "in memory:", runsinmem
    if len(names) >6:
      PrintError("Too many runs (>6) active in WORK/RCFG dir.") 
    # cases:  RCFG entries     Memory entries
    #                       >                  -> new partition was loaded
    #                       <                  -> part. deleted
    #                       =                  -> html/table not changed
    names.sort() ; names.reverse()
    for runname in names:                      #loop over RCFG dir
      runn= string.split(runname,'.')[0][1:]
      if self.parts.has_key(runn):             #old run
        for ix in range(len(runsinmem)):
          if runsinmem[ix]== runn: ixdel=ix
        del runsinmem[ixdel]   # OK, old run, still active
      else:
        self.parts[runn]= self.procRcfg(runname, runn)  # new run
        newhtml='yes'
    print "gone runs:", runsinmem, " deleting them in memory (self.parts)..."
    for ix in range(len(runsinmem)):
      del self.parts[runsinmem[ix]]     
      newhtml='yes'
    return newhtml
  def updatePartitions(self):
    html= self.getRuns()
    print "updatePartitions recreate html(None if not needed):", html
  def procRcfg(self, runname, runn):
    """runname: rNNNNN.rcfg"""
    infile= open(os.path.join(RCFGDIR, runname), "r")
    section=''
    while 1:
      line= infile.readline()
      if not line: break
      if line[0] == "\n": continue
      if line[0] == "#": continue
      if string.find(line, "PARTITION:")==0:
        partname= string.strip(string.split(line,':')[1])
        part= Partition(partname, runn)
        continue
      if string.find(line, "CLUSTERS:")==0:
        section="CLUSTERS:"
        continue
      if string.find(line, "PFS:")==0:   # ignore lines after CLUSTERS
        section="PFS:"
        continue
      if section=="CLUSTERS:":
        ncl= string.split(line)
        if len(ncl)<2:
          PrintError("bad line in %s file"%runname)
        else:
          part.addCluster(ncl)
    part.doltucltab()
    infile.close()
    print "procRcfg: %s %s"%(runname, runn)
    #part.printpart()
    return part
  def updateBusys(self, busyl0s):
    """busyl0s: list of 26 strings:
    busyL0 date time minute usecs1 usecs2...
                100ms
    busyL0     -busy time calculated from busy/L0
    date:      - date of measurement
    time:      - time of measurement  
    minute/100ms  -interval of measurement
    usecs1,... -average busy time in usecs or 
    -          -detector not in partition or
    dead       -busy ON all the time
    rc:
"""
    nltus= len(busyl0s)
    ll= string.split(busyl0s)
    if busyl0s[:6]!="busyL0":
      print "Bad line length:%d:%s:"%(len(busyl0s), busyl0s)
      return "Bad line"
    self.mesdti= ll[1:4]   # [date, time, interval]:20.05.2008 23:09:34 minute
    busys= ll[4:]
    #print "updateBusys:parts",self.parts 
    for rn in self.parts.keys():
      #print "updateBusys: part:%s %s"%(rn,self.parts[rn].name), "busys:",busys
      for ltun in self.parts[rn].myltus.keys():
        ltu= self.parts[rn].myltus[ltun]
        #print "ltudetnum:",ltu.detnum
        if ltu.detnum==None:
          PrintError("ltu %s is not found in DB, but is in partition %s"\
            %(ltu.name, self.parts[rn].name))
          continue
        idet= int(ltu.detnum)
        if len(busys)<=idet:
          print "busy not given for ltu:", ltu.name
          continue
        ltu.updateBusy(busys[idet])
    return None
  def getRatiosClusters(self, part):
    retxt= "<h2>%s %s</h2>\n"%(part.name,part.run)
    cwd= os.getcwd(); os.chdir(MONSCALWDIR);
    fnames= glob.glob("%s_*.png"%part.run); os.chdir(cwd); fnames.sort() # TPC last
    for fn1 in fnames:
      #retxt= retxt +'\n'+ fn1
      rclusname= fn1[fn1.rindex('_')+1:-4]
      #if fn1[-3:]=="TPC":continue
      if rclusname=="TPC":continue
      retxt= retxt+ "<h3>%s:</h3>"%rclusname
      retxt= retxt+ '<img src="ratios/%s" title="%s" alt="%s" width="100%%" height="80%%" /><br>\n'%\
        (fn1, rclusname,rclusname)
    retxt= retxt+"<br>\n"
    return retxt
  def makeHtml(self, fname="busyL0.html", fnl20="ratios.html"):
    print "-------------------------------makeHtml:%s"%fname
    #print "Parts:", self.printparts()
    self.htmlStart(fname=fname)
    ratios_html= """<html>
<head>
<meta HTTP-EQUIV="Refresh" CONTENT=20>
</head>
<body>
"""
    for prun in self.parts:
      part= self.parts[prun]
      # for the whole partition:
      part.partmax=(0,'')   #('maxbusyInPartition','ltuname')
      # for each cluster separately:
      maxs={}   # cluster_hw:(maxbusy_in_this_clust, slowest_ltu_name)
      #print "makeHtml:hw_cluser ltuname, ltubusy, maxs --------------- partition:",part.name,part.run
      print "makeHtml:---------------------- ",part.name,part.run
      #part.printpart()
      ratios_html= ratios_html+self.getRatiosClusters(part)
      for ltuname in part.myltus:
        ltu= part.myltus[ltuname]
        for clu in ltu.clusters:
          clhw= clu[1]
          #print "makeHtml2:",clhw, ltu.name, ltu.busy, maxs
          if ltu.busy=='dead': continue
          if maxs.has_key(clhw):
            if ltu.busy=='-' or ltu.busy==None:
              print "makeHtml:ProbablyError:",ltu.name, ltu.busy, maxs[clhw][0]
              continue
            if int(ltu.busy) <= int(maxs[clhw][0]): continue
            maxs[clhw]= (ltu.busy, ltu.name)
          else:
            if len(maxs) >=6:
              PrintError("Too many clusters (>6). Check WORK/RCFG/*")
            else:
              if ltu.busy=='-' or ltu.busy==None: continue
              maxs[clhw]= (ltu.busy, ltu.name)
        #print "makeHtml1:", ltu.name, ltu.busy
        if ltu.busy=='-' or ltu.busy==None: continue
        if ltu.busy=='dead': continue
        if int(ltu.busy) > int(part.partmax[0]):
          part.partmax= (int(ltu.busy), ltu.name)
      # maxs: max. 6x (maxbusy,ltu_name) for partition part
      # self.partmax: total maximum for whole partition
      #print "makeHtml4:",part.name,part.run, part.partmax
      for clhw in maxs: print "makeHtml3:",clhw,":",maxs[clhw]
      for ltuname in part.myltus:
        ltu= part.myltus[ltuname]   # all LTUS in this partition
        for cluhw in maxs:
          if maxs[cluhw][1] == ltuname:
            ltu.color= colorSLOWEST
          else:     
            ltu.color= colorOK
        #print "makeHtml5:ltu:",ltuname
      self.htmlPartition(part)
    self.htmlEnd()
    if len(self.parts)==0:
      ratios_html= ratios_html + "<h2>No global partition(s) running</h2>\n"
    ratios_html= ratios_html+"""
<br>
</body>
</html>
"""
    #print ratios_html
    f= open("htmls/%s"%fnl20, 'w')
    f.write(ratios_html)
    f.close()
  def get_clockinfo(self):
    fn= os.path.join(os.environ['HOME'],"CNTRRD/htmls/clockinfo")
    try:
      #f= open("/home/alice/trigger/CNTRRD/htmls/clockinfo", 'r')
      f= open(fn, 'r')
    except:
      f=None
    if f:
      rc= f.readlines()[0]
      f.close()
    else: 
      PrintError('Where is /home/alice/trigger/CNTRRD/htmls/clockinfo?')
      rc='clock: unknown'
    return(rc)
  def htmlStart(self, fname="busyL0.html"):
    #self.htmlf=open(os.path.join(RCFGDIR, "busyL0.html"), "w")
    self.htmlf=open("htmls/"+fname, "w")
    # refreshsecs in seconds is: t2-t1 +60 + 1
    # t2:time when this page was shown (certainly later then t1)
    # t1:time when counters received from DIM server:
    # +1 to be sure we get new status
    # self.mesdti[1]   # 12:23:01 
    t2= time.localtime()   #!time of creation (not display !)
    print "htmlStart:t2:",t2, "t1:",self.mesdti[1]
    t2= t2[5]
    t1= int(string.split(self.mesdti[1],':')[2])
    if t2<t1:
      refreshsecs= 60+t2-t1
    else:
      refreshsecs=t2-t1
    refreshsecs= refreshsecs+61
    clockinfo= self.get_clockinfo()
    print "htmlStart:t2:",t2, "t1:",t1, "refresh:",refreshsecs
    #refreshsecs= 10
    #<!--meta HTTP-EQUIV="Refresh" CONTENT=%d-->
    # following line after <head>:
    #<meta HTTP-EQUIV="Refresh" CONTENT=10>
    head="""<html>
<head>
<meta HTTP-EQUIV="Refresh" CONTENT=10>
<style type="text/css">
img.header { width: 40px; }
td.bar { vertical-align:top; height:100; }
big {font-size: 2em; font-weight: bold; }
</style>
<script type="text/javascript">
var refint="refinterval is %d";
//function load() {
//window.status="Page is loaded";
//document.write(refint);
//}
//document.write(refint);
</script>
</head>

<!--
<body onload="load()">
-->
<body>
<table border="1" width=100%%>
<tr>
<th>%s<br>%s<br><big>%s</big><br>Last %s</th>
<th><img class="header" src="pngs/spd.png" /></th>
<th><img class="header" src="pngs/sdd.png" /></th>
<th><img class="header" src="pngs/ssd.png" /></th>
<th><img class="header" src="pngs/tpc.png" /></th>
<th><img class="header" src="pngs/trd.png" /></th>
<th><img class="header" src="pngs/tof.png" /></th>
<th><img class="header" src="pngs/hmpid.png" /></th>
<th><img class="header" src="pngs/phos.png" /></th>
<th><img class="header" src="pngs/cpv.png" /></th>
<th><img class="header" src="pngs/pmd.png" /></th>
<th><img class="header" src="pngs/muon_trk.png" /></th>
<th><img class="header" src="pngs/muon_trg.png" /></th>
<th><img class="header" src="pngs/fmd.png" /></th>
<th><img class="header" src="pngs/t0.png" /></th>
<th><img class="header" src="pngs/v0.png" /></th>
<th><img class="header" src="pngs/zdc.png" /></th>
<th><img class="header" src="pngs/acorde.png" /></th>
<th><img class="header" src="pngs/emcal.png" /></th>
<th><img class="header" src="pngs/daq.png" /></th>
</tr>
<th>Av. busy [usecs]</th>
"""%(refreshsecs, clockinfo, self.mesdti[0], self.mesdti[1], self.mesdti[2])
    # now we go through all LTUs 2 times:
    # 1. found for each LTU, if it is the worst
    # in some cluster. The LTU can be slowest in more clusters!
    gltus={}
    for clu in Clusters: clu.clean()
    for ln in range(len(ltusdb.ltus)):
      ltuname= ltusdb.ltus[ln].name
      for partkey in self.parts:
        part= self.parts[partkey]
        if part.myltus.has_key(ltuname):
          ltu= part.myltus[ltuname]
          for cluofltu in ltu.clusters: #[(clust_name, hw_clust_number),...]
            #ltun= Clusters[cluofltu[1]-1].lastltu
            #if lastltu!
            #print "htmlStart2:", ltu.busy,':',ltu.name, cluofltu
            if ltu.busy=='-' or ltu.busy==None:
              print "ProbablyError:",ltu.name, ltu.busy
              continue
            if ltu.busy=='dead': continue
            if int(ltu.busy) > Clusters[cluofltu[1]-1].lastbusy:
              Clusters[cluofltu[1]-1].lastbusy= int(ltu.busy)
              Clusters[cluofltu[1]-1].lastltu= ltuname
    # 2. create 1 row (busys) with 'slowest' in given cluster background
    for ln in range(len(ltusdb.ltus)):
      ltuname= ltusdb.ltus[ln].name
      for partkey in self.parts:
        part= self.parts[partkey]
        if part.myltus.has_key(ltuname):
          ltu= part.myltus[ltuname]   #  [(clust_name, hw_clust_number),...]
          #print "htmlStart:", ltuname,part.name, ltu.busy
          if gltus.has_key(ltuname):
            PrintError("in partition %s. Detector %s is already in partition %s"%(part.name, ltuname, gltus[ltuname]))
          else:
            bgcol='ffffff'
            nclu=0   # number of cases when this ltu is slowest (max 6)
            for clu in Clusters:
              if clu.lastltu==ltu.name:   # ltu is slowest in this
                bgcol= clu.color ; nclu=nclu+1
                print "slowest:", ltu.name, nclu,
            nclu=len(ltu.clusters)  # let's show numb of clusters ltu belongs to
            if nclu>0:
              nclus="<br>(%d)"%nclu
            else:
              nclus=""
            # just number of clusters LTU belongs to:
            #head = head + '<td bgcolor="#%s" > <b>%s%s</b> </td>\n' % \
            #  (bgcol,ltu.busy, nclus)
            # show all clusters ltu belongs to:
            if ltu.busy=='dead': ltubusy='BUSY'
            else: ltubusy= ltu.busy
            head = head + """<td><table>
<tr><td bgcolor="%s" colspan=6> <big>%s</big> </td></tr>
<tr>%s</tr>
</table></td>
""" % (bgcol,ltubusy, ltu.ltucltab)    #bgcol not working anyhow % (bgcol,ltu.busy, ltu.ltucltab)
            gltus[ltuname]=part.name
      if not gltus.has_key(ltuname): 
        head= head +'<td> </td>\n' 
    self.htmlf.write(head)
    self.htmlf.write("</tr>\n\n")
  def htmlPartition(self, part):
    parthead="""
<th><big>%s</big><br>Run:<big>%s</big></th>
"""%(part.name, part.run)
    self.htmlf.write(parthead)
    for ln in range(len(ltusdb.ltus)):
      ltuname= ltusdb.ltus[ln].name
      if part.myltus.has_key(ltuname):
        ltu= part.myltus[ltuname]   
        if ltu.busy=='dead':
          partltu='<td bgcolor="red"></td>\n'
        elif ltu.busy=='-':
          partltu='<td bgcolor="%s"></td>\n'%colorINTERR
        else:
          max= part.partmax[0]
          if max == 0:
            max=1;
            print "htmlPartition: part.partmax is 0, using 1"
          #print "htmlPartition1: %s: %s %s"%(ltuname, ltu.busy, ltu.color)
          try:
            height= MaxBusyBarHeightInPixs * (1.* int(ltu.busy) / max)
          except:
            print "htmlPartition1except: %s: %s %s %d"%(ltuname, ltu.busy, ltu.color, height)
          if ltu.color==colorSLOWEST:
            busybar="busybarSLOW"
          else:
            busybar="busybar"
          partltu="""
<td bgcolor="#ffffff" valign="top"> <table> <tr>
 <td valign="top" height=100> <img src="pngs/busybar.png" height=%s /> </td></tr>
 <tr><td valign="bottom"> <b>%s</b> </td></tr> </table>
</td>
"""%(height, ltu.busy)
          partltu="""
<td bgcolor="#ffffff" valign="top"> <table> <tr>
 <td valign="top" height=100> <img src="pngs/%s.png" height=%s /> </td></tr>
</table>
</td>
"""%(busybar, height)
      else:
        # print ltuname," -is not in this partition,", part.name
        partltu='<td></td>\n'
      self.htmlf.write(partltu)
    self.htmlf.write("</tr>\n\n")
  def htmlEnd(self):
    end="""</tr></table>
<table><tr> <th>Clusters:</th>
"""
    for ixc in range(len(Clusters)):
      end= end+ '<td bgcolor="%s" width=10><b>%d</b></td>\n'% \
        (Clusters[ixc].color, ixc+1)
    end= end + """
<td width=10>&nbsp&nbsp</td>
<td bgcolor="%s"><b>&nbsp busy</b></td>
<td bgcolor="%s"><b>&nbsp slowest in the cluster</b></td>
<td bgcolor="red"><b>&nbsp BUSY</b></td>
<td bgcolor="%s"><b>&nbsp internal error</b></td>
"""%(colorOK, colorSLOWEST, colorINTERR)
    end= end + """
</tr></table>
</tr>
</table>
<br>
<img src="pngs/inputs.png" title="CTP inputs" alt="CTP inputs" width="100%" />
</body>
</html>
"""
    self.htmlf.write(end)
    self.htmlf.close()

def main():
  runs= ActivePartitions()
  if len(sys.argv)>1: 
    p1=sys.argv[1];
    if p1=='htmlfifo':
      htmlf=open("/tmp/htmlfifo", "r")
      while 1:
        if htmlf==None:
          print "error reading input, stopping..."
          break
        busyl0s= htmlf.readline()
        #print "busyl0s:",busyl0s
        if not busyl0s: 
          print "stdin EOF, stopping..."
          break
        if busyl0s[0]=='q':
          print "quit from /tmp/htmlfifo, stopping..."
          break
        #busyl0s= string.strip(sys.stdin.readline())
        runs.updatePartitions()
        if runs.updateBusys(busyl0s): continue
        runs.makeHtml()
        #sys.stdout.write(busyl0s+"\n")
        #sys.stdout.flush()
      # end of the loop (1 min)
    elif p1=='busyL0':
      busyl0s= sys.stdin.readline()
    else:
      print p1,"?-unknown, starting just test (1 pass)"
      # following in the loop (over stdin or DIM...)
      busyl0s="busyL0 22.2.2008 12:23:01 minute 2 11 22 33 44 55 dead 77 88 99 1010 1111 1212 1313 1414 808 1616 1717 1818 1919 2020 2121 2222 2323 2424"
    runs.updatePartitions()
    runs.updateBusys(busyl0s)
    runs.makeHtml("busyL0debug.html")
  else: 
    p1=None
    print """Usage:
aj@tp:~$ cd CNTRRD
aj@tp:~/CNTRRD$ $VMECFDIR/CNTRRD/htmlCtpBusysClock.py test

htmlCtpBusys.py htmlfifo         htmlfifo: take lines from /tmp/htmlfifo
                busyL0           take just 1 line from stdin
                test             take just 1 line prepared in main()

"""

#main()
if __name__ == "__main__":
  main()

