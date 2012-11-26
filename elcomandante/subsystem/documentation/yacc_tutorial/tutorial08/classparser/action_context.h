/**
 * \file action_context.h
 * \author Dennis Terhorst
 * \date Mon Oct  5 17:15:14 CEST 2009
 */
#ifndef ACTION_CONTEXT
#define ACTION_CONTEXT

#include <iostream>
#include <sstream>
#include <string>
#include <map>	// map, multimap
#include <vector>
#include <stack>
#include "packet_description_t.h"	// this has to be included before sclient! because of different include of packet_t.h
#include <subsystem/selectable_sclient.h>
#include "sensations.h"
#include "sens_value_t.h"	// only for temporary list
#include "exceptions.h"
#include "thread_t.h"		// thread_t and mutex_t

//#include "value_t.h"
//#include "action.tab.h"
//#include "action.lex.h"		// definitions of flex

/** \brief parser and scanner context ('scope' in terms of subscript)
 *
 * This class is supposed to be run with the thread_t::run() function for parallel execution of scopes.
 */
class action_context : public thread_context_t
{
	mutex_t	mutex;
public:
	// maybe this is dirty, but where to keep senslist better?
	sens_list_t senslist;			///< temporary list of value senses for building packet description with parser. NOT FOR PUBLIC ACCESS!
	static const int waitfor_usec;		///< interval at which waitfor() checks for packet arrivals

private:
	/**
	 * \name Variables (deprecated)
	 */
	///@{
		typedef std::map<std::string, std::string> variables_t;
		variables_t variables;			// variables (deprecated)
	///@}

	/**
	 * \name Procedures
	 */
	///@{
		typedef std::string  proc_name_t;
		typedef std::string  proc_t;
		typedef std::map<proc_name_t, proc_t> procedures_t;
		procedures_t procedures;		// defined procedures
		  typedef std::pair<procedures_t::iterator, procedures_t::iterator> procedures_range_t;
	///@}

	/**
	 * \name Subscriptions
	 */
	///@{
		typedef std::string aboname_t;
		typedef std::multimap<aboname_t, packet_description_t> subscriptions_t;
		subscriptions_t subscriptions;			// aboname  1 : n  packet_description_t
		typedef std::pair<aboname_t, packet_description_t> subscription_t;
		typedef std::pair<subscriptions_t::iterator, subscriptions_t::iterator> subscriptions_range_t;
	///@}

	/**
	 * \name Packet Types
	 */
	///@{
		typedef std::vector<packet_description_t> packet_types_t;
		packet_types_t packet_types;		// packet_description
	///@}

	/* *****************************************************************************
	 * Context Properties
	 */
	std::string name;				// context name

	/** \name Context Management
	 *
	 * These private members help to keep track of all action_context
	 * objects and their relations. After execution of a context (i.e.
	 * return from run() call) thread_t::thread() is supposed to delete the context
	 * and it's parents upto the first parent referenced by at least one
	 * other thread (or parent==NULL).
	 */
	///@{

		/**
		 * \brief  list of child contexts
		 *
		 * governed by con- and destructor (i.e. no 'new' and 'delete' for these pointers!)
		 */
		typedef std::vector<action_context*> parallel_context_vec_t;
		parallel_context_vec_t parallel_context;

		action_context* child;		///< child context pointer
		action_context* parent;		///< parent context pointer
		thread_t* mythread;
		int refcount;			///< number of contexts referencing this as parent
	///@} end context management
	
	int wantexit;			///< exit is requested if set >0
	std::stack<std::istream*> yyin_stack;	///< parser input stack (eg. for 'load' command)
//	std::stack<YY_BUFFER_STATE> lexer_stack;
public:
	sclient_selectable* scptr;	///< sclient connection pointer FIXME: this should be private!
	void* scanner;			///< bison parser state
	int esc_depth;			///< flex tokenizer escape depth counter
	int result;			///< context "return" value

private:
	std::istream* is;		///< stream pointer to read input from (public for flex)
	std::stringstream os;		///< stringstream storing executed stuff
	friend int yy_get_next_buffer(void*);	///< flex input function (see action.l) needs access to \c is and \c os

/*
 *                 P U B L I C   F U N C T I O N S
 */
public:
	action_context(action_context* ParentContext=NULL, std::istream* is = &std::cin) throw();

