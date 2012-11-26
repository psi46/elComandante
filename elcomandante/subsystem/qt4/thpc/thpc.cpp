#include "thpc.h"

thpc::thpc(QMainWindow *parent) : QMainWindow(parent) {
        setupUi(this);

        temp_1 -> display(5.67);

	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(showTime()));
	timer->start(1000);
	showTime();

	thpc_thread *my_thpc_thread1 = new thpc_thread("test1");	// T1
	thpc_thread *my_thpc_thread2 = new thpc_thread("test2");	// H1
	thpc_thread *my_thpc_thread3 = new thpc_thread("test3");	// T2
	thpc_thread *my_thpc_thread4 = new thpc_thread("test4");	// H2
	thpc_thread *my_thpc_thread5 = new thpc_thread("test5");	// ambient press
	thpc_thread *my_thpc_thread6 = new thpc_thread("test6");	// buffer press

	connect(my_thpc_thread1, SIGNAL(received(double )), this, SLOT(setT1(double )));
	connect(my_thpc_thread2, SIGNAL(received(double )), this, SLOT(setH1(double )));
	connect(my_thpc_thread3, SIGNAL(received(double )), this, SLOT(setT2(double )));
	connect(my_thpc_thread4, SIGNAL(received(double )), this, SLOT(setH2(double )));

	connect(my_thpc_thread5, SIGNAL(received(double )), this, SLOT(setAmbient(double )));
	connect(my_thpc_thread6, SIGNAL(received(double )), this, SLOT(setBuffer(double )));
        
	my_thpc_thread1->start();
	my_thpc_thread2->start();
	my_thpc_thread3->start();
	my_thpc_thread4->start();
	my_thpc_thread5->start();
	my_thpc_thread6->start();
}

thpc::~thpc(){
}

void thpc::showTime()
{
	QTime time = QTime::currentTime();
	QString text = time.toString("hh:mm:ss");
	if ((time.second() % 2) == 0)
		text[5] = ' ';
	clock->display(text);
}

void thpc::setT1(double val){ temp_1->display(val); }
void thpc::setH1(double val){ humi_1->display(val); }
void thpc::setT2(double val){ temp_2->display(val); }
void thpc::setH2(double val){ humi_2->display(val); }
void thpc::setAmbient(double val){ 
	ambient_pres->display(val);
	useable_pres->setMinimum((int)val);
}
void thpc::setBuffer(double val){ 
	buffer_pres->display(val); 
	if(val > useable_pres->maximum()){
		useable_pres->setMaximum((int)val);
	}
	useable_pres->setValue((int)val);
}
