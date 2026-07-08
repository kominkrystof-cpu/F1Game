#include "Physics.h"
#include <cmath>

namespace Physics {

float CalculateDistance(Vector2 a, Vector2 b) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    return sqrtf(dx * dx + dy * dy);
}

Vector2 Normalize(Vector2 v) {
    float len = sqrtf(v.x * v.x + v.y * v.y);
    if (len == 0) return {0, 0};
    return {v.x / len, v.y / len};
}

float DotProduct(Vector2 a, Vector2 b) {
    return a.x * b.x + a.y * b.y;
}

Vector2 RotatePoint(Vector2 point, Vector2 center, float angle) {
    float radians = angle * 3.14159265f / 180.0f;
    float cosA = cosf(radians);
    float sinA = sinf(radians);
    
    float dx = point.x - center.x;
    float dy = point.y - center.y;
    
    return {
        center.x + dx * cosA - dy * sinA,
        center.y + dx * sinA + dy * cosA
    };
}

}
