#include "handlermsgthread.h"

#include <QException>
#include <QSqlTableModel>
#include <QList>
#include <QException>
#include <QQueue>
#include <QDebug>
#include <QDateTime>

#include <QList>
#include <QVariant>

#include "sqlserverdb.h"
#include "tcpserver.h"
#include "server.h"
#include "tcpclientsocket.h"

HandlerMsgThread::HandlerMsgThread(SqlserverDB *db,Server* tcs):db(db),tcs(tcs)
{

}

void HandlerMsgThread::run()
{
    bool tok;
    while(true){
        QString sql = "select top 1 id,projectId,msgType,msg,state,rksj from index_msgtransfer where state=1 order by rksj desc";
        qDebug() << sql;
        QVector<QString> datas = db->query(sql);
        QVector<QString>::ConstIterator it,in_it;
        for(it=datas.constBegin();it!=datas.constEnd();it++)
        {
            QString data = *it;
            QStringList data_items = data.split(',');
            if(data_items.size() >= 6)
            {
                QString tmp = data_items[0];
                int id = tmp.toInt(&tok);
                tmp = data_items[1];
                int project_id = tmp.toInt(&tok);
                tmp = data_items[2];
                int msg_type = tmp.toInt(&tok);
                tmp = data_items[3];
                QString msg = tmp;
                tmp = data_items[5];
                QString rksj = tmp;

                QDateTime rksj_time = QDateTime::fromString(rksj,"yyyy-MM-dd hh:mm:ss");
                QDateTime cur_time = QDateTime::currentDateTime();
                if(qAbs(rksj_time.secsTo(cur_time)) < 10)
                {
                    if(!project_structure.contains(project_id) || project_structure[project_id].size() == 0)
                    {
                        sql = "select mcode from index_motor where project=" + QString::number(project_id);
                        QVector<QString> mcodes = db->query(sql);
                        QMap<QString,int> mcode_map;
                        for(in_it=mcodes.constBegin();in_it!=mcodes.constEnd();in_it++)
                        {
                            QString mcodes = *in_it;
                            QStringList mcode_items = mcodes.split(',');
                            if(mcode_items.size() >= 1)
                            {
                                mcode_map[mcode_items[0]] = 1;
                            }
                        }
                        project_structure[project_id] = mcode_map;
                    }

                    QList<TcpClientSocket*> clients = tcs->tcpClientSocketList;
                    QList<TcpClientSocket*>::ConstIterator tIt;
                    for(tIt=clients.constBegin();tIt!=clients.constEnd();tIt++)
                    {
                        TcpClientSocket* tc = *tIt;
                        if(tc->project_id == 0)
                        {
                            if(tc->m_code.length() > 0)
                            {
                                if(project_structure[project_id].contains(tc->m_code))
                                {
                                    tc->project_id = project_id;
                                }
                            }
                        }

                        if(tc->project_id == project_id)
                        {
                            QString msg = "1#recordwave";
                            //QString ip = tc->peerAddress().toString();
                            //qint64 r = tc->write(msg.toUtf8());
                            //qDebug() << QString::number(r);
                            tc->addMsg(msg);
                            break;
                        }
                    }
                }

                sql = "update index_msgtransfer set state=0 where id=" + QString::number(id);
                db->updatasql(sql);
            }
        }
        sleep(1);
    }
}
