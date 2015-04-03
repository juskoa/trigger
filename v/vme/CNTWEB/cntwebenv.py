#!/usr/bin/python
import os,string

HOST=os.popen('hostname -s').read().rstrip()
BASEDIR="/var/www/html/"
#IMAGES="http://"+HOST+"/imgs/"
IMAGES="/imgs/"
if HOST=='pcalicebhm05':
  _pref="/data/ClientCommonRootFs/usr/local/trigger/vd/"
  #WORKDIR="/home/trigger/v/vme/"
elif HOST=='pcalicebhm10':
  #_pref="/home/dl/root/usr/local/trigger/devel/v/"
  _pref="/home/dl6/local/trigger/v/"
else:
  #_pref="/data/dl/root/usr/local/trigger/stable/v/"
  _pref="/home/dl6/local/trigger/v/"
VMEBDIR= _pref+"vmeb/"
VMECFDIR= _pref+"vme/"
dbctp= _pref+"vme/CFG/ctp/DB/"
#WORKDIR="/home/trigger/v/vme/"

BASEHELPS="../../../htmls/"

