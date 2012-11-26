/**
 * submanage.cpp
 *
 * Dennis Terhorst
 * Tue Jul 22 19:05:13 CEST 2008
 */
#include "submanage.h"
#include <QWidget>
#include <QMetaType>

submanage::submanage(QMainWindow *parent) : QMainWindow(parent) {
        setupUi(this);
	ThreadView = new frm_scThread(this);
	scThread = new sclient_qthread();
	statusbar->showMessage("starting...",10000);

	connect(actionE_xit, SIGNAL(activated()), qApp, SLOT(quit()));
	connect(scThread, SIGNAL(started()), this, SLOT(setThreadStatusBar()));
	connect(scThread, SIGNAL(finished()), this, SLOT(setThreadStatusBar()));
	connect(scThread, SIGNAL(terminated()), this, SLOT(setThreadStatusBar()));

	qRegisterMetaType<packet_t>("packet_t");
	connect(scThread, SIGNAL(received(packet_t)), this, SLOT(DisplayPacket(packet_t)));
	connect(this, SIGNAL(packet_ready(packet_t)), scThread, SLOT(sendpacket(packet_t)));

//	connect(btnGO, SIGNAL(clicked()), scThread, SLOT(start()));
	connect(ThreadView, SIGNAL(GO_clicked()), this, SLOT(btnGO_clicked()));

	connect(btnSend, SIGNAL(clicked()), this, SLOT(btnSend_clicked()));
	connect(btnSubscribe, SIGNAL(clicked()), this, SLOT(btnSubscribe_clicked()));
	connect(btn_scStatus, SIGNAL(clicked()), this, SLOT(btn_scStatus_clicked()));

	connect(scThread, SIGNAL(myheartbeat()), ThreadView, SLOT(myheartbeat()));
/*	QHeaderView *header = abos_tree->header();
	header->setStretchLastSection(false);
	header->setResizeMode(0, QHeaderView::Stretch);
	header->setResizeMode(1, QHeaderView::ResizeToContents);

	abos_tree->setColumnCount(2);    

	QStringList head_label;
    	head_label << "Abo" << "Clients";

	abos_tree->setHeaderLabels(head_label);

 	QTreeWidgetItem *cities = new QTreeWidgetItem(abos_tree);
        cities->setText(0, tr("Cities"));
        QTreeWidgetItem *Item = new QTreeWidgetItem(cities);
        Item->setText(0, tr("Oslo"));
        Item->setText(1, tr("Yes"));

	Item = new QTreeWidgetItem(cities);
        Item->setText(0, tr("EST"));
        Item->setText(1, tr("Yes"));*/
}

submanage::~submanage() {
	if (scThread->isRunning()) {
		scThread->quit();
		scThread->wait(3000);
	}
	delete scThread;
	delete ThreadView;
}
void submanage::setThreadStatusBar() {
	if (scThread->isRunning()) statusbar->showMessage("sclient thread running.");
	if (scThread->isFinished())statusbar->showMessage("sclient thread finished.");
}

void submanage::DisplayPacket(packet_t packet) {
	cmbType->setCurrentIndex( packet.type );
	txtName->setText( packet.name() );
	txtData->setText( packet.data() );
}

void submanage::btnSend_clicked() {
	char data[MAX_PACKETLEN];
	const char* name = txtName->text().toStdString().c_str();
	const char* textdata = txtData->text().toStdString().c_str();
	strncpy(data, textdata, MAX_PACKETLEN);
	strncat(data, "\n", MAX_PACKETLEN-strlen(data));
	packet_t packet;
	packet.type = cmbType->currentIndex();
	packet.setName(name);
	packet.setData(data, strlen(data) );
	emit packet_ready(packet);
}

void submanage::btnGO_clicked() {
	scThread->start();
}

void submanage::btnSubscribe_clicked() {
	const char* name = txtName->text().toStdString().c_str();
	packet_t packet;
	// FIXME: actually sclient::subscribe() function should be used here!
	packet.type = PKT_SUBSCRIBE;
	packet.setName(name);
	packet.setData("",0);
	emit packet_ready(packet);
}

void submanage::btn_scStatus_clicked() {
	if (ThreadView->isVisible())
		ThreadView->hide();
	else
		ThreadView->show();
}
