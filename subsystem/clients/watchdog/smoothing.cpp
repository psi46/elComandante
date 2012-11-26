/**
 *	\file smoothing.cpp
 *	\author Dennis Terhorst <terhorst@physik.rwth-aachen.de>
 *
 *	This class roughly averages the input and calculates
 *	deviations
 */
#include "smoothing.h"
#include <math.h>

smooth_t::smooth_t(float Smoothing) {
	smoothing = Smoothing;
	average = deviation = 0;
	values=0;
}

smooth_t::~smooth_t() { }

smooth_t smooth_t::operator=(double val) {
	lastval = val;
	average = (smoothing*average + (double)val) / (smoothing+1);
	deviation = ( smoothing*deviation + fabs((double)val-average) )/( smoothing+1 );
	values++;
	return *this;
}

smooth_t::operator double() { return lastval; }
bool smooth_t::isvalid()	 { return ( values > smoothing); }
double smooth_t::avg()	 { return average; }
double smooth_t::dev()	 { return deviation; }

