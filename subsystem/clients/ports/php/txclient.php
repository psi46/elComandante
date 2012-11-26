<?php
	require('./subserver.php');
	send_subserver('127.0.0.1', "/test", "Hallo Welt\n", 1);
	echo "<br>packet gesendet\n";
?>
