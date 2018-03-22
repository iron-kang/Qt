#ifndef DATATYPE_H
#define DATATYPE_H

#include <stdint.h>

#define PID_NUM 18

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

typedef enum {
    PID_ROLL_KP = 0,
    PID_ROLL_KI,
    PID_ROLL_KD,
    PID_PITCH_KP,
    PID_PITCH_KI,
    PID_PITCH_KD,
    PID_YAW_KP,
    PID_YAW_KI,
    PID_YAW_KD,
    PID_ROLL_RATE_KP,
    PID_ROLL_RATE_KI,
    PID_ROLL_RATE_KD,
    PID_PITCH_RATE_KP,
    PID_PITCH_RATE_KI,
    PID_PITCH_RATE_KD,
    PID_YAW_RATE_KP,
    PID_YAW_RATE_KI,
    PID_YAW_RATE_KD,
} PID_PARA_t;

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

enum {
    STATUS_BAT = 0,
    STATUS_IMU
};

typedef struct _info {
    AXIS attitude;
    float thrust[4];
    float bat;
    uint8_t status;
}Info;

#endif // DATATYPE_H
