/**
 * \file action_context.cpp
 * \author Dennis Terhorst
 * \date Mon Oct  5 17:15:14 CEST 2009
 */

#include "action_context.h"
#include <algorithm>	// for find()
#include <unistd.h>	// for usleep()
#include <fstream>	// for ifstream
#include <deque>	// used in showPosition()
using namespace std;

#undef NDEBUG
#ifdef NDEBUG
#	define cerr if (0) cerr
#endif

const int action_context::waitfor_usec = 1000;

action_context::action_context(action_context* ParentContext, std::istream* is) throw()
{
	wantexit=0;
	refcount=0;
	mythread=NULL;
	parent = ParentContext;
	child = NULL;
	if (ParentContext == NULL) {	// this is top context
		name = "*";
	} else {			// this is sub-context
		parent->parallel_context.push_back(this);
		parent->refcount++;
		name="unnamed";
		scptr = ParentContext->scptr;
	}
	//cerr << "new context() " << Name() << endl;
	init_scanner();
	esc_depth=0;
	this->is = is;
	result = 0;
	cerr << "NEW context() "<< this << " called " << Name() << " parent = " << parent << endl;
}

action_context::~action_context() throw(errno_exception<EBUSY>)
{
	cerr << "~context() "<< this << " begin: " << Name() << " return: " << result << endl;
	//if (refcount > 0) throw(errno_exception<EBUSY>("cannot delete referenced context_t"));
	cerr << "~context() " << Name() << " not referenced. good." << endl;
	if (parent != NULL) { // unregister this from parent context
		cerr << "~context(): " << Name() << " unregister from parent context" << endl;
		parent->parallel_context.erase(find(parent->parallel_context.begin(), parent->parallel_context.end(), this));
		parent->refcount--;

		/* do not delete parents here if their refcount==0, there might
		 * be more statements to execute! the same problem arieses with
		 * a hypothetical delete_parents() function. The thread_t class
		 * has to do this cleanup, because it is the only class knowing
		 * there is nothing more to execute.
		 */

	}
	cerr << "~context(): " << Name() << " destroy scanner" << endl;
	destroy_scanner();
	//cerr << "~context(): " << Name() << " delete istream" << endl;
	//if ( is != &cin ) delete is;	// deleting is is none of our businiess!!!
	cerr << "~context() "<< this << " end " << Name() << " return: " << result << endl;
}


void action_context::setName(std::string newname) throw() {
	mutex.lock();		// [
	name = newname;
	cerr << "context "<< this << " now called " << Name() << endl;
	mutex.unlock();		// ]
}

string action_context::Name() const throw() {
	if ( parent == NULL ) { return name; }
	return parent->Name() + "/" + name;
	
}

#include <stack>
void action_context::showPosition(unsigned int context_lines) throw() {
	mutex.lock();		// [
	stringstream prev;
	prev.str(os.str());
	string line;
	deque<std::string> lines;

	do {	
		getline(prev, line);
		lines.push_back(line);
		if (lines.size() > context_lines) lines.pop_front();
	} while (! prev.eof());
	for (deque<std::string>::iterator l=lines.begin(); l!=lines.end(); ++l)
		if (l==lines.end()-1)
			std::cout << ">>>>>>\t" << *l << "\t <<<<<<<<<<<<<<<<<<<<<<<" << endl;
		else
			std::cout << "\t" << *l << endl;

	if (is == &cin) {
		std::cout << "\t[to be read from stdin...]" << endl;
	} else {
		std::stack<std::string> lines;
		bool first=true;
		while (! getline(*is, line).eof()) {	// read and display complete istream
			lines.push(line);
			if (lines.size() > context_lines) break;
			if (first && (line == "\n" || line == "")) continue;
			std::cout << "\t" << line << endl;
			first=false;
		}
		while (!lines.empty()) {			// unread complete istream
			std::string::reverse_iterator ch;
			for ( ch=lines.top().rbegin(); ch!=lines.top().rend(); ++ch) {
				is->putback(*ch);
			}
			lines.pop();
		}
	}
	mutex.unlock();		// ]
}


action_context* action_context::RootParent() throw() {
	if (parent != NULL) return parent->RootParent();
	return this;
}

/*
 * show all subthreads and their execution positions
 */
