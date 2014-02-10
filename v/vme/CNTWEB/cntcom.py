#!/usr/bin/python
import os,string

HOST=os.popen('hostname').read().rstrip()
BASEDIR="/var/www/html/"
#IMAGES="http://"+HOST+"/imgs/"
IMAGES="/imgs/"
if HOST=='pcalicebhm05':
  VMEBDIR="/data/ClientCommonRootFs/usr/local/trigger/vd/vmeb/"
  #WORKDIR="/home/trigger/v/vme/"
elif HOST=='pcalicebhm10':
  #VMEBDIR="/home/dl/root/usr/local/trigger/devel/v/vmeb/"
  VMEBDIR="/home/dl6/local/trigger/v/vmeb/"
else:
  VMEBDIR="/home/dl6/local/trigger/v/vmeb/"
  #WORKDIR="/home/trigger/v/vme/"

BASEHELPS="../../../htmls/"

def getStartEnd(cfg):
  """cfg.period: month week day '10 hours' hour 3months 'Show period of'
  ltuname: one of names in LTUS
  rc: fromto,pixwidth
  """
  period= cfg.period ; pixwidth="600"
  if period=='Show period of':
    #return "makeImage:%s %s\n"%(cfg.customperiod, cfg.startgraph)
    mh='h'; customperiod=1
    fromto="-s now-8h"
    try:
      mh= cfg.customperiod[-1]
      customperiod= int(cfg.customperiod[:-1])
    except:
      fromto="-s now-4h"
    else:
      if mh=='m' or mh=='h' or mh=='d':
        fromto= "-s'%s' -e's+%i%c'"%(cfg.startgraph, customperiod,mh)
  elif period=='3months':
    fromto='-s now-3m'
  elif period=='month':
    fromto='-s now-1m'
  elif period=='week':
    fromto='-s now-7d'
  elif period=='day':    # this is default (no -s par. present)
    fromto='-s now-1d'
    pixwidth= "1440"
  elif period=='10 hours':
    fromto='-s now-10h'
  elif period=='hour':
    fromto='-s now-1h'
  else:
    fromto='' 
  return fromto, pixwidth
def userrequest(cfg):
  if cfg.period=="Show period of":
    cuperiod= cfg.customperiod + ' from:' + cfg.startgraph
  else:
    cuperiod= cfg.period
  #<INPUT TYPE="reset" VALUE="reset last selection">
  usrreq= """
</TABLE><BR>
Show last:
<INPUT TYPE="submit" NAME="period" VALUE="hour" TITLE="averaged per minute"
onMouseOver="window.status='average per minute';return true">
<INPUT TYPE="submit" NAME="period" VALUE="10 hours" TITLE="averaged per minute"
onMouseOver="window.status='average per minute';return true">
<INPUT TYPE="submit" NAME="period" VALUE="day" TITLE="averaged per minute" 
onMouseOver="window.status='average per minute';return true">
<INPUT TYPE="submit" NAME="period" VALUE="week" TITLE="averaged per hour"
onMouseOver="window.status='average per hour';return true">
<INPUT TYPE="submit" NAME="period" VALUE="month" TITLE="averaged per hour"
onMouseOver="window.status='average per hour';return true">
<INPUT TYPE="submit" NAME="period" VALUE="3months" TITLE="averaged per day"
onMouseOver="window.status='average per day';return true">
<br><br>
<INPUT TYPE="submit" NAME="period" VALUE="Show period of">
<INPUT TYPE="text" NAME="customperiod" SIZE="4" VALUE="%s" TITLE="e.g.: 40m (minutes) 3h (hours)"
onMouseOver="window.status='Examples: 3h (3hours)   50m (50minutes)   40d (40days)'; return true">
&nbsp&nbsp&nbsp starting from:
<INPUT TYPE="text" NAME="startgraph" SIZE="16" VALUE="%s" TITLE="e.g.: 12:40 17.04.08"
onMouseOver="window.status='Examples: 12:40 17.04.08 (April 17th)      22:30  (1.5 hour before last midnight)'; return true">
</FORM>
Rates (signals/sec) over last %s:
"""%(cfg.customperiod,cfg.startgraph, cuperiod)
  return usrreq
def HtmlEditPrint(fn,**args):
  """
  fn           -name of an html file to be printed to stdout
  a1='valuea1' -#a1# is replaced by 'valuea1' before print
  FCa1='fn'    -#FCa1# is replaced by content of the file fn
                or by '' if fn==''
  ! Most 1 replacement in 1 line !
  """
  #print fn,':',args
  prn=""
  htmlf=open(fn)
  for line in htmlf.read().splitlines():
    i1=-1
    for par in args.keys():
      parh='#'+par+'#'
      i1=string.find(line, parh)
      if i1!=-1:
        if par[0:2]=='FC':
          if args[par]=='':
            prn= prn+'\n'+ string.replace(line, parh, args[par], 1)
          else:
            i2=i1+len(parh)
            if line[:i1]: prn= prn+'\n'+ line[:i1]
            trginf=open(args[par])
            for l in trginf.read().splitlines(): 
              if l: prn= prn+'\n'+ l
            trginf.close()
            #print parh,i1,i2,':',args[par]
            prn= prn+'\n'+ line[i2:]
        else: 
          prn= prn+'\n'+ string.replace(line, parh, args[par], 1)
        break
    if i1==-1:
      prn= prn+'\n'+ line
  htmlf.close()
  return prn

