#ifndef SAVESAMPLETHREAD_H
#define SAVESAMPLETHREAD_H

#include <QThread>

class SqlserverDB;
class SaveSampleThread : public QThread
{
    Q_OBJECT
public:
    SaveSampleThread(SqlserverDB *db);
protected:
    void run();

private :
    SqlserverDB* db;
};

#endif // SAVESAMPLETHREAD_H
