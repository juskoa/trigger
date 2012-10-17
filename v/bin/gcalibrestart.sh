#/bin/bash
#  delayed stop/start gcalib + send sms
function gpid() {
go=`gcalib.sh status`
#echo "go:$go"
read -a goa <<< "$go"
next="notok" ; pid=""
for index in "${!goa[@]}" ;do
  val=${goa[index]}
  #echo "$index $val"
  if [ "$next" == "ok" ] ;then
    pid=$val
    #echo "found pid:$pid"
    break
  fi
  if [ "$val" == "pid:" ] ;then 
    next="ok"
  else
    next="notok"
  fi
done
}

. $VMECFDIR/../bin/auxfunctions
#echo server: $server
#oldpid=A ; newpid=2
#ssh trigger@$server "mail2oncall.bash gcalib $oldpid restarted, new: $newpid"
#exit
gpid ; oldpid=$pid
gcalib.sh stop >/dev/null
sleep 1
gcalib.sh start >/dev/null
gpid ; newpid=$pid
echo "old:$oldpid new:$newpid"
#send sms with old/new pid
ssh trigger@$server "mail2oncall.bash gcalib $oldpid restarted, new: $newpid"
