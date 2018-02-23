#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebEngineView>
#include <QTcpSocket>
#include <QTimer>

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
    QWebEngineView *m_view;
    QTcpSocket *m_client;
    QTimer *tim_info;

    bool isConnect;

private slots:
    void getInfo();
    void readyRead();
    void connected();
    void disConnected();
    void on_btn_connect_clicked();
};

#endif // MAINWINDOW_H
