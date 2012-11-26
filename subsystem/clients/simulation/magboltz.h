/**
 * \file magboltz.h
 *
 * wrapper class for magboltz child process
 *
 * \author Dennis Terhorst <terhorst@physik.rwth-aachen.de>
 * \date Thu Aug 28 12:15:08 CEST 2008
 */
#ifndef MAGBOLTZ_H
#define MAGBOLTZ_H

#define MAG_RUNNING	1
#define MAG_FORKED	2

struct magboltz_parameter_t {
//---------------------------------------------------------------
// INPUT CARDS :
//----------------------------------------------------------
/**
 * \name FIRST CARD: 2I10,F10.5  :  NGAS,NMAX,EFINAL
 */
//@{
	///  NGAS:  NUMBER OF GASES IN MIXTURE
	int	ngas;
	///  NMAX: NUMBER OF REAL COLLISIONS ( MULTIPLE OF 1*10**7 )
	///  USE NMAX = BETWEEN 2 AND 5 FOR INELASTIC GAS TO OBTAIN 1% ACCURACY
	///      NMAX = ABOVE 10 FOR BETTER THAN 0.5% ACCURACY.
	///      NMAX = AT LEAST 10 FOR PURE ELASTIC GASES LIKE ARGON
	///      HIGHER VALUES THAN NMAX=214 CAN ONLY BE USED ON COMPUTERS SUCH
	///      AS DEC ALPHAS WITH TRUE 64 BIT INTEGERS. PCS ARE LIMITED TO
	///      31 BIT INTEGERS...
	int	nmax;
	///    EFINAL = UPPER LIMIT OF THE ELECTRON ENERGY IN ELECTRON VOLTS.
	///    EFINAL = 0.0 (PROGRAM AUTOMATICALLY CALCULATES UPPER INTEGRATION
	///                 ENERGY LIMIT)
	float	efinal;
//@}
//-------------------------------------------------------------
/**
 * \name SECOND CARD : 6I5   : NGAS1 , NGAS2, NGAS3 , NGAS4 , NGAS5 , NGAS6
 */
//@{
	///       NGAS1,ETC :  GAS NUMBER IDENTIFIERS (BETWEEN 1 AND 80)
	///                   SEE GAS LIST BELOW FOR IDENTIFYING NUMBERS.
	int gas[6];
//@}
//
//-------------------------------------------------------------
/**
 * \name THIRD CARD: 8F10.4  : FRAC1,FRAC2,FRAC3,FRAC4,FRAC5,FRAC6,TEMP,TORR
 */
//@{
	///  FRAC1,ETC : PERCENTAGE FRACTION OF GAS1,ETC
	double fraction[6];
	///  TEMP : TEMPERATURE OF GAS IN CENTIGRADE
	double temp;
	///  TORR :  PRESSURE OF GAS IN TORR
	double torr;
//@}
// ------------------------------------------------------------
/**
 * \name FOURTH CARD : 6F10.3  : EFIELD,BMAG,BTHETA
 */
//@{
	///  EFIELD : ELECTRIC FIELD IN VOLTS/ CM.
	double	efield;
	///   BMAG  : MAGNITUDE OF THE MAGNETIC FIELD IN KILOGAUSS
	double	bmag;
	///  BTHETA : ANGLE BETWEEN THE ELECTRIC AND MAGNETIC FIELDS IN DEGREES.
	double	btheta;
//@}
//-----------------------------------------------------------------------
// CARD 4*N+1 USES NGAS=0 TO TERMINATE CORRECTLY
//-----------------------------------------------------------------------
};

class magboltz {
private:
	pthread_t	magboltz_thread;
	int	status;

	magboltz() {
		
	}
	void run(magboltz_parameter_t parm) {
		// create thread
		//
			printf("%10i%10i%10.5f\n", 3, scatters, 0.0);
			printf("%5i%5i%5i%5i%5i%5i\n",2,1,11,77,77,77);	// Ar, CF4, iC4H10, n/a, n/a, n/a);
			printf("%10.4f%10.4f%10.4f%10.4f%10.4f%10.4f%10.4f%10.4f\n",
				100.0-(double)cffour-(double)isobut,
				double(cffour),double(isobut),
				0.0,0.0,0.0,
				double(tempC),double(pressure_mmHg));
			printf("%10.3f%10.3f%10.3f\n",(double)efield,(double)bfield,(double)theta_E_B);
			printf("%1i\n",0);
	}
};

#endif //ndef MAGBOLTZ_H
