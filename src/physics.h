#ifndef PHYSICS_H
#define PHYSICS_H

#include "game.h"
#include "player.h"

int physicsProcess(float deltaTime, Player &player, World &world, Camera3D &camera);
float getBoost();

#endif
