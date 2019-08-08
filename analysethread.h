#ifndef ANALYSETHREAD_H
#define ANALYSETHREAD_H

#include <QThread>

class SqlserverDB;

class AnalyseThread : public QThread
{
public:
    AnalyseThread(SqlserverDB *db);
protected:
    void run();

private :
    SqlserverDB* db;
};

#endif // ANALYSETHREAD_H
