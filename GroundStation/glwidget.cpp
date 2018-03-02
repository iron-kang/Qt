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
}

void GLWidget::initializeGL()
{
    cout<<"GL init"<<endl;
    glClearColor(.1, .1, .1, 1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
}

void GLWidget::paintGL()
{
    cout<<"GL paint"<<endl;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glRotatef(0.5, 1, 1, 1);
    glutWireTeapot(1);
}

void GLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, (float)w/h, 0.01, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, 0, 5, 0, 0, 0, 0, 1, 0);
}
