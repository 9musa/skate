#include "raylib.h"
#include "chip8.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


const char *footer = u8"© 2026 Benevolence Labs";
char **roms = NULL; // rom names
char **romPaths = NULL; // rom paths
int romCount = 0; // no of elements = size of list / size of one element
int selectedRom = 0;
const int windowWIDTH = 640;
const int windowHEIGHT = 360;
const int SCALE = 10;
const Color AMBER = { 255, 191, 0, 255 };
Font FONT;
Vector2 titleTextSize;
Vector2 listTextSize;
Vector2 footerSize;
int codepoints[224];

void initDisplay (void) {
    InitWindow(windowWIDTH, windowHEIGHT, "C-8");
    for (int i = 0; i < 224; i++)
        codepoints[i] = 32 + i;
    FONT = LoadFontEx("assets/Michroma-Regular.ttf", 60, codepoints, 224);
}
void scanFolder (void) {
    FilePathList files = LoadDirectoryFiles("roms"); // scans directory
    roms = malloc(files.count * sizeof(char*)); // allocates memory to roms
    romPaths = malloc(files.count * sizeof(char*)); // and romPaths
    romCount = 0;
    // loops through every file
    for (unsigned int i = 0; i < files.count; i++) {
        if (IsFileExtension(files.paths[i], ".ch8")) {
            romPaths[romCount] = strdup(files.paths[i]); // saves path
            const char* fileName = GetFileNameWithoutExt(files.paths[i]); // strips path
            roms[romCount] = strdup(fileName); // saves name
            romCount++; // increments pointer
        }
    }
    UnloadDirectoryFiles(files); // cleanup
}
void drawMenu (void) {
    BeginDrawing();
    ClearBackground(BLACK);
    titleTextSize = MeasureTextEx(FONT, "C-8", 60, 2);
    footerSize = MeasureTextEx(FONT, footer, 20, 1);
    DrawTextEx(FONT, "C-8", (Vector2){ (windowWIDTH / 2) - (titleTextSize.x / 2) , 40 }, 60, 2, AMBER);
    for (int i = 0; i < romCount; i++) {
        const char* displayText;
        if (i == selectedRom) {
            displayText = TextFormat("> %s", roms[i]);
        }
        else {
            displayText = roms[i];
        }
        listTextSize = MeasureTextEx(FONT, displayText, 30, 2);
        DrawTextEx(FONT, displayText, (Vector2){ (windowWIDTH / 2) - (listTextSize.x / 2) , (140 + (i * 20)) }, 30, 2, AMBER);
    }
    DrawTextEx(FONT, footer, (Vector2){ windowWIDTH - footerSize.x - 10, windowHEIGHT - footerSize.y - 8 }, 20, 1, DARKGRAY);
    EndDrawing();
}
void updateMenu (void) {
    if (IsKeyPressed(KEY_UP) && selectedRom > 0) {
        selectedRom--;
    }
    if (IsKeyPressed(KEY_DOWN) && selectedRom < romCount - 1) {
        selectedRom++;
    }
    if (IsKeyPressed(KEY_ENTER)) {
        loadROM(romPaths[selectedRom]);
        state = STATE_RUNNING;
    }
}
void drawDisplay (void) {
    BeginDrawing();
    ClearBackground(BLACK);
    footerSize = MeasureTextEx(FONT, footer, 20, 1);
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (disp[y * WIDTH + x]) {
                DrawRectangle(x * SCALE, y * SCALE, SCALE, SCALE, AMBER);
            }
        }
    }
    DrawTextEx(FONT, footer, (Vector2){ windowWIDTH - footerSize.x - 10, windowHEIGHT - footerSize.y - 8 }, 20, 1, DARKGRAY);
    // need to add small footer under ts too
    EndDrawing();
}
void closeDisplay (void) {
    UnloadFont(FONT);
    CloseWindow();
}