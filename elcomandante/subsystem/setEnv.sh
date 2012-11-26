#!/bin/sh

MYIP=`netstat -an --inet | grep -v "Local Address" | grep -v "127.0.0.1" | tail -n1 | cut -b21- | cut -f1 -d":"`
PORT=12334

echo "export SUBSERVER=${MYIP}:${PORT}";
export SUBSERVER="${MYIP}:${PORT}"
