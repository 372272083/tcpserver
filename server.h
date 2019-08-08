#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QObject>
#include "tcpclientsocket.h"

class SqlserverDB;

class Server : public QTcpServer
{
    Q_OBJECT
public:
    Server(SqlserverDB *db,QObject *parent=0,int port=0);
    QList<TcpClientSocket*> tcpClientSocketList;

    void closeServer();
private:
    SqlserverDB* mdb;
signals:
    void updateServer(QString,int);
    void updateClientList();
public slots:
    void updateClients(QString,int);
    void slotDisconnected(int);
protected:
//    void incomingConnection(int socketDescriptor);
    void incomingConnection(qintptr socketDescriptor);
};

#endif // SERVER_H
