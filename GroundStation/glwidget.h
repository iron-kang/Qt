#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QGLWidget>
#include <drone.h>
#include <datatype.h>

class GLWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit GLWidget(QWidget *parent=0);

    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);
    void setInfo(Info *info);

private:
    Drone m_drone;
    Info m_info;
    float roll, pitch, yaw;
    int width, height;

    void drawCoord();
};

#endif // GLWIDGET_H
