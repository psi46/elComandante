#include <time.h>
#include <sys/time.h>
#include "smoothing.h"
#define DEFAULT_RATE_SMOOTHING	20

class ratewatch_t {
private:
	struct timeval lastbeat;
	smooth_t* interval;
	int RATE_SMOOTHING;

	// return seconds t0-t1
	double timeval_diff(struct timeval *t0, struct timeval *t1);
public:
	ratewatch_t(unsigned int smoothing = DEFAULT_RATE_SMOOTHING);
	virtual ~ratewatch_t();

	// registers a new clockbeat
	// returns last interval (sec)
	double beat();

	// returns true, if current time is past lastbeat
	// plus interval() plus <sigmas> number of
	// interval_sigma();
	bool isLate(double sigmas);
	bool isEarly(double sigmas);
	bool isValid();

	// returns average time interval between beats
	// in seconds, and a kind of standard deviation.
	double interval_avg();
	double interval_dev();

	// returns 1/interval  [Hz]
	double rate_avg();
	double rate_dev();
};

