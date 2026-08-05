#include "physics.h"
#include "objects.h"
#include "player.h"
#include "raymath.h"
#include <math.h>

static void applyFriction(float deltaTime, Player &player) {
    player.movement.velocity.x *= (1.0f - player.movement.friction * deltaTime);
    player.movement.velocity.z *= (1.0f - player.movement.friction * deltaTime);
}

static void applyGravity(float deltaTime, Player &player, World &world) {
    player.movement.velocity.y -= world.gravity * deltaTime;
}

static void floorCheck(Player &player) {
    if (player.position.y <= -120.0f) {
        player.position.y = -120.0f;
        player.collision.grounded = true;
        if (player.movement.velocity.y < 0)
            player.movement.velocity.y = 0;
    }
}

static void applyJump(Player &player) {
    if (player.movement.bufferTimer > 0 && player.collision.grounded) {
        player.movement.velocity.y = 55.0f;
        player.movement.bufferTimer = 0;
    }
}

static void bufferCountdown(float deltaTime, Player &player) {
    if (player.movement.bufferTimer > 0) {
        player.movement.bufferTimer -= deltaTime;
    }
}

static void getCollidingBodies(Player &player, StaticBody allBodies[], int count) {
    player.collision.bodyCount = 0;
    for (int i = 0; i < count; i++) {
        if (CheckCollisionBoxes(player.collision.aabb, allBodies[i].aabb)) {
            if (player.collision.bodyCount < 16) {
                player.collision.bodies[player.collision.bodyCount++] = &allBodies[i];
            }
        }
    }
}

static float boost = 0.0f;
static const float BOOST_MAX = 10.0f;
static const float ANGLE_MAX = 90.0f;
static const float ANGLE_MIN = -ANGLE_MAX;

static void updateBoost(float deltaTime, Player &player) {
    boost += fmaxf(0.0f, ( (4.0f * BOOST_MAX) / (ANGLE_MAX - ANGLE_MIN) ) * ( (fabsf(player.pitch) * RAD2DEG) - ( (ANGLE_MIN + ANGLE_MAX) / 2.0f) ) * deltaTime);
}

float getBoost() {
    return boost;
}

static const float SLOPE_FLOOR_Y = 0.85f;
static const float SLOPE_WALL_Y  = 0.30f;

static void slide(Player &player, const Vector3 &n, float gravity, float deltaTime) {
    float sdx = n.y * n.x;
    float sdy = -1.0f + n.y * n.y;
    float sdz = n.y * n.z;
    float len = sqrtf(sdx*sdx + sdy*sdy + sdz*sdz);
    if (len > 1e-4f) {
        float slideAccel = gravity * sqrtf(1.0f - n.y * n.y);
        float inv        = slideAccel / len;
        player.movement.velocity.x += sdx * inv * deltaTime;
        player.movement.velocity.y += sdy * inv * deltaTime;
        player.movement.velocity.z += sdz * inv * deltaTime;
    }
}

static void resolveBodyTriangles(Player &player, StaticBody &body, float gravity, float deltaTime) {
    const BoundingBox &box = player.collision.aabb;
    for (int t = 0; t < body.triCount; t++) {
        Triangle &tri = Objects::registry.trianglePool[body.triOffset + t];

        // ------------------- BROAD PHASE -------------------
        if (fmaxf(fmaxf(tri.v0.x, tri.v1.x), tri.v2.x) < box.min.x ||
            fminf(fminf(tri.v0.x, tri.v1.x), tri.v2.x) > box.max.x) continue;
        if (fmaxf(fmaxf(tri.v0.y, tri.v1.y), tri.v2.y) < box.min.y ||
            fminf(fminf(tri.v0.y, tri.v1.y), tri.v2.y) > box.max.y) continue;
        if (fmaxf(fmaxf(tri.v0.z, tri.v1.z), tri.v2.z) < box.min.z ||
            fminf(fminf(tri.v0.z, tri.v1.z), tri.v2.z) > box.max.z) continue;

        // ------------------- NARROW PHASE -------------------
        const Vector3 &n = tri.normal;
        float cx = (box.min.x + box.max.x) * 0.5f;
        float cy = (box.min.y + box.max.y) * 0.5f;
        float cz = (box.min.z + box.max.z) * 0.5f;
        float hx = (box.max.x - box.min.x) * 0.5f;
        float hy = (box.max.y - box.min.y) * 0.5f;
        float hz = (box.max.z - box.min.z) * 0.5f;

        float r = fabsf(hx*n.x) + fabsf(hy*n.y) + fabsf(hz*n.z);
        float d = n.x*(cx-tri.v0.x) + n.y*(cy-tri.v0.y) + n.z*(cz-tri.v0.z);

        if (d < 0.0f || d >= r) continue;

        float depth = r - d;
        player.position.x += n.x * depth;
        player.position.y += n.y * depth;
        player.position.z += n.z * depth;
        player.UpdateAABB();

        float vDotN = player.movement.velocity.x*n.x + player.movement.velocity.y*n.y + player.movement.velocity.z*n.z;
        if (vDotN < 0.0f) {
            player.movement.velocity.x -= n.x * vDotN;
            player.movement.velocity.y -= n.y * vDotN;
            player.movement.velocity.z -= n.z * vDotN;
        }

        if (n.y > SLOPE_FLOOR_Y) {
            player.collision.grounded = true;
        } else if (n.y > SLOPE_WALL_Y) {
            slide(player, n, gravity, deltaTime);
        }
    }
}

int physicsProcess(float deltaTime, Player &player, World &world, Camera3D &camera) {
    player.collision.grounded = false;

    applyFriction(deltaTime, player);
    applyGravity(deltaTime, player, world);

    player.position.x += player.movement.velocity.x * deltaTime;
    player.position.z += player.movement.velocity.z * deltaTime;
    player.position.y += player.movement.velocity.y * deltaTime;

    floorCheck(player);

    player.UpdateAABB();

    int bodyCount = Objects::registry.bodyCount;
    StaticBody *allBodies = Objects::registry.bodies;
    getCollidingBodies(player, allBodies, bodyCount);

    if (player.collision.bodyCount > 0) {
        for (int i = 0; i < player.collision.bodyCount; i++) {
            resolveBodyTriangles(player, *player.collision.bodies[i], world.gravity, deltaTime);
        }
    }

    if (!player.collision.wasGrounded && player.collision.grounded) {
        float hSpeed = sqrtf(player.movement.velocity.x * player.movement.velocity.x + player.movement.velocity.z * player.movement.velocity.z);
        if (hSpeed > 0.001f) {
            player.movement.velocity.x += boost * (player.movement.velocity.x / hSpeed);
            player.movement.velocity.z += boost * (player.movement.velocity.z / hSpeed);
            player.movement.maxSpeed += boost;
        }
        boost = 0.0f;
        player.collision.groundTimer = 0.0f;
    }

    if (player.collision.grounded) {
        player.collision.groundTimer += deltaTime;
        if (player.collision.groundTimer > 1.5f) {
            player.movement.maxSpeed = player.movement.speed;
        }
    } else {
        updateBoost(deltaTime, player);
    }

    player.collision.wasGrounded = player.collision.grounded;

    applyJump(player);
    bufferCountdown(deltaTime, player);

    Vector3 newCamPos = (Vector3){player.position.x, player.position.y + player.collision.height, player.position.z};
    Vector3 moveDiff = Vector3Subtract(newCamPos, camera.position);
    camera.position = newCamPos;
    camera.target = Vector3Add(camera.target, moveDiff);

    return player.collision.bodyCount;
}
