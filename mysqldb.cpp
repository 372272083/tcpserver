#include "mysqldb.h"


#include <QSql>
#include <QSqlDatabase>
#include <QSqlRecord>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>
#include <QVariantList>
#include <QSqlTableModel>

#include <QException>

#include <QStringList>
#include <QDebug>
#include <QVector>
#include <QDateTime>
#include <QException>
#include <QSqlError>

MySQLDB::MySQLDB(QString dbsource,QString host,QString username,QString password)
{
    this->dbsource = dbsource;
    this->host = host;
    this->username = username;
    this->password = password;
    database = QSqlDatabase::addDatabase("QMYSQL");   //数据库驱动类型为SQL Server
    qDebug()<<"ODBC driver?"<<database.isValid();
    QString dsn = QString::fromLocal8Bit(this->dbsource.toLocal8Bit());      //数据源名称
    database.setHostName(this->host);                        //选择本地主机，127.0.1.1
    database.setDatabaseName(dsn);                            //设置数据源名称
    database.setUserName(this->username);                               //登录用户
    database.setPassword(this->password);                           //密码
}

bool MySQLDB::reConnect()
{
    database.close();
    database = QSqlDatabase::addDatabase("QODBC");   //数据库驱动类型为SQL Server
    qDebug()<<"ODBC driver?"<<database.isValid();
    QString dsn = QString::fromLocal8Bit(this->dbsource.toLocal8Bit());      //数据源名称
    database.setHostName(this->host);                        //选择本地主机，127.0.1.1
    database.setDatabaseName(dsn);                            //设置数据源名称
    database.setUserName(this->username);                               //登录用户
    database.setPassword(this->password);                           //密码
}

bool MySQLDB::open()
{
    if (database.isOpen())
    {
        return true;
    }
    database.open();

    bool result = database.isOpen();
    if (!result)
    {
        qDebug()<<"Error: "<<database.lastError().text();
        //qDebug()<<"Error: "<<database.lastError().type().number();
    }
    return result;
}

bool MySQLDB::createtable(QString sql)
{
    QSqlQuery sql_query;

    sql_query.prepare(sql); //创建表
    if(!sql_query.exec()) //查看创建表是否成功
    {
        return false;
    }

    return true;
}

bool MySQLDB::updatasql(QString sql)
{
    QSqlQuery sql_query(database);

    sql_query.prepare(sql); //
    if(!sql_query.exec()) //
    {
        return false;
    }

    return true;
}

