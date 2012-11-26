#include "thpc.h"
#include <QApplication>
#include <stdio.h>

int main( int argc, char* argv[]){
        QApplication a(argc, argv);
        thpc mythpc;
        mythpc.show();
        return a.exec();
}

