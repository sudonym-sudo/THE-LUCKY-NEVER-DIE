#include "input.h"
#include "objects.h"
#include "raymath.h"

Vector2 getKeyVector() {
    return (Vector2){
            (float)IsKeyDown(KEY_W) - (float)IsKeyDown(KEY_S),
            (float)IsKeyDown(KEY_D) - (float)IsKeyDown(KEY_A),
    };
}

void inputProcess(float deltaTime, Player &player, Camera3D &camera) {
    Vector2 keyVector = getKeyVector();

    if (Vector2Length(keyVector) > 0) {
        Vector3 forward = Vector3Subtract(camera.target, camera.position);
        forward.y = 0;
        forward = Vector3Normalize(forward);
        Vector3 right = Vector3CrossProduct(forward, camera.up);
        Vector2 inputDir = Vector2Normalize(keyVector);

        Vector3 moveDir = Vector3Add(
                Vector3Scale(forward, inputDir.x),
                Vector3Scale(right, inputDir.y));

        float prevSpeed = Vector2Length((Vector2){player.movement.velocity.x, player.movement.velocity.z});

        player.movement.velocity.x += moveDir.x * player.movement.acceleration * deltaTime;
        player.movement.velocity.z += moveDir.z * player.movement.acceleration * deltaTime;

        float newSpeed = Vector2Length((Vector2){player.movement.velocity.x, player.movement.velocity.z});
        float maxSpeed = fmaxf(player.movement.maxSpeed, prevSpeed);
        if (newSpeed > maxSpeed) {
            player.movement.velocity.x *= maxSpeed / newSpeed;
            player.movement.velocity.z *= maxSpeed / newSpeed;
        }
    }

    if (IsKeyPressed(KEY_SPACE)) {
        player.movement.bufferTimer = player.movement.bufferTime;
    }

    if (IsKeyPressed(KEY_E)) {
        int count = 0;
        const int *ids = Objects::Find("type", "weapon", &count);
        if (count > 0) {
            int itemId = ids[0];
            bool stashed = false;
            for (int i = 0; i < Player::MAX_INVENTORY_SIZE; i++) {
                if (player.inventory.items[i] == itemId) { stashed = true; break; }
            }
            if (stashed) player.Unstash(itemId, true);
            else player.Stash(itemId, true);
        }
    }
    if (IsKeyPressed(KEY_Q)) {
        // hold item
        int count = 0;
        const int *ids = Objects::Find("type", "weapon", &count);
        if (count > 0) {
            int itemId = ids[0];
            player.Hold(itemId);
        }
    }

    if (IsKeyPressed(KEY_R)) {
        player.Hold(-1);
    }
}