	virtual ~action_context() throw(errno_exception<EBUSY>);


	/**
	 * \name Context Information and Manipulation
	 */
	///@{
		void   setName(std::string newname) throw();
		std::string Name() const throw();

		void showPosition(unsigned int context_lines=3) throw();

		/// get follow parent pointers to the top context
		action_context* RootParent() throw();

		/// show execution tree of all threads
		void showTree(std::string pretext="") throw();

		/// get the thread responsible for this context
		thread_t* Thread() const  throw(errno_exception<EFAULT>);
		
 		/// set the contexts managing thread
		void setThread(thread_t* thr);

		/// check if this is an interactive context
		bool isInteractive() const throw();

		int RefCount() const;
		action_context* Parent() const;

		/// load the given file into the current context
		void loadFile(std::string filename) throw(errno_exception<EBADFD>);

		/// execute the given text next in this context
		void exec(std::string text) throw();

		/// execute string in a subcontext (serial call to action_parse)
		int    run_subcontext(std::string text, std::string name="unnamed") throw();

		/// execute this context (main loop for this context calling action_parse, blocking)
		int    run() throw();

		/// parallel execute this context (create thread calling action_context::run() of this context)
		int    pcall(const std::string procname) throw(errno_exception<EEXIST>, errno_exception<EINVAL>);

		/// abort execution of this context
		void abort() throw();

		/// request the abort state of this context
		bool wantabort() throw();
	///@}	end group Context Manipulation

	/**
	 * \name Variable manipulation/access functions (deprecated)
	 */
	///@{
		std::string var_set(std::string name, std::string value) throw();
		std::string var_get(std::string name) throw();
		bool   var_isset(std::string name) throw();
		void   var_list() throw();
		void   var_show() throw();
	///@}

	/**
	 * \name Procedure definition/manipulation functions
	 */
	///@{
		void   proc_add(proc_name_t name, proc_t value) throw(errno_exception<EEXIST>);
		bool   proc_isset(proc_name_t name) throw();
		void   proc_list() throw();
		void   proc_show(proc_name_t name = "all") throw();
		proc_t proc_get(proc_name_t name) throw(errno_exception<EINVAL>);
		int    proc_run(proc_name_t name) throw();
	///@}

	/**
	 * \name packet_description_t manipulation/access functions
	 */
	///@{
		void pkt_add(packet_description_t& pktdesc) throw(errno_exception<EEXIST>);
		bool pkt_isdef(std::string name) throw();
		void pkt_parse(std::string name, std::string data, packet_type_t type=PKT_DATA) throw(errno_exception<EPROTO>, errno_exception<EBADMSG>);
		void pkt_list() throw();
	///@}

	/**
	 * \name input stream stack
	 * THESE FUNCTIONS ARE NOT IN USE/WORKING RIGHT NOW
	 */
	///@{
		void push_yyin(std::istream* new_yyin) throw();
		int pop_yyin() throw();
	///@}

	/**
	 * \name Operations in This Context
	 */
	///@{
		sensation_t* lookup(std::string name) throw(errno_exception<ENOMSG>);
		/// fully qualified lookup
		sensation_t* fqlookup(aboname_t abo, std::string proto, std::string fieldname) throw(errno_exception<ENOMSG>);

		/// wait for a packet to be received
		void waitfor(aboname_t abo, std::string proto) throw(errno_exception<ENOPROTOOPT>);

		void abo_list(aboname_t aboname="all") throw();
		/// subscribe a packets of type \c pkt_type_name on an abo \c aboname
		void subscribe(std::string pkt_type_name, aboname_t aboname) throw(errno_exception<EFAULT>,errno_exception<EEXIST>);
		/// unsubscribe a packets of abo \c aboname
		void unsubscribe(aboname_t aboname) throw(errno_exception<EFAULT>);

		/// send a text message to this contexts subserver
		int send(aboname_t aboname, std::string text, packet_type_t type = PKT_DATA) throw();
		
	///@} end Operations in This Context
	
private:
	/** \name Scanner Initialisation and Cleanup
	 *  Defined in action.l
	 */
	///@{
	void init_scanner() throw();
	void destroy_scanner() throw();
	///@} end Scanner Maintanance
};

// unfortunately the bison parse function does not throw() exceptions...
int action_parse(action_context*) /* no throw() here! */;

#endif // ACTION_CONTEXT
