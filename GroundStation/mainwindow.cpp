#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <stdio.h>
#include <QDebug>
#include "datatype.h"
#include <iostream>

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_scene = new QGraphicsScene();

    m_view = new QGraphicsView(parent);

    m_web = new QWebEngineView(ui->map);
//    QGraphicsProxyWidget *proxy __attribute__((__unused__)) = m_scene->addWidget(this);

    UI_Init();
    m_page = m_web->page();
    m_web->move(-100, 0);
//    page->load(QUrl(QStringLiteral("https://maps.google.com")));

    m_client = new QTcpSocket(this);
    connect(m_client,SIGNAL(readyRead()),this,SLOT(readyRead()));
    connect(m_client,SIGNAL(connected()),this,SLOT(connected()));
    connect(m_client,SIGNAL(disconnected()),this,SLOT(disConnected()));

    imuPitchPix = QPixmap(*ui->icon_pitch->pixmap());
    imuRollPix = QPixmap(*ui->icon_roll->pixmap());

    isConnect = false;
    tim_info = new QTimer(this);
    connect(tim_info, SIGNAL(timeout()), this, SLOT(getInfo()));

    QTimer *tim_map = new QTimer(this);
    connect(tim_map, SIGNAL(timeout()), this, SLOT(updateMap()));

//    m_view->setScene(m_scene);
//    m_view->showFullScreen();
//    m_view->setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
//    m_view->setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );

    QString ip_address="192.168.1.1";
    m_client->connectToHost(ip_address, 80);
    tim_info->start(100);
//    tim_map->start(2000);
    updateMap();

}

MainWindow::~MainWindow()
{
    cout<<"deconstruct"<<endl;
    m_client->disconnectFromHost();
    m_client->close();
    delete m_client;
    delete ui;
}

void MainWindow::UI_Init()
{
    ui->txt_pitch->setStyleSheet("QLabel {color:rgb(255, 128, 0);background-color:rgba(0, 0, 0, 0)}");
    ui->txt_roll->setStyleSheet("QLabel {color:rgb(255, 128, 0);background-color:rgba(0, 0, 0, 0)}");
    ui->txt_yaw->setStyleSheet("QLabel {color:rgb(255, 128, 0);background-color:rgba(0, 0, 0, 0)}");
}

void MainWindow::readyRead()
{
    char *ret;
    Info info;

    ret = m_client->readAll().data();
    memcpy(&info, ret, sizeof(Info));
//    printf("roll: %f, pitch: %f, Pitch: %f\n", info.attitude.x, info.attitude.y, info.attitude.z);
    ui->txt_roll->setText(QString::number(info.attitude.x, 'f', 2));
    ui->txt_pitch->setText(QString::number(info.attitude.y, 'f', 2));

    QPixmap mapImuPitch(imuPitchPix.size());
    mapImuPitch.fill(Qt::transparent);
    QPainter p_imuPitch(&mapImuPitch);

    QPixmap mapImuRoll(imuRollPix.size());
    mapImuRoll.fill(Qt::transparent);
    QPainter p_imuRoll(&mapImuRoll);

    p_imuPitch.translate(128, 128);
    p_imuPitch.rotate(info.attitude.y);
    p_imuPitch.translate(-128, -128);
    p_imuPitch.drawPixmap(0, 0, imuPitchPix);
    ui->icon_pitch->setPixmap(mapImuPitch);

    p_imuRoll.translate(128, 128);
    p_imuRoll.rotate(info.attitude.x);
    p_imuRoll.translate(-128, -128);
    p_imuRoll.drawPixmap(0, 0, imuRollPix);
    ui->icon_roll->setPixmap(mapImuRoll);

}

void MainWindow::connected()
{
    printf("connect\n");
}

void MainWindow::disConnected()
{
    printf("disconnect\n");
}

void MainWindow::updateMap()
{
    m_page->load(QUrl(QStringLiteral("https://maps.googleapis.com/maps/api/staticmap?center=22.6054185,120.3262534&zoom=16"
                                     "&size=600x600&maptype=roadmap"
                                   "&markers=color:red%7Clabel:H%7C22.6054185,120.3262534"
                                   "&key=AIzaSyBc8rZgqD1Q4S84lPYuHpyoaQNMl0Bw4Tk")));
}

void MainWindow::action(char act)
{
    m_sockMutex.lock();
    cmd[0] = '@';
    cmd[1] = '#';
    cmd[2] = act;

    m_client->write(cmd, 3);
    m_client->waitForBytesWritten();
    m_sockMutex.unlock();
}

void MainWindow::getInfo()
{
    action('A');
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

void MainWindow::on_horizontalSlider_sliderMoved(int position)
{
    char cmd[500];
    sprintf(cmd,
    "https://maps.googleapis.com/maps/api/staticmap?center=22.6054185,120.3262534&zoom=%d"
                                         "&size=600x600&maptype=roadmap"
                                       "&markers=color:red%%7Clabel:H%%7C22.6054185,120.3262534"
                                       "&key=AIzaSyBc8rZgqD1Q4S84lPYuHpyoaQNMl0Bw4Tk", position);
    m_page->load(QUrl(QString(cmd)));
}

void MainWindow::on_btn_up_clicked()
{
    action('a');
}
