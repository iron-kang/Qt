#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QGLWidget>
#include <drone.h>

class GLWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit GLWidget(QWidget *parent=0);

    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);

private:
    Drone m_drone;
    float roll, pitch, yaw;
    int width, height;

    void drawCoord();
};

#endif // GLWIDGET_H
