#include "drone.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <math.h>

#define AXIS_RADIUS 0.08
#define AXIS_LEHGTH 1.2
#define MOTOR_POS   (sqrt(2)*AXIS_LEHGTH/2)
#define MIN_THRUST 0.4
#define THRUST_FACTOR (MIN_THRUST + (0.5 - MIN_THRUST)*2)

Drone::Drone()
{

}

void Drone::DrawUnitCylinder(int numSegs, float topSize, float bottomSize)
{
    int i;
    float Px[numSegs];
    float Py[numSegs];
    float AngIncr = (2.0f * M_PI) / (float)numSegs;
    float Ang = AngIncr;
    Px[0] = 1;
    Py[0] = 0;
    for (i = 1; i < numSegs; i++, Ang += AngIncr)
    {
        Px[i] = (float)cos(Ang);
        Py[i] = (float)sin(Ang);
    }
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    // Top
    glNormal3f(0, 1, 0);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0, 1, 0);
    for (i = 0; i < numSegs; i++)
        glVertex3f(topSize * Px[i], 1, -topSize * Py[i]);
    glVertex3f(topSize * Px[0], 1, -topSize * Py[0]);
    glEnd();
    // Bottom
    glNormal3f(0, -1, 0);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0, -1, 0);
    for (i = 0; i < numSegs; i++)
        glVertex3f(bottomSize * Px[i], -1, bottomSize * Py[i]);
    glVertex3f(bottomSize * Px[0], -1, bottomSize * Py[0]);
    glEnd();
    // Sides
    glBegin(GL_QUAD_STRIP);
    for (i = 0; i < numSegs; i++)
    {
        glNormal3f(Px[i], 0, -Py[i]);
        glVertex3f(topSize * Px[i], 1, -topSize * Py[i]);
        glVertex3f(bottomSize * Px[i], -1, -bottomSize * Py[i]);
    }
    glNormal3f(Px[0], 0, -Py[0]);
    glVertex3f(topSize * Px[0], 1, -topSize * Py[0]);
    glVertex3f(bottomSize * Px[0], -1, -bottomSize * Py[0]);
    glEnd();
    glPopMatrix();
}

void Drone::DrawDrone(float roll, float pitch, float yaw)
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glRotatef(roll, 0, 0, 90);
    glRotatef(yaw, 0, 90, 0);
    glRotatef(pitch, 90, 0, 0);


    glPushMatrix();
    glColor3f(0.3, 0.3, 0.3);
    glScalef(0.5, 0.4, 0.8);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.2, 0.2, 0.2);
    glRotatef(90, -90, 0, 90);
    glScalef(AXIS_RADIUS, AXIS_LEHGTH, AXIS_RADIUS);
    DrawUnitCylinder(50, 1, 1);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.2, 0.2, 0.2);
    glRotatef(-90, 90, 0, 90);
    glScalef(AXIS_RADIUS, AXIS_LEHGTH, AXIS_RADIUS);
    DrawUnitCylinder(50, 1, 1);
    glPopMatrix();

    /* Left Forward */
    DrawThrottle(MOTOR_POS, MOTOR_POS, 0, COLOR{1.0, 0.1, 0.1});
    glPushMatrix();
    glColor3f(0.9, 0, 0);
    glTranslatef(MOTOR_POS, 0, MOTOR_POS);
    glScalef(.2, .1, .2);
    DrawUnitCylinder(50, 1, .75);
    glPopMatrix();

    /* Right Forward */
    DrawThrottle(-MOTOR_POS, MOTOR_POS, 0, COLOR{1.0, 0.1, 0.1});
    glPushMatrix();
    glColor3f(0.9, 0, 0);
    glTranslatef(-MOTOR_POS, 0, MOTOR_POS);
    glScalef(.2, .1, .2);
    DrawUnitCylinder(50, 1, .75);
    glPopMatrix();

    /* Right Back */
    DrawThrottle(-MOTOR_POS, -MOTOR_POS, 0, COLOR{0.1, 1, 0.1});
    glPushMatrix();
    glColor3f(0, .9, 0);
    glTranslatef(-MOTOR_POS, 0, -MOTOR_POS);
    glScalef(.2, .1, .2);
    DrawUnitCylinder(50, 1, .75);
    glPopMatrix();

    /* Left Back */
    DrawThrottle(MOTOR_POS, -MOTOR_POS, 0, COLOR{0.1, 1, 0.1});
    glPushMatrix();
    glColor3f(0, .9, 0);
    glTranslatef(MOTOR_POS, 0, -MOTOR_POS);
    glScalef(.2, .1, .2);
    DrawUnitCylinder(50, 1, .75);
    glPopMatrix();

    /*Camera*/
    glTranslatef(0, -0.2, 0.2);
    glPushMatrix();
    glColor3f(0.6, .6, 0.6);
    glScalef(.2, .1, .2);
    DrawUnitCylinder(50, 1, 1);
    glPopMatrix();

//    glRotatef(yaw, 0, 90, 0);
    glPushMatrix();
    glColor3f(0.6, .6, 0.6);
    glTranslatef(0.15, -0.25, 0);
    glRotatef(90, 0, 0, 90);
    glScalef(.2, .06, .2);
    DrawUnitCylinder(50, 1, 1);
    glPopMatrix();

    glTranslatef(0, -0.25, 0);
//    glRotatef(yaw, 90, 0, 0);
    glPushMatrix();
    glColor3f(0.8, .8, 0.8);
    glScalef(0.2, 0.2, 0.1);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, 0, 0.1);
    glColor3f(0, 0, 0.6);
    glRotatef(90, 90, 0, 0);
    glScalef(.075, .06, .075);
    DrawUnitCylinder(50, 1, 1);
    glPopMatrix();

    glPopMatrix();
}

void Drone::DrawThrottle(float x, float z, float thrust, COLOR color)
{
    thrust = 1;
    if (thrust < 0.4 || thrust > 1) return;
    glPushMatrix();
    glTranslatef(x, 0.2+(thrust-MIN_THRUST)*2/THRUST_FACTOR*0.4+0.1, z);
    glColor3f(color.r*thrust, color.g*thrust, color.b*thrust);
    glScalef(.2, .2, .2);
    DrawUnitCylinder(4, .01, 1);
    glTranslatef(0, -(1+(thrust-MIN_THRUST)*2/THRUST_FACTOR), 0);
    glScalef(.4, (thrust-MIN_THRUST)*2/THRUST_FACTOR, .4);
    DrawUnitCylinder(40, 1, 1);
    glPopMatrix();
}
