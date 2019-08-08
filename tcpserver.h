#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QDialog>
#include <QListWidget>
#include <QLabel>
#include <QLineEdit>
#include <QQueue>
#include <QMap>
#include <QPushButton>
#include "server.h"
#include "sqlserverdb.h"
#include "savesamplethread.h"
//#include "handlermsgthread.h"
#include "analysethread.h"

class QCheckBox;

class TcpServer : public QDialog
{
    Q_OBJECT
    
public:
    TcpServer(QWidget *parent = 0,Qt::WindowFlags f=0);
    ~TcpServer();    

    static QMap<int,QQueue<QString>> msg_queue;
    static bool enable_analyse;

private:
    QListWidget *ContentListWidget;
    QLabel *PortLabel;
    QLineEdit *PortLineEdit;
    QPushButton *CreateBtn;
    QListWidget *clientWidget;
    QCheckBox *isAnalyse;
    int port;
    Server *server;

    SaveSampleThread* saveThread;
    AnalyseThread* analyseThread;
    //HandlerMsgThread* handlerMsgThread;

    SqlserverDB *db;
public slots:
    void slotCreateServer();
    void updateServer(QString,int);

    void timeCheck();
    void startServer();
    void updateClientList();

    void checkChange();
};

#endif // TCPSERVER_H
