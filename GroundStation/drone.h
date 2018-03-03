#ifndef DRONE_H
#define DRONE_H

#include "datatype.h"

typedef struct _color
{
    float r;
    float g;
    float b;
}COLOR;

class Drone
{
public:
    Drone();
    void DrawUnitCylinder(int numSegs, float topSize, float bottomSize);
    void DrawDrone(Axis3f attitude, float *thrust);

private:
    void DrawThrottle(float x, float z, float thrust, COLOR color);
};

#endif // DRONE_H
