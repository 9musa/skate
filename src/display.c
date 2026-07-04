#include "raylib.h"

const int windowW = 640;
const int windowH = 360;

int initDisplay (void) {
    InitWindow(windowW, windowH, "C-8");
    return 0;
}
int drawDisplay (void) {
    BeginDrawing();
    ClearBackground(BLACK);
    EndDrawing();
    return 0;
}
int closeDisplay (void) {
    CloseWindow();
    return 0;
}