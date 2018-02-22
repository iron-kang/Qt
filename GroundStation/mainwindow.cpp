#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_view = new QWebEngineView(ui->map);
    QWebEnginePage *page = m_view->page();
    page->load(QUrl(QStringLiteral("https://maps.google.com")));

    m_client = new QTcpSocket(this);
    connect(m_client,SIGNAL(readyRead()),this,SLOT(slotReadyRead()));
    connect(m_client,SIGNAL(connected()),this,SLOT(slotConnected()));
    connect(m_client,SIGNAL(disconnected()),this,SLOT(slotDisConnected()));

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(getInfo()));
    timer->start(1000);

    QString ip_address="123.3.2.1";
    m_client->connectToHost(ip_address, 80);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::getInfo()
{
    char cmd[50];
    cmd[0] = '@';
    cmd[1] = '#';
    const char *str = "Hello Qt!";
    memcpy(&cmd[2], str, strlen(str));

    m_client->write(cmd, 2+strlen(str));
}
