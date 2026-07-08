#ifndef PHYSICS_H
#define PHYSICS_H

#include <cmath>

struct Vector2 {
    float x;
    float y;
};

namespace Physics {
    float CalculateDistance(Vector2 a, Vector2 b);
    Vector2 Normalize(Vector2 v);
    float DotProduct(Vector2 a, Vector2 b);
    Vector2 RotatePoint(Vector2 point, Vector2 center, float angle);
}

#endif
