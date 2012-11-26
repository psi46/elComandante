/**
 * sclient_qthread.cpp
 *
 * Dennis Terhorst
 * Tue Jul 22 23:06:19 CEST 2008
 */
#ifndef UNI_THREAD_H
#define UNI_THREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <subsystem/sclient.h>

class sclient_qthread : public QThread {
	Q_OBJECT
	private:
		QMutex mutex;
		QWaitCondition condition;
		bool abort;
		bool restart;

		sclient *me;

	public:
		sclient_qthread(QObject* parent=0);
		~sclient_qthread();

	//	void sendpacket(packet_t packet);
	protected:
		void run();		// QThread virtual, called from QThread::start()

		const sclient* getsclient();
	public:
	signals:
		void received(packet_t );
		void connection_failed();
		void connection_established();
		void connection_terminated();
		void send_error();
		void receive_error();
		void myheartbeat();
	public slots:
		void sendpacket(packet_t);
};
#endif

