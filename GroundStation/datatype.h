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

typedef struct _info {
    Axis3f attitude;
}Info;

#endif // DATATYPE_H
