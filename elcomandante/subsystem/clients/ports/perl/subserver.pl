#!/usr/bin/perl -w
use strict;
use IO::Socket;

# send data to subserver
# parameter:
# 	1.	hostname
# 	2.	abo name
# 	3.	data
# 	4.	packet_type
sub send_subserver{
	my ($hostname, $abo, $data, $packet_type) = ($_[0],$_[1], $_[2], $_[3]);
	my $MAXLEN = 2043;
	my $PORTNO = 12334;
	my $packet;
	if(length($data) < $MAXLEN){
		$packet = sprintf("%s%s%s%s%s%s%s", chr(0), chr($packet_type), chr(0), chr(length($abo)+1), $abo, chr(0), $data); 
	}else{
		die "data too long";
	}
	socket(SOCKET, PF_INET, SOCK_DGRAM, getprotobyname('udp')) or die "socket:$!";
	my $portaddr = sockaddr_in($PORTNO, inet_aton($hostname));
	send(SOCKET, $packet, 0, $portaddr) == length($packet) or die "cannot send packet:$!";
}
1;
