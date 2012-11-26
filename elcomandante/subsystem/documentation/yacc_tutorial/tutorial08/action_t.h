/**
 * \file action_t.h
 * \author Dennis Terhorst
 * \date Wed Jul  8 12:18:33 CEST 2009
 */
#ifndef ACTION_T_H
#define ACTION_T_H

#include <string>
#include <iostream>
using namespace std;

class action_t {
	string name;
	string action_text;
public:
	action_t(string Name, string Action_Text) {
		name = Name;
		action_text = Action_Text;
	}
	virtual ~action_t() {};
	void run() {
		cout << "ACTION " << name << ".run();" << endl;
		//action_context.push_back(new action_context_t());	// add new context to stack
		//parse(action_text); // interpretation
		//while ( action_context.working() );
		//action_context.pop_back();				// remove this actions context from stack
	}
};


#endif //ndef ACTION_T_H
