#ifndef UNI_THREAD_H
#define UNI_THREAD_H

#include <QThread>

#include "../../sclient.h"
#include "../../packet_t.h"

// set local address to one of INADDR_ANY, INADDR_LOOPBACK, inet_aton(), inet_addr(), gethostbyname()
#define SVR_ADDR		"127.0.0.1"
#define SVR_PORT		12334

class uni_thread : public QThread{
	Q_OBJECT
	private: 
		int wantexit;
		sclient *me;
		packet_t packet;

	public:
		uni_thread();
		~uni_thread();
		void run();
		sclient* getsclient();
		int getwantexit() {return wantexit;};	

	signals:
		void received(double );

};
#endif

