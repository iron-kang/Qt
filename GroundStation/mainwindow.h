#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebEngineView>
#include <QTcpSocket>
#include <QTimer>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMutex>
#include <QtCharts/QChartGlobal>
#include <deque>
#include <QFileSystemWatcher>
#include "datatype.h"
#include "joysticthread.h"
#include "arduino-serial-lib.h"

QT_CHARTS_BEGIN_NAMESPACE
class QChartView;
class QChart;
class QLineSeries;
QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QWebEngineView *m_web;
    QGraphicsScene *m_scene;
    QGraphicsView *m_view;
    QTcpSocket *m_infoSock, *m_cmdSock;
    QTimer *timer_info;
    QTimer *timer_thrust;
    QPixmap imuPitchPix, imuRollPix;
    QWebEnginePage *m_page;
    QMutex m_InfoSockMutex, m_CmdSockMutex;
    QChartView *m_chart_roll, *m_chart_pitch, *m_chart_height;
    QLineSeries *m_series_roll, *m_series_pitch, *m_series_height;
    Info info;
    QFileSystemWatcher *devWatcher;
    JoysticThread *joysticThread;
    PidParam pid_attitude;
    PidParam pid_rate;

    bool isConnect;
    bool islock;
    char buf_info[100];
    char buf_cmd[100];
    uint8_t buf_dist[12];
    char thrust_val;
    int scale;
    int uart_3dr433;
    int uav_x, uav_y;

    deque<float> que_roll;
    deque<float> que_pitch;
    deque<float> que_height;

    void UI_Init();
    void action(char cmd, int val);
    void command(char act, char val);
    void mode_flight();
    void mode_setting();
    void update_PID();

private slots:
    void thrustHandle(char c, char val);
    void lockMotor();
    void stopMotor();
    void updatePID();
    void rebootUAV();
    void pollThrust();
    void getInfo();
    void getDist();
    void updateMap();
    void readyRead();
    void connected();
    void disConnected();
    void showModifiedDirectory(QString str);
    void on_btn_connect_clicked();
    void on_horizontalSlider_sliderMoved(int position);
    void on_btn_up_clicked();
    void on_btn_down_clicked();
};

#endif // MAINWINDOW_H