QMap<int,QString> MySQLDB::queryWaveSql(int type)
{
    QMap<int,QString> result;
    QSqlQuery query(database);
    QString tableName = "index_electricchargewave";
    if(type == 1)
    {
        tableName = "index_vibratewave";
    }
    QString sql = "select TOP 12 dcode from " + tableName + " where wid<>2 order by rksj desc";

    bool success = query.exec(sql);

    if (success)
    {
        while (query.next() && result.size() == 0) {
            QSqlRecord rec = query.record(); //
            QString dcode = rec.value("dcode").toString();
            sql = "select TOP 12 rksj from " + tableName + " where wid<>2 and dcode='" + dcode + "' order by rksj desc";

            success = query.exec(sql);
            QMap<QString,int> tmp;
            if (success)
            {
                while (query.next()) {
                    QSqlRecord rec = query.record(); //

                    QString rksj = rec.value("rksj").toString();
                    int invalid_dot_index = rksj.indexOf(".",0);
                    if (invalid_dot_index > 0)
                    {
                        rksj = rksj.mid(0,invalid_dot_index);
                    }
                    QDateTime dt = QDateTime::fromString(rksj,"yyyy-MM-dd hh:mm:ss");

                    QList<QString> dts = tmp.keys();
                    QList<QString>::const_iterator it;
                    bool flag = true;
                    for(it=dts.constBegin();it!=dts.constEnd();it++)
                    {
                        QString item = *it;
                        QDateTime item_dt = QDateTime::fromString(item,"yyyy-MM-dd hh:mm:ss");
                        if(qAbs(item_dt.secsTo(dt)) < 2)
                        {
                            tmp[item] = tmp[item] + 1;
                            flag = false;
                            break;
                        }
                    }
                    if(flag)
                    {
                        tmp[rksj] = 1;
                    }
                }

                QString find_dt = "";
                QList<QString> dts = tmp.keys();
                QList<QString>::const_iterator it;
                for(it=dts.constBegin();it!=dts.constEnd();it++)
                {
                    QString item = *it;
                    if(tmp[item] >=6) //find integrate wave
                    {
                        find_dt = item;
                        break;
                    }
                }
                if (find_dt.length()>0)
                {
                    QDateTime dt = QDateTime::fromString(find_dt,"yyyy-MM-dd hh:mm:ss");
                    QDateTime min_dt = dt.addSecs(-2);
                    QDateTime max_dt = dt.addSecs(2);

                    sql = "select id,stype,pipe,sample_data from " + tableName + " where wid<>2 and dcode='" + dcode + "'  and rksj >'" + min_dt.toString("yyyy-MM-dd hh:mm:ss") + "' and rksj < '" + max_dt.toString("yyyy-MM-dd hh:mm:ss") + "'";
                    success = query.exec(sql);
                    //qDebug() << sql;

                    int row = query.size();
                    //if(row == 6)
                    {
                        bool tok;
                        QString config = "";
                        while (query.next()) {
                            QSqlRecord rec = query.record(); //

                            int stype = rec.value("stype").toInt(&tok);
                            int pipe = rec.value("pipe").toInt(&tok);
                            QString sample_data = rec.value("sample_data").toString();
                            if(0 == type)
                            {
                                switch (pipe) {
                                case 1:
                                    if(0==stype)
                                    {
                                        result[1] = sample_data;
                                        config += "1:" + rec.value("id").toString() + ",";
                                    }
                                    else
                                    {
                                        result[4] = sample_data;
                                        config += "4:" + rec.value("id").toString() + ",";
                                    }
                                    break;
                                case 2:
                                    if(0==stype)
                                    {
                                        result[2] = sample_data;
                                        config += "2:" + rec.value("id").toString() + ",";
                                    }
                                    else
                                    {
                                        result[5] = sample_data;
                                        config += "5:" + rec.value("id").toString() + ",";
                                    }
                                    break;
                                case 3:
                                    if(0==stype)
                                    {
                                        result[3] = sample_data;
                                        config += "3:" + rec.value("id").toString() + ",";
                                    }
                                    else
                                    {
                                        result[6] = sample_data;
                                        config += "6:" + rec.value("id").toString() + ",";
                                    }
                                    break;
                                default:
                                    break;
                                }
                            }
                            else
                            {
                                result[pipe] = sample_data;
                                config += QString::number(pipe) + ":" + rec.value("id").toString() + ",";
                            }
                        }
                        config += "t:"+find_dt;
                        result[0] = config;
                        result[50] = dcode;
                        sql = "update " + tableName + " set wid=2 where wid<>2 and dcode='" + dcode + "'  and rksj >'" + min_dt.toString("yyyy-MM-dd hh:mm:ss") + "' and rksj < '" + max_dt.toString("yyyy-MM-dd hh:mm:ss") + "'";
                        query.exec(sql);
                    }
                }
            }
        }
    }
    else
    {
        return result;
    }

    return result;
}

