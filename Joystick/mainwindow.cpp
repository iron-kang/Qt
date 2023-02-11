#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <linux/input.h>
#include <stdlib.h>
#include <QDesktopWidget>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    const char* s = getenv("DEBUG");
    debug = false;
    if (s)
        debug = atoi(s);

    qDebug()<<"debug: "<<debug;
    buttons[BTN_TRIGGER] = ui->btn_trigger;
    buttons[BTN_THUMB]   = ui->btn_thumb;
    buttons[BTN_THUMB2]  = ui->btn_thumb2;
    buttons[BTN_TOP]     = ui->btn_top;
    buttons[BTN_TOP2]    = ui->btn_top2;
    buttons[BTN_PINKIE]  = ui->btn_pinkie;

//    ui->logo->hide();
//    ui->company->hide();

    connect(&jsWatcher, SIGNAL(notify(const QString &)), this, SLOT(checkConnected(const QString &)));
    connect(&jsThread, SIGNAL(sendX(int)), this, SLOT(axisXChanged(int)));
    connect(&jsThread, SIGNAL(sendY(int)), this, SLOT(axisYChanged(int)));
    connect(&jsThread, SIGNAL(sendButton(int, int)), this, SLOT(buttonRecv(int, int)));
    jsWatcher.isRuning = true;
    jsWatcher.start();
    jsThread.start();

    QString js_path = jsWatcher.get_path();
    checkConnected(js_path);
    this->setWindowFlags(Qt::FramelessWindowHint);

    resizeLayout();
}

MainWindow::~MainWindow()
{
    jsWatcher.stop();
    jsWatcher.quit();
    jsWatcher.wait();
    delete ui;
}

