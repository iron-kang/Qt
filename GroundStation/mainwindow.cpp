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
#define STATE_NUM 3

using namespace std;

string status_display[STATE_NUM] = {
  "Battery abnormal",
  "IMU abnormal",
  "GPS not found"
};

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

    m_infoSock = new QTcpSocket(this);
    connect(m_infoSock,SIGNAL(readyRead()),this,SLOT(readyRead()));
    connect(m_infoSock,SIGNAL(connected()),this,SLOT(connected()));
    connect(m_infoSock,SIGNAL(disconnected()),this,SLOT(disConnected()));

    m_cmdSock = new QTcpSocket(this);

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
//    m_infoSock->connectToHost(ip_address, 80);
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
    connect(joysticThread, SIGNAL(js_reboot()), this, SLOT(rebootUAV()));
    connect(joysticThread, SIGNAL(js_stop()), this, SLOT(stopMotor()));

    connect(ui->btn_setpid, SIGNAL(clicked()), this, SLOT(updatePID()));
    connect(ui->btn_reboot, SIGNAL(clicked()), this, SLOT(rebootUAV()));

    islock = true;
}

MainWindow::~MainWindow()
{
    cout<<"deconstruct"<<endl;
    m_infoSock->disconnectFromHost();
    m_infoSock->close();
    m_cmdSock->disconnectFromHost();
    m_cmdSock->close();

    delete m_infoSock;
    delete m_cmdSock;
    delete ui;
}

void MainWindow::UI_Init()
{
    ui->txt_pitch->setStyleSheet("QLabel {color:rgb(255, 255, 255);background-color:rgba(0, 0, 0, 0)}");
    ui->txt_roll->setStyleSheet("QLabel {color:rgb(255, 255, 255);background-color:rgba(0, 0, 0, 0)}");
    ui->txt_yaw->setStyleSheet("QLabel {color:rgb(255, 255, 255);background-color:rgba(0, 0, 0, 0)}");
    ui->txt_bat->setStyleSheet("QLabel {color:rgb(255, 255, 255);background-color:rgba(0, 0, 0, 0)}");
    ui->txt_status->setStyleSheet("QLabel {color:rgb(255, 0, 0);background-color:rgba(0, 0, 0, 0)}");

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

void MainWindow::stopMotor()
{
    command('D', 0);
    islock = true;
    if (timer_thrust->isActive())
        timer_thrust->stop();
}

void MainWindow::lockMotor()
{
//    if (!islock)

        command('d', 0);
//    else
//        action('D', 0);

//    islock = !islock;
        islock = false;
}

void MainWindow::showModifiedDirectory(QString path)
{
    joysticThread->device_connect();
}

void MainWindow::pollThrust()
{
    command('B', thrust_val);

}

void MainWindow::thrustHandle(char c, char val)
{
    if (!isConnect || islock) return;

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
    else
        command(c, val);

}

void MainWindow::readyRead()
{
    int ret;
    char buf[100];

    //ret = m_infoSock->readAll().data();
    ret = m_infoSock->read(buf, 100);

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
    ui->val_bat->setText(QString::number(info.bat, 'f', 1));
}

void MainWindow::mode_flight()
{
    uint8_t status = info.status;
    string status_str;

    ui->txt_roll->setText(QString::number(info.attitude.x, 'f', 2));
    ui->txt_pitch->setText(QString::number(info.attitude.y, 'f', 2));
    ui->txt_yaw->setText(QString::number(info.attitude.z, 'f', 2));
    ui->txt_bat->setText(QString::number(info.bat, 'f', 1));
    ui->val_gps_altitude->setText(QString::number(info.gps.altitude, 'f', 2));
    ui->val_gps_latitude->setText(QString::number(info.gps.latitude, 'f', 2));
    ui->val_gps_longitude->setText(QString::number(info.gps.longitude, 'f', 2));

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

    for (uint8_t i = 0; i < STATE_NUM; i++)
    {
        if (status & 1)
            status_str += status_display[i] + "\n";
        status >>= 1;
    }
    ui->txt_status->setText(QString::fromStdString(status_str));
}

void MainWindow::update_PID()
{
    ui->ed_att_roll_kp->setText(QString::number(pid_attitude.roll[KP], 'f', 3));
    ui->ed_att_roll_ki->setText(QString::number(pid_attitude.roll[KI], 'f', 3));
    ui->ed_att_roll_kd->setText(QString::number(pid_attitude.roll[KD], 'f', 3));
    ui->ed_att_pitch_kp->setText(QString::number(pid_attitude.pitch[KP], 'f', 3));
    ui->ed_att_pitch_ki->setText(QString::number(pid_attitude.pitch[KI], 'f', 3));
    ui->ed_att_pitch_kd->setText(QString::number(pid_attitude.pitch[KD], 'f', 3));
    ui->ed_att_yaw_kp->setText(QString::number(pid_attitude.yaw[KP], 'f', 3));
    ui->ed_att_yaw_ki->setText(QString::number(pid_attitude.yaw[KI], 'f', 3));
    ui->ed_att_yaw_kd->setText(QString::number(pid_attitude.yaw[KD], 'f', 3));

    ui->ed_rat_roll_kp->setText(QString::number(pid_rate.roll[KP], 'f', 3));
    ui->ed_rat_roll_ki->setText(QString::number(pid_rate.roll[KI], 'f', 3));
    ui->ed_rat_roll_kd->setText(QString::number(pid_rate.roll[KD], 'f', 3));
    ui->ed_rat_pitch_kp->setText(QString::number(pid_rate.pitch[KP], 'f', 3));
    ui->ed_rat_pitch_ki->setText(QString::number(pid_rate.pitch[KI], 'f', 3));
    ui->ed_rat_pitch_kd->setText(QString::number(pid_rate.pitch[KD], 'f', 3));
    ui->ed_rat_yaw_kp->setText(QString::number(pid_rate.yaw[KP], 'f', 3));
    ui->ed_rat_yaw_ki->setText(QString::number(pid_rate.yaw[KI], 'f', 3));
    ui->ed_rat_yaw_kd->setText(QString::number(pid_rate.yaw[KD], 'f', 3));
}

void MainWindow::connected()
{
    ui->btn_connect->setIcon(QIcon(":/pic/icon-connect.png"));
    isConnect = true;
    qDebug()<<"connect";
    timer_info->start(100);
    action('a', 0);
    islock = false;
}

void MainWindow::disConnected()
{
    ui->btn_connect->setIcon(QIcon(":/pic/icon-disconnect.png"));
    isConnect = false;
    if (timer_info->isActive())
    {
        timer_info->stop();
        m_infoSock->disconnected();
        m_infoSock->close();
        m_cmdSock->disconnected();
        m_cmdSock->close();
    }
    qDebug()<<"disconnect";
}

void MainWindow::rebootUAV()
{
    if (!isConnect) return;
    command('c', 0);
    timer_info->stop();
    m_infoSock->disconnected();
    m_infoSock->close();
    m_cmdSock->disconnected();
    m_cmdSock->close();
}

void MainWindow::updateMap()
{
    m_page->load(QUrl(QStringLiteral("https://maps.googleapis.com/maps/api/staticmap?center=22.6004779,120.3127385&zoom=16"
                                     "&size=600x600&maptype=hybrid"//roadmap"
                                   "&markers=color:red%7Clabel:H%7C22.6004779,120.3127385"
                                   "&key=AIzaSyBc8rZgqD1Q4S84lPYuHpyoaQNMl0Bw4Tk")));
}

void MainWindow::updatePID()
{
    if (!isConnect) return;

    m_InfoSockMutex.lock();
    buf_info[0] = '@';
    buf_info[1] = '#';
    buf_info[2] = 'C';

    pid_attitude.roll[KP] = atof(ui->ed_att_roll_kp->text().toStdString().c_str());
    pid_attitude.roll[KI] = atof(ui->ed_att_roll_ki->text().toStdString().c_str());
    pid_attitude.roll[KD] = atof(ui->ed_att_roll_kd->text().toStdString().c_str());
    pid_attitude.pitch[KP] = atof(ui->ed_att_pitch_kp->text().toStdString().c_str());
    pid_attitude.pitch[KI] = atof(ui->ed_att_pitch_ki->text().toStdString().c_str());
    pid_attitude.pitch[KD] = atof(ui->ed_att_pitch_kd->text().toStdString().c_str());
    pid_attitude.yaw[KP] = atof(ui->ed_att_yaw_kp->text().toStdString().c_str());
    pid_attitude.yaw[KI] = atof(ui->ed_att_yaw_ki->text().toStdString().c_str());
    pid_attitude.yaw[KD] = atof(ui->ed_att_yaw_kd->text().toStdString().c_str());
    memcpy(&buf_info[3], &pid_attitude, sizeof(PidParam));

    pid_rate.roll[KP] = atof(ui->ed_rat_roll_kp->text().toStdString().c_str());
    pid_rate.roll[KI] = atof(ui->ed_rat_roll_ki->text().toStdString().c_str());
    pid_rate.roll[KD] = atof(ui->ed_rat_roll_kd->text().toStdString().c_str());
    pid_rate.pitch[KP] = atof(ui->ed_rat_pitch_kp->text().toStdString().c_str());
    pid_rate.pitch[KI] = atof(ui->ed_rat_pitch_ki->text().toStdString().c_str());
    pid_rate.pitch[KD] = atof(ui->ed_rat_pitch_kd->text().toStdString().c_str());
    pid_rate.yaw[KP] = atof(ui->ed_rat_yaw_kp->text().toStdString().c_str());
    pid_rate.yaw[KI] = atof(ui->ed_rat_yaw_ki->text().toStdString().c_str());
    pid_rate.yaw[KD] = atof(ui->ed_rat_yaw_kd->text().toStdString().c_str());
    memcpy(&buf_info[3+sizeof(PidParam)], &pid_rate, sizeof(PidParam));

    m_infoSock->write(buf_info, 3+sizeof(PidParam)*2);
    m_infoSock->waitForBytesWritten();
    m_InfoSockMutex.unlock();

    action('a', 0);
}

void MainWindow::action(char act, int val)
{
    m_InfoSockMutex.lock();
    buf_info[0] = '@';
    buf_info[1] = '#';
    buf_info[2] = act;
    buf_info[3] = val;

    m_infoSock->write(buf_info, 4);
    m_infoSock->waitForBytesWritten();
    m_InfoSockMutex.unlock();
}


void MainWindow::command(char act, char val)
{
    m_CmdSockMutex.lock();
    buf_cmd[0] = '@';
    buf_cmd[1] = '#';
    buf_cmd[2] = act;
    buf_cmd[3] = val;

    qDebug()<<"cmd: "<<act<<", "<<val;
    m_cmdSock->write(buf_cmd, 4);
    m_cmdSock->waitForBytesWritten();
    m_CmdSockMutex.unlock();
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
        m_infoSock->connectToHost(ip_address, 80);
        m_cmdSock->connectToHost(ip_address, 80);

    }
    else {
        qDebug()<<"close socket";
        timer_info->stop();
        m_infoSock->disconnected();
        m_infoSock->close();
        m_cmdSock->disconnected();
        m_cmdSock->close();
    }
}

void MainWindow::on_horizontalSlider_sliderMoved(int position)
{
    char cmd[500];
    sprintf(cmd,
    "https://maps.googleapis.com/maps/api/staticmap?center=22.6004779,120.3127385&zoom=%d"
                                         "&size=600x600&maptype=hybrid"
                                       "&markers=color:red%%7Clabel:H%%7C22.6004779,120.3127385"
                                       "&key=AIzaSyBc8rZgqD1Q4S84lPYuHpyoaQNMl0Bw4Tk", position);
    m_page->load(QUrl(QString(cmd)));
}

void MainWindow::on_btn_up_clicked()
{
    command('B', '+');
}

void MainWindow::on_btn_down_clicked()
{
    action('B', '-');
}
