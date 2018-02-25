#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebEngineView>
#include <QTcpSocket>
#include <QTimer>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMutex>

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
    QTcpSocket *m_client;
    QTimer *tim_info;
    QPixmap imuPitchPix, imuRollPix;
    QWebEnginePage *m_page;
    QMutex m_sockMutex;

    bool isConnect;
    char cmd[50];

    void UI_Init();
    void action(char cmd);

private slots:
    void getInfo();
    void updateMap();
    void readyRead();
    void connected();
    void disConnected();
    void on_btn_connect_clicked();
    void on_horizontalSlider_sliderMoved(int position);
    void on_btn_up_clicked();
};

#endif // MAINWINDOW_H
