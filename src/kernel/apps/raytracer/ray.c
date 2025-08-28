#include "ray.h"

void Ray_PointAtLength(const Ray* ray, float length, Vector3* out) {
    // point = direction*t + origin
    out->x = ray->direction.x * length + ray->origin.x;
    out->y = ray->direction.y * length + ray->origin.y;
    out->z = ray->direction.z * length + ray->origin.z;
}
