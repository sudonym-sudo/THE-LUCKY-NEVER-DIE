#ifndef GAME_H
#define GAME_H

#include "raylib.h"

struct Triangle {
    Vector3 v0, v1, v2;
    Vector3 normal;
};

namespace Objects { struct ObjectRegistry; }

struct StaticBody {
    Model model;
    Vector3 position;
    float rotation;
    Vector3 scale;
    BoundingBox aabb;
    int triOffset;
    int triCount;
    Color tint = WHITE;
    int instanceId = 0;

    void UpdateAABB();
    void ExtractTriangles(Objects::ObjectRegistry &reg);
    void draw();
};

struct Item {
    int id;
    const char *name;
};

enum { MAX_ITEMS = 32 };

struct World {
    float gravity = 120.0f;
};

#endif
