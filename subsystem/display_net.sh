#!/bin/sh

(echo -e "m"; sleep 1; echo "lg"; sleep 1; echo "x" ) |\
	./client /server/clientinfo |\
	sed -e "s/len=./len=3/" | \
	neato -Tpng > show_net.png
echo "generating output..."
#kuickshow show_net.png
display show_net.png
