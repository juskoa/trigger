<?php

function page_header(){
echo <<<EOF
<HTML>
  <HEAD>
    <LINK href="style.css" rel="stylesheet" type="text/css">
    <meta http-equiv="Content-type" value="text/html; charset=UTF-8" />
  </HEAD>
  <BODY class='defualt'>
EOF;
}

function page_footer(){
echo <<<EOF
  </BODY>
</HTML>
EOF;
}

?>