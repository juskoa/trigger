<?php
  include 'page_config.php';
  include 'status_config.php';
  page_header();


// echo $_SERVER['REMOTE_ADDR'];
//   echo "<br>";
// echo $_SERVER['REMOTE_USER'];
//    phpinfo();
  $serviceName = $_GET['servname'];
  $pcName = $_GET['pcname'];
  $serviceCmd = $_GET['servcmd'];
  if ($serviceCmd == "log"){
    $upperServisName=strtoupper($serviceName);
    $serviceCmdFull = "$cmdssh$pcName tail -n $tailNum $vmeCfDir$serviceName/WORK/LTU-$upperServisName.log";
  }
  else if ($serviceCmd == "start") {
    $serviceCmdFull = "$cmdssh_nohup$pcName $cmd $serviceName $serviceCmd $cmdssh_post";
  }
  else {
    $serviceCmdFull = "$cmdssh$pcName $cmd $serviceName $serviceCmd";
  }

  $array_out = array();
  $out = exec($serviceCmdFull,$array_out,$retVal);
  $num = count($array_out);
//   echo "$out-$retVal-$num";
//   echo "<br>";
  echo "<textarea name='log' cols=100 rows=10>";
  if ($retVal == 0) {
    for ($i = 0; $i < $num; $i++) {
      echo "$array_out[$i]\n";
    }

//   echo "<br>Command '$serviceCmdFull' was successfull";
  }
  else {
    echo "Error running command '$serviceCmdFull'";
  }
  echo "</textarea>";

  echo "<FORM action='index.php' method='POST'>";
  echo "<INPUT TYPE='button' VALUE='Back' onClick='history.go(-1);return true;'>";
  echo "<INPUT type='submit' value='Back & refresh' >";
  echo "</FORM>";

  page_footer();
?>
