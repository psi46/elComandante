/*
 * \file aggregator.cpp
 *
 * This program is a small subclient which subscribes multiple abos
 * and then parses the numbers into one abo. It can be used to
 * gather information from different sources, either for overview
 * purposes, or for input reorganisation of another program.
 *
 * The code seen below is for TPC condition overview in the T2K/nd280
 * gas monitor chamber, but it can easily be modified for any other
 * purpose.
 *
 * \author Dennis Terhorst <terhorst@physik.rwth-aachen.de>
 * \date Thu Aug 28 12:54:03 CEST 2008
 */

#include <string.h>	// strcmp()
#include <subsystem/sclient.h>
#include <subsystem/daemon.h>
#include <subsystem/error.h>
#include <signal.h>
#include <unistd.h>	// alarm()
#include <time.h>

#define SCLIENT_ID	"aggregator"


#define ABO_PRESSURE	"/chamber/pressure"
#define ABO_MIXTURE	"/gassystem/mixture"
#define ABO_TEMPERATURE	"/ambient/temperature"
#define ABO_EFIELD	"/chamber/efield"

#define ABO_RESULTS	"/chamber/status"

/**
 * This class helps in collecting different numbers to a more
 * combined view.
 */
class condition_t {
public:
	double pressure;	// P	torr
	double temperature;	// T	centigrade
	double cffour;		// CF4    fraction (percent)
	double isobut;		// iC4H10 fraction (percent)
	double efield;		// E	V/cm
	double bfield;		// E	V/cm

	condition_t() {	// initial conditions
		pressure=760.;		// P	torr
		temperature=20.;	// T	centigrade
		cffour=3.;		// CF4    fraction (percent)
		isobut=2.;		// iC4H10 fraction (percent)
		efield=300.;		// E	V/cm
	}

	int snprint(char* buffer, int length) {
		return snprintf(buffer, length, "P=%.2lf torr, T=%.1lfdegC, E=%.1lfV/cm, B=%.3lfkG, Ar:CF4:iC4H10=%.1lf:%.1lf:%.1lf\n",
			pressure, temperature, efield, bfield, 100.-cffour,isobut,cffour,isobut);
	}
};

// GLOBAL CONDITION VARIABLE
condition_t gas;

/**
 * \brief output rate selection
 *
 * To regulate the rate output is generated the update mode can be
 * selected. Depending upon the input rates and needed output precision,
 * different options are available.
 *
 * \note These modes define output rates. To define the
 * 	 mode of aggregation see \ref aggregation_mode_t.
 */
//nji = not jet implemented
typedef enum {
	on_each_update,		///< each change in any field will cause a line of output
	on_each_time_interval,	///< output will be generated periodically, regardless of updates
//nji	on_each_full update,	///< output will be generated if all fields have been updated
	on_update_or_interval,	///< output will be generated if an update is available or the timer expires
	on_update_and_interval	///< output will be generated ir an update is available and at least UPDATE_INTERVAL seconds have expired
//nji	on_command		///< output will be generated if a command is send via the control channel
} update_mode_t;

//update_mode_t updatemode = on_each_update;
update_mode_t updatemode = on_update_and_interval;

/// used if on_each_time_interval or on_update_and_interval is selected (seconds)
#define UPDATE_INTERVAL		10

/**
 * \brief select the aggregators mode of operation
 *
 * To change the way output is calculated the aggregation mode can be
 * selected. Depending upon the input rates and needed output precision,
 * different options are available.
 *
 * \note These modes define output calculations. To define the
 * 	 rate of output see \ref update_mode_t.
 */
typedef enum {
	fixed_values		///< conditions are constant between updates
//nji	extrapolate1,		///< the last two conditions are used to extrapolate linearly
//nji	extrapolate3		///< the last three conditions are used to extrapolate quadratic
} aggregation_mode_t;


/// PARSE FUNCTIONS
//@{
int parse_pressure(packet_t packet) {
	time_t Time;
	double val;
	if ( sscanf(packet.data(), "%ld %lf", &Time, &val) != 2 ) {
		eprintf("read error or malformed packet:\n");
		eprintf("\t\"%s\"\n", packet.data());
		return -1;
	}
	printf("new pressure:   \tP = %lf\n", val);
	gas.pressure = val;
	return 0;
}