void action_context::showTree(std::string pretext) throw() {
	mutex.lock();		// [
	action_context* cptr = this;
	std::cout << pretext << this->Name() << " @ " << this;	// FIXME: maybe add execution pointer/line information here
	if ( parent!=NULL )
		std::cout << "\t(parent=" << parent << ")";
	else
		std::cout << "\t(parent=" << "NULL" << ")";
	std::cout << endl;
	while ((cptr=cptr->child) != NULL) {
		std::cout << pretext << " -> child @ " << cptr;
		cptr->showTree(pretext+"\t\t");
	}
	if (child != NULL) std::cout << endl;
	for (parallel_context_vec_t::iterator pcontext = parallel_context.begin();
					      pcontext !=parallel_context.end();
						++pcontext) {
		std::cout << pretext << " -> parallel context @ " << (*pcontext) << ":" << endl;
		(*pcontext)->showTree(pretext+"\t");
	}
	mutex.unlock();		// ]
	return;
}


/**
 * \brief get this contexts thread_t
 *
 * eventually recurses parents until mythread!=NULL
 */
thread_t* action_context::Thread() const throw(errno_exception<EFAULT>) {
	if (mythread == NULL) {
		if (parent != NULL) return parent->Thread();
		throw(errno_exception<EFAULT>("no thread information found"));
	} // else { this is root of a thread
	return mythread;
	// }
}

/**
 * \brief set the contexts managing thread
 * this function should be called after establishing a parallel thread
 */
void action_context::setThread(thread_t* thr) {
	mutex.lock();		// [
	mythread = thr;
	mutex.unlock();		// ]
}


/*
 * A context is considered interactive if the input is coming from \c cin.
 */
bool action_context::isInteractive() const throw() {
	return (is == &cin);
}

int action_context::RefCount() const {
	cerr << "context " << Name() << "->RefCount() = "<< refcount << endl;
	return refcount;
}

action_context* action_context::Parent() const {
	return parent;
}

/**
 * This function reads in the contents of \c filename and prepends it to the parsers input stream by calling this->exec()
 */
void action_context::loadFile(std::string filename) throw(errno_exception<EBADFD>) {
	// no mutex.lock(): no changes to context and this->exec() below locks itself
	ifstream loadis(filename.c_str());
	if ( loadis.fail() ) {
		throw(errno_exception<EBADFD>(string("opening ")+filename+" failed"));
	}
	loadis >> noskipws;
	string subtext;
	char c;
	while ( loadis.good() ) {		// load complete file into subtext
		loadis >> c;
		subtext.push_back(c);
	}
	loadis.close();
	this->exec(subtext);
	return;
}

/**
 * This function prepends the given text to the input stream using istream::putback() function.
 */
void action_context::exec(std::string text) throw() {
	mutex.lock();		// [
	// putback the subtext into this parsers input;
	for (string::reverse_iterator s=text.rbegin(); s!=text.rend(); ++s) {
		is->putback(*s);
	}
	mutex.unlock();		// ]
	return;
}

/**
 * This context is requested to abort any action, if this function is called.
 */
void action_context::abort() throw() {
	mutex.lock();		// [
	if ( child == NULL ) {
		wantexit++;
		result++;	// anounce an error, to distinguish ctrl-c from normal termination
		cerr << "kill this context " << Name() <<  endl; 
	} else { // if ( child != NULL ) {
		child->wantexit++;		  // kill parser
		child->is->setstate(ios::eofbit); // kill lexer
		child->abort();			  // propagate down
		cerr << "killed child context " << child->Name() << endl;
	}
	mutex.unlock();		// ]
}
bool action_context::wantabort() throw() { return (wantexit>0); }


/*
 *  Variable manipulation/access functions	(obsolete)
 */
string action_context::var_set(std::string name, std::string value) throw() {
	return variables[name] = value;
}

string action_context::var_get(std::string name) throw() {
	variables_t::iterator var = variables.find(name);
	if ( var == variables.end() ) { // not found
		return string("");
	}
	return var->second;		// found
}

bool action_context::var_isset(std::string name) throw() {
	return( variables.find(name) != variables.end() );
}

void action_context::var_list() throw() {
	for (variables_t::iterator var=variables.begin(); var!=variables.end(); ++var)
		std::cout << var->first << " = " << var->second << endl;
	std::cout << variables.size() << " entries" << endl;
}

void action_context::var_show() throw() {
	variables_t::iterator var = variables.find(name);
	if ( var == variables.end() ) { // not found
		std::cout << "undefined";
	}
	std::cout << var->second;		// found
}


