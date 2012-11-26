#ifndef THPC_THREAD_H
#define THPC_THREAD_H

#include <QThread>

#include "../../sclient.h"
#include "../../packet_t.h"

// set local address to one of INADDR_ANY, INADDR_LOOPBACK, inet_aton(), inet_addr(), gethostbyname()
#define SVR_ADDR		"127.0.0.1"
#define SVR_PORT		12334

class thpc_thread : public QThread{
	Q_OBJECT
	private: 
		int wantexit;
		sclient *me;
		packet_t packet;

	public:
		thpc_thread(const char* abo);
		~thpc_thread();
		void run();

	signals:
		void received(double );

};
#endif

