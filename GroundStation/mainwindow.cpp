#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <stdio.h>
#include <QDebug>
#include <iostream>
#include "glwidget.h"
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QBoxLayout>
#include <QDebug>

#define SAMPLENUM 50

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
//    this->tabew->setStyleSheet("QDialog{background-color:rgba(10, 10, 10, 100);border: 2px outset cyan;border-radius:3px;}");
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
    timer_info = new QTimer(this);
    connect(timer_info, SIGNAL(timeout()), this, SLOT(getInfo()));

    QTimer *timer_map = new QTimer(this);
    connect(timer_map, SIGNAL(timeout()), this, SLOT(updateMap()));

    timer_thrust = new QTimer(this);
    connect(timer_thrust, SIGNAL(timeout()), this, SLOT(pollThrust()));

//    m_view->setScene(m_scene);
//    m_view->showFullScreen();
//    m_view->setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
//    m_view->setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );

//    QString ip_address="192.168.1.1";
//    m_client->connectToHost(ip_address, 80);
//    timer_info->start(100);
//    tim_map->start(2000);
    updateMap();

    devWatcher = new QFileSystemWatcher(this);
    devWatcher->addPath("/dev/input");
    connect(devWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(showModifiedDirectory(QString)));

    joysticThread = new JoysticThread(this);
    connect(joysticThread, SIGNAL(thrustEvent(char, char)), this, SLOT(thrustHandle(char, char)));
    connect(joysticThread, SIGNAL(connectNet()), this, SLOT(on_btn_connect_clicked()));
    connect(joysticThread, SIGNAL(js_motorLock()), this, SLOT(lockMotor()));

    connect(ui->btn_setpid, SIGNAL(clicked()), this, SLOT(updatePID()));
    connect(ui->btn_reboot, SIGNAL(clicked()), this, SLOT(rebootUAV()));

    islock = false;
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
    ui->txt_pitch->setStyleSheet("QLabel {color:rgb(255, 255, 255);background-color:rgba(0, 0, 0, 0)}");
    ui->txt_roll->setStyleSheet("QLabel {color:rgb(255, 255, 255);background-color:rgba(0, 0, 0, 0)}");
    ui->txt_yaw->setStyleSheet("QLabel {color:rgb(255, 255, 255);background-color:rgba(0, 0, 0, 0)}");

    QChart *chart = new QChart();
    chart->setTitle("Roll");
    m_series_roll = new QLineSeries(chart);
    chart->addSeries(m_series_roll);
    chart->createDefaultAxes();
    chart->axisX()->setRange(0, SAMPLENUM);
    chart->axisY()->setRange(-90, 90);
    m_chart_roll = new QChartView(chart);
    m_chart_roll->setRenderHint(QPainter::Antialiasing, true);
    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(m_chart_roll);
    ui->chart_roll->setLayout(layout);


    chart = new QChart();
    chart->setTitle("Pitch");
    m_series_pitch = new QLineSeries(chart);
    chart->addSeries(m_series_pitch);
    chart->createDefaultAxes();
    chart->axisX()->setRange(0, SAMPLENUM);
    chart->axisY()->setRange(-90, 90);
    m_chart_pitch = new QChartView(chart);
    m_chart_pitch->setRenderHint(QPainter::Antialiasing, true);
    layout = new QHBoxLayout();
    layout->addWidget(m_chart_pitch);
    ui->chart_pitch->setLayout(layout);


    for (int i = 0; i < SAMPLENUM; i++)
    {
        m_series_roll->append(i, 0);
        m_series_pitch->append(i, 0);
    }

    QPalette pal = window()->palette();
    pal.setColor(QPalette::Window, QRgb(0x9e8965));
    window()->setPalette(pal);

    ui->chart_roll->setStyleSheet("QWidget { background-color: rgb(158, 137, 101) }");
    m_chart_roll->chart()->setTheme(QChart::ChartThemeBrownSand);
    m_chart_roll->chart()->legend()->hide();

    ui->chart_pitch->setStyleSheet("QWidget { background-color: rgb(158, 137, 101) }");
    m_chart_pitch->chart()->setTheme(QChart::ChartThemeBrownSand);
    m_chart_pitch->chart()->legend()->hide();

}

void MainWindow::lockMotor()
{
    if (!islock)

        action('d', 0);
    else
        action('D', 0);

    islock = !islock;
}

void MainWindow::showModifiedDirectory(QString path)
{
    joysticThread->device_connect();
}

void MainWindow::pollThrust()
{
    action('B', thrust_val);

}

void MainWindow::thrustHandle(char c, char val)
{
    if (!isConnect || !islock) return;

    if (c == 'B')
    {
#if 1
        if (!timer_thrust->isActive() && val != 'o')
        {
            qDebug()<<"start thrust timer.."<<isConnect;
            thrust_val = val;
            timer_thrust->start(500);
        }
        else if (val == 'o')
            timer_thrust->stop();
#endif
    }
    action(c, (int)val);
}