/*
 *  Procedure definition/manipulation functions
 */
void action_context::proc_add(proc_name_t name, proc_t value) throw(errno_exception<EEXIST>) {
	mutex.lock();		// [
	if ( procedures.find(name) != procedures.end() ) {	// existing definition
		mutex.unlock();		// ]
		throw( errno_exception<EEXIST>("definition for procedure " + name + " {...} exists"));
	}
	procedures[name] = value;
	mutex.unlock();		// ]
	return;
}

bool action_context::proc_isset(proc_name_t name) throw() {
	return( procedures.find(name) != procedures.end() );
}

void action_context::proc_list() throw() {
	mutex.lock();		// [
	for (procedures_t::iterator proc=procedures.begin(); proc!=procedures.end(); ++proc)
		std::cout << "procedure " << proc->first << endl;
	std::cout << procedures.size() << " procedures defined." << endl;
	mutex.unlock();		// ]
}

void action_context::proc_show(proc_name_t name) throw() {
	procedures_range_t range;
	if ( name != "all" ) {
		range = procedures.equal_range(name);	// will find one element, because this is a map (not multimap)
	} else {
		range.first = procedures.begin();
		range.second = procedures.end();
	}
	if ( range.first == range.second ) {
		std::cout << "no procedure '" << name << "' defined" << endl;
		return;
	}
	for (procedures_t::iterator proc=range.first; proc!=range.second; ++proc)
		std::cout << "procedure " << proc->first << " {" << proc->second << "}" << endl;
	return;
}

action_context::proc_t action_context::proc_get(proc_name_t name) throw(errno_exception<EINVAL>) {
	procedures_t::iterator proc = procedures.find(name);
	if ( proc == procedures.end() ) { // not found here
		if (parent != NULL) {
			return parent->proc_get(name);
		}
		throw errno_exception<EINVAL>("no procedure with name "+name);
	}
	return proc->second;		// found
}

int action_context::proc_run(proc_name_t name) throw() {
	//cerr << "execution of procedure " << name << "..." << endl;
	try {
		cerr << "execution of procedure {" << this->proc_get(name) << "}" << endl;
		return run_subcontext(this->proc_get(name), name);
	}
	catch (errno_exception<EINVAL> &e) {
		cerr << "execution of procedure " << name << " failed: " << e.what() << endl;
		return -1;
	}
	return 0;
}

int action_context::run_subcontext(std::string text, std::string name) throw() {
	cerr << "int action_context::run_subcontext(std::string text, std::string name) throw() {" << endl;
	int res=-1;
	try {
		//cerr << "execution of procedure {" << text << "}" << endl;
		mutex.lock();		// [
		istringstream subis(text);
		subis >> noskipws;
		action_context* subcontext = new action_context(this, &subis);
		subcontext->setName(name);
		child = subcontext;
		//refcount++;	// subcontext increases refcount in this object itself (FIXME:check this)
		mutex.unlock();		// ]
		subcontext->run();
		cerr << "run_subcontext():subcontext.result = " << subcontext->result << endl;
		mutex.lock();		// [
		res = subcontext->result;
		//refcount--;
		child = NULL;
		mutex.unlock();		// ]
		try {
			if (subcontext->RefCount() >0) throw(errno_exception<EBUSY>("predelete refcount check>0 throw"));
			delete subcontext;
			cerr << "run_subcontext(): subcontext deleted" << endl;
		} catch (errno_exception<EBUSY> &e) {	// subcontext might have created a thread using this context.
			cerr << "run_subcontext(): subcontext busy, delete deferred: " << e.what() << endl;
		}
		//turn subcontext.result;
	}
	catch (errno_exception<EINVAL> &e) {
		child = NULL;
		cerr << "execution of subcontext failed: " << e.what() << endl;
	}
	cerr << "int action_context::run_subcontext(std::string text, std::string name) throw() }" << endl;
	return res;
}


int action_context::run() throw() {
	cerr << "int action_context::run() throw() {" << endl;
	int ret=1;
	while ( ret != 0 && !wantabort() ) {
		try {
			if ( isInteractive() ) cerr << "starting parser" << endl;
			if ( isInteractive() ) cerr << Name() << " > " << flush;
			cerr << "starting parser for context " << this->Name() << endl;
		//	this->showPosition();
			cerr << "starting parser for context " << this->Name() << endl;
			ret = action_parse(this);	// parser locks mutex inside YY_INPUT() defined in action.l
			cerr << "parsing finished (context.result=" << result << ")" << endl;
		}
		catch (general_exception &e) {
			cerr << "parsing exception: " << e.what() << endl;
		}
		if ( ! isInteractive() ) break;
		cerr << "re"; // -starting parser
	}
	cerr << "int action_context::run() throw() }" << endl;
	return ret;
}

