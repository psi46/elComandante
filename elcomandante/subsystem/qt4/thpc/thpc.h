#ifndef THPC_H
#define THPC_H

#include "ui_thpc.h"
#include <qdatetime.h>
#include <qtimer.h>

#include "thpc_thread.h"

class thpc : public QMainWindow, public Ui::MainWindow{
        Q_OBJECT
	
	private slots:
		void showTime();
		void setT1(double val);
		void setH1(double val);
		void setT2(double val);
		void setH2(double val);
		void setAmbient(double val);
		void setBuffer(double val);

	protected:
		void run();

        public:
                thpc (QMainWindow *parent = 0);
                ~thpc();
};
#endif
