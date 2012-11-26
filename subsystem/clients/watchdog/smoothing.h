/**
 *	\file smoothing.h
 *	\author Dennis Terhorst <terhorst@physik.rwth-aachen.de>
 *
 *	This class roughly averages the input and calculates
 *	deviations. All is done in double precision.
 */
#ifndef SMOOTHING_H
#define SMOOTHING_H

class smooth_t {
private:
	double	average, deviation;
	float	smoothing;	// number of values to average
	int	values;		// number of values given
	double	lastval;	// last value given
public:

	// CONSTRUCTOR
	smooth_t(float Smoothing);
	// DESTRUCTOR
	virtual ~smooth_t();

	// OPERATORS

	// store operator
	smooth_t operator=(double val);

	// restore operator
	operator double();

	// REQUEST FUNCTIONS

	// avg/dev validity
	bool isvalid();

	// avgerage
	double avg();

	// deviation
	double dev();

};

#endif