QMap<int,QMap<QString,QString>> MySQLDB::queryFeatureSql(int type,QString dtime,int tolance)
{
    QMap<int,QMap<QString,QString>> result;
    QSqlQuery query(database);
    QString tableName = "index_electriccharge";
    if(type == 1)
    {
        tableName = "index_vibrate";
    }

    QDateTime dt = QDateTime::fromString(dtime,"yyyy-MM-dd hh:mm:ss");
    QDateTime min_dt = dt.addSecs(0-tolance);
    QDateTime max_dt = dt.addSecs(tolance);

    QString sql = "select * from " + tableName + " where rksj >'" + min_dt.toString("yyyy-MM-dd hh:mm:ss") + "' and rksj < '" + max_dt.toString("yyyy-MM-dd hh:mm:ss") + "'";
    bool success = query.exec(sql);
    if(success)
    {
        bool tok;
        while (query.next()) {
            QSqlRecord rec = query.record(); //

            int pipe = rec.value("pipe").toInt(&tok);
            if(0 == type) //electric charge
            {
                if(!result.contains(pipe))
                {
                    QMap<QString,QString> item;
                    item["u"] = rec.value("u").toString();
                    item["i"] = rec.value("i").toString();
                    item["f"] = rec.value("f").toString();
                    item["factor"] = rec.value("factor").toString();
                    item["p"] = rec.value("p").toString();
                    item["q"] = rec.value("q").toString();
                    item["s"] = rec.value("s").toString();
                    result[pipe] = item;
                }
            }
            else
            {
                if(!result.contains(pipe))
                {
                    QMap<QString,QString> item;
                    item["vibrate_e"] = rec.value("vibrate_e").toString();
                    item["speed_e"] = rec.value("speed_e").toString();
                    result[pipe] = item;
                }
            }
        }
    }
    return result;
}

bool MySQLDB::execSql(QString sql,QMap<QString,QVariantList> binds)
{
    if (!database.isOpen())
    {
        return false;
    }
    database.transaction();
    QSqlQuery query(database);
    query.prepare(sql);

    QList<QString> keys = binds.keys();
    int len = keys.size();
    for(int i=0;i<len;i++)
    {
        QString bindName = ":"+keys[i];
        query.bindValue(bindName,binds[keys[i]]);
    }
    bool flag = true;
    if(!query.execBatch())
    {
        qDebug() << query.lastError().text();
        flag = false;
    }
    if(!flag)
    {
        database.rollback();
    }
    else
    {
        database.commit();
    }
    return flag;
}

bool MySQLDB::execSql(QQueue<QString> sqls)
{
    if (!database.isOpen())
    {
        return false;
    }
    database.transaction();
    QSqlQuery query(database);
    bool flag = true;
    while(sqls.size()>0)
    {
        QString sql = sqls.dequeue();
        flag = query.exec(sql);
        if (!flag)
        {
            break;
        }
    }
    if(!flag)
    {
        database.rollback();
    }
    else
    {
        database.commit();
    }
    return flag;
}

QSqlTableModel* MySQLDB::model(QString table,QStringList headers)
{
    QSqlTableModel *model = new QSqlTableModel(nullptr,database);
    model->setTable(table);
    model->select();
    qDebug() << model->rowCount() << " " << table;

    QListIterator<QString> i(headers);
    int index = 1;
    while(i.hasNext())
    {
        model->setHeaderData(index,Qt::Horizontal, i.next());
        index++;
    }

    return model;
}

QSqlTableModel* MySQLDB::modelNoHeader(QString table)
{
    QSqlTableModel *model = new QSqlTableModel(nullptr,database);
    model->setTable(table);
    model->select();

    return model;
}

int MySQLDB::getMaxId(QString table)
{
    QSqlQuery sql_query(database);

    QString sql = "select max(id) as mId from " + table;
    if(!sql_query.exec(sql)) //
    {
        return 0;
    }
    else
    {
        while (sql_query.next()) {
            QSqlRecord rec = sql_query.record(); //
            try{
                QString id_str = rec.value("mId").toString();
                if(id_str.length() > 0)
                {
                    bool tok;
                    int id = id_str.toInt(&tok);
                    return id+1;
                }
            }
            catch(QException e)
            {

            }
        }
    }

    return 0;
}

void MySQLDB::close()
{
    database.close();
}

bool MySQLDB::status()
{
    return database.isOpen();
}
