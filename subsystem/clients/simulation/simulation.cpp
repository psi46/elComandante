
		if ( pressure.changed()|| temperature.changed()|| cffour.changed()|| isobut.changed()|| efield.changed() ) {
			// recalc here
			printf("recalculating...\n");
			me.printf("0.0");
			printf("bla1\n");
			// magboltz input
			int scatters = 10;	///< number of scatters times 10**7
			double tempC = 20.0;	///< Temperature centigrate
			double bfield= 0.2;	///< Magnetic field (kilogauss)
			double theta_E_B=0.0;	///< angle between E and B
			printf("bla2\n");
			double pressure_mmHg = 760.0*(double)pressure/1013.0;
			// PARAMETERS
			printf("bla3\n");
			printf("%10i%10i%10.5f\n", 3, scatters, 0.0);
			printf("bla4\n");
			// GASES
			printf("%5i%5i%5i%5i%5i%5i\n",2,1,11,77,77,77);	// Ar, CF4, iC4H10, n/a, n/a, n/a);
			// FRACTIONS, TEMPERATURE, PRESSURE
			printf("%10.4f%10.4f%10.4f%10.4f%10.4f%10.4f%10.4f%10.4f\n",
				100.0-(double)cffour-(double)isobut,
				double(cffour),double(isobut),
				0.0,0.0,0.0,
				double(tempC),double(pressure_mmHg));
			// EL-MAG FIELD
			printf("%10.3f%10.3f%10.3f\n",(double)efield,(double)bfield,(double)theta_E_B);
			// DONE
			printf("%1i\n",0);

		}
