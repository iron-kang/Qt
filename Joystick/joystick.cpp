#include "joystick.h"
#include <fcntl.h>
#include <unistd.h>
#include <QtDebug>
#include <libudev.h>
#include <linux/input.h>
#include <QFile>

static int hotplug_callback(struct libusb_context *ctx, struct libusb_device *dev,
                     libusb_hotplug_event event, void *user_data) {
  static libusb_device_handle *dev_handle = nullptr;
  struct libusb_device_descriptor desc;

  bool *isConnect = static_cast<bool*>(user_data);

  (void)libusb_get_device_descriptor(dev, &desc);

  if (LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED == event) {
    *isConnect = true;
    qDebug()<<"plug";
  } else if (LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT == event) {
      qDebug()<<"unplug";
      *isConnect = false;
    if (dev_handle) {

      libusb_close(dev_handle);
      dev_handle = nullptr;
    }
  } else {
    qDebug()<<"Unhandled event:"<< event;
  }

  return 0;
}

QString JoystickWatcher::get_path()
{
    struct udev_list_entry *devices, *dev_list_entry;
    /* Create the udev object */
    std::shared_ptr< struct udev > udev(udev_new(), udev_unref);
    if (!udev) {
        qDebug()<<"Can't create udev";
        return js_path;
    }

    std::shared_ptr< struct udev_enumerate > enumerate(udev_enumerate_new(udev.get()), udev_enumerate_unref);
    udev_enumerate_add_match_subsystem(enumerate.get(), "input");
    udev_enumerate_scan_devices(enumerate.get());
    devices = udev_enumerate_get_list_entry(enumerate.get());
    udev_list_entry_foreach(dev_list_entry, devices) {
        const char *path;
        path = udev_list_entry_get_name(dev_list_entry);
        std::shared_ptr< struct udev_device > dev(udev_device_new_from_syspath(udev.get(), path), udev_device_unref);

        /* usb_device_get_devnode() returns the path to the device node
           itself in /dev. */
        const char *tmp_path = udev_device_get_devnode(dev.get());
        printf("tmp_path = %s\n", tmp_path);
        struct udev_device *pdev = udev_device_get_parent_with_subsystem_devtype(
                               dev.get(),
                               "usb",
                               "usb_device");
        if (!pdev || !tmp_path)
            continue;


        std::string vid = udev_device_get_sysattr_value(pdev,"idVendor");
        std::string pid = udev_device_get_sysattr_value(pdev, "idProduct");
        printf("vid: %s, pid: %s, %x\n", vid.c_str(), pid.c_str(), stoi(vid, 0, 16));

        if ((stoi(vid, 0, 16) == JOYSTICK_VID) && (stoi(pid, 0, 16) == JOYSTICK_PID)) {
            js_path = QString::fromLocal8Bit(tmp_path);
            break;
        }
    }

    qDebug() << "path: " << js_path;
    return js_path;
}

void JoystickWatcher::run()
{
    bool isConnectd = false;
    libusb_init(nullptr);
    int rc = libusb_hotplug_register_callback(nullptr, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED |
                                            LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, 0, JOYSTICK_VID, JOYSTICK_PID,
                                            LIBUSB_HOTPLUG_MATCH_ANY, hotplug_callback, &isConnectd,
                                            &callback_handle);


    printf("joystick watcher thread.....\n");
    if (LIBUSB_SUCCESS != rc) {
        qDebug()<<"Error creating a hotplug callback";
        libusb_exit(nullptr);
        return;
    }

    while (isRuning) {
        libusb_handle_events(nullptr);
        qDebug()<<"connect: "<<isConnectd;
        printf("connected: %d\n", isConnectd);
        if (isConnectd && js_path.isEmpty()) {
            js_path = get_path();
            emit notify(js_path);
        }
        else {
            qDebug()<<__func__ << " "<<js_path<<": "<<QFile::exists(js_path) << ", isConnectd = " << isConnectd;
            if (!QFile::exists(js_path)) {
                js_path.clear();
                emit notify(js_path);
            }
        }
        old_js_path = js_path;
    }
}

void JoystickWatcher::stop()
{
    isRuning = false;
    libusb_hotplug_deregister_callback(nullptr, callback_handle);
    libusb_exit(nullptr);
}

JoystickThread::JoystickThread()
{
    active = false;
}

void JoystickThread::connect(QString dev)
{
    msleep(500);
    if ((fd = open(dev.toStdString().c_str(), O_RDONLY)) < 0) {
        qDebug() << dev << " open fail";
        active = false;
        return;
    }
    qDebug() << __func__ << ": " << dev;
    active = true;
}

void JoystickThread::disconnect()
{
    active = false;
    close(fd);
    fd = -1;
}

void JoystickThread::run()
{
    struct input_event ev[64];
    int i;
    printf("joystick thread....\n");

    while(true)
    {
        if (!active)
        {
            usleep(1000);
            continue;
        }

        ssize_t rb = read(fd, ev, sizeof(ev));

        for(i=0; i<(int)(rb/sizeof(struct input_event));i++)
        {
            if(EV_KEY == ev[i].type)
            {
                emit sendButton(ev[i].code, ev[i].value);
                if(ev[i].value == KEY_PRESS)
                {

                    switch(ev[i].code) {
                        case BTN_TRIGGER:
                            qDebug()<<"BTN_TRIGGER";
                            break;
                        case BTN_THUMB:
                            qDebug()<<"BTN_THUMB";
                            break;
                        case BTN_TOP2:
                            qDebug()<<"BTN_TOP2";
                            break;
                        case BTN_PINKIE:
                            qDebug()<<"BTN_PINKIE";
                            break;
                        case BTN_THUMB2:
                            qDebug()<<"BTN_THUMB2";
                            break;
                        case BTN_TOP:
                            qDebug()<<"BTN_TOP";
                            break;
                    }
                } else if(ev[i].value == KEY_RELEASE) {


                }

            }
            else if (EV_ABS == ev[i].type)
            {
                if (ABS_X == ev[i].code)
                {
                    emit sendX(ev[i].value);

                } else if (ABS_Y == ev[i].code) {
                    emit sendY(ev[i].value);
                }
            }
        }
    }

}
