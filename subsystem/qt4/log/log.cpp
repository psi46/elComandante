/**
 * Jochen Steinmann <jochen.steinmann@rwth-aachen.de>
 */
#include "log.h"

log::log(QMainWindow *parent) : QMainWindow(parent) {
        setupUi(this);
	my_log_thread1 = new log_thread();

	connect(aboname, SIGNAL(returnPressed()), this, SLOT(setAbo()));

	connect(my_log_thread1, SIGNAL(received(char*, int )), this, SLOT(setDisplay(char*, int )));
}

log::~log(){
}

void log::setAbo(){
	if(!old_abo.isNull()){
		printf("-> unsubscribe %s \n",(const char *) old_abo.toAscii().constData());
		my_log_thread1->getsclient()->unsubscribe((const char *) old_abo.toAscii().constData());
	}
	printf("-> subscribe %s \n",(const char *) aboname->text().toAscii().constData());
	my_log_thread1->getsclient()->subscribe((const char *) aboname->text().toAscii().constData());

	old_abo = aboname->text();
	if(my_log_thread1->getwantexit()){	// Thread not running
		printf("Thread started\n");
		my_log_thread1->start();
	}
}

void log::setDisplay(char* val, int length){ 
	val[length] = 0;
	QString display = val;

	display.replace(QRegExp("\n"),"");	// replace the \n at the end of the line ?!

	if(display.contains("ready", Qt::CaseInsensitive)){
		logoutput->append(time.currentTime().toString() + " <font color=\"green\">" +  display + "</font>");
	}else if(display.contains("busy", Qt::CaseInsensitive)){
		logoutput->append(time.currentTime().toString() + " <font color=\"yellow\">" +  display + "</font>");
	}else if(display.contains("error", Qt::CaseInsensitive)){
		logoutput->append(time.currentTime().toString() + " <font color=\"red\">" +  display + "</font>");
	}else{
		logoutput->append(time.currentTime().toString() + " " + display);
	}
}

