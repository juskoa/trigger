<?php
  include 'page_config.php';
  include 'status_config.php';
  page_header();

ob_start();

echo '<table class="normal"><thead>';
  echo "<th class='normal'>Name</th>";
  echo "<th class='normal'>Status</th>";
  echo "<th class='normal'>Logs</th>";
echo "</thead><tbody>";

echo "<tr>";
reset($allservices);
$i_pc=0;
do {
echo "<tr>";
  $k = key ($allservices);
  $val = current ($allservices);
  $val_pc = $allservices_pc[$i_pc];
  $i_pc++;
  $out = exec("$cmdssh$val_pc $cmd $val $service_status",$array_out);
  $num = count($array_out)-1;
//   for ($i = 0; $i < $num; ++$i) 
//     $status
  echo "<td class='serviceName'>$val</td>";

//   if (strpos($array_out[$num],$stoped))
  if ($out == $started)
    echo "<td class='green'>started</td>";
  else
    echo "<td class='red'>stoped</td>";
  echo "<FORM action='cmd.php' method='GET'>";
    echo "<td class='normal'>";
      echo "<INPUT type='hidden' name='servname' value='$val'>";
      echo "<INPUT type='hidden' name='pcname' value='$val_pc'>";
      if ( $managerHost == $_SERVER['REMOTE_ADDR']) {
      echo "<INPUT type='submit' name='servcmd' value='start' >";
      echo "<INPUT type='submit' name='servcmd' value='stop' >";
      echo "<INPUT type='submit' name='servcmd' value='restart' >";
      }
      echo "<INPUT type='submit' name='servcmd' value='log' >";
    echo "</td>";
  echo "</FORM>";

echo "</tr>";

 ob_flush();
 flush();
 sleep(1);

} while (next ($allservices));

echo "</tbody></table>";
  page_footer();
?>
