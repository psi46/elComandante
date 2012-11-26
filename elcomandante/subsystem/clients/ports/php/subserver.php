<?php
function send_subserver($hostname, $abo, $data, $packet_type){	
	define("MAXLEN", 2043);
	define("PORTNO", 12334);
	if(strlen($data) < MAXLEN){
		$packet = chr(0).chr($packet_type).chr(0).chr(strlen($abo)+1).$abo.chr(0).$data;
	}else{
		echo "data too long";
		return false;
	}
	//$packet = chr(0).chr($packet_type).chr(0).chr(strlen($abo)+1).$abo.chr(0).$data;
	$sock = socket_create(AF_INET, SOCK_DGRAM, 0);
	$send_ret = socket_sendto($sock, $packet, strlen($packet), 0, $hostname, PORTNO);
	socket_close($sock);
	return true;
}
?>