int parse_temperature(packet_t packet) {
	time_t Time;
	double val;
	if ( sscanf(packet.data(), "%ld %lf", &Time, &val) != 2 ) {
		eprintf("read error or malformed packet:\n");
		eprintf("\t\"%s\"\n", packet.data());
		return -1;
	}
	printf("new temperature:\tT = %lf\n", val);
	gas.temperature = val;
	return 0;
}

int parse_mixture(packet_t packet) {
	gas.cffour = 3.;
	gas.isobut = 2.;
	printf("new mixture:    \tAr:CF4:iC4H10 = %.1lf:%.1lf:%.1lf\n",
		100.-gas.cffour-gas.isobut,
		gas.cffour, gas.isobut);
	return 0;
}

int parse_efield(packet_t packet) {
	time_t Time;
	double val;
	if ( sscanf(packet.data(), "%ld %lf", &Time, &val) != 2 ) {
		eprintf("read error or malformed packet:\n");
		eprintf("\t\"%s\"\n", packet.data());
		return -1;
	}
	printf("new efield:     \tE = %lf\n", val);
	gas.efield = val;
	return 0;
}
//@}

volatile int wantexit=0;

/// length of output string buffer
#define BUFLEN 200

/**
 * signal handler for correct sclient termination
 */
void sighand(int sig) {
	eprintf("cought signal %d.", sig);
	wantexit=1;
}

sclient* meptr = NULL;
void sig_alarm(int sig) {
	char	buffer[BUFLEN];
	gas.snprint(buffer, BUFLEN);
	if (meptr != NULL) meptr->printf("%s", buffer);
	alarm(UPDATE_INTERVAL);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main(void) {

	catch_signals(sighand);	// install signal handlers

	// connect to the subserver as usual
	sclient me;
	if (!me.isOK()) {
		eperror("could not initialize subclient");
		return 1;
	}
	meptr = &me;

	// setup connections for our experiment
	me.setid(SCLIENT_ID);
	me.subscribe(ABO_PRESSURE);
	me.subscribe(ABO_MIXTURE);
	me.subscribe(ABO_TEMPERATURE);
	me.subscribe(ABO_EFIELD);
	me.setDefaultSendname(ABO_RESULTS);

	if ( updatemode == on_each_time_interval ||
	     updatemode == on_update_or_interval    ) {
		signal(SIGALRM, sig_alarm);	// install handler 
		alarm(UPDATE_INTERVAL);		// set alarm
	}

	// enter the main loop
	packet_t rxpacket;
	char	buffer[BUFLEN];
	time_t now, lastupdate;
	time(&lastupdate);
	while ( ! wantexit ) {
		time(&now);
		if ( updatemode == on_each_update ||
		     updatemode == on_update_or_interval ||
		    (updatemode == on_update_and_interval && (now-lastupdate)>UPDATE_INTERVAL)
		    ) {
			gas.snprint(buffer, BUFLEN);
			me.printf("%s", buffer);
			time(&lastupdate);
		}
		// read packet
		if ( me.recvpacket(rxpacket) < 0 ) {
			eprintf("Error receiving a packet\n");
			continue;
		}
//		printf("parsing: %s\n", rxpacket.data());
		if        ( strncmp(rxpacket.name(), ABO_PRESSURE, strlen(ABO_PRESSURE)) == 0 ) { // PRESSURE
			parse_pressure(rxpacket);
		} else if ( strncmp(rxpacket.name(), ABO_MIXTURE, strlen(ABO_MIXTURE) ) == 0 ) { // MIXTURE
			parse_mixture(rxpacket);
		} else if ( strncmp(rxpacket.name(), ABO_TEMPERATURE, strlen(ABO_TEMPERATURE) ) == 0 ) { // MIXTURE
			parse_temperature(rxpacket);
		} else if ( strncmp(rxpacket.name(), ABO_EFIELD, strlen(ABO_EFIELD) ) == 0 ) { // MIXTURE
			parse_efield(rxpacket);
		}
//		printf("parsing done\n");
	}// end main loop

//	printf("exit\n");
	return 0;
}
//------------------------------------------------------------------------------
