#include "joysticthread.h"
#include <QDebug>
#include "ui_mainwindow.h"
#include <QSocketNotifier>

#define KEY_RELEASE 0
#define KEY_PRESS 1
#define KEY_KEEPING_PRESSED 2
#define X_CENTER -512
#define Y_CENTER -1
#define RX_CENTER -512
#define RY_CENTER -513

JoysticThread::JoysticThread(QObject *parent) :
    QObject(parent)
{
    fd_desktop_controller = -1;
    device_connect();
}

void JoysticThread::device_connect()
{
    FILE *pip_dev = NULL;
    char str_event[20];
    char *tmp;

    pip_dev = popen("cat /proc/bus/input/devices | grep -A5 \"Microsoft X-Box 360 pad\" | grep Handlers | awk '{print $2}'", "r");

    if (!pip_dev) {
        qDebug()<<"Can't find Desktop controller device";
        pclose(pip_dev);
        return;
    }

    fgets(str_event, sizeof(str_event)-1, pip_dev);


    tmp = strstr(str_event, "event");
    if ( !tmp )
    {
        qDebug()<<"Can't find Desktop controller device";
        close(fd_desktop_controller);
        fd_desktop_controller = -1;
        return;
    }
    tmp[strlen(tmp)-1] = '\0';
    sprintf(dev_path, "/dev/input/%s", strstr(str_event, "event"));
    pclose(pip_dev);

    if (fd_desktop_controller > 0) return;

    if ((fd_desktop_controller = open(dev_path,O_RDWR| O_NOCTTY | O_SYNC)) < 0)
    {
        qDebug()<<"Couldn't open Desktop controller device";
        return;
    }
    else
        qDebug()<<"Desktop controller open success(!! "<<fd_desktop_controller;

    QSocketNotifier *notifier;
    notifier = new QSocketNotifier(fd_desktop_controller, QSocketNotifier::Read, this);
    connect(notifier, SIGNAL(activated(int)),this, SLOT(readJoyStickData()));
}

JoysticThread::~JoysticThread()
{
    qDebug()<<"joystick destory";
    close(fd_desktop_controller);
}

void JoysticThread::readJoyStickData()
{
     int i, temp_x, temp_y;
    size_t rb = read(fd_desktop_controller, ev, sizeof(ev));
//    qDebug()<<rb;
    for(i=0; i<(int)(rb/sizeof(struct input_event));i++)
    {
        if(EV_KEY == ev[i].type)
        {
            if(ev[i].value == KEY_PRESS)
            {
//                    qDebug()<<"key: "<<ev[i].code;
                switch(ev[i].code) {
                    case BTN_X:
                        qDebug()<<"BTN_X";
                        break;
                    case BTN_Y:
                        qDebug()<<"BTN_Y";
                        break;
                    case BTN_A:
                        qDebug()<<"BTN_A";
                        break;
                    case BTN_B:
                        qDebug()<<"BTN_B";
                        break;
                    case BTN_MODE:
                        qDebug()<<"BTN_MODE";
                        emit js_motorLock();
                        break;
                    case BTN_SELECT:
                        qDebug()<<"BTN_SELECT";
                        break;
                    case BTN_START:
                        qDebug()<<"BTN_START";
                        emit connectNet();
                        break;
                    case BTN_TL:
                        qDebug()<<"BTN_TL";
                        break;
                    case BTN_TR:
                        qDebug()<<"BTN_TR";
                        break;
                    case BTN_THUMBR:
                        qDebug()<<"BTN_THUMBR";
                        break;
                    case BTN_THUMBL:
                        qDebug()<<"BTN_THUMBL";
                        break;
                }
            } else if(ev[i].value == KEY_RELEASE) {

                qDebug()<<"key release";
            }

        }
        else if (EV_ABS == ev[i].type)
        {
//                qDebug()<<"key: "<<ev[i].code;
            switch (ev[i].code) {
            case ABS_X:
                temp_x = ev[i].value-X_CENTER;

//                    qDebug()<<"x: "<<temp_x;
                break;
            case ABS_Y:
                temp_y = ev[i].value-Y_CENTER;

                if (temp_y < 0) emit thrustEvent('B', '+');
                else if (temp_y > 0) emit thrustEvent('B', '-');
                else emit thrustEvent('B', 'o');
//                    qDebug()<<"y: "<<temp_y;
                break;
            case ABS_RX:
                temp_x = ev[i].value-RX_CENTER;
                if (temp_x > 0) emit thrustEvent('b', 'r');
                else if (temp_x < 0) emit thrustEvent('b', 'l');
                else emit thrustEvent('b', 's');
//                    qDebug()<<"rx: "<<temp_x;
                break;
            case ABS_RY:
                temp_y = ev[i].value-RY_CENTER;
                if (temp_y > 0) emit thrustEvent('b', 'b');
                else if (temp_y < 0) emit thrustEvent('b', 'f');
                else emit thrustEvent('b', 'S');
//                    qDebug()<<"ry: "<<temp_y;
                break;
            case ABS_HAT0X:
                temp_x = ev[i].value;
                qDebug()<<"hatx: "<<temp_x;
                break;
            case ABS_HAT0Y:
                temp_y = ev[i].value;
                qDebug()<<"haty: "<<temp_y;
                break;
            }

        }
    }

}
