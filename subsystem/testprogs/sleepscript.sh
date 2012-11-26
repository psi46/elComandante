#!/bin/sh

while true; do
	i=$(( $i + 1 ));
	echo "ping $i";
	sleep 1;
done | ./client /sleeptest/info
