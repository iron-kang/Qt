#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <linux/input.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    printf("%s ....\n", __func__);
    qDebug()<<__func__;
    buttons[BTN_TRIGGER] = ui->btn_trigger;
    buttons[BTN_THUMB]   = ui->btn_thumb;
    buttons[BTN_THUMB2]  = ui->btn_thumb2;
    buttons[BTN_TOP]     = ui->btn_top;
    buttons[BTN_TOP2]    = ui->btn_top2;
    buttons[BTN_PINKIE]  = ui->btn_pinkie;

    connect(&jsWatcher, SIGNAL(notify(const QString &)), this, SLOT(checkConnected(const QString &)));
    connect(&jsThread, SIGNAL(sendX(int)), this, SLOT(axisXChanged(int)));
    connect(&jsThread, SIGNAL(sendY(int)), this, SLOT(axisYChanged(int)));
    connect(&jsThread, SIGNAL(sendButton(int, int)), this, SLOT(buttonRecv(int, int)));
    jsWatcher.isRuning = true;
    jsWatcher.start();
    jsThread.start();

    QString js_path = jsWatcher.get_path();
    qDebug() << "js path: " << js_path;
    printf("js path = %s....\n", js_path.toStdString().c_str());
    checkConnected(js_path);

}

MainWindow::~MainWindow()
{
    jsWatcher.stop();
    jsWatcher.quit();
    jsWatcher.wait();
    delete ui;
}

void MainWindow::checkConnected(const QString &js_path)
{
    if (!js_path.isEmpty()) {
//        this->ui->js_enable->setPixmap(QPixmap(":/resource/img/joystick.png"));

        jsThread.connect(js_path);
        ui->statusBar->showMessage("Joystick connected");
        ui->statusBar->setStyleSheet("QStatusBar{color : green;background-color:rgba(255, 255, 255, 255);}");
    }
    else {
//        this->ui->js_enable->setPixmap(QPixmap(":/resource/img/joystick_disconnect.png"));
        jsThread.disconnect();
        ui->statusBar->showMessage("Joystick disconnected");
        ui->statusBar->setStyleSheet("QStatusBar{color : red;background-color:rgba(255, 255, 255, 255);}");
    }

}

void MainWindow::buttonRecv(int btn, int press)
{
    if (press)
        buttons[btn]->setPixmap(QPixmap(":/resource/img/button.png"));
    else
        buttons[btn]->setPixmap(QPixmap(":/resource/img/button_release.png"));
}

void MainWindow::axisXChanged(int val)
{
    ui->x_val->setText(QString::number(val));
    int x = int(val/4095.0*ui->thumb->width())-ui->cursor->width()/2;
    int y = this->ui->cursor->y();
    this->ui->cursor->move(x, y);
    qDebug()<<__func__<<": "<< val<<", "<<x;
}

void MainWindow::axisYChanged(int val)
{
    ui->y_val->setText(QString::number(val));
    int x = this->ui->cursor->x();
    int y = int(val/4095.0*ui->thumb->width())-ui->cursor->height()/2;
    this->ui->cursor->move(x, y);
    qDebug()<<__func__<<": "<< val<<", "<<y;
}
