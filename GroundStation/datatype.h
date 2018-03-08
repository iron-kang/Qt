#ifndef DATATYPE_H
#define DATATYPE_H

typedef enum {
    LEFT_FRONT = 0,
    LEFT_BACK,
    RIGHT_FRONT,
    RIGHT_BACK
} motor_t;

typedef enum {
    MODE_FLIGTH = 0,
    MODE_SETTING,
} uimode;

typedef union {
  struct {
        float x;
        float y;
        float z;
  };
  float axis[3];
} Axis3f;

typedef struct _info {
    Axis3f attitude;
    float thrust[4];
}Info;

#endif // DATATYPE_H
