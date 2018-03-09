#ifndef JOYSTICTHREAD_H
#define JOYSTICTHREAD_H

#include <QThread>
#include <linux/input.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include "ui_mainwindow.h"

enum JOYSTICK_STAT {
    HORI_R = 0,
    HORI_L,
    HORI_C,
    VERT_T,
    VERT_B,
    VERT_C
};

class JoysticThread : public QThread
{
    Q_OBJECT
public:
    explicit JoysticThread(QObject *parent = 0);
    void run();
    void device_connect();
    bool active;

    ~JoysticThread();

private:
    struct input_event ev[64];
    char dev_path[50];
    int fd_desktop_controller;
    int abs_x, abs_y, abs_rx, abs_ry;

signals:
    void thrustEvent(char c, char val);
    void connectNet();

};

#endif // JOYSTICTHREAD_H
