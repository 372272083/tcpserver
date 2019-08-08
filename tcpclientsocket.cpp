#include "tcpclientsocket.h"

#include "tcpserver.h"

#include <QPixmap>
#include <QDateTime>
#include <QBuffer>
#include <QImageReader>

#include "sqlserverdb.h"

#include <QDebug>
#include <QTimer>
#include <QException>

TcpClientSocket::TcpClientSocket(SqlserverDB *db,QObject *parent) : mdb(db)
{
    connect(this,SIGNAL(readyRead()),this,SLOT(dataReceived()));
    connect(this,SIGNAL(disconnected()),this,SLOT(slotDisconnected()));

    expLen = 0;
    msgType = 0;
    clientName = "";
    m_code = "";
    project_id = 0;

    datastate = false;

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerInterval()));
    timer->start(1500);
}

void TcpClientSocket::timerInterval()
{
    datastate = !datastate;
}

void TcpClientSocket::dataReceived()
{
    while(bytesAvailable()>0)
    {
        if(project_id > 0 && datastate)
        {
            QString sql = "select top 1 id,projectId,msgType,msg,state,rksj from index_msgtransfer where state=1 and projectId = " + QString::number(project_id) + " order by rksj desc";

            bool tok;
            QVector<QString> datas = mdb->query(sql);
            QVector<QString>::ConstIterator it;
            for(it=datas.constBegin();it!=datas.constEnd();it++)
            {
                QString data = *it;
                QStringList data_items = data.split(',');
                if(data_items.size() >= 6)
                {
                    QString tmp = data_items[0];
                    int id = tmp.toInt(&tok);
                    tmp = data_items[2];
                    int msg_type = tmp.toInt(&tok);
                    tmp = data_items[3];
                    QString msg = tmp;
                    tmp = data_items[5];
                    QString rksj = tmp;

                    QString r_msg;
                    QDateTime rksj_time = QDateTime::fromString(rksj,"yyyy-MM-dd hh:mm:ss");
                    QDateTime cur_time = QDateTime::currentDateTime();
                    if(qAbs(rksj_time.secsTo(cur_time)) < 10)
                    {
                        switch (msg_type) {
                        case 1:
                            r_msg = "1#recordwave";
                            write(r_msg.toUtf8());
                            flush();
                            break;
                        default:
                            break;
                        }
                    }

                    sql = "update index_msgtransfer set state=0 where id=" + QString::number(id);
                    mdb->updatasql(sql);
                }
            }
        }
        int length = bytesAvailable();
        //char buf[8192];
        QByteArray buf = read(8192);
        //read(buf,length);

        //QString msg=buf;
        if (length > 0)
        {
            if (buf[0] == 0 && buf[1] == 0xFF && buf[2] == 0xFF)
            {
                msgType = buf[5];
                unsigned char len1 = buf[3];
                unsigned char len2 = buf[4];
                expLen = len1 * 0xFF + len2;
                int tmp = buf.size();
                for(int i=6;i<tmp;i++)
                {
                    buffer.append(buf[i]);
                }
            }
            else
            {
                buffer.append(buf);
            }

            if (expLen == buffer.size())
            {
                QByteArray uncompress_buffer;
                QString msg="";

                uncompress_buffer = qUncompress(buffer);
                if(msgType != 127)
                {
                    msg = QString::fromUtf8(uncompress_buffer,uncompress_buffer.size());
                }

                QString showMsg;
                switch (msgType) {
                case 0: //teamViewer
                    showMsg = "teamViewer password";
                    break;
                case 1: //charge
                    showMsg = "charge measure";
                    break;
                case 2: //charge freq
                    showMsg = "charge freq";
                    break;
                case 3: //charge wave
                    showMsg = "charge wave";
                    break;
                case 4: //vibrate
                    showMsg = "vibrate measure";
                    break;
                case 5: //vibrate freq
                    showMsg = "vibrate freq";
                    break;
                case 6: //vibrate wave
                    showMsg = "vibrate wave";
                    break;
                case 7: //temperature
                    showMsg = "temperature measure";
                    break;
                case 101: //init
                    showMsg = "init";
                    clientName = msg;
                    break;
                case 80: //diagnose
                    showMsg = "diagnose info";
                    break;
                case 127: //component data
                    showMsg = "component info";
                    break;
                default:
                    break;
                }

                showMsg += "-" + QString::number(msgType);

                if (msgType == 0)
                {
                    QBuffer imgbuffer(&uncompress_buffer);
                    imgbuffer.open(QIODevice::ReadOnly);
                    QImageReader reader(&imgbuffer,"bmp");
                    QImage img = reader.read();
                    if(!img.isNull()){

                        QString imgName = "D:/bmp_tmp.bmp";
                        if (clientName.length()>0)
                            imgName = "D:/bmp_" + clientName + ".bmp";
                        img.save(imgName);
                        qDebug() << imgName;
                    }

                    //TcpServer::msg_queue.enqueue(msg);
                    expLen = 0;
                    buffer.clear();
                    write(QString::number(msgType).toUtf8());
                    emit updateClients(showMsg,showMsg.length());
                    continue;
                }
                else if (msgType == 101)
                {
                    //TcpServer::msg_queue.enqueue(msg);
                    expLen = 0;
                    buffer.clear();
                    write(QString::number(msgType).toUtf8());
                    emit updateClients(showMsg,showMsg.length());
                    continue;
                }
                else if(102 == msgType)
                {
                    QString data = "sync";
                    //QString return_data = msgType + "#" + data;
                    //write(return_data.toUtf8());
                    //continue;
                    QStringList items = msg.split('#');
                    QStringList::ConstIterator it=items.constBegin(),endit=items.constEnd();
                    emit updateClients(data,data.length());
                    int sub_type = 0;
                    QString i_name = "";
                    for(;it != endit;it++)
                    {
                        QString item = *it;
                        if (item.trimmed().length() > 0)
                        {
                            QStringList datas = item.split(':');

                            if(datas.size() == 2)
                            {
                                bool tok;
                                QString dataType_str = datas[0];
                                int dataType = dataType_str.toInt(&tok);
                                sub_type = dataType;

                                dataType += 100;

                                if(tok)
                                {
                                    QString data = datas[1];
                                    QStringList dataitems = data.split(';');
                                    QStringList::ConstIterator inIt=dataitems.constBegin(),inEndIt=dataitems.constEnd();
                                    for(;inIt != inEndIt;inIt++)
                                    {
                                        QStringList items;
                                        QString dataitem = *inIt;
                                        switch (sub_type) {
                                        case 1:
                                            items = dataitem.split(',');
                                            try
                                            {
                                                if(items.size() >= 5)
                                                {
                                                    QString code = items[0];
                                                    QString sql = "select mcode from index_motor where mcode='" + code + "'";
                                                    int row = mdb->querysqlcount(sql);
                                                    if(row > 0)
                                                    {
                                                        i_name = code;
                                                        break;
                                                    }
                                                }
                                            }
                                            catch(QException e)
                                            {

                                            }
                                            break;
                                        case 2:
                                            items = dataitem.split(',');
                                            try
                                            {
                                                if(items.size() >= 6)
                                                {
                                                    QString code = items[0];
                                                    QString sql = "select code from index_device where code='" + code + "'";
                                                    int row = mdb->querysqlcount(sql);
                                                    if(row > 0)
                                                    {
                                                        i_name = code;
                                                        break;
                                                    }
                                                }
                                            }
                                            catch(QException e)
                                            {

                                            }
                                            break;
                                        default:
                                            break;
                                        }
                                    }

                                    ///////////////// ready write to database
                                    /*
                                    inIt=dataitems.constBegin(),inEndIt=dataitems.constEnd();
                                    for(;inIt != inEndIt;inIt++)
                                    {
                                        QString dataitem = *inIt;
                                        if (TcpServer::msg_queue.contains(dataType))
                                        {
                                            TcpServer::msg_queue[dataType].enqueue(dataitem);
                                        }
                                        else
                                        {
                                            QQueue<QString> dataqueue;
                                            dataqueue.enqueue(dataitem);
                                            TcpServer::msg_queue[dataType] = dataqueue;
                                        }
                                    }
                                    */
                                }
                            }
                        }
                    }

                    expLen = 0;
                    buffer.clear();
                    QString return_msg = QString::number(msgType) + "#sync#" + QString::number(sub_type);
                    if(i_name.length() > 0)
                    {
                        return_msg += "#1#" + i_name;
                    }
                    else
                    {
                        return_msg += "#0#";
                    }
                    write(return_msg.toUtf8());
                    continue;
                }
                else if(127 == msgType)
                {
                    int acc_len = 20;
                    for(int n=0;n<10;n++)
                    {
                        unsigned char s_len1 = uncompress_buffer[n*2];
                        unsigned char s_len2 = uncompress_buffer[n*2+1];
                        int i_len = s_len1 * 0xFF + s_len2;
                        if(i_len == 0)
                        {
                            break;
                        }

                        QByteArray i_buffer = uncompress_buffer.mid(acc_len,i_len);
                        int i_msgType = i_buffer[5];
                        int tmp = i_buffer.size();
                        QByteArray i_buffer_buffer;
                        for(int i=6;i<tmp;i++)
                        {
                            i_buffer_buffer.append(i_buffer[i]);
                        }
                        QByteArray i_uncompress_buffer = qUncompress(i_buffer_buffer);
                        msg = QString::fromUtf8(i_uncompress_buffer,i_uncompress_buffer.size());
                        if(i_msgType == 1 || i_msgType == 4 || i_msgType == 7)
                        {
                            if (TcpServer::msg_queue.contains(i_msgType))
                            {
                                TcpServer::msg_queue[i_msgType].enqueue(msg);
                            }
                            else
                            {
                                QQueue<QString> fqueue;
                                fqueue.enqueue(msg);
                                TcpServer::msg_queue[i_msgType] = fqueue;
                            }
                        }
                        acc_len += i_len;
                    }

                    QString data = "component info";
                    emit updateClients(data,data.length());
                    expLen = 0;
                    buffer.clear();
                    write(QString::number(msgType).toUtf8());
                    continue;
                }

                if(msgType == 80 && m_code.length() == 0)
                {
                    QString item = msg;
                    if (item.length() > 0)
                    {
                        QStringList units = item.split('#');

                        if(units.size() == 3)
                        {
                            m_code = units[0];
                            QString sql = "select project from index_motor where mcode = '" + m_code + "'";
                            QVector<QString> projects = mdb->query(sql);
                            QVector<QString>::ConstIterator it;
                            for(it=projects.constBegin();it!=projects.constEnd();it++)
                            {
                                QString items = *it;
                                QStringList item_data = items.split(',');
                                if(item_data.size() >=1 )
                                {
                                    bool tok;
                                    QString tmp = item_data[0];
                                    project_id = tmp.toInt(&tok);
                                }
                                break;
                            }
                        }
                    }
                }
                if (TcpServer::msg_queue.contains(msgType))
                {
                    TcpServer::msg_queue[msgType].enqueue(msg);
                }
                else
                {
                    QQueue<QString> fqueue;
                    fqueue.enqueue(msg);
                    TcpServer::msg_queue[msgType] = fqueue;
                }

                //TcpServer::msg_queue.enqueue(msg);
                expLen = 0;
                buffer.clear();
                write(QString::number(msgType).toUtf8());
                QString itemName;
                if(clientName.length() > 0)
                {
                    itemName = clientName + "-" + this->peerAddress().toString();
                }
                else
                {
                    itemName = "No-" +this->peerAddress().toString();
                }
                QDateTime dt = QDateTime::currentDateTime();
                QString dt_str = dt.toString("yyyy-MM-dd HH:mm:ss");
                showMsg = dt_str + ": " + itemName + " - " + showMsg;
                emit updateClients(showMsg,showMsg.length());
            }
        }
    }
}

void TcpClientSocket::addMsg(QString msg)
{
    msgs.push_back(msg);
}

void TcpClientSocket::slotDisconnected()
{
    emit disconnected(this->socketDescriptor());
    expLen = 0;
    buffer.clear();
}
