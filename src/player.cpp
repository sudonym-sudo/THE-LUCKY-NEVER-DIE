#include "player.h"
#include "raymath.h"
#include <cmath>
#include <float.h>

bool Player::isValidItemId(int itemId) {
    return itemId >= 0 && itemId < Player::MAX_INVENTORY_SIZE;
}

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

    Vector3 baseOffset = Vector3Add(Vector3Scale(forward, visual.armConfig.dist),
        Vector3Scale((Vector3){0, 1, 0}, visual.armConfig.height));
    Vector3 leftArmPos = Vector3Add(camera.position,
        Vector3Add(baseOffset, Vector3Scale(right, -visual.armConfig.width)));
    Vector3 rightArmPos = Vector3Add(camera.position,
        Vector3Add(baseOffset, Vector3Scale(right, visual.armConfig.width)));

    UpdateModelOrientation(&visual.armModel, camera);

    DrawModelEx(visual.armModel, leftArmPos, (Vector3){0, 1, 0}, 0.0f, (Vector3){1, 1, 1}, RED);
    DrawModelEx(visual.armModel, rightArmPos, (Vector3){0, 1, 0}, 0.0f, (Vector3){1, 1, 1}, RED);
}

void Player::Stash(int itemId, bool canPickup) {
    if (!isValidItemId(itemId)) return;
    if (inventory.items[itemId] > 0) {
        inventory.items[itemId]--;
        if (canPickup) {
            return; // TODO
        }
    }
}

void Player::Unstash(int itemId, bool canDrop) {
    if (!isValidItemId(itemId)) return;
    if (inventory.items[itemId] < 32) {
        inventory.items[itemId]++;
        if (canDrop) {
            return; // TODO
        }
    }
}

void Player::Hold(int itemId) {
    if (!isValidItemId(itemId)) return;
    inventory.hand = itemId;
}
