/**
 * \file transfer_function_t.h
 *
 * \author Dennis Terhrost <terhorst@physik.rwth-aachen.de>
 * \date Jul 27 2008
 *
 * \brief preliminary callibration class
 */
#ifndef TRANSFER_FUNCTION_T_H
#define TRANSFER_FUNCTION_T_H

#include "../../packet_t.h"

#ifdef PACKET_T_H
#  define MAX_NAMELEN	MAX_PACKETLEN
#else
#  warn Using nonstandard MAX_NAMELEN 128
#  define MAX_NAMELEN 128
#endif

#define MAX_CALIB_PARMS	10

/////////////////////////////////////////////////////////////////
// Generic Transfer Functions (calibration)
/**
 * \brief preliminary callibration class
 *
 * This class shall be used as general calibration function parent
 * class.
 */
class transfer_function_t {
public:
	char	name[MAX_NAMELEN];		// output name
	char	raw_name[MAX_NAMELEN];	// input name
	int	raw_arg;	// input arg number
	double	parm[MAX_CALIB_PARMS];	// calibration parameters
	
	transfer_function_t(char* Name, char* Raw_name, int Raw_arg);
	
	void setParameter(int p, double val);
	double getParameter(int p);
	
	virtual void initparm();
	virtual double calibration(char* raw);
	virtual double uncalibration(char* raw);
}; // end class transfer_t
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
// Linear Transfer Function
class linear_cal : public transfer_function_t {
public:
	linear_cal(char* Name, char* Raw_name, int Raw_arg);
	virtual void initparm();
	virtual double calibration(char* raw);
	virtual double uncalibration(char* raw);
};
/////////////////////////////////////////////////////////////////

#endif // ndef TRANSFER_FUNCTION_T_H
