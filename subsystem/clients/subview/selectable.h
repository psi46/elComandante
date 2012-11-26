/**
 *	selectable.h	09-Oct-2007
 *	Dennis Terhorst <dennis.terhorst@rwth-aachen.de>
 */
#ifndef SELECTABLE_H
#define SELECTABLE_H

#include <sys/select.h>


#define MAX_SELECTABLE	128

/**
 * \defgroup CHK_MODES Filedescriptor Check Modes
 * These constants should be returned by the selectable::getchecks() function
 * which is overloaded in child classes.
 */
//@{
#define CHK_READ	1
#define CHK_WRITE	2
#define CHK_RDWR	3
#define CHK_EXCEPT	4
//@}


/**
 * \brief management of multiplexed i/o
 *
 * All classes derived from selectable will be checked for in multiplexed i/o
 * \c select() call. Derived classes must overload gechecks() function to provide
 * the mode for which the file descriptor should be monitored (CHK_READ, CHK_WRITE
 * or CHK_EXCEPT). The overloaded getfd() function must return the file descriptor
 * which should be monitored.
 *
 * The actual \c select() system call is done with selectable::run() after preparing
 * the fd_sets for the instanciated selecectable objects (e.g. input, serial, ...).
 */
class selectable {
private:
	/// list of all selectables instanciated
	static selectable* sel[MAX_SELECTABLE];
	/// _n_umber _o_f instanciated _sel_ectables;
	static int nosel;

	/// index of sel[] of this instance
	int thissel;
	unsigned char ready;

	// these should be overloaded
	virtual int  getfd()=0; //	{ return -1; }		///< return fd for the fd_set
	virtual int  getchecks()=0; //{ return  0; }		///< select which fd_sets the fd will be added to

	// set default handlers doing nothing
	virtual void fdr_call()	{ return;    }		///< read fd ready stub
	virtual void fdw_call()	{ return;    }		///< write fd ready stub
	virtual void fde_call()	{ return;    }		///< fd exception ready stub



public:
	selectable();

	virtual ~selectable();

	static int run();

	static int run(long sec, long usec);

	int isready(unsigned char type) const;
};

#endif
