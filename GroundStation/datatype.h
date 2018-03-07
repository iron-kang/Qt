#ifndef DATATYPE_H
#define DATATYPE_H

typedef union {
  struct {
        float x;
        float y;
        float z;
  };
  float axis[3];
} Axis3f;

typedef enum {
    LEFT_FRONT = 0,
    LEFT_BACK,
    RIGHT_FRONT,
    RIGHT_BACK
} motor_t;

typedef struct _info {
    Axis3f attitude;
    float thrust[4];
}Info;

#endif // DATATYPE_H