void MainWindow::readyRead()
{
//    char *ret;
    char buf[100];

    //ret = m_client->readAll().data();
    m_client->read(buf, 100);

    switch(buf[0]) {
    case 'A':
        memcpy(&info, &buf[1], sizeof(Info));
        if (que_roll.size() == SAMPLENUM)
        {
            que_roll.pop_front();
            que_pitch.pop_front();
        }
        que_roll.push_back(info.attitude.x);
        que_pitch.push_back(info.attitude.y);
        break;
    case 'a':
        memcpy(&pid_attitude, &buf[1], sizeof(PidParam));
        memcpy(&pid_rate, &buf[1+sizeof(PidParam)], sizeof(PidParam));
        update_PID();
        break;
    default:
        qDebug()<<"head: "<<buf[0];
        break;
    }

//    printf("roll: %f, pitch: %f, Pitch: %f\n", info.attitude.x, info.attitude.y, info.attitude.z);
    switch (ui->tabWidget->currentIndex()) {
    case MODE_FLIGTH:
        mode_flight();
        break;
    case MODE_SETTING:
        mode_setting();
        break;
    }

}

void MainWindow::mode_setting()
{
    QPointF p;
//    qDebug()<<info.attitude.x<<", "<<info.attitude.y<<", "<<info.attitude.z;
    ui->val_roll->setText(QString::number(info.attitude.x, 'f', 2));
    ui->val_pitch->setText(QString::number(info.attitude.y, 'f', 2));
    ui->val_yaw->setText(QString::number(info.attitude.z, 'f', 2));

    ui->val_MLF->setText(QString::number(info.thrust[LEFT_FRONT], 'f', 3));
    ui->val_MLB->setText(QString::number(info.thrust[LEFT_BACK], 'f', 3));
    ui->val_MRF->setText(QString::number(info.thrust[RIGHT_FRONT], 'f', 3));
    ui->val_MRB->setText(QString::number(info.thrust[RIGHT_BACK], 'f', 3));


    for (int i = 0; i < (int)que_roll.size(); i++)
    {
        p = m_series_roll->at(i);
        m_series_roll->replace(i, p.y(), i, que_roll[i]);
        m_series_roll->setColor(QColor(160, 62, 47));

        p = m_series_pitch->at(i);
        m_series_pitch->replace(i, p.y(), i, que_pitch[i]);
        m_series_pitch->setColor(QColor(79, 104, 29));
    }

    ui->opengl->setInfo(&info);
}

