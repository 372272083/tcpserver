#include "savesamplethread.h"

#include <QException>
#include <QSqlTableModel>
#include <QList>
#include <QException>
#include <QQueue>
#include <QDebug>

#include <QList>
#include <QVariant>

#include "sqlserverdb.h"
#include "tcpserver.h"

SaveSampleThread::SaveSampleThread(SqlserverDB *db):db(db)
{

}

void SaveSampleThread::run()
{
    int row = 0;
    int maxCount = 0;
    int maxExcueteNum = 5;
    while(true)
    {
        if(TcpServer::msg_queue.size() > 0)
        {
            if(TcpServer::msg_queue.contains(101)) //motor sync
            {
                QQueue<QString> sqls;
                while (TcpServer::msg_queue[101].size() > 0)
                {
                    QString info = TcpServer::msg_queue[101].dequeue();
                    QStringList items = info.split(',');

                    try
                    {
                        if(items.size() == 5)
                        {
                            QString code = items[0];
                            QString sql = "select mcode from index_motor where mcode='" + code + "'";
                            row = db->querysqlcount(sql);
                            if(row==0)
                            {
                                sql = "insert into index_motor (mcode,name,motor_type,bearing_type,manufacture_date) values ('";
                                sql += items[0] + "','" + items[1] + "','" + items[2] + "','" + items[3] + "','" + items[4] + "')";
                                sqls.enqueue(sql);
                            }
                        }
                    }
                    catch(QException e)
                    {

                    }
                }
                db->execSql(sqls);
            }

            if(TcpServer::msg_queue.contains(102)) //device sync
            {
                QQueue<QString> sqls;
                while (TcpServer::msg_queue[102].size() > 0)
                {
                    QString info = TcpServer::msg_queue[102].dequeue();
                    QStringList items = info.split(',');

                    try
                    {
                        if(items.size() == 6)
                        {
                            QString code = items[0];
                            QString sql = "select code from index_device where code='" + code + "'";
                            row = db->querysqlcount(sql);
                            if(row==0)
                            {
                                sql = "insert into index_device (code,name,dmodel,dtype,ipaddress,port) values ('";
                                sql += items[0] + "','" + items[1] + "','" + items[2] + "','" + items[3] + "','" + items[4] + "'," + items[5] + ")";
                                sqls.enqueue(sql);
                            }
                        }
                    }
                    catch(QException e)
                    {

                    }
                }
                db->execSql(sqls);
            }

            if(TcpServer::msg_queue.contains(103)) //motortype sync
            {
                QQueue<QString> sqls;
                while (TcpServer::msg_queue[103].size() > 0)
                {
                    QString info = TcpServer::msg_queue[103].dequeue();
                    QStringList items = info.split(',');

                    try
                    {
                        if(items.size() == 10)
                        {
                            QString code = items[0];
                            QString sql = "select model from index_motortype where model='" + code + "'";
                            row = db->querysqlcount(sql);
                            if(row==0)
                            {
                                sql = "insert into index_motortype (model,work_mode,power_rating,rated_voltage,rated_current,poleNums,center_height,factor,insulate,rotate) values ('";
                                sql += items[0] + "','" + items[1] + "'," + items[2] + "," + items[3] + "," + items[4] + "," + items[5] + "," + items[6] + "," + items[7] + "," + items[8] + "," + items[9] + ")";
                                sqls.enqueue(sql);
                            }
                        }
                    }
                    catch(QException e)
                    {

                    }
                }
                db->execSql(sqls);
            }

            if(TcpServer::msg_queue.contains(104)) //bearingtype sync
            {
                QQueue<QString> sqls;
                while (TcpServer::msg_queue[104].size() > 0)
                {
                    QString info = TcpServer::msg_queue[104].dequeue();
                    QStringList items = info.split(',');

                    try
                    {
                        if(items.size() >= 8)
                        {
                            QString code = items[0];
                            QString sql = "select model from index_bearingtype where model='" + code + "'";
                            row = db->querysqlcount(sql);
                            if(row==0)
                            {
                                sql = "insert into index_bearingtype (model,btype,rin,rout,contact_angle,bearpitch,rotated,rotaten) values ('";
                                sql += items[0] + "','" + items[1] + "'," + items[2] + "," + items[3] + "," + items[4] + "," + items[5] + "," + items[6] + "," + items[7] + ")";
                                sqls.enqueue(sql);
                            }
                        }
                    }
                    catch(QException e)
                    {

                    }
                }
                db->execSql(sqls);
            }

            if(TcpServer::msg_queue.contains(105)) //devicetype sync
            {
                QQueue<QString> sqls;
                while (TcpServer::msg_queue[105].size() > 0)
                {
                    QString info = TcpServer::msg_queue[105].dequeue();
                    QStringList items = info.split(',');

                    try
                    {
                        if(items.size() == 4)
                        {
                            QString code = items[0];
                            QString sql = "select model from index_devicetype where model='" + code + "'";
                            row = db->querysqlcount(sql);
                            if(row==0)
                            {
                                sql = "insert into index_devicetype (model,dtype,pipenum) values ('";
                                sql += items[0] + "','" + items[1] + "'," + items[2] + ")";
                                sqls.enqueue(sql);
                            }
                        }
                    }
                    catch(QException e)
                    {

                    }
                }
                db->execSql(sqls);
            }

            if(TcpServer::msg_queue.contains(106)) //devicepipes sync
            {
                QQueue<QString> sqls;
                while (TcpServer::msg_queue[106].size() > 0)
                {
                    QString info = TcpServer::msg_queue[106].dequeue();
                    QStringList items = info.split(',');

                    try
                    {
                        if(items.size() == 5)
                        {
                            QString code = items[0];
                            QString mcode = items[3];
                            QString sql = "select dcode,motor from index_devicepipes where dcode='" + code + "' and motor='" + mcode + "'";
                            row = db->querysqlcount(sql);
                            if(row==0)
                            {
                                sql = "insert into index_devicepipes (dcode,pipeCode,name,motor) values ('";
                                sql += items[0] + "','" + items[1] + "','" + items[2] + "','" + items[3] + "')";
                                sqls.enqueue(sql);
                            }
                        }
                    }
                    catch(QException e)
                    {

                    }
                }
                db->execSql(sqls);
            }

            //save info
            if(TcpServer::msg_queue.contains(1)) //charge measure
            {
                maxCount = 0;
                QQueue<QString> sqls;
                while (TcpServer::msg_queue[1].size() > 0)
                {
                    QString info = TcpServer::msg_queue[1].dequeue();
                    QStringList items = info.split(';');
                    QStringList::ConstIterator it=items.constBegin(),endit=items.constEnd();

                    try
                    {
                        row = 0;
                        for(;it != endit;it++)
                        {
                            QString item = *it;
                            if (item.length() > 0)
                            {
                                QStringList units = item.split('#');

                                if(units.size() == 13)
                                {
                                    QString sql = "insert into index_electriccharge (u,i,f,factor,p,q,s,others,pqs,dcode,mcode,pipe,rksj,wid) values (";
                                    sql += units[0] + "," + units[1] + "," + units[2] + "," + units[3] + "," + units[4] + "," + units[5] + "," + units[6];
                                    sql += ",'" + units[7] + "','" + units[8] + "','" + units[9] + "','" + units[10] + "'," + units[11] + ",'" + units[12] + "',0)";
                                    sqls.enqueue(sql);
                                }
                            }
                        }
                    }
                    catch(QException e)
                    {

                    }
                    maxCount++;
                    if (maxCount > maxExcueteNum)
                    {
                        break;
                    }
                }
                db->execSql(sqls);
            }

            if(TcpServer::msg_queue.contains(2)) //charge freq
            {
                row = 0;
                QMap<QString,QVariantList> binds;
                while (TcpServer::msg_queue[2].size() > 0)
                {
                    QString info = TcpServer::msg_queue[2].dequeue();
                    QStringList items = info.split(';');
                    QStringList::ConstIterator it=items.constBegin(),endit=items.constEnd();

                    try
                    {
                        for(;it != endit;it++)
                        {
                            QString item = *it;
                            if (item.length() > 0)
                            {
                                QStringList units = item.split('#');

                                if(units.size() == 6)
                                {
                                    if(binds.contains("sample_freq"))
                                    {
                                        binds["sample_freq"].append(units[0]);
                                    }
                                    else
                                    {
                                        QVariantList freqs;
                                        freqs.append(units[0]);
                                        binds.insert("sample_freq",freqs);
                                    }
                                    if(binds.contains("dcode"))
                                    {
                                        binds["dcode"].append(units[2]);
                                    }
                                    else
                                    {
                                        QVariantList dcodes;
                                        dcodes.append(units[2]);
                                        binds.insert("dcode",dcodes);
                                    }
                                    if(binds.contains("mcode"))
                                    {
                                        binds["mcode"].append(units[3]);
                                    }
                                    else
                                    {
                                        QVariantList mcodes;
                                        mcodes.append(units[3]);
                                        binds.insert("mcode",mcodes);
                                    }
                                    if(binds.contains("pipe"))
                                    {
                                        binds["pipe"].append(units[4]);
                                    }
                                    else
                                    {
                                        QVariantList pipes;
                                        pipes.append(units[4]);
                                        binds.insert("pipe",pipes);
                                    }
                                    if(binds.contains("rksj"))
                                    {
                                        binds["rksj"].append(units[5]);
                                    }
                                    else
                                    {
                                        QVariantList rksjs;
                                        rksjs.append(units[5]);
                                        binds.insert("rksj",rksjs);
                                    }
                                    if(binds.contains("stype"))
                                    {
                                        binds["stype"].append(units[1]);
                                    }
                                    else
                                    {
                                        QVariantList stypes;
                                        stypes.append(units[1]);
                                        binds.insert("stype",stypes);
                                    }
                                    if(binds.contains("wid"))
                                    {
                                        binds["wid"].append(0);
                                    }
                                    else
                                    {
                                        QVariantList wids;
                                        wids.append(0);
                                        binds.insert("wid",wids);
                                    }
                                }
                            }
                        }
                    }
                    catch(QException e)
                    {

                    }
                    maxCount++;
                    if (maxCount > maxExcueteNum)
                    {
                        break;
                    }
                }
                if(binds.size()>0)
                {
                    QString sql = "insert into index_electricchargewavefreq (sample_freq,dcode,mcode,pipe,rksj,stype,wid) values (:sample_freq,:dcode,:mcode,:pipe,:rksj,:stype,:wid)";
                    db->execSql(sql,binds);
                }
            }
            if(TcpServer::msg_queue.contains(3)) //charge wave
            {
                int tmp_count = 0;
                QMap<QString,QVariantList> datas;
                while (TcpServer::msg_queue[3].size() > 0)
                {
                    QString info = TcpServer::msg_queue[3].dequeue();

                    QStringList units = info.split('#');

                    try
                    {
                        //QSqlTableModel* model = db->modelNoHeader("index_electricchargewave");

                        if(units.size() == 6)
                        {
                            /*
                            model->insertRow(row);
                            model->setData(model->index(row,1),units[0]);
                            model->setData(model->index(row,2),units[1]);
                            model->setData(model->index(row,3),units[2]);
                            model->setData(model->index(row,4),units[3]);
                            model->setData(model->index(row,5),units[4]);
                            model->setData(model->index(row,6),units[5]);
                            model->setData(model->index(row,7),"0");
                            model->submitAll();
                            */

                            if(datas.contains("sample_data"))
                            {
                                datas["sample_data"].append(units[0]);
                            }
                            else
                            {
                                QVariantList waves;
                                waves.append(units[0]);
                                datas.insert("sample_data",waves);
                            }
                            if(datas.contains("stype"))
                            {
                                datas["stype"].append(units[1]);
                            }
                            else
                            {
                                QVariantList stypes;
                                stypes.append(units[1]);
                                datas.insert("stype",stypes);
                            }
                            if(datas.contains("dcode"))
                            {
                                datas["dcode"].append(units[2]);
                            }
                            else
                            {
                                QVariantList dcodes;
                                dcodes.append(units[2]);
                                datas.insert("dcode",dcodes);
                            }
                            if(datas.contains("mcode"))
                            {
                                datas["mcode"].append(units[3]);
                            }
                            else
                            {
                                QVariantList mcodes;
                                mcodes.append(units[3]);
                                datas.insert("mcode",mcodes);
                            }
                            if(datas.contains("pipe"))
                            {
                                datas["pipe"].append(units[4]);
                            }
                            else
                            {
                                QVariantList pipes;
                                pipes.append(units[4]);
                                datas.insert("pipe",pipes);
                            }
                            if(datas.contains("rksj"))
                            {
                                datas["rksj"].append(units[5]);
                            }
                            else
                            {
                                QVariantList rksjs;
                                rksjs.append(units[5]);
                                datas.insert("rksj",rksjs);
                            }
                            if(datas.contains("wid"))
                            {
                                datas["wid"].append(0);
                            }
                            else
                            {
                                QVariantList wids;
                                wids.append(0);
                                datas.insert("wid",wids);
                            }
                        }

                        //delete model;
                    }
                    catch(QException e)
                    {

                    }
                    tmp_count++;
                    if(tmp_count > 10)
                    {
                        break;
                    }
                }
                if(datas.size()>0)
                {
                    QString sql = "insert into index_electricchargewave (sample_data,dcode,mcode,pipe,rksj,stype,wid) values (:sample_data,:dcode,:mcode,:pipe,:rksj,:stype,:wid)";
                    db->execSql(sql,datas);
                }
            }
            if(TcpServer::msg_queue.contains(4)) //vibrate measure
            {
                maxCount = 0;
                QQueue<QString> sqls;
                while (TcpServer::msg_queue[4].size() > 0)
                {
                    QString info = TcpServer::msg_queue[4].dequeue();
                    QStringList items = info.split(';');
                    QStringList::ConstIterator it=items.constBegin(),endit=items.constEnd();

                    try
                    {
                        row = 0;
                        for(;it != endit;it++)
                        {
                            QString item = *it;
                            if (item.length() > 0)
                            {
                                QStringList units = item.split('#');

                                if(units.size() == 6)
                                {
                                    QString sql = "insert into index_vibrate (vibrate_e,speed_e,dcode,mcode,pipe,rksj,wid) values (";
                                    sql += units[0] + "," + units[1] + ",'" + units[2] + "','" + units[3] + "'," + units[4] + ",'" + units[5] + "',0)";
                                    sqls.enqueue(sql);
                                }
                            }
                        }
                    }
                    catch(QException e)
                    {

                    }
                    maxCount++;
                    if (maxCount > maxExcueteNum)
                    {
                        break;
                    }
                }
                if(sqls.size()>0)
                {
                    db->execSql(sqls);
                }
            }
            if(TcpServer::msg_queue.contains(5)) //vibrate freq
            {
                row = 0;
                QMap<QString,QVariantList> binds;

                while (TcpServer::msg_queue[5].size() > 0)
                {
                    QString info = TcpServer::msg_queue[5].dequeue();
                    QStringList items = info.split(';');
                    QStringList::ConstIterator it=items.constBegin(),endit=items.constEnd();

                    try
                    {
                        for(;it != endit;it++)
                        {
                            QString item = *it;
                            if (item.length() > 0)
                            {
                                QStringList units = item.split('#');

                                if(units.size() == 6)
                                {
                                    if(binds.contains("sample_freq"))
                                    {
                                        binds["sample_freq"].append(units[0]);
                                    }
                                    else
                                    {
                                        QVariantList freqs;
                                        freqs.append(units[0]);
                                        binds.insert("sample_freq",freqs);
                                    }
                                    if(binds.contains("dcode"))
                                    {
                                        binds["dcode"].append(units[2]);
                                    }
                                    else
                                    {
                                        QVariantList dcodes;
                                        dcodes.append(units[2]);
                                        binds.insert("dcode",dcodes);
                                    }
                                    if(binds.contains("mcode"))
                                    {
                                        binds["mcode"].append(units[3]);
                                    }
                                    else
                                    {
                                        QVariantList mcodes;
                                        mcodes.append(units[3]);
                                        binds.insert("mcode",mcodes);
                                    }
                                    if(binds.contains("pipe"))
                                    {
                                        binds["pipe"].append(units[4]);
                                    }
                                    else
                                    {
                                        QVariantList pipes;
                                        pipes.append(units[4]);
                                        binds.insert("pipe",pipes);
                                    }
                                    if(binds.contains("rksj"))
                                    {
                                        binds["rksj"].append(units[5]);
                                    }
                                    else
                                    {
                                        QVariantList rksjs;
                                        rksjs.append(units[5]);
                                        binds.insert("rksj",rksjs);
                                    }
                                    if(binds.contains("stype"))
                                    {
                                        binds["stype"].append(units[1]);
                                    }
                                    else
                                    {
                                        QVariantList stypes;
                                        stypes.append(units[1]);
                                        binds.insert("stype",stypes);
                                    }
                                    if(binds.contains("wid"))
                                    {
                                        binds["wid"].append(0);
                                    }
                                    else
                                    {
                                        QVariantList wids;
                                        wids.append(0);
                                        binds.insert("wid",wids);
                                    }
                                }
                            }
                        }
                    }
                    catch(QException e)
                    {

                    }
                    maxCount++;
                    if (maxCount > maxExcueteNum)
                    {
                        break;
                    }
                }
                if(binds.size() > 0)
                {
                    QString sql = "insert into index_vibratewavefreq (sample_freq,dcode,mcode,pipe,rksj,stype,wid) values (:sample_freq,:dcode,:mcode,:pipe,:rksj,:stype,:wid)";
                    db->execSql(sql,binds);
                }
            }
            if(TcpServer::msg_queue.contains(6)) //vibrate wave
            {
                int tmp_count = 0;
                QMap<QString,QVariantList> datas;
                while (TcpServer::msg_queue[6].size() > 0)
                {
                    QString info = TcpServer::msg_queue[6].dequeue();

                    QStringList units = info.split('#');

                    try
                    {
                        //QSqlTableModel* model = db->modelNoHeader("index_vibratewave");
                        if(units.size() == 6)
                        {
                            /*
                            model->insertRow(row);
                            model->setData(model->index(row,1),units[0]);
                            model->setData(model->index(row,2),units[1]);
                            model->setData(model->index(row,3),units[2]);
                            model->setData(model->index(row,4),units[3]);
                            model->setData(model->index(row,5),units[4]);
                            model->setData(model->index(row,6),units[5]);
                            model->setData(model->index(row,7),"0");
                            model->submitAll();
                            */

                            if(datas.contains("sample_data"))
                            {
                                datas["sample_data"].append(units[0]);
                            }
                            else
                            {
                                QVariantList waves;
                                waves.append(units[0]);
                                datas.insert("sample_data",waves);
                            }
                            if(datas.contains("stype"))
                            {
                                datas["stype"].append(units[1]);
                            }
                            else
                            {
                                QVariantList stypes;
                                stypes.append(units[1]);
                                datas.insert("stype",stypes);
                            }
                            if(datas.contains("dcode"))
                            {
                                datas["dcode"].append(units[2]);
                            }
                            else
                            {
                                QVariantList dcodes;
                                dcodes.append(units[2]);
                                datas.insert("dcode",dcodes);
                            }
                            if(datas.contains("mcode"))
                            {
                                datas["mcode"].append(units[3]);
                            }
                            else
                            {
                                QVariantList mcodes;
                                mcodes.append(units[3]);
                                datas.insert("mcode",mcodes);
                            }
                            if(datas.contains("pipe"))
                            {
                                datas["pipe"].append(units[4]);
                            }
                            else
                            {
                                QVariantList pipes;
                                pipes.append(units[4]);
                                datas.insert("pipe",pipes);
                            }
                            if(datas.contains("rksj"))
                            {
                                datas["rksj"].append(units[5]);
                            }
                            else
                            {
                                QVariantList rksjs;
                                rksjs.append(units[5]);
                                datas.insert("rksj",rksjs);
                            }
                            if(datas.contains("wid"))
                            {
                                datas["wid"].append(0);
                            }
                            else
                            {
                                QVariantList wids;
                                wids.append(0);
                                datas.insert("wid",wids);
                            }
                        }

                        //delete model;
                    }
                    catch(QException e)
                    {

                    }
                    tmp_count++;
                    if(tmp_count > 10)
                    {
                        break;
                    }
                }
                if(datas.size()>0)
                {
                    QString sql = "insert into index_vibratewave (sample_data,dcode,mcode,pipe,rksj,stype,wid) values (:sample_data,:dcode,:mcode,:pipe,:rksj,:stype,:wid)";
                    db->execSql(sql,datas);
                }
            }
            if(TcpServer::msg_queue.contains(7)) //temperature measure
            {
                maxCount = 0;
                QQueue<QString> sqls;
                while (TcpServer::msg_queue[7].size() > 0)
                {
                    QString info = TcpServer::msg_queue[7].dequeue();
                    QStringList items = info.split(';');
                    QStringList::ConstIterator it=items.constBegin(),endit=items.constEnd();

                    try
                    {
                        row = 0;
                        for(;it != endit;it++)
                        {
                            QString item = *it;
                            if (item.length() > 0)
                            {
                                QStringList units = item.split('#');

                                if(units.size() == 5)
                                {
                                    QString sql = "insert into index_temperature (temp,dcode,mcode,pipe,rksj,wid) values (";
                                    sql += units[0] + ",'" + units[1] + "','" + units[2] + "'," + units[3] + ",'" + units[4] + "',0)";
                                    sqls.enqueue(sql);
                                }
                            }
                        }
                    }
                    catch(QException e)
                    {

                    }
                    maxCount++;
                    if (maxCount > maxExcueteNum)
                    {
                        break;
                    }
                }
                if(sqls.size()>0)
                {
                    db->execSql(sqls);
                }
            }

            if(TcpServer::msg_queue.contains(80)) //diagnose info
            {
                maxCount = 0;
                QQueue<QString> sqls;
                while (TcpServer::msg_queue[80].size() > 0)
                {
                    QString info = TcpServer::msg_queue[80].dequeue();
                    //QStringList items = info.split(';');
                    //QStringList::ConstIterator it=items.constBegin(),endit=items.constEnd();

                    try
                    {
                        QStringList units = info.split('#');

                        if(units.size() == 3)
                        {
                            QString sql = "insert into index_motordiagnoseinfo (mcode,diagnoseinfo,rksj,wid) values ('";
                            sql += units[0] + "','" + units[1] + "','" + units[2] + "',0)";
                            sqls.enqueue(sql);
                        }
                    }
                    catch(QException e)
                    {

                    }
                    maxCount++;
                    if (maxCount > maxExcueteNum)
                    {
                        break;
                    }
                }
                if(sqls.size()>0)
                {
                    db->execSql(sqls);
                }
            }
        }

        sleep(2);
    }
}