void MainWindow::resizeLayout()
{
    QSize ui_size = frameGeometry().size();
    QSize screen_size = QApplication::desktop()->geometry().size();
    int w, h, x, y;

    float w_scale = float(screen_size.width())/ui_size.width();
    float h_scale = float(screen_size.height())/ui_size.height();

    w = int(ui->frame->width()*w_scale);
    h = int(ui->frame->height()*h_scale);
    x = int(ui->frame->x()*w_scale);
    y = int(ui->frame->y()*h_scale);
    ui->frame->move(x, y);
    ui->frame->resize(w, h);

    w = int(ui->thumb->width()*w_scale);
    h = int(ui->thumb->height()*h_scale);
    x = int(ui->thumb->x()*w_scale);
    y = int(ui->thumb->y()*h_scale);
    ui->thumb->move(x, y);
    ui->thumb->resize(w, h);

    w = int(ui->cursor->width()*w_scale);
    h = int(ui->cursor->height()*h_scale);
    x = int(ui->cursor->x()*w_scale);
    y = int(ui->cursor->y()*h_scale);
    ui->cursor->move(x, y);
    ui->cursor->resize(w, h);

    w = int(ui->title->width()*w_scale);
    h = int(ui->title->height()*h_scale);
    x = int(ui->title->x()*w_scale);
    y = int(ui->title->y()*h_scale);
    ui->title->move(x, y);
    ui->title->resize(w, h);

    w = int(ui->x_val->width()*w_scale);
    h = int(ui->x_val->height()*h_scale);
    x = int(ui->x_val->x()*w_scale);
    y = int(ui->x_val->y()*h_scale);
    ui->x_val->move(x, y);
    ui->x_val->resize(w, h);

    w = int(ui->y_val->width()*w_scale);
    h = int(ui->y_val->height()*h_scale);
    x = int(ui->y_val->x()*w_scale);
    y = int(ui->y_val->y()*h_scale);
    ui->y_val->move(x, y);
    ui->y_val->resize(w, h);

    w = int(ui->btn_pinkie->width()*w_scale);
    h = int(ui->btn_pinkie->height()*h_scale);
    x = int(ui->btn_pinkie->x()*w_scale);
    y = int(ui->btn_pinkie->y()*h_scale);
    ui->btn_pinkie->move(x, y);
    ui->btn_pinkie->resize(w, h);

    w = int(ui->btn_thumb->width()*w_scale);
    h = int(ui->btn_thumb->height()*h_scale);
    x = int(ui->btn_thumb->x()*w_scale);
    y = int(ui->btn_thumb->y()*h_scale);
    ui->btn_thumb->move(x, y);
    ui->btn_thumb->resize(w, h);

    w = int(ui->btn_thumb2->width()*w_scale);
    h = int(ui->btn_thumb2->height()*h_scale);
    x = int(ui->btn_thumb2->x()*w_scale);
    y = int(ui->btn_thumb2->y()*h_scale);
    ui->btn_thumb2->move(x, y);
    ui->btn_thumb2->resize(w, h);

    w = int(ui->btn_top->width()*w_scale);
    h = int(ui->btn_top->height()*h_scale);
    x = int(ui->btn_top->x()*w_scale);
    y = int(ui->btn_top->y()*h_scale);
    ui->btn_top->move(x, y);
    ui->btn_top->resize(w, h);

    w = int(ui->btn_top2->width()*w_scale);
    h = int(ui->btn_top2->height()*h_scale);
    x = int(ui->btn_top2->x()*w_scale);
    y = int(ui->btn_top2->y()*h_scale);
    ui->btn_top2->move(x, y);
    ui->btn_top2->resize(w, h);

    w = int(ui->btn_trigger->width()*w_scale);
    h = int(ui->btn_trigger->height()*h_scale);
    x = int(ui->btn_trigger->x()*w_scale);
    y = int(ui->btn_trigger->y()*h_scale);
    ui->btn_trigger->move(x, y);
    ui->btn_trigger->resize(w, h);

    w = int(ui->label_2->width()*w_scale);
    h = int(ui->label_2->height()*h_scale);
    x = int(ui->label_2->x()*w_scale);
    y = int(ui->label_2->y()*h_scale);
    ui->label_2->move(x, y);
    ui->label_2->resize(w, h);

    w = int(ui->label_3->width()*w_scale);
    h = int(ui->label_3->height()*h_scale);
    x = int(ui->label_3->x()*w_scale);
    y = int(ui->label_3->y()*h_scale);
    ui->label_3->move(x, y);
    ui->label_3->resize(w, h);

    w = int(ui->label_4->width()*w_scale);
    h = int(ui->label_4->height()*h_scale);
    x = int(ui->label_4->x()*w_scale);
    y = int(ui->label_4->y()*h_scale);
    ui->label_4->move(x, y);
    ui->label_4->resize(w, h);

    w = int(ui->label_5->width()*w_scale);
    h = int(ui->label_5->height()*h_scale);
    x = int(ui->label_5->x()*w_scale);
    y = int(ui->label_5->y()*h_scale);
    ui->label_5->move(x, y);
    ui->label_5->resize(w, h);

    w = int(ui->label_6->width()*w_scale);
    h = int(ui->label_6->height()*h_scale);
    x = int(ui->label_6->x()*w_scale);
    y = int(ui->label_6->y()*h_scale);
    ui->label_6->move(x, y);
    ui->label_6->resize(w, h);

    w = int(ui->label_7->width()*w_scale);
    h = int(ui->label_7->height()*h_scale);
    x = int(ui->label_7->x()*w_scale);
    y = int(ui->label_7->y()*h_scale);
    ui->label_7->move(x, y);
    ui->label_7->resize(w, h);

    w = int(ui->label_8->width()*w_scale);
    h = int(ui->label_8->height()*h_scale);
    x = int(ui->label_8->x()*w_scale);
    y = int(ui->label_8->y()*h_scale);
    ui->label_8->move(x, y);
    ui->label_8->resize(w, h);

    w = int(ui->label_9->width()*w_scale);
    h = int(ui->label_9->height()*h_scale);
    x = int(ui->label_9->x()*w_scale);
    y = int(ui->label_9->y()*h_scale);
    ui->label_9->move(x, y);
    ui->label_9->resize(w, h);

    w = int(ui->company->width()*w_scale);
    h = int(ui->company->height()*h_scale);
    x = int(ui->company->x()*w_scale);
    y = int(ui->company->y()*h_scale);
    ui->company->move(x, y);
    ui->company->resize(w, h);

    w = int(ui->logo->width()*w_scale);
    h = int(ui->logo->height()*h_scale);
    x = int(ui->logo->x()*w_scale);
    y = int(ui->logo->y()*h_scale);
    ui->logo->move(x, y);
    ui->logo->resize(w, h);

    this->resize(screen_size);
}

void MainWindow::checkConnected(const QString &js_path)
{
    if (!js_path.isEmpty()) {
        jsThread.connect(js_path);
        ui->statusBar->showMessage("Joystick connected");
        ui->statusBar->setStyleSheet("QStatusBar{color : green;background-color:rgba(255, 255, 255, 255);}");
    }
    else {
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
    if (debug)
        qDebug()<<__func__<<": "<< val<<", "<<x;
}

void MainWindow::axisYChanged(int val)
{
    ui->y_val->setText(QString::number(val));
    int x = this->ui->cursor->x();
    int y = int(val/4095.0*ui->thumb->width())-ui->cursor->height()/2;
    this->ui->cursor->move(x, y);
    if (debug)
        qDebug()<<__func__<<": "<< val<<", "<<y;
}
