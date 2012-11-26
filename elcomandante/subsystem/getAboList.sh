#!/bin/bash

( echo -e "m"; sleep 0.01; echo "la"; sleep 0.01; echo "x" ) |\
	client /server/clientinfo 2>/dev/null |\
	grep "/" |\
	cut -d " " -f 2 |\
	sed ':a;N;$!ba;s/\n/ /g'
