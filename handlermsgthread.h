#ifndef HANDLERMSGTHREAD_H
#define HANDLERMSGTHREAD_H

#include <QThread>
#include <QMap>

class SqlserverDB;
class Server;

class HandlerMsgThread : public QThread
{
    Q_OBJECT
public:
    HandlerMsgThread(SqlserverDB *db,Server* tcs);
protected:
    void run();

private :
    SqlserverDB* db;
    Server* tcs;

    QMap<int,QMap<QString,int>> project_structure;
};

#endif // HANDLERMSGTHREAD_H
