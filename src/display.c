#include "raylib.h"
#include "chip8.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


const char *footer = u8"© 2026 Benevolence Labs";
char **roms = NULL; // rom names
char **romPaths = NULL; // rom paths
int romCount = 0; // no of elements = size of list / size of one element
int selectedRom = 0; // highlighted
const int windowWIDTH = 1280;
const int windowHEIGHT = 680;
const int SCALE = 10;
Image icon;
const Color AMBER = { 255, 191, 0, 255 };
Font titleFont;
Font listFont;
Font footerFont;
Vector2 titleTextSize;
Vector2 listTextSize;
Vector2 footerSize;
int codepoints[224]; // ©


void initDisplay (void) {
    InitWindow(windowWIDTH, windowHEIGHT, "SC-8");
    for (int i = 0; i < 224; i++)
        codepoints[i] = 32 + i;
    icon = LoadImage("assets/logo.png");
    SetWindowIcon(icon);
    UnloadImage(icon);
    titleFont = LoadFontEx("assets/fonts/Michroma-Regular.ttf", 80, NULL, 0);
    listFont = LoadFontEx("assets/fonts/Michroma-Regular.ttf", 30, NULL, 0);
    footerFont = LoadFontEx("assets/fonts/Michroma-Regular.ttf", 20, codepoints, 224);
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
    titleTextSize = MeasureTextEx(titleFont, "SC-8", 80, 2);
    footerSize = MeasureTextEx(footerFont, footer, 20, 1);
    DrawTextEx(titleFont, "SC-8", (Vector2){ (windowWIDTH / 2) - (titleTextSize.x / 2) , 40 }, 80, 2, AMBER);
    int startIndex = 0;
    if (selectedRom >= 20) {
        startIndex = selectedRom - 19; // Keeps the selected item at the bottom of the view
    }
    int endIndex = startIndex + 20;
    if (endIndex > romCount) {
        endIndex = romCount;
    }
    for (int i = startIndex; i < endIndex; i++) {
        const char* displayText;
        if (i == selectedRom) {
            displayText = TextFormat("> %s", roms[i]);
        }
        else {
            displayText = roms[i];
        }
        listTextSize = MeasureTextEx(listFont, displayText, 30, 2);
        int relativeRow = i - startIndex;
        DrawTextEx(listFont, displayText, (Vector2){ (windowWIDTH / 2) - (listTextSize.x / 2) , (200 + (relativeRow * 20)) }, 30, 2, AMBER);
    }
    DrawTextEx(footerFont, footer, (Vector2){ windowWIDTH - footerSize.x - 10, windowHEIGHT - footerSize.y - 8 }, 20, 1, DARKGRAY);
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
    if (IsKeyPressed(KEY_ESCAPE)) {
        state = STATE_MENU;
    }
}

void drawDisplay (void) {
    BeginDrawing();
    ClearBackground(BLACK);
    footerSize = MeasureTextEx(footerFont, footer, 20, 1);
    int currentWidth  = highResMode ? 128 : 64;
    int currentHeight = highResMode ? 64  : 32;
    int currentScale  = highResMode ? SCALE : (SCALE * 2);
    for (int y = 0; y < currentHeight; y++) {
        for (int x = 0; x < currentWidth; x++) {
            if (disp[y * 128 + x]) {
                DrawRectangle(x * currentScale, y * currentScale, currentScale, currentScale, AMBER);
            }
        }
    }
    DrawTextEx(footerFont, footer, (Vector2){ windowWIDTH - footerSize.x - 10, windowHEIGHT - footerSize.y - 8 }, 20, 1, DARKGRAY);
    EndDrawing();
}

void closeDisplay (void) {
    UnloadFont(titleFont);
    UnloadFont(listFont);
    UnloadFont(footerFont);
    CloseWindow();
}