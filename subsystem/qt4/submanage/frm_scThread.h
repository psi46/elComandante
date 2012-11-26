/**
 * \file frm_scThread.h
 * \author Dennis Terhorst
 * \date Tue Jul 29 2008
 *
 * Widget code
 */
#ifndef FRM_SCTHREAD_H
#define FRM_SCTHREAD_H

#include <QMainWindow>
#include "ui_frm_scThread.h"

class frm_scThread : public QMainWindow, public Ui::frm_scThread {
	Q_OBJECT

	private slots:
		void btnGO_clicked();
	public:
		frm_scThread(QMainWindow* parent = 0);
		~frm_scThread();
	public slots:
		void myheartbeat();

	signals:
		void GO_clicked();
};
#endif //ndef FRM_SCTHREAD_H
