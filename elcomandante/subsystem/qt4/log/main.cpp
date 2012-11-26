/**
 * Jochen Steinmann <jochen.steinmann@rwth-aachen.de>
 */
#include "log.h"
#include <QApplication>
#include <stdio.h>

int main( int argc, char* argv[]){
        QApplication a(argc, argv);
        log mylog;
        mylog.show();
        return a.exec();
}

