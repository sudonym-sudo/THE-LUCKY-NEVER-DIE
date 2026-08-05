#include "debug.h"
#include "raylib.h"
#include <cstdio>
#include <cstring>

Debug::Debug() : count(0) {}

void Debug::Log(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char buf[MSG_LEN];
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (count < MAX_LOGS) count++;

    for (int i = count - 1; i > 0; i--)
        strncpy(messages[i], messages[i - 1], MSG_LEN);

    strncpy(messages[0], buf, MSG_LEN - 1);
    messages[0][MSG_LEN - 1] = '\0';
}

void Debug::Clear()
{
    count = 0;
}

void Debug::Draw(Font font)
{
    int fontSize = 28;
    int lineHeight = fontSize + 6;
    int xPad = 10;
    int yStart = 10;
    int screenW = GetScreenWidth();

    for (int i = 0; i < count; i++) {
        Vector2 size = MeasureTextEx(font, messages[i], fontSize, 1);
        int x = screenW - xPad - (int)size.x;
        int y = yStart + i * lineHeight;
        DrawTextEx(font, messages[i], (Vector2){(float)x, (float)y}, fontSize, 1, BLACK);
    }
}
