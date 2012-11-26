qmake -project
qmake "LIBS+=-L$HOME/lib -lsubsystem" "INCLUDEPATH+=$HOME/lib"
make

