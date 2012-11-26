#ifndef UNI_H
#define UNI_H

#include "ui_uni.h"
#include "uni_thread.h"

#include <QtDebug>
#include <QString>


class uni : public QMainWindow, public Ui::MainWindow{
        Q_OBJECT
	
	private slots:
		void setDisplay(double val);
		void setAbo();
	
	private:
		uni_thread *my_uni_thread1;
		QString old_abo;

	protected:
		void run();

        public:
                uni (QMainWindow *parent = 0);
                ~uni();
};
#endif
