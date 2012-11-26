/**
 * Jochen Steinmann <jochen.steinmann@rwth-aachen.de>
 */

#include "submanage.h"
#include <QApplication>
#include <stdio.h>

int main( int argc, char* argv[]){
        QApplication a(argc, argv);
        submanage mymanage;
        mymanage.show();
        return a.exec();
}

