/**
 * Jochen Steinmann <jochen.steinmann@rwth-aachen.de>
 */

#ifndef SUBMANAGE_H
#define SUBMANAGE_H

//#include <QTreeWidgetItem>
//#include <QTreeWidget>
//#include <QHeaderView>
//#include <QStringList>
#include <QThread>

#include "ui_submanage.h"
#include "sclient_qthread.h"
#include "frm_scThread.h"

class submanage : public QMainWindow, public Ui::MainWindow{
        Q_OBJECT
	private:
		QThread* scThread;
		frm_scThread* ThreadView;

        public:
                submanage (QMainWindow *parent = 0);
                ~submanage();
	private slots:
		void setThreadStatusBar();
		void DisplayPacket(packet_t );
		void btnSend_clicked();
		void btnGO_clicked();
		void btnSubscribe_clicked();
		void btn_scStatus_clicked();

	private: signals:
		void packet_ready(packet_t );
};
#endif
