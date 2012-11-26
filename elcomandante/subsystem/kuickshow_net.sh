#!/bin/sh

(echo -e "m"; sleep 1; echo "lg"; sleep 1; echo "x" ) |\
	./client /server/clientinfo | tee show_net.dot |\
	neato -Tpng >show_net.png
echo "generating output..."
kuickshow show_net.png
