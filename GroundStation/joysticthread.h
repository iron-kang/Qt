#ifndef JOYSTICTHREAD_H
#define JOYSTICTHREAD_H

#include <QThread>
#include <linux/input.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include "ui_mainwindow.h"

class JoysticThread : public QObject
{
    Q_OBJECT
public:
    explicit JoysticThread(QObject *parent = 0);
    void device_connect();

    ~JoysticThread();

private:
    struct input_event ev[64];
    char dev_path[50];
    uint8_t thumb_clk_cnt;
    int fd_desktop_controller;
    int abs_x, abs_y, abs_rx, abs_ry;

signals:
    void thrustEvent(char c, char val);
    void connectNet();
    void js_motorLock();
    void js_reboot();
    void js_stop();

private slots:
    void readJoyStickData();

};

#endif // JOYSTICTHREAD_H
