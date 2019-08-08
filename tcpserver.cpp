#include "tcpserver.h"
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>

#include <QTimer>
#include <QSqlTableModel>

QMap<int,QQueue<QString>> TcpServer::msg_queue;
bool TcpServer::enable_analyse = false;

TcpServer::TcpServer(QWidget *parent,Qt::WindowFlags f)
    : QDialog(parent,f)
{
    setWindowTitle(tr("MFDS Server"));
    this->setMinimumSize(600,400);

    ContentListWidget = new QListWidget;
    clientWidget = new QListWidget;

    PortLabel = new QLabel(tr("接听端口："));
    PortLineEdit = new QLineEdit;

    CreateBtn = new QPushButton(tr("启动数据服务"));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *header = new QHBoxLayout();
    header->addWidget(PortLabel);
    header->addWidget(PortLineEdit);
    header->addWidget(CreateBtn);
    header->addStretch();

    isAnalyse = new QCheckBox(this);
    isAnalyse->setText(tr("Analyse data"));
    connect(isAnalyse, SIGNAL(clicked()), this, SLOT(checkChange()));
    header->addWidget(isAnalyse);
    if(enable_analyse)
    {
        isAnalyse->setChecked(true);
    }
    else
    {
        isAnalyse->setChecked(false);
    }

    mainLayout->addLayout(header);
    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->addWidget(clientWidget);
    clientWidget->setFixedWidth(200);

    contentLayout->addWidget(ContentListWidget);
    mainLayout->addLayout(contentLayout);

    port=65134;
    PortLineEdit->setText(QString::number(port));

    db = new SqlserverDB("mfds","10.10.101.58","sa","CMIE@cmie");
    if(db->open())
    {
        //createtables();
    }
    else
    {
        QMessageBox::information(this, tr("Infomation"), tr("Read config file failed,application will exit!"));
        this->close();  //读取配置文件失败
    }

    server = nullptr;
    connect(CreateBtn,SIGNAL(clicked()),this,SLOT(slotCreateServer()));

    QTimer::singleShot(10*1000, this, SLOT(startServer()));

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timeCheck()));
    timer->start(1000);

    saveThread = new SaveSampleThread(db);
    saveThread->start();

    analyseThread = nullptr;
    //handlerMsgThread = nullptr;
    if(enable_analyse)
    {
        analyseThread = new AnalyseThread(db);
        analyseThread->start();
    }
}

void TcpServer::checkChange()
{
    bool state = isAnalyse->isChecked();
    if(state)
    {
        enable_analyse = true;
        qDebug() << "true";
    }
    else
    {
        enable_analyse = false;
        qDebug() << "false";
    }

    if(enable_analyse)
    {
        analyseThread = new AnalyseThread(db);
        analyseThread->start();
    }
    else
    {
        analyseThread->terminate();
        analyseThread->wait();
        analyseThread = nullptr;
    }
}

void TcpServer::timeCheck()
{
    static int count = 0;//sub windows
    static int dbServerCount = 0;
    if(!db->status())
    {
        if (dbServerCount < count)
        {
            dbServerCount = count + 30;
            db->reConnect();
            db->open();

        }
        count++;
    }
    else
    {
        count=0;
    }
}

TcpServer::~TcpServer()
{
    saveThread->terminate();
    saveThread->wait();
    saveThread = nullptr;

    if(nullptr != analyseThread)
    {
        analyseThread->terminate();
        analyseThread->wait();
        analyseThread = nullptr;
    }

    //sendThread->terminate();
    //sendThread->wait();
    if(nullptr != server)
    {
        server->closeServer();
        delete server;
    }

    db->close();
    delete db;
}

void TcpServer::startServer()
{
    if(nullptr == server)
    {
        //启动服务器
        server = new Server(db,this,port);
        //检测更新服务器信号
        connect(server,SIGNAL(updateServer(QString,int)),this,SLOT(updateServer(QString,int)));
        connect(server,SIGNAL(updateClientList()),this,SLOT(updateClientList()));

        CreateBtn->setEnabled(false);

        //handlerMsgThread = new HandlerMsgThread(db,server);
        //handlerMsgThread->start();
    }
}

//创建数据服务
void TcpServer::slotCreateServer()
{
    //启动服务器
    server = new Server(db,this,port);
    //检测更新服务器信号
    connect(server,SIGNAL(updateServer(QString,int)),this,SLOT(updateServer(QString,int)));
    connect(server,SIGNAL(updateClientList()),this,SLOT(updateClientList()));

    CreateBtn->setEnabled(false);

    //handlerMsgThread = new HandlerMsgThread(db,server);
    //handlerMsgThread->start();
}

//更新客服端列表
void TcpServer::updateClientList()
{
    clientWidget->clear();
    QList<TcpClientSocket*> clients = server->tcpClientSocketList;
    QList<TcpClientSocket*>::const_iterator it;
    for(it=clients.constBegin();it!=clients.constEnd();it++)
    {
        TcpClientSocket* client = *it;
        QString itemName;
        if(client->clientName.length() > 0)
        {
            itemName = client->clientName + "-" + client->peerAddress().toString();
        }
        else
        {
            itemName = "No-" +client->peerAddress().toString();
        }
        clientWidget->addItem(itemName);
    }
}

//更新服务器
void TcpServer::updateServer(QString showmsg,int length)
{
    ContentListWidget->addItem(showmsg.left(length));
    if (ContentListWidget->count() > 500)
    {
        ContentListWidget->clear();
    }
//    PortLineEdit->setText(msg);
}
