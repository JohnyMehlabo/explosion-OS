#pragma once

typedef struct {
    float x;
    float y;
    float z;
} Vector3;

void Vector3_Normalize(const Vector3* in, Vector3* out);
float Vector3_Dot(const Vector3* a, const Vector3* b);