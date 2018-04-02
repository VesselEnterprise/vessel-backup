<?php

  require_once "../core/file.class.php";

  //Test packing a SHA-1 hash into binary
  $hash = '0D67699C1C6D2FBAF3230061923810AB3DF32EBB';

  $bin = hex2bin($hash);

  echo $bin;

?>
