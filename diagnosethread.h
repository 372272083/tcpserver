#ifndef DIAGNOSETHREAD_H
#define DIAGNOSETHREAD_H


#include <QThread>

class SqlserverDB;

class DiagnoseThread : public QThread
{
public:
    DiagnoseThread(SqlserverDB *db);
protected:
    void run();

private :
    SqlserverDB* db;
};

#endif // DIAGNOSETHREAD_H
