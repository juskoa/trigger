<?php
  include 'page_config.php';
//   include 'services_config.php';
  include 'status_config.php';
  page_header();

ob_start();

echo '<table class="normal"><thead>';
  echo "<th class='normal'>Name</th>";
  echo "<th class='normal'>Status</th>";
  echo "<th class='normal'>Manage</th>";
echo "</thead><tbody>";

echo "<tr>";

reset($allservices);
$num = count($allservices);
echo "$num";
do {
echo "<tr>";
  $k = key ($allservices);
  $val = current ($allservices);

  $out = exec("$cmd/$val $service_status",$array_out);
  $num = count($array_out)-1;
//   for ($i = 0; $i < $num; ++$i) 
//     $status
  echo "<td class='serviceName'>$val</td>";

  if (strpos($array_out[$num],$strstarted))
    echo '<td class="green">started</td>';
  else
    echo '<td class="red">stoped</td>';
  echo "<FORM action='cmd.php' method='GET'>";
    echo "<td class='normal'>";
      echo "<INPUT type='hidden' name='servname' value='$val'>";
      echo "<INPUT type='submit' name='servcmd' value='start' >";
      echo "<INPUT type='submit' name='servcmd' value='stop' >";
      echo "<INPUT type='submit' name='servcmd' value='restart' >";
      echo "<INPUT type='submit' name='servcmd' value='log' >";
    echo "</td>";
  echo "</FORM>";

echo "</tr>";

 ob_flush();
 flush();
 sleep(1);


} while (next ($allservices));

echo "</tbody></table>";


// echo "Done";
  page_footer();

?>