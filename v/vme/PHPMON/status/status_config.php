<?php

  $started = "0";
  $cmdssh= "sudo -u trigger ssh -q -2 trigger@";
  $cmdssh_nohup="sudo -u trigger nohup ssh -q -f -2 trigger@";
  $cmdssh_post="&";
  $cmd="ltuproxy.sh";
  $service_status="status";

  $tailNum="40";

//   set_max_execution_timeset(2);
  $pit="1";
  if ($pit == "1"){
    // $managerHost is host that can e manager (start,stop,restart)
    // aldaqacr07:
    $managerHost="alidcscom835";
    $dbctp="/home/dl6/local/trigger/v/vme/CFG/ctp/DB";
    $vmeCfDir="v/";
  }
  else {
    // $managerHost is host that can e manager (start,stop,restart)
    // alidcscom002:
    $managerHost="10.160.33.201";
    $dbctp="/home/dl6/local/trigger/v/vme/CFG/ctp/DB";
    $vmeCfDir="v/";
  }

  //$out = exec("sudo -u trigger cat $dbctp/ttcparts.cfg");
  //$out = exec("cat $dbctp/ttcparts.cfg",$outaa);
  //echo "sudo:$dbctp:$outaa[1] <br>";
  //$out = exec("sudo -u trigger cat $dbctp/ttcparts.cfg | awk '{print $1}'",$array_out);
  $out = exec("cat $dbctp/ttcparts.cfg | awk '{print $1}'",$array_out);
  $num = count($array_out);
  //echo "out: $out num: $num<br>";
  for ($i = 0; $i < $num; ++$i) {

    $rest = substr($array_out[$i],0,1);
    if ($rest != "#")
      $allservices[$i]= $array_out[$i];
    //echo "$array_out[$i]<br>";
  }
//$allservices = array("hmpid","muon_trk");


  //$out2 = exec("sudo -u trigger cat $dbctp/ttcparts.cfg | awk '{print $2}'",$array_out2);
  $out2 = exec("cat $dbctp/ttcparts.cfg | awk '{print $2}'",$array_out2);
  $num = count($array_out2);
  for ($i = 0; $i < $num; ++$i) {
    $rest = substr($array_out[$i],0,1);
    if ($rest != "#")
      $allservices_pc[$i]= $array_out2[$i];
      //echo "$allservices_pc[$i]<br>";
  }


?>
