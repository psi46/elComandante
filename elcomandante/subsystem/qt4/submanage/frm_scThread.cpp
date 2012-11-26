/**
 * \file frm_scThread.cpp
 * \author Dennis Terhorst
 * \date Tue Jul 29 2008
 *
 * Widget code
 */
#include "frm_scThread.h"
#include <QVBoxLayout>
#include <stdio.h>

frm_scThread::frm_scThread(QMainWindow* parent) : QMainWindow(parent) {
	setupUi(this);
	connect(btnGO, SIGNAL(clicked()), this, SLOT(btnGO_clicked()));
	setWindowFlags(Qt::Tool);
	//setLayout(new QVBoxLayout ());	// FIXME
};
frm_scThread::~frm_scThread() {};

void frm_scThread::myheartbeat() {
	dial->setValue((dial->value()+1)%100);
	return;
}

void frm_scThread::btnGO_clicked() {
	emit GO_clicked();
}
