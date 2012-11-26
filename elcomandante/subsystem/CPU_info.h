/**
 *	CPU_info class for getting ifnos about the CPU and the state of the System
 *	Jochen Steinmann <jochen.steinmann@rwth-aachen.de>
 */
#ifndef GET_CPU_INFO_H
#define GET_CPU_INFO_H

#include <stdio.h>

class CPU_info {
	private:
		/// System loads
		static long user, system, nice, idle;
		/// Load average
		static float load;	

		static FILE *fp_stat;
		static FILE *fp_loadavg;

	public:		
		CPU_info();
		~CPU_info();
		static int getStats();
		static float actual();
		static float total();
		static float average();
		static char* getIP();
		static char* getHostname();
};
#endif //ndef GET_CPU_INFO_H

