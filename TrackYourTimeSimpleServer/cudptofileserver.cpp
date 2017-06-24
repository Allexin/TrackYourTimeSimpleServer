#include "cudptofileserver.h"
#include <QDateTime>

//const QString EXTERNAL_TRACKER_PREFIX = "TYTET";
const QString OVERRIDE_TRACKER_PREFIX = "TYTOT";
const QString EXTERNAL_TRACKER_FORMAT_VERSION = "1";
const int IDLE_TIMEOUT = 60;

void cUDPtoFileServer::processState(QString host, QString UserName, QString AppName, QString AppState, int idleCounter)
{
    sUserInfo userInfo;
    userInfo.UserName = UserName;
    userInfo.AppName = AppName;
    userInfo.AppState = AppState;
    userInfo.IdleCounter = idleCounter;
    userInfo.LastPackageTime = QDateTime::currentSecsSinceEpoch();

    bool newUser = true;
    if (m_States.contains(host)){
        newUser = m_States[host].UserName!=UserName;
    }

    m_States[host] = userInfo;

    if (newUser){
        qDebug() << "new user " << UserName << " connection from " << host;
    }

}

cUDPtoFileServer::cUDPtoFileServer(QString outputFileName, QString stateFileName, QObject *parent) : QObject(parent),
    m_OutputCSV(outputFileName)
{
    m_Server.bind(QHostAddress::Any, UDP_PORT);
    connect(&m_Server, SIGNAL(readyRead()), this, SLOT(readyRead()));

    if (!m_OutputCSV.open(QIODevice::WriteOnly | QIODevice::Append)) {
        qCritical() << "can't access file " << outputFileName;
    }

    connect(&m_Timer,SIGNAL(timeout()),this, SLOT(onTimerOutput()));
    m_Timer.start(1000);

    if (!stateFileName.isEmpty()){
        m_StateFileName = stateFileName;
        connect(&m_StateTimer,SIGNAL(timeout()),this,SLOT(onTimerState()));
        m_StateTimer.start(5000);
    }
}

void cUDPtoFileServer::readyRead()
{
    QByteArray buffer;
    buffer.resize(m_Server.pendingDatagramSize());

    QHostAddress sender;
    quint16 senderPort;

    m_Server.readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);
    QString Data(buffer);
    onDataReady(Data,sender.toString());
}

void cUDPtoFileServer::onDataReady(QString data, QString host)
{
    QString dataFix = data.simplified().replace("%20"," ");
    QStringList list = dataFix.split('&');
    QMap<QString,QString> pairs;
    for (int i = 0; i<list.size(); i++){
        QStringList pair = list[i].split('=');
        if (pair.size()==2){
            pairs[pair[0]]=pair[1];
        }
    }

    if (pairs["VERSION"].compare(EXTERNAL_TRACKER_FORMAT_VERSION)!=0){
        qWarning() << "unknown exterinal tracker with VERSION=" << pairs["VERSION"];
        return;
    }

    QString state = pairs["STATE"].trimmed();
    if (state.isEmpty()){
        qWarning() << "external tracker state is empty";
        return;
    }

    QString userName = pairs["USER_NAME"].trimmed();
    if (userName.isEmpty()){
        qWarning() << "external tracker userName is empty";
        return;
    }

    if (pairs["PREFIX"].compare(OVERRIDE_TRACKER_PREFIX)==0){
        if (!pairs.contains("APP_FILENAME")){
            qWarning() << "override tracker APP_FILENAME not defined";
            return;
        }
        if (!pairs.contains("USER_INACTIVE_TIME")){
            qWarning() << "override tracker USER_INACTIVE_TIME not defined";
            return;
        }

        processState(host, userName, pairs["APP_FILENAME"],pairs["STATE"],pairs["USER_INACTIVE_TIME"].toInt());
    }
    else{
        qWarning() << "unknown exterinal tracker with PREFIX=" << pairs["PREFIX"];
    }
}

void cUDPtoFileServer::onTimerOutput()
{
    QTextStream streamCSV(&m_OutputCSV);
    QMap<QString, sUserInfo>::iterator i = m_States.begin();
    while (i != m_States.end()) {
        QString ip = i.key();
        sUserInfo& info = i.value();
        info.IdleCounter++;
        if (info.IdleCounter<IDLE_TIMEOUT){
            streamCSV
                      << QDate::currentDate().toString("dd.MM.yyyy") << ","
                      << QTime::currentTime().toString("h:mm:ss") << ","
                      << ip << ","
                      << info.UserName << ","
                      << info.AppName << ","
                      << info.AppState << '\n';
        }
        ++i;
    }
    m_OutputCSV.flush();
}

void cUDPtoFileServer::onTimerState()
{
    QFile f(m_StateFileName);
    if (f.open(QIODevice::WriteOnly)){
        QTextStream s(&f);
        s << QString::number(QDateTime::currentSecsSinceEpoch()) << '\n';
        s << QString::number(m_States.size()) << '\n';
        QMap<QString, sUserInfo>::const_iterator i = m_States.constBegin();
        while (i != m_States.constEnd()) {
            QString ip = i.key();
            const sUserInfo& info = i.value();
                s
                  << info.UserName << '\n'
                  << info.AppName << '\n'
                  << info.AppState << '\n'
                  << info.IdleCounter << '\n'
                  << info.LastPackageTime << '\n';
            ++i;
        }

    }
}
