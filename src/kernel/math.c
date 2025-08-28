#include "math.h"
#include "stdint.h"

// float sqrt(float number) {
//     if (number < 0.0f) {
//         return -1.0f;  // Return -1.0 to indicate an error (sqrt of negative number)
//     }
//     if (number == 0.0f) {
//         return 0.0f;
//     }

//     float x = number;
//     float guess = number / 2.0f;
//     const float epsilon = 1e-6f;  // Desired precision

//     while (fabs(guess * guess - x) > epsilon) {
//         guess = (guess + x / guess) / 2.0f;
//     }

//     return guess;
// }

float sqrt(float number) {
    float result;
    __asm__ __volatile__ (
        "fsqrt"
        : "=t"(result)
        : "0"(number)
    );
    return result;
}

float fabs(float x) {
    return x > 0 ? x : -x;
}

float fmin(float x, float y) {
    return x > y ? y : x;
}

float fmax(float x, float y) {
    return x > y ? x : y;
}

float round(float x) {
    uint32_t wholePart = (uint32_t)x;
    float decimalPart = x - wholePart;

    if (decimalPart >= 0.5) return wholePart+1;
    else return wholePart;
}