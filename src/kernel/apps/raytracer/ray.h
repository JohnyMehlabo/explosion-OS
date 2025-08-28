#pragma once
#include "vector.h"

typedef struct {
    Vector3 origin;
    Vector3 direction;
} Ray;

void Ray_PointAtLength(const Ray* ray, float length, Vector3* out);