void MainWindow::mode_flight()
{
    ui->txt_roll->setText(QString::number(info.attitude.x, 'f', 2));
    ui->txt_pitch->setText(QString::number(info.attitude.y, 'f', 2));
    ui->txt_yaw->setText(QString::number(info.attitude.z, 'f', 2));

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

void MainWindow::update_PID()
{
    ui->ed_att_roll_kp->setText(QString::number(pid_attitude.roll[KP], 'f', 5));
    ui->ed_att_roll_ki->setText(QString::number(pid_attitude.roll[KI], 'f', 5));
    ui->ed_att_roll_kd->setText(QString::number(pid_attitude.roll[KD], 'f', 5));
    ui->ed_att_pitch_kp->setText(QString::number(pid_attitude.pitch[KP], 'f', 5));
    ui->ed_att_pitch_ki->setText(QString::number(pid_attitude.pitch[KI], 'f', 5));
    ui->ed_att_pitch_kd->setText(QString::number(pid_attitude.pitch[KD], 'f', 5));
    ui->ed_att_yaw_kp->setText(QString::number(pid_attitude.yaw[KP], 'f', 5));
    ui->ed_att_yaw_ki->setText(QString::number(pid_attitude.yaw[KI], 'f', 5));
    ui->ed_att_yaw_kd->setText(QString::number(pid_attitude.yaw[KD], 'f', 5));

    ui->ed_rat_roll_kp->setText(QString::number(pid_rate.roll[KP], 'f', 5));
    ui->ed_rat_roll_ki->setText(QString::number(pid_rate.roll[KI], 'f', 5));
    ui->ed_rat_roll_kd->setText(QString::number(pid_rate.roll[KD], 'f', 5));
    ui->ed_rat_pitch_kp->setText(QString::number(pid_rate.pitch[KP], 'f', 5));
    ui->ed_rat_pitch_ki->setText(QString::number(pid_rate.pitch[KI], 'f', 5));
    ui->ed_rat_pitch_kd->setText(QString::number(pid_rate.pitch[KD], 'f', 5));
    ui->ed_rat_yaw_kp->setText(QString::number(pid_rate.yaw[KP], 'f', 5));
    ui->ed_rat_yaw_ki->setText(QString::number(pid_rate.yaw[KI], 'f', 5));
    ui->ed_rat_yaw_kd->setText(QString::number(pid_rate.yaw[KD], 'f', 5));
}

void MainWindow::connected()
{
    ui->btn_connect->setIcon(QIcon(":/pic/icon-connect.png"));
    isConnect = true;
    printf("connect\n");
}

void MainWindow::disConnected()
{
    ui->btn_connect->setIcon(QIcon(":/pic/icon-disconnect.png"));
    isConnect = false;
    printf("disconnect\n");
}

void MainWindow::rebootUAV()
{
    action('c', 0);
    m_client->disconnected();
    m_client->close();
}

void MainWindow::updateMap()
{
    m_page->load(QUrl(QStringLiteral("https://maps.googleapis.com/maps/api/staticmap?center=22.6004779,120.3127385&zoom=16"
                                     "&size=600x600&maptype=roadmap"
                                   "&markers=color:red%7Clabel:H%7C22.6004779,120.3127385"
                                   "&key=AIzaSyBc8rZgqD1Q4S84lPYuHpyoaQNMl0Bw4Tk")));
}

void MainWindow::updatePID()
{
    if (!isConnect) return;

    m_sockMutex.lock();
    cmd[0] = '@';
    cmd[1] = '#';
    cmd[2] = 'C';

    pid_attitude.roll[KP] = atof(ui->ed_att_roll_kp->text().toStdString().c_str());
    pid_attitude.roll[KI] = atof(ui->ed_att_roll_ki->text().toStdString().c_str());
    pid_attitude.roll[KD] = atof(ui->ed_att_roll_kd->text().toStdString().c_str());
    pid_attitude.pitch[KP] = atof(ui->ed_att_pitch_kp->text().toStdString().c_str());
    pid_attitude.pitch[KI] = atof(ui->ed_att_pitch_ki->text().toStdString().c_str());
    pid_attitude.pitch[KD] = atof(ui->ed_att_pitch_kd->text().toStdString().c_str());
    pid_attitude.yaw[KP] = atof(ui->ed_att_yaw_kp->text().toStdString().c_str());
    pid_attitude.yaw[KI] = atof(ui->ed_att_yaw_ki->text().toStdString().c_str());
    pid_attitude.yaw[KD] = atof(ui->ed_att_yaw_kd->text().toStdString().c_str());
    memcpy(&cmd[3], &pid_attitude, sizeof(PidParam));

    pid_rate.roll[KP] = atof(ui->ed_rat_roll_kp->text().toStdString().c_str());
    pid_rate.roll[KI] = atof(ui->ed_rat_roll_ki->text().toStdString().c_str());
    pid_rate.roll[KD] = atof(ui->ed_rat_roll_kd->text().toStdString().c_str());
    pid_rate.pitch[KP] = atof(ui->ed_rat_pitch_kp->text().toStdString().c_str());
    pid_rate.pitch[KI] = atof(ui->ed_rat_pitch_ki->text().toStdString().c_str());
    pid_rate.pitch[KD] = atof(ui->ed_rat_pitch_kd->text().toStdString().c_str());
    pid_rate.yaw[KP] = atof(ui->ed_rat_yaw_kp->text().toStdString().c_str());
    pid_rate.yaw[KI] = atof(ui->ed_rat_yaw_ki->text().toStdString().c_str());
    pid_rate.yaw[KD] = atof(ui->ed_rat_yaw_kd->text().toStdString().c_str());
    memcpy(&cmd[3+sizeof(PidParam)], &pid_rate, sizeof(PidParam));

    m_client->write(cmd, 3+sizeof(PidParam)*2);
    m_client->waitForBytesWritten();
    m_sockMutex.unlock();

    action('a', 0);
}

void MainWindow::action(char act, int val)
{
    m_sockMutex.lock();
    cmd[0] = '@';
    cmd[1] = '#';
    cmd[2] = act;
    cmd[3] = val;

    m_client->write(cmd, 4);
    m_client->waitForBytesWritten();
    m_sockMutex.unlock();
}

void MainWindow::getInfo()
{
    action('A', 0);
}

void MainWindow::on_btn_connect_clicked()
{
    if (!isConnect)
    {
        QString ip_address="192.168.123.1";
        m_client->connectToHost(ip_address, 80);
        timer_info->start(100);
        action('a', 0);
        islock = false;
    }
    else {

        timer_info->stop();
        m_client->disconnected();
        m_client->close();

    }
}

void MainWindow::on_horizontalSlider_sliderMoved(int position)
{
    char cmd[500];
    sprintf(cmd,
    "https://maps.googleapis.com/maps/api/staticmap?center=22.6004779,120.3127385&zoom=%d"
                                         "&size=600x600&maptype=roadmap"
                                       "&markers=color:red%%7Clabel:H%%7C22.6004779,120.3127385"
                                       "&key=AIzaSyBc8rZgqD1Q4S84lPYuHpyoaQNMl0Bw4Tk", position);
    m_page->load(QUrl(QString(cmd)));
}

void MainWindow::on_btn_up_clicked()
{
    action('a', '+');
}

void MainWindow::on_btn_down_clicked()
{
    action('a', '-');
}
