#!/usr/bin/perl -wt
require("./subserver.pl");
send_subserver("127.0.0.1", "/test", "Hallo Welt\n", 1);

