#!/bin/sh

NAME=subscript;
echo "listing processes named $NAME with child processes:";
ps aux | grep "$NAME" | grep -v "grep";
PIDS=`ps x | grep "$NAME" | grep -v "grep" | cut -b-5`;
if [[ "$PIDS" == "" ]]; then
	echo "no such process";
	exit;
fi
for pid in ; do
	ps f --sid $pid;
done
