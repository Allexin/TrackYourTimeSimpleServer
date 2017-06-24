#ifndef CUDPTOFILESERVER_H
#define CUDPTOFILESERVER_H

#include <QObject>
#include <QUdpSocket>
#include <QDataStream>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDataStream>
#include <QFile>
#include <QTimer>

struct sUserInfo{
    QString UserName;
    QString AppName;
    QString AppState;
    int IdleCounter;
    qint64 LastPackageTime;
};

class cUDPtoFileServer : public QObject
{
    Q_OBJECT
public:
    static const int    UDP_PORT = 25855;
protected:
    QTimer                      m_StateTimer;
    QString                     m_StateFileName;

    QTimer                      m_Timer;
    QFile                       m_OutputCSV;

    QUdpSocket                  m_Server;
    QMap<QString, sUserInfo>    m_States;
    void processState(QString host, QString UserName, QString AppName, QString AppState, int idleCounter);
public:
    explicit cUDPtoFileServer(QString outputFileName, QString stateFileName, QObject *parent = 0);

signals:

public slots:
    void readyRead();
    void onDataReady(QString data, QString host);
    void onTimerOutput();
    void onTimerState();
};

#endif // CUDPTOFILESERVER_H
