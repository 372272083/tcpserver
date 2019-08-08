#ifndef TCPCLIENTSOCKET_H
#define TCPCLIENTSOCKET_H

#include <QTcpSocket>
#include <QObject>
#include <QVector>

class SqlserverDB;

class TcpClientSocket : public QTcpSocket
{
    Q_OBJECT
public:
    TcpClientSocket(SqlserverDB *db,QObject *parent=0);

    QString clientName;
    QString getProjectData(QString name);

    QString m_code;
    int project_id;

    void addMsg(QString);
private:
    int expLen;
    int msgType;
    QByteArray buffer;

    QVector<QString> msgs;
    SqlserverDB *mdb;
    bool datastate;
signals:
    void updateClients(QString,int);
    void disconnected(int);
protected slots:
    void dataReceived();
    void slotDisconnected();
    void timerInterval();
};

#endif // TCPCLIENTSOCKET_H
