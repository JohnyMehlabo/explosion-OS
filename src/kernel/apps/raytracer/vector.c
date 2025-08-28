#include "vector.h"
#include "math.h"

// in and out can be the same vector
void Vector3_Normalize(const Vector3* in, Vector3* out) {
    float len = sqrt(in->x*in->x + in->y*in->y + in->z*in->z);
    out->x = in->x / len;
    out->y = in->y / len;
    out->z = in->z / len;
}

float Vector3_Dot(const Vector3* a, const Vector3* b) {
    return a->x*b->x + a->y*b->y + a->z*b->z;
}