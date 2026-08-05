#include "game.h"
#include "objects.h"
#include "raymath.h"
#include <cmath>
#include <float.h>

void StaticBody::UpdateAABB() {
    BoundingBox modelAABB = GetModelBoundingBox(model);

    Matrix transform = MatrixScale(scale.x, scale.y, scale.z);
    transform = MatrixMultiply(transform, MatrixRotateY(rotation * DEG2RAD));
    transform = MatrixMultiply(transform, MatrixTranslate(position.x, position.y, position.z));

    Vector3 corners[8] = {
            {modelAABB.min.x, modelAABB.min.y, modelAABB.min.z},
            {modelAABB.min.x, modelAABB.min.y, modelAABB.max.z},
            {modelAABB.min.x, modelAABB.max.y, modelAABB.min.z},
            {modelAABB.min.x, modelAABB.max.y, modelAABB.max.z},
            {modelAABB.max.x, modelAABB.min.y, modelAABB.min.z},
            {modelAABB.max.x, modelAABB.min.y, modelAABB.max.z},
            {modelAABB.max.x, modelAABB.max.y, modelAABB.min.z},
            {modelAABB.max.x, modelAABB.max.y, modelAABB.max.z}};

    for (int i = 0; i < 8; i++)
        corners[i] = Vector3Transform(corners[i], transform);

    aabb.min = corners[0];
    aabb.max = corners[0];

    for (int i = 1; i < 8; i++) {
        aabb.min.x = fminf(aabb.min.x, corners[i].x);
        aabb.min.y = fminf(aabb.min.y, corners[i].y);
        aabb.min.z = fminf(aabb.min.z, corners[i].z);
        aabb.max.x = fmaxf(aabb.max.x, corners[i].x);
        aabb.max.y = fmaxf(aabb.max.y, corners[i].y);
        aabb.max.z = fmaxf(aabb.max.z, corners[i].z);
    }
}

void StaticBody::ExtractTriangles(Objects::ObjectRegistry &reg) {
    triOffset = reg.trianglePoolCount;
    triCount = 0;

    Matrix transform = MatrixScale(scale.x, scale.y, scale.z);
    transform = MatrixMultiply(transform, MatrixRotateY(rotation * DEG2RAD));
    transform = MatrixMultiply(transform, MatrixTranslate(position.x, position.y, position.z));

    for (int m = 0; m < model.meshCount; m++) {
        Mesh &mesh = model.meshes[m];
        for (int t = 0; t < mesh.triangleCount; t++) {
            if (reg.trianglePoolCount >= Objects::MAX_TOTAL_TRIANGLES) {
                TraceLog(LOG_WARNING, "Triangle pool full — triangles truncated");
                return;
            }

            unsigned int i0, i1, i2;
            if (mesh.indices) {
                i0 = mesh.indices[t * 3 + 0];
                i1 = mesh.indices[t * 3 + 1];
                i2 = mesh.indices[t * 3 + 2];
            } else {
                i0 = (unsigned int)(t * 3 + 0);
                i1 = (unsigned int)(t * 3 + 1);
                i2 = (unsigned int)(t * 3 + 2);
            }

            Vector3 lv0 = {mesh.vertices[i0*3], mesh.vertices[i0*3+1], mesh.vertices[i0*3+2]};
            Vector3 lv1 = {mesh.vertices[i1*3], mesh.vertices[i1*3+1], mesh.vertices[i1*3+2]};
            Vector3 lv2 = {mesh.vertices[i2*3], mesh.vertices[i2*3+1], mesh.vertices[i2*3+2]};

            Vector3 wv0 = Vector3Transform(lv0, transform);
            Vector3 wv1 = Vector3Transform(lv1, transform);
            Vector3 wv2 = Vector3Transform(lv2, transform);

            Vector3 cross = Vector3CrossProduct(Vector3Subtract(wv1, wv0), Vector3Subtract(wv2, wv0));
            float len = Vector3Length(cross);
            if (len < 1e-6f) continue;

            Triangle &tri = reg.trianglePool[reg.trianglePoolCount];
            tri.v0 = wv0;
            tri.v1 = wv1;
            tri.v2 = wv2;
            tri.normal = Vector3Scale(cross, 1.0f / len);

            reg.trianglePoolCount++;
            triCount++;
        }
    }
}

void StaticBody::draw() {
    DrawModelEx(model, position, (Vector3){0, 1, 0}, rotation, scale, tint);
}