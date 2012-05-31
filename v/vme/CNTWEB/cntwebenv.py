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
  VMEBDIR="/home/dl/root/usr/local/trigger/vd/vmeb/"
else:
  _pref="/data/dl/root/usr/local/trigger/v/"
VMEBDIR= _pref+"vmeb/"
VMECFDIR= _pref+"vme/"
dbctp= _pref+"vme/CFG/ctp/DB/"
#WORKDIR="/home/trigger/v/vme/"

BASEHELPS="../../../htmls/"