/*void action_context::parallel_run() throw() {
	

	return;
}*/

int action_context::pcall(const std::string procname) throw(errno_exception<EEXIST>, errno_exception<EINVAL>) {

	mutex.lock();		// [
	cerr << "PARALLEL execution of procedure " << procname << ": start" << endl;
	cerr << "         construct context for " << procname << "." << endl;
	// construct new context for thread
	istringstream* subis = new istringstream();
	subis->str(this->proc_get(procname));	// may throw EINVAL
	(*subis) >> noskipws;
	action_context* subcontext = new action_context(this, subis);	// create subcontext
	subcontext->setName(procname);		// FIXME: maybe check for existing thread here and throw EEXIST

	// start thread and register it in the context
	cerr << "         create thread for " << procname << "." << endl;
	thread_t* newthread = new thread_t(Thread(), subcontext);
	subcontext->setThread(newthread);
	cerr << "         starting thread of " << procname << "." << endl;
	newthread->run();

	cerr << "PARALLEL execution of procedure " << procname << ": done" << endl;
	mutex.unlock();		// ]
	return 0;
}

/*
 * packet_description_t manipulation/access functions
 */
void action_context::pkt_add(packet_description_t& pktdesc) throw(errno_exception<EEXIST>) {
/*	for (packet_types_t::iterator pkttype =packet_types.begin();
                                      pkttype!=packet_types.end();
				    ++pkttype) {
		if ( pkttype->Name() == pktdesc.Name() ) {	// delete old definition
			//cerr << "warning: replacing existing definition: " << pkttype->Name() << ": " << &(*pkttype) << endl;
			throw(errno_exception<EEXIST>("definition for packet type "+pkttype->Name()+" exists"));
			//packet_types.erase(pkttype);
		}
	}*/
	if ( pkt_isdef(pktdesc.Name()) ) {
			throw(errno_exception<EEXIST>("definition for packet type "+pktdesc.Name()+" exists"));
	}
	mutex.lock();		// [
	packet_types.push_back(pktdesc);
	mutex.unlock();		// ]
	return;
}

bool action_context::pkt_isdef(std::string name) throw() {
	mutex.lock();		// [
	for (packet_types_t::iterator pkttype =packet_types.begin();
                                      pkttype!=packet_types.end();
				    ++pkttype) {
		if ( pkttype->Name() == name ) { 
			mutex.unlock();		// ]
			return true;
		}
	}
	mutex.unlock();		// ]
	return false;
}


void action_context::pkt_parse(std::string aboname, std::string data, packet_type_t type) throw(errno_exception<EPROTO>, errno_exception<EBADMSG>) {

	//cerr << Name() << ": action_context::pkt_parse(" << aboname << ", " << data << ", " << type << ")" << endl;
	
	// try parsing in subcontexts first
	// child context is registered as parallel context
	bool parsed_in_subcontext = false;
	for (parallel_context_vec_t::iterator parcontext=parallel_context.begin(); parcontext!=parallel_context.end(); ++parcontext) {
		try {
			//cerr << "asking parallel context "<< (*parcontext)->Name() << " @" << (*parcontext) << " to parse packet..." << endl;
			(*parcontext)->pkt_parse(aboname, data, type);
			parsed_in_subcontext = true;
			//cerr << "       parallel context "<< (*parcontext)->Name() << " parsed packet." << endl;
		}
		catch (errno_exception<EPROTO> &e)  { /* ignore */ }
		catch (errno_exception<EBADMSG> &e) { /* ignore */ }
	}

	mutex.lock();		// [
	//cerr << "looking for\n -> abo " << aboname << endl;
	subscriptions_range_t subscriptions_range = subscriptions.equal_range(aboname);	// find protocols (packet_descriptions) registered for this abo
	if (subscriptions_range.first == subscriptions_range.second) {
		mutex.unlock();		// ]
//		if ( parent != NULL ) {		// FIXME: reverse logic?!
//			parent->pkt_parse(aboname, data, type);
//			return;
//		}
		//cerr << Name() << ": action_context::pkt_parse() has no registered protos" << endl;
		if (parsed_in_subcontext) return; else throw errno_exception<EPROTO>("no registered protocols for abo " + aboname);
	}
	bool success=false;
	for ( subscriptions_t::iterator subscription  = subscriptions_range.first;
                                        subscription != subscriptions_range.second;
                                      ++subscription) { // for each protocol
		//packettypename_t packettypename = subscription->second;
		//cerr << " -> type " << packettypename << endl;
		//packets_t::iterator pkt;
		//if ( (pkt = packets.find(packettypename)) == packets.end() ) {		// find packet description for this protocol
		//	throw errno_exception<EPROTO>("could not find packet description for protocol "+packettypename );
		//}

		//cerr << " --> found! trying... " << endl;
		try {
			if ( subscription->second.read_packet(data.c_str()) < 0 ) {		// try read with packet description
				throw errno_exception<EBADMSG>("packet does not fit this type"); // workaround! read_packet should throw() this! FIXME
			}
			success=true;							// successfull
			//return; do not return, packet could fit multiple defs!
		}
		catch (errno_exception<EBADMSG> &e) {					 // ignore non parsing packets
			//cerr << "                failed: " << e.what() << endl;
		}
	}
	mutex.unlock();		// ]
	if (success) return;
	throw errno_exception<EBADMSG>("action_context: parsing of packet " + aboname + " failed!");
}

