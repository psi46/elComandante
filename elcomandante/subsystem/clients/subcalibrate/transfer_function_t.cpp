/**
 * transfer_function_t.cpp
 *
 * Dennis Terhrost <terhorst@physik.rwth-aachen.de>
 */
#include "transfer_function_t.h"
#include <stdlib.h>

/////////////////////////////////////////////////////////////////
// class transfer_function_t
//
transfer_function_t::transfer_function_t(char* Name, char* Raw_name, int Raw_arg) {
	strncpy(name, Name, MAX_NAMELEN);
	strncpy(raw_name, Raw_name, MAX_NAMELEN);
	raw_arg = Raw_arg;
	initparm();
}

void transfer_function_t::setParameter(int p, double val) {
	if (p<0 || p>MAX_CALIB_PARMS) return;
	parm[p] = val;
	return;
}
double transfer_function_t::getParameter(int p) {
	if (p<0 || p>MAX_CALIB_PARMS) return 0.0;
	return parm[p];
}
					
void transfer_function_t::initparm() {
	bzero(parm, sizeof(parm));
	return;
}
double transfer_function_t::calibration(char* raw) {
	return (strtod(raw,NULL));
}
double transfer_function_t::uncalibration(char* raw) {
	return (strtod(raw,NULL));
}
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
// Linear Transfer Function
//
linear_cal::linear_cal(char* Name, char* Raw_name, int Raw_arg)
	: transfer_function_t(Name,Raw_name,Raw_arg)
{
	return;
}

void linear_cal::initparm() {
	bzero(parm, sizeof(parm));
	parm[1]=1.0;
	return;
}
double linear_cal::calibration(char* raw) {
	return parm[0]+parm[1]*strtod(raw,NULL);
}
double linear_cal::uncalibration(char* raw) {
	if (parm[1] == 0.0) return 0.0;
	return (strtod(raw,NULL)-parm[0])/parm[1];
}
/////////////////////////////////////////////////////////////////


// EOF

