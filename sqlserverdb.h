#ifndef SQLSERVERDB_H
#define SQLSERVERDB_H

class QString;
class QStringList;
class QSqlTableModel;

#include <QSqlDatabase>
#include <QQueue>
#include <QVector>
#include <QList>
#include <QVariant>
#include <QMap>

class SqlserverDB
{
public:
    SqlserverDB(QString dbsource,QString host,QString username,QString password);

    bool reConnect();
    bool open();
    void close();
    bool status();
    bool createtable(QString sql);

    QMap<int,QString> queryWaveSql(int type);
    QMap<int,QMap<QString,QString>> queryFeatureSql(int type,QString time,int tolance=2);

    bool updatasql(QString sql);
    QVector<QString> query(QString sql,QString seperator=",");
    int querysqlcount(QString sql);

    bool execSql(QQueue<QString> sqls);
    bool execSql(QString sql,QMap<QString,QVariantList> binds);

    int getMaxId(QString table);

    QSqlTableModel* model(QString table,QStringList headers);
    QSqlTableModel* modelNoHeader(QString table);

private:
    QString dbsource;
    QString host;
    QString username;
    QString password;
    QSqlDatabase database;
};

#endif // SQLSERVERDB_H
