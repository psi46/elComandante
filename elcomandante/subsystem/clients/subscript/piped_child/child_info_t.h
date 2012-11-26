#ifndef CHILD_INFO_T_H
#define CHILD_INFO_T_H

#include "child_t.h"

// mask whence to resume (child_t::restart)
#define CLD_RESET_MASK_EXITED	0x01
#define CLD_RESET_MASK_KILLED	0x02
#define CLD_RESET_MASK_ANY	0x03

#ifndef MAX_NAMELEN
#warning MAX_NAMELEN redefinition must comply with abo.h
#define MAX_NAMELEN 256
#endif
#ifndef MAX_IDLEN
#warning MAX_IDLEN redefinition must comply with clientinfo.h
#define MAX_IDLEN 256
#endif

// processes stdio redirections
#define DEFAULT_IN_PREF "/process"
#define DEFAULT_OUT_PREF "/process"
#define DEFAULT_ERR_PREF "/process"
#define DEFAULT_IN_SUFF "stdin"
#define DEFAULT_OUT_SUFF "stdout"
#define DEFAULT_ERR_SUFF "stderr"

class child_info_t {
// PRIVATE DATA MEMBERS
private:
	child_t* mychild;
	char	inabo[MAX_NAMELEN];  // MAX_NAMELEN defined in sclient.h <- abo.h
	char	outabo[MAX_NAMELEN];
	char	errabo[MAX_NAMELEN];
	char	clientid[MAX_IDLEN]; // MAX_NAMELEN defined in sclient.h <- clientinfo.h

// PUBLIC FUNCTIONS
public:
	child_info_t(int argc, char* argv[], char* ClientID=NULL);
	virtual ~child_info_t();

	child_t *Child();

	int setClientID(char* to);
	int setInabo(char* to);
	int setOutabo(char* to);
	int setErrabo(char* to);

	const char* ClientID() const;
	const char* Inabo() const;
	const char* Outabo() const;
	const char* Errabo() const;

	void PrintInfo();
};


#endif //ndef CHILD_INFO_T_H
