#include "server.h"

#include "sqlserverdb.h"

Server::Server(SqlserverDB *db,QObject *parent,int port)
    :QTcpServer(parent),mdb(db)
{
    listen(QHostAddress::Any,port);
}
int i =0;
void Server::incomingConnection(qintptr socketDescriptor)
{
    //第链接一个会新创建一个
    TcpClientSocket *tcpClientSocket=new TcpClientSocket(mdb,this);
    connect(tcpClientSocket,SIGNAL(updateClients(QString,int)),this,SLOT(updateClients(QString,int)));
    connect(tcpClientSocket,SIGNAL(disconnected(int)),this,SLOT(slotDisconnected(int)));

    tcpClientSocket->setSocketDescriptor(socketDescriptor);

    tcpClientSocketList.append(tcpClientSocket);

    QString itemName;
    if(tcpClientSocket->clientName.length() > 0)
    {
        itemName = tcpClientSocket->clientName + "-" + tcpClientSocket->peerAddress().toString() + " - Connected";
    }
    else
    {
        itemName = "No-" +tcpClientSocket->peerAddress().toString() + " - Connected";
    }

    emit updateServer(itemName,itemName.length());

    qDebug()<<i;
    i++;
    emit updateClientList();
}

void Server::updateClients(QString msg,int length)
{
    emit updateServer(msg,length);
    if (msg.compare("init") == 0)
    {
        emit updateClientList();
    }
}

void Server::slotDisconnected(int descriptor)
{
    for(int i=0;i<tcpClientSocketList.count();i++)
    {
        TcpClientSocket *item = tcpClientSocketList.at(i);
        if(item->socketDescriptor()==descriptor)
        {
            QString itemName;
            if(item->clientName.length() > 0)
            {
                itemName = item->clientName + "-" + item->peerAddress().toString() + " - Disconnected";
            }
            else
            {
                itemName = "No-" +item->peerAddress().toString() + " - Disconnected";
            }
            item->disconnectFromHost();
            item->close();
            tcpClientSocketList.removeAt(i);

            emit updateServer(itemName,itemName.length());
            break;
        }
    }

    emit updateClientList();
    return;
}

void Server::closeServer()
{
    int count = tcpClientSocketList.count() - 1;
    for(int i=count;i >= 0;i--)
    {
        TcpClientSocket *item = tcpClientSocketList.at(i);
        item->disconnectFromHost();
        item->close();
        tcpClientSocketList.removeAt(i);
    }
}
