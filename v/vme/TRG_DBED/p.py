grep -e 'lass Trg' -e '  def ' parted.py >p.py
class TrgInput:
  def __init__(self, name):
  def prt(self):
class TrgInpDet:
  def __init__(self, name):
  def findInput(self, name):
  def addInput(self, inp):
  def getinputnames(self):
  def prt(self):
class TrgDescriptor:
  def __init__(self, name, inps=[]):
  def show(self,master):
  def prt(self):
class TrgPF:
  def __init__(self, name, vals):
class TrgSHR:
  def __init__(self, name, helptext=None):
  def show(self,master):
  def hide(self):
  def updt(self, event=None):
  def getValue(self):
  def setValue(self, newvaluetxt):
  def getValueHexa(self):
  def save(self, outfile):
  def __init__(self):
  def initTDS(self):
  def findInput(self, name):
  def findTrgInpDet(self, name):
  def findPF(self, name):
  def findTD(self, name):
  def findLTU(self, name):
  def getltunames(self):
  def inp2DetName(self, inpname):
  def doTDSbut(self, master, cmd, trdeactive):
class TrgClass:
  def __init__(self, clstring, mycluster):
  def prtClass(self):
  def setCluster(self, mycluster):
  def getTXTpfs(self):
  def getPFs(self):
  def getTXTbcrnd(self):
  def getTXTbcms(self):
  def getBCMASKs(self):
  def getTXTcls(self):
  def hide(self):
  def showClass(self):
  def l0prcmd(self, event=None):
  def getclsname(self):
  def clsnamecmd(self, event=None):
  def modadi(self, adibutinst, ixoi):
  def rarecmd(self):
  def modpfs(self, mli, ix):
  def modtds(self, mywxinstance, ix):
  def modButton(self):
  def hideclass(self,event):
class TrgCluster:
  def __init__(self, clusname, partition=None, cls=None, outdets=None):
  def getCLS(self):
  def prt(self):
  def hide(self):
  def doTDSbuttons(self):
  def doCLSbutton(self, ixcls):
  def doTDShead(self):
  def refreshTDShead(self):
  def show(self,master):
  def modcls(self, cls):
  def modtdsl(self, mwl, modix):
  def modoutl(self):
  def delete(self, event=None):
  def save(self, outfile):
  def getltunames(self):
  def getinpdets(self):
  def dobits(self, bitlist, ltl):
  def setActiveCluster(self, event=None):
  def activeCluster(self,event=None):
  def deactiveCluster(self,event=None):
class TrgPartition:
  def __init__(self, name):
  def donewtd(self):
  def show(self,master,name=None):
  def hide(self):
  def destroy(self):
  def prt(self):
  def getRR(self):   # get Required Resources
  def save(self, partname):
  def checkPartition(self):
  def savepcfg(self, fnw):
  def savercfg(self, line=""):
  def allocShared(self, l0funs):
  def addpf(self,pfname):
  def loadfile(self, inf):
  def showShared(self, minst, ix):
  def hideShared(self, event=None):
  def addCluster(self, minst,ix ):
  def findCluster(self, clu):
  def rmCluster(self, clu):
  def renActiveCluster(self, minst, ix):
  def clusterRenamed(self, event=None):
  def delActiveCluster(self, minst, ix):
  def setActiveCluster(self, actclu):
  def getn(x):return string.split(x,'.')[0]
  def __init__(self, master, partname):
  def doNames(self):
  def refreshNames(self, sf1):
  def doSaveEntry(self):
  def cancelPart(self, minst=None, ix=None):
  def quitPart(self, minst, ix):
  def savePart(self, minst, ix):
  def savePartAs(self, minst, ix):
  def savePartAs2(self, event=None):
  def loadPartition(self,partname=None):
  def changeShown(self,menuinstance, ix):
  def checkall(self):
