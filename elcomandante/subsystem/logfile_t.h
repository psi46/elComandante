/*
 * \file logfile_t.h
 * \author Dennis Terhorst
 * \date Sun Jul 27 2008
 */
#ifndef LOG_H
#define LOG_H

//#include "packet_t.h"
//#define MAX_LOG_NAMELEN	MAX_PACKETLEN
#define MAX_LOG_NAMELEN	2048

/**
 * \brief path prefix for all logfiles
 *
 * Given a logfile name as construtor argument, this prefix is prepended
 * to construct the full logfile name. Be aware that giving a relative
 * path might not be what you want, as other functions like daemonize_me()
 * change the working directory of the process.
 *
 * <B>NOTE:</B> Give path to logs without trailing slash!
 */
#ifndef DEFAULT_LOGDIR
	#define DEFAULT_LOGDIR	"~/DATA" 
	#warning "USE default logdir from h file" 
#else
	#warning "USE default logdir from MAKEFILE" 
#endif


/**
 * \brief logfile rotation interval
 *
 * This variable may be set to any of <TT>struct tm</tt> field names.
 * Whenever the field changes in localtime a new file will be opened.
 *
 * Valid entries are: <TT>tm_sec, tm_min, tm_hour, tm_mday, tm_mon,
 * tm_year, tm_wday, tm_yday</tt> and <TT>tm_isdst</TT>. Where
 * all values with "day" have the same effect of reopening the logfile
 * once a day (when the day value changes, i.e. at midnight).
 *
 */
#define LOGFILE_ROTATION	tm_yday


/**
 * \brief log directory permissions
 *
 * A newly created subdirectory will be given these permissions. Be sure to include
 * S_IRWXU for this class to make sense (see <TT>man open(2)</TT>).
 */
#define LOGDIR_PERMISSIONS	S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH

/**
 * \brief log file permissions
 *
 * A newly created logfile will be given these permissions. Be sure to include
 * S_IWUSR for this class to make sense (see <TT>man open(2)</TT>).
 */
#define LOGFILE_PERMISSIONS	S_IRUSR | S_IWUSR | S_IRGRP

/**
 * \name Filename Character Replacement
 * The name of a file may only contain chars given in \c FILENAME_GOODCHARS, all
 * other chars will be replaced by \c FILENAME_BADCHARREPLACE.
 *
 * If you want to make use of the automatic subdirectory functionality be sure
 * to include '/' in \c GOODCHARS.
 */
//@{
#define FILENAME_GOODCHARS	"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-_/"
#define FILENAME_BADCHARREPLACE '_'
//@}


//
/**
 * \brief manages logfile handles
 *
 * This class stores informations about a logfile and handles file open and
 * close calls.
 *
 * Upon open of a file the needed parent directories are created as needed, if
 * the directory delimiter ('/') is defined in \ref FILENAME_GOODCHARS.
 *
 * It is possible to rotate logfiles on a specified time basis, i.e. one file
 * per day or per hour, etc. . For details about the rotation feature see the
 * wantnewfile() function.
 */
class logfile_t {
private:
	int fd;
	char name[MAX_LOG_NAMELEN];
	static char LOGDIR[MAX_LOG_NAMELEN];	// class wide log directory
	int last;	///< last time value, used for log-rotation
public:
	/**
	 * The \p Name given will be checked against FILENAME_GOODCHARS,
	 * changed if needed and the result is copied to name[]. The file
	 * is not opened until openfile() is called.
	 */
	logfile_t(const char* Name);
	logfile_t(const logfile_t& copy);

	/// close the file
	~logfile_t();

	/// check logfile status
	bool isOK();

	/// open the file
	int openfile();

	logfile_t& operator= (const logfile_t& other);

	bool operator== (const logfile_t& other) const;

	/**
	 * \brief print to the logfile
	 *
	 * Use this function like printf()
	 */
	int print(const char* format, ...);

	/**
	 * \brief path prefix for all logfiles
	 *
	 * Given a logfile name as construtor argument, this prefix is prepended
	 * to construct the full logfile name. Be aware that giving a relative
	 * path might not be what you want, as other functions like daemonize_me()
	 * change the working directory of the process.
	 *
	 * <B>NOTE:</B> Give path to logs without trailing slash!
	 */
	static int setLogdirectory(char* logdir);
	static const char* getLogdirectory();
private:
	int closefile();
	int wantnewfile();
	int reopen();
	void mktimestamp(char* buffer, int buflen);
};

#endif
