#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <QThread>
#include <libusb.h>

#define KEY_RELEASE 0
#define KEY_PRESS 1
#define JOYSTICK_VID 0x231d
#define JOYSTICK_PID 0x0118
#define MAX_VAL 4095
#define MIN_VAL 0

class JoystickWatcher:public QThread
{
    Q_OBJECT
public:
    bool isRuning;
    void run();
    void stop();
    QString get_path();
private:
    QString js_path, old_js_path;
    libusb_hotplug_callback_handle callback_handle;

signals:
    void notify(const QString &js_path);

};

class JoystickThread : public QThread
{
    Q_OBJECT
public:
    JoystickThread();

    void run();
    void connect(QString dev);
    void disconnect();
private:
    int fd;
    bool active;
signals:
    void sendX(int val);
    void sendY(int val);
    void sendButton(int btn, int press);
};

#endif // JOYSTICKTHREAD_H
