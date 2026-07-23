#include "player.h"
#include "objects.h"
#include "raymath.h"
#include <cmath>
#include <cstdio>
#include <float.h>

inline bool Player::isValid(int itemId) { return itemId > 0; }

void Player::UpdateAABB() {
    float bodyHeight = collision.height + 0.5f;
    collision.aabb.min = (Vector3){position.x - collision.width / 2, position.y, position.z - collision.depth / 2};
    collision.aabb.max = (Vector3){position.x + collision.width / 2, position.y + bodyHeight, position.z + collision.depth / 2};
}

void Player::UpdateModelOrientation(Model *model, Camera3D camera) {
    Vector3 dir = Vector3Normalize(Vector3Subtract(camera.target, camera.position));

    yaw = atan2f(dir.x, dir.z);

    float horizontalDist = sqrtf(dir.x * dir.x + dir.z * dir.z);
    pitch = -atan2f(dir.y, horizontalDist);

    Matrix rotation = MatrixMultiply(MatrixRotateX(pitch), MatrixRotateY(yaw));
    model->transform = rotation;
}

void Player::DrawArms(Camera3D camera) {
    forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 right = Vector3Normalize(Vector3CrossProduct((Vector3){0, 1, 0}, forward));

    Vector3 baseOffset = Vector3Add(Vector3Scale(forward, visual.armConfig.dist), Vector3Scale((Vector3){0, 1, 0}, visual.armConfig.height));
    Vector3 leftArmPos = Vector3Add(camera.position, Vector3Add(baseOffset, Vector3Scale(right, -visual.armConfig.width)));
    Vector3 rightArmPos = Vector3Add(camera.position, Vector3Add(baseOffset, Vector3Scale(right, visual.armConfig.width)));

    Vector3 handGripOffset = Vector3Scale(forward, visual.heldModelConfig.dist);
    handGripPosition = Vector3Add(camera.position, Vector3Add(baseOffset, handGripOffset));

    UpdateModelOrientation(&visual.armModel, camera);

    // ARMS

    DrawModelEx(visual.armModel, leftArmPos, (Vector3){0, 1, 0}, 0.0f, (Vector3){1, 1, 1}, RED);
    DrawModelEx(visual.armModel, rightArmPos, (Vector3){0, 1, 0}, 0.0f, (Vector3){1, 1, 1}, RED);

    // HOLDING

    if(inventory.hand == 0) return;

    DrawModelEx(visual.heldModel, handGripPosition, (Vector3){0, 1, 0}, 0.0f, (Vector3){1, 1, 1}, RED);


}


void Player::Stash(int itemId, bool canPickup) {
    if (!canPickup || !isValid(itemId)) return;
    for (int i = 0; i < MAX_INVENTORY_SIZE; i++)
        if (inventory.items[i] == itemId) return; // already stashed

    for (int i = 0; i < MAX_INVENTORY_SIZE; i++) {
        if (inventory.items[i] == 0) {
            inventory.items[i] = itemId;
            inventory.count++;
            break;
        }
    }
    Objects::Despawn(itemId);
}

void Player::Unstash(int itemId, bool canDrop) {
    if (!canDrop || !isValid(itemId)) return;

    bool found = false;
    for (int i = 0; i < MAX_INVENTORY_SIZE; i++) {
        if (inventory.items[i] == itemId) {
            inventory.items[i] = 0;
            inventory.count--;
            found = true;
            break;
        }
    }
    if (!found) return;

    Vector3 scale = {1.0f, 1.0f, 1.0f};
    const char *scaleAttr = Objects::Get(itemId, "scale");
    if (scaleAttr) sscanf(scaleAttr, "%f,%f,%f", &scale.x, &scale.y, &scale.z);

    Vector3 dropPos = Vector3Add(position, Vector3Scale(forward, 3.0f));
    Objects::Spawn(itemId, dropPos, scale, 0.0f);
}

void Player::Hold(int itemId) {
    if (itemId == -1) {
        if (inventory.hand != 0) {
            UnloadModel(visual.heldModel);
            inventory.hand = 0;
        }
        return;
    }

    if (!isValid(itemId) || inventory.hand == itemId) return;
    inventory.hand = itemId;

    const char *modelPath = Objects::Get(inventory.hand, "model");
    if (modelPath) visual.heldModel = LoadModel(modelPath);
}
