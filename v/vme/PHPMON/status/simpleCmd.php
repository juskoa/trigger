<?php
  include 'page_config.php';
//   require 'ssh.php';
//   include 'services_config.php';
  page_header();
//   $out = system("sudo -u trigger sh myssh.sh ltuproxy.sh active | grep address",$array_out);
    $out = system("sudo -u trigger sh myssh.sh ltuproxy.sh muon_trk status",$array_out);
//   $out = exec("ssh -2 trigger@altri1 ltuproxy.sh active | grep address",$array_out);
  $num = count($array_out);
  for ($i = 0; $i < $num; ++$i) {
      echo "\t$array_out[$i]<br>\n";
  }

  page_footer();
?>