#!/bin/bash
allvmes='alidcsvme002 alidcsvme003 alidcsvme004 alidcsvme005 alidcsvme006 alidcsvme007'
echo "ltu_proxys (i.e. proxy code AND DIMservices linked in server):"
for hn in $allvmes ;do
  prxs=`ssh -2 "trigger@$hn" ps -C ltu_proxy o user,pid,args 2>&1 |grep -v -e USER -e Scientific`
  if [ -n "$prxs" ] ;then
    echo "$hn:"
    echo "$prxs" | colrm 16 56
  fi
done
exit
echo "ltudimservers (i.e. proxy code NOT linked in server):"
for hn in $allvmes ;do
  prxs=`ssh -2 "trigger@$hn" ps -C ltudimserver o user,pid,args 2>&1 |grep -v -e USER -e Scientific`
  if [ -n "$prxs" ] ;then
    echo "$hn:"
    echo "$prxs" | colrm 16 68
  fi
done