void action_context::pkt_list() throw() {
	mutex.lock();		// [
	std::cout << "Defined packet types:" << endl;
	for (packet_types_t::iterator pkt=packet_types.begin(); pkt!=packet_types.end(); ++pkt) {
		std::cout << pkt->Name() << ": " << &(*pkt) << endl;
	}
	mutex.unlock();		// ]
	return;
}

void action_context::push_yyin(std::istream* new_yyin) throw() {
	yyin_stack.push(is);
	is = new_yyin;
}

int action_context::pop_yyin() throw() {
	if ( yyin_stack.size() == 0 ) return 0;	// tell yyinput there is nothing more to read or
	is = yyin_stack.top();		// switch back to old input stream
	yyin_stack.pop();			// remove entry from stack
	return 1;				// and tell yyinput that we have more to read
}

/**
 * \brief lookup the sensation of a field name (obsolete/deprecated)
 *
 * This function just reads packet descriptions and returns first match of any
 * protocol on any abo! This only works if there are no duplicate field names,
 * none at all! What you probably want is a fqlookup (see below).
 */
sensation_t* action_context::lookup(std::string name) throw(errno_exception<ENOMSG>) {

	mutex.lock();		// [
	for (packet_types_t::iterator pkt=packet_types.begin(); pkt!=packet_types.end(); ++pkt) {
		try {
			sensation_t* ret = pkt->lookup(name);
			mutex.unlock();		// ]
			return ret;
		}
		catch (errno_exception<ENOMSG> &e) {
			/* ignore */
		}
	}
	if (parent != NULL) {
		mutex.unlock();		// ]
		return parent->lookup(name);
	}
	mutex.unlock();		// ]
	throw errno_exception<ENOMSG>("no such value in this context");
}

/** \brief lookup the sensation of a field name
 *
 * This function just reads the packet description for the given protocol on
 * the given abo and returns first sensation with matching field name
 */
sensation_t* action_context::fqlookup(std::string abo, std::string proto, std::string fieldname) throw(errno_exception<ENOMSG>) {
	// first search this context
	//find abos with name <abo> in this context
	mutex.lock();		// [
	subscriptions_range_t subscriptions_range = subscriptions.equal_range(abo);
	for (subscriptions_t::iterator subscription =subscriptions_range.first;
				       subscription!=subscriptions_range.second;
				     ++subscription) {
		if ( subscription->second.Name() == proto ) { // find protocols with name <proto> for abo
			try {
				sensation_t* ret = subscription->second.lookup(fieldname); // find field with name <fieldname> in packet description
				mutex.unlock();		// ]
				return ret;
			}
			catch (errno_exception<ENOMSG> &e) { /* ignore */ }
		}
	}
	// not found, ask parent context or die
	if (parent != NULL) {
		mutex.unlock();		// ]
		return parent->fqlookup(abo, proto, fieldname);
	}
	mutex.unlock();		// ]
	throw errno_exception<ENOMSG>("no such value in this context");
}

