/**
 * Jochen Steinmann <jochen.steinmann@rwth-aachen.de>
 */
#ifndef LOG_H
#define LOG_H

#include "ui_log.h"
#include "log_thread.h"

#include <QtDebug>
#include <QString>
#include <QTime>


class log : public QMainWindow, public Ui::MainWindow{
        Q_OBJECT
	
	private slots:
		void setDisplay(char* val, int length);
		void setAbo();
	
	private:
		log_thread *my_log_thread1;
		QString old_abo;
		QTime time;

	protected:
		void run();

        public:
                log (QMainWindow *parent = 0);
                ~log();
};
#endif
