#include "uni.h"
#include <QApplication>
#include <stdio.h>

int main( int argc, char* argv[]){
        QApplication a(argc, argv);
        uni myuni;
        myuni.show();
        return a.exec();
}

