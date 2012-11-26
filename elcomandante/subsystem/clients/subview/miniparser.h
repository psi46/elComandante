/**
 * \file miniparser.h
 * \author Dennis Terhorst
 * \date Thu Sep 10 16:38:45 CEST 2009
 */
#ifndef MINIPARSER_H
#define MINIPARSER_H
#include <iostream>	// cerr
#include <unistd.h>	// pipe, write, close
#include <stdio.h>	// fclose

#include <sstream>
#include <vector>
#include <string>
#include "global.h"
#include <unistd.h>	// usleep()
#include <stdlib.h>	// getenv()

typedef std::vector<std::string> argv_vec_t;

int parse(std::string line) {
	// parse into argv
	std::istringstream in(line); //(std::string(line));
	std::string arg;
	argv_vec_t argv;
	while (!in.eof() ) {
		in >> arg;
		//wout << "Parsing argument " << argv.size() << ":" << arg <<"\n";
		argv.push_back(arg);
	}
	if (argv.size() < 1) return -1;

	// commands
	if        (argv[0] == "hallo"|| argv[0] == "hello") {
		std::string username = getenv("USER");
		if (username =="") username="unknown user"; else username = std::string("user ") + username;
		wout << argv[0] << " " << username << "\n";
	} else if (argv[0] == "exit") {
		global.wantexit++;
	} else if (argv[0] == "subscribe") {
		if (argv.size() != 2) { wout << argv[0] << ": wrong number of parameters!\n"; return -1; }
		if (argv[1][0] != '/') {
			wout << argv[0] << ": abo name does not start with \"/\", i don't like to subscribe that!\n";
			return -1;
		}
		wout << "subscribing " << argv[1] << "\n";
		global.meptr->subscribe(argv[1].c_str());
		return 0;
	} else if (argv[0] == "unsubscribe") {
		if (argv.size() != 2) { wout << argv[0] << ": wrong number of parameters!\n"; return -1; }
		if (argv[1][0] != '/') {
			wout << argv[0] << ": abo name does not start with \"/\"!\n";
			return -1;
		}
		wout << "unsubscribing " << argv[1] << "\n";
		global.meptr->unsubscribe(argv[1].c_str());
		return 0;
	} else if (argv[0] == "list") {
		wout << "sending management command \"la\"\n";
		packet_t packet;
		packet.setName("");
		packet.setData("la",3);
		packet.type = PKT_MANAGEMENT;
		global.meptr->sendpacket(packet);
		return 0;
	} else if (argv[0] == "help") {
		wout << "available commands are:\n";
		wout << "  subscribe <abo>\n";
		wout << "  unsubscribe <abo>\n";
		wout << "  help\n";
		wout << "  exit\n";
	} else {
		wout << "unknown command \""<< argv[0] << "\", try \"help\"\n";
		return -1;
	}
	return 0;
}

#endif //ndef MINIPARSER_H
