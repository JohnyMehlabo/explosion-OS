#include "renderer.h"
#include "ray.h"
#include "math.h"
#include "utils/array.h"
#include "stdbool.h"

typedef struct {
    Vector3 albedo;
} Material;

typedef struct {
    Vector3 center;
    float radius;
    Material material;
} Sphere;

Sphere spheres[] = {
    {
        .center = {-0.7, 0.0, 1.5},
        .radius = 0.5,
        .material = {
            .albedo = {0.0, 0.0, 1.0}
        }
    },
    {
        .center = {1.0, 0.0, 2.0},
        .radius = 0.5,
        .material = {
            .albedo = {0.0, 1.0, 0.0}
        }
    },
    {
        .center = {0.0, 0.0, 1.0},
        .radius = 0.5,
        .material = {
            .albedo = {1.0, 1.0, 0.0}
        }
    }
};


float Renderer_RaySphereIntersection(const Ray* ray, const Sphere* sphere) {
    Vector3 oc;
    oc.x = ray->origin.x - sphere->center.x;
    oc.y = ray->origin.y - sphere->center.y;
    oc.z = ray->origin.z - sphere->center.z;
    float a = Vector3_Dot(&ray->direction, &ray->direction);
    float b = 2 * Vector3_Dot(&oc, &ray->direction);
    float c = Vector3_Dot(&oc, &oc) - sphere->radius * sphere->radius;
    float discriminant = b * b - 4 * a * c;
    
    // If discriminant is less than 0, there are no intersections
    if (discriminant < 0) return -1.0;

    return (-b - sqrt(discriminant)) / (2*a);
}

void Renderer_SphereHitNormal(const Sphere* sphere, const Vector3* hitPosition, Vector3* normalOut) {
    normalOut->x = hitPosition->x - sphere->center.x;
    normalOut->y = hitPosition->y - sphere->center.y;
    normalOut->z = hitPosition->z - sphere->center.z;

    Vector3_Normalize(normalOut, normalOut);
}

void Renderer_PerPixel(float x, float y, Vector3* colorOut) {
    // Create our ray with origin 0,0 pointing to the corresponding pixel
    Ray ray = {0};
    Vector3 direction = {x, y, 1.0};
    Vector3_Normalize(&direction, &direction);
    ray.direction = direction;

    static Vector3 noHitColor = {0.0, 0.0, 0.0};
    
    static Vector3 lightDirection = {0.0, 1.0, 1.0};

    // Make sure light direction is normalized
    Vector3_Normalize(&lightDirection, &lightDirection);
    
    // Set color so if nothing hits this one is returned
    *colorOut = noHitColor;

    float closestDst = 3.402823466e38;
    const Sphere* closestSphere;
    bool didHit = false;

    for (uint32_t i = 0; i < ARRAY_SIZE(spheres); i++) {
        const Sphere* sphere = &spheres[i];
        float distance = Renderer_RaySphereIntersection(&ray, sphere);
        if (distance > 0 && distance < closestDst) {
            closestDst = distance;
            closestSphere = sphere;
            didHit = true;
        }
    }
    if (didHit) {
        // Calculate hit position
        Vector3 hitPosition;
        Ray_PointAtLength(&ray, closestDst, &hitPosition);

        // Calculate normal at hit position
        Vector3 normal;
        Renderer_SphereHitNormal(closestSphere, &hitPosition, &normal);

        // TODO: Inverse light dir should only be calculated once
        Vector3 inverseLightDir;
        inverseLightDir.x = -lightDirection.x;
        inverseLightDir.y = -lightDirection.y;
        inverseLightDir.z = -lightDirection.z;
        
        float lightAmount = Vector3_Dot(&normal, &inverseLightDir);
        float clampedLightAmount = lightAmount > 0 ? lightAmount : 0;

        colorOut->x = closestSphere->material.albedo.x * clampedLightAmount;
        colorOut->y = closestSphere->material.albedo.y * clampedLightAmount;
        colorOut->z = closestSphere->material.albedo.z * clampedLightAmount;
    } 
}