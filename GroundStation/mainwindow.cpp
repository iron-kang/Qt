#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <stdio.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_view = new QWebEngineView(ui->map);
    QWebEnginePage *page = m_view->page();
    page->load(QUrl(QStringLiteral("https://maps.google.com")));

    m_client = new QTcpSocket(this);
    connect(m_client,SIGNAL(readyRead()),this,SLOT(readyRead()));
    connect(m_client,SIGNAL(connected()),this,SLOT(connected()));
    connect(m_client,SIGNAL(disconnected()),this,SLOT(disConnected()));

    isConnect = false;
    tim_info = new QTimer(this);
    connect(tim_info, SIGNAL(timeout()), this, SLOT(getInfo()));

    QString ip_address="123.3.2.1";
    m_client->connectToHost(ip_address, 80);
    tim_info->start(1000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::readyRead()
{
    printf("ready read\n");
}

void MainWindow::connected()
{
    printf("connect\n");
}

void MainWindow::disConnected()
{
    printf("disconnect\n");
}

void MainWindow::getInfo()
{
    static int cnt = 0;
    char cmd[50];
    cmd[0] = '@';
    cmd[1] = '#';
//    const char *str = "Hello Qt!";
//    memcpy(&cmd[2], str, strlen(str));

    m_client->write(cmd, 2);//+0*strlen(str));
    ui->pitch->setText(QString::number(cnt++));
}

void MainWindow::on_btn_connect_clicked()
{
    if (!isConnect)
    {
        tim_info->start(1000);
        QString ip_address="123.3.2.1";
        m_client->connectToHost(ip_address, 80);
        ui->btn_connect->setText("Disconnect");
        isConnect = true;
    }
    else {
        isConnect = false;
        tim_info->stop();
        m_client->disconnected();
        m_client->close();
        ui->btn_connect->setText("Connect");
    }
}
