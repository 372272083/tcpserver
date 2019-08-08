#include "diagnosethread.h"

#include <QException>
#include <QList>
#include <QException>
#include <QQueue>
#include <QDebug>

#include <QList>

#include "sqlserverdb.h"
#include "tcpserver.h"

DiagnoseThread::DiagnoseThread(SqlserverDB *db):db(db)
{

}

void DiagnoseThread::run()
{
    while(true)
    {
        if(!TcpServer::enable_analyse)
        {
            continue;
        }
        //故障诊断
        sleep(2);
    }
}
