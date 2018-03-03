#include "glwidget.h"
#include <iostream>
#include <GL/glut.h>
#include <QTimer>


using namespace std;

GLWidget::GLWidget(QWidget *parent) : QGLWidget(parent)
{
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateGL()));
    timer->start(10);
    m_info.attitude.x = 0;
    m_info.attitude.y = 0;
    m_info.attitude.z = 0;

}

void GLWidget::initializeGL()
{
    cout<<"GL init"<<endl;
    glClearColor(.0, .0, .0, 0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, 2, 4, 0, 0.1f, 0, 0, 1, 0);
    drawCoord();
//    glRotatef(0.5, 1, 1, 1);
//    glutWireTeapot(1);
//    roll += 0.2;
//    yaw += 0.5;
//    pitch += 0.5;
    m_drone.DrawDrone(m_info.attitude, m_info.thrust);
}

void GLWidget::resizeGL(int w, int h)
{
    width = w;
    height = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, (float)w/h, 0.01, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
//    gluLookAt(0, 0, 5, 0, 0, 0, 0, 1, 0);
    gluLookAt(0, 2, 4, 0, 0.1f, 0, 0, 1, 0);
}

void GLWidget::drawCoord()
{
    /*X-axis*/
    glColor3f(1, 0.2, 0.2);
    glBegin(GL_LINES);
    glVertex3f(width, 0, 0);
    glVertex3f(-width, 0, 0);
    glEnd();
    /*Y-axis*/
    glColor3f(0.2, 1, 0.2);
    glBegin(GL_LINES);
    glVertex3f(0, height, 0);
    glVertex3f(0, -height, 0);
    glEnd();
    /*Z-axis*/
    glColor3f(0.0, .5, 1);
    glBegin(GL_LINES);
    glVertex3f(0, 0, height);
    glVertex3f(0, 0, -height);
    glEnd();
}

void GLWidget::setInfo(Info *info)
{
    m_info.attitude = info->attitude;
    memcpy(m_info.thrust, info->thrust, sizeof(float)*4);
}
