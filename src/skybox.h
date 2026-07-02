#ifndef SKYBOX_H
#define SKYBOX_H

#include "raylib.h"

namespace Skybox {
    void Load(const char *panoramaPath);
    void Draw();
    void Unload();
}

#endif
