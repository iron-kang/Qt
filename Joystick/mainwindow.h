#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <map>
#include "joystick.h"

#define WIDTH 1280
#define HEIGHT 720

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void checkConnected(const QString &js_path);
    void axisXChanged(int val);
    void axisYChanged(int val);
    void buttonRecv(int btn, int press);

private:
    Ui::MainWindow *ui;
    JoystickWatcher jsWatcher;
    JoystickThread jsThread;
    bool debug;

    std::map<int, QLabel*> buttons;

    void resizeLayout();
};

#endif // MAINWINDOW_H
