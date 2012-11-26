#include <stdio.h>
#include <unistd.h>
#include "ratewatch.h"

int main(void) {

	ratewatch_t deltaT;

	int beats=0;
	int earlys=0;
	int lates=0;

	//while (getchar() == '\n') {
	while ( true ) {
		usleep(1000000);
		bool late=deltaT.isLate(4);
		bool early=deltaT.isEarly(4);
		bool valid=deltaT.isValid();
		beats++;
		if (early && valid) earlys++;
		if (late && valid) lates++;
		printf("%ld: %s %s%s, lastint=%lf, deltaT=(%lf+-%lf)sec == rate=(%lf+-%lf)Hz, N=%d, Late=%d, %f%%, Early=%d, %f%%\n",
			time(NULL),
			valid?"valid":"INVAL",
			late?"LA":"--",
			early?"ER":"--",
			deltaT.beat(),
			deltaT.interval_avg(),
			deltaT.interval_dev(),
			deltaT.rate_avg(),
			deltaT.rate_dev(),
			beats,
			lates,
			(float)lates/(float)beats,
			earlys,
			(float)earlys/(float)beats
			);
		fflush(stdout);
	}
	

	return 0;
}
