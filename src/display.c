#include "raylib.h"
#include "chip8.h"

const int windowWIDTH = 640;
const int windowHEIGHT = 360;
const int SCALE = 10;
const Color AMBER = { 255, 191, 0, 255 };

void initDisplay (void) {
    InitWindow(windowWIDTH, windowHEIGHT, "C-8");
}
void drawDisplay (void) {
    BeginDrawing();
    ClearBackground(BLACK);
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (disp[y * WIDTH + x]) {
                DrawRectangle(x * SCALE, y * SCALE, SCALE, SCALE, AMBER);
            }
        }
    }
    EndDrawing();
}
void closeDisplay (void) {
    CloseWindow();
}