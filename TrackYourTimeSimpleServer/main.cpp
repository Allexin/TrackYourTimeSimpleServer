#include <QCoreApplication>
#include <QDebug>
#include "cudptofileserver.h"

int main(int argc, char *argv[])
{
    QString outputFileName;
    QString stateFileName;

    qDebug() << "available arguments: ";
    qDebug() << "-output <fileName> - CSV table output";
    qDebug() << "-state <fileName> - current state of all available connections";
    for (int i = 1; i<argc-1; i++){
        QString arg = argv[i];
        if (arg=="-output")
            outputFileName = argv[i+1];
        if (arg=="-state")
            stateFileName = argv[i+1];
    }
    if (outputFileName.isEmpty()){
        outputFileName = "output.csv";
        qDebug() << "output file name(-output) not set - using default name " << outputFileName;
    }
    if (stateFileName.isEmpty()){
        qDebug() << "state file name(-state) not set - state saving disabled";
    }

    QCoreApplication a(argc, argv);

    cUDPtoFileServer server(outputFileName,stateFileName);

    int result = a.exec();
    return result;
}
