#include "ratewatch.h"

// private function
double ratewatch_t::timeval_diff(struct timeval *t0, struct timeval *t1) {
	double tolast = (t0->tv_usec - t1->tv_usec)/1e6;
	tolast += difftime(t0->tv_sec, t1->tv_sec);
	return tolast;
}

// constructor
ratewatch_t::ratewatch_t(unsigned int smoothing) {
	RATE_SMOOTHING = smoothing;
	interval = new smooth_t(RATE_SMOOTHING);
	gettimeofday(&lastbeat, NULL);
}

// destructor
ratewatch_t::~ratewatch_t() {
	delete interval;
}

double ratewatch_t::beat() {
	struct timeval now;
	double lastint;
	gettimeofday(&now, NULL);
	*interval = ( lastint=timeval_diff(&now, &lastbeat) );
	lastbeat = now;
	return lastint;
}

bool ratewatch_t::isLate(double sigmas) {
	struct timeval now;
	gettimeofday(&now, NULL);
	
	return (timeval_diff(&now, &lastbeat) > interval->avg()+(sigmas*interval->dev()) );
}

bool ratewatch_t::isEarly(double sigmas) {
	struct timeval now;
	gettimeofday(&now, NULL);
	
	return (timeval_diff(&now, &lastbeat) < 1.0/(rate_avg()+(sigmas*rate_dev())) );
}

bool   ratewatch_t::isValid()      { return interval->isvalid(); }
double ratewatch_t::interval_avg() { return interval->avg(); }
double ratewatch_t::interval_dev() { return interval->dev(); }
double ratewatch_t::rate_avg()    { return 1.0/interval->avg(); }
double ratewatch_t::rate_dev()    { return 1.0/(interval->avg()*interval->avg())*interval->dev(); }


