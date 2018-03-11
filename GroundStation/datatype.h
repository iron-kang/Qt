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

typedef struct {
    float x;
    float y;
    float z;
} AXIS;

typedef struct {
    float roll[3];
    float pitch[3];
    float yaw[3];
} PidParam;

typedef enum {
    KP = 0,
    KI,
    KD
}PID_ID;

typedef struct _info {
    PidParam pid_attitude;
    PidParam pid_rate;
    AXIS attitude;
    float thrust[4];
}Info;

#endif // DATATYPE_H