void action_context::waitfor(aboname_t abo, std::string proto) throw(errno_exception<ENOPROTOOPT>) {
	mutex.lock();		// [
	// first search this context
	//find abos with name <abo> in this context
	subscriptions_range_t subscriptions_range = subscriptions.equal_range(abo);
	for (subscriptions_t::iterator subscription =subscriptions_range.first;
				       subscription!=subscriptions_range.second;
				     ++subscription) {
		if ( subscription->second.Name() == proto ) { // find protocols with name <proto> for abo
			mutex.unlock();		// ] do not lock this context while waiting!
			while ( ! (subscription->second.hasData() || wantabort()) ) {
				usleep(waitfor_usec);
			}
			return;
		}
	}
	// not found, ask parent context or die
	if (parent != NULL) {
		mutex.unlock();		// ]
		return parent->waitfor(abo, proto);
	}
	mutex.unlock();		// ]
	throw errno_exception<ENOPROTOOPT>("no such subscription in this context");
	
}

void action_context::abo_list(aboname_t aboname) throw() {
	mutex.lock();		// [
	subscriptions_range_t range;
	if ( aboname != "all" ) {
		range = subscriptions.equal_range(aboname);
	} else {
		range.first = subscriptions.begin();
		range.second = subscriptions.end();
	}
	std::cout << "Abo packet type definitions (" << aboname << "):" << endl;
	for (subscriptions_t::iterator subscription=range.first; subscription!=range.second; ++subscription) {
		std::cout << subscription->first << "\tspeaks " << subscription->second.Name() << ": " << &(subscription->second) << endl;
	}
	mutex.unlock();		// ]
	return;
}

void action_context::subscribe(std::string pkt_type_name, std::string aboname) throw(errno_exception<EFAULT>,errno_exception<EEXIST>) {
	mutex.lock();		// [
	if (scptr == NULL) { // maybe better throw one of EAGAIN, ENOTCONN, ECHILD
		mutex.unlock();		// ]
		throw errno_exception<EFAULT>("action_context::subscribe(): No subclient?! scptr==NULL!");
	}

	// find packet_description
	packet_types_t::iterator pkttype = find(packet_types.begin(), packet_types.end(), pkt_type_name);
	//cerr << "subscribe(): found proto " << pkttype->Name() << " for type " << pkt_type_name << endl;
	//cerr << "subscribe(): abo " << aboname << " <=> proto " << pkttype->Name() << endl;
	if (pkttype == packet_types.end()) {
		mutex.unlock();		// ]
		throw errno_exception<EFAULT>("action_context::subscribe(): No such packet_type!");
	}

	// check if this is already subscribed
	subscriptions_range_t subs = subscriptions.equal_range(aboname);
	for (subscriptions_t::iterator sub=subs.first; sub!=subs.second; ++sub) {
		if (sub->second == pkt_type_name) {
			mutex.unlock();		// ]
			throw(errno_exception<EEXIST>("already subscribed "+pkt_type_name+" on "+aboname));
		}
	}
	subscriptions.insert(subscription_t(aboname, (*pkttype)));		// register packet_type to abo
	scptr->subscribe(aboname.c_str());			// subscribe abo from server
	mutex.unlock();		// ]
	return;
}

void action_context::unsubscribe(aboname_t aboname) throw(errno_exception<EFAULT>) {
	mutex.lock();		// [
	if (scptr == NULL) {
		mutex.unlock();		// ]
		throw(errno_exception<EFAULT>("scptr==NULL"));
	}
	scptr->unsubscribe(aboname.c_str()); 
	subscriptions_range_t subs = subscriptions.equal_range(aboname);
	subscriptions.erase(subs.first, subs.second);
	mutex.unlock();		// ]
}

int action_context::send(aboname_t aboname, std::string text, packet_type_t type) throw() {
	packet_t packet;
	packet.type = type;
	packet.setName(aboname.c_str());
	packet.setData(text.c_str(), text.size());
	mutex.lock();		// [
	if (scptr == NULL) {
		cerr << "scptr==NULL!" << endl;
		mutex.unlock();		// ]
		return -1;
	}
	//int ret = scptr->aprintf(aboname.c_str(), text.c_str());
	int ret = scptr->sendpacket(packet);
	mutex.unlock();		// ]
	try {
		RootParent()->pkt_parse(aboname, text, type);
	}
	catch (errno_exception<EPROTO> &e)  { /* ignore */ }
	catch (errno_exception<EBADMSG> &e) { /* ignore */ }

	return ret;
}
