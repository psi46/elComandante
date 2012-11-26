#include "uni.h"

uni::uni(QMainWindow *parent) : QMainWindow(parent) {
        setupUi(this);
	my_uni_thread1 = new uni_thread();

	connect(aboname, SIGNAL(returnPressed()), this, SLOT(setAbo()));

	connect(my_uni_thread1, SIGNAL(received(double )), this, SLOT(setDisplay(double )));
}

uni::~uni(){
}

void uni::setAbo(){
	if(!old_abo.isNull()){
		printf("-> unsubscribe %s \n",(const char *) old_abo.toAscii().constData());
		my_uni_thread1->getsclient()->unsubscribe((const char *) old_abo.toAscii().constData());
	}
	printf("-> subscribe %s \n",(const char *) aboname->text().toAscii().constData());
	my_uni_thread1->getsclient()->subscribe((const char *) aboname->text().toAscii().constData());

	old_abo = aboname->text();
	if(my_uni_thread1->getwantexit()){	// Thread not running
		printf("Thread started\n");
		my_uni_thread1->start();
	}
}

void uni::setDisplay(double val){ 
	display->display(val);	
}

