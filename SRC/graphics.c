#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <raylib.h>
#include <string.h>

#include "./INCLUDES/cJSON.h"

#include "cpu.h"
#include "mem.h"
#include "assembler.h"
#include "tests.h"
#include "util.h"
#include "graphics.h"

HBYTE GRAPHICS_INIT(cJSON* GRAPHICS_CONFIG, char* PLATFORM) {
    // Load resolution from CONFIG
    // Default is 256W x 240H
    cJSON* ITEM = cJSON_GetObjectItem(GRAPHICS_CONFIG, "RES");
    // Parse window
    cJSON* DATA = cJSON_GetArrayItem(ITEM, 0);
    int WIDTH = DATA->valueint;
    DATA = cJSON_GetArrayItem(ITEM, 1);
    int HEIGHT = DATA->valueint;

    // Format window title from CONFIG
    ITEM = cJSON_GetObjectItem(GRAPHICS_CONFIG, "TITLE");
    // Parse platform-dependent description
    DATA = cJSON_GetObjectItem(ITEM, "DESC");
    char BUFF[256];
    char* MSG = DATA->valuestring;
    // Parse version number
    DATA = cJSON_GetObjectItem(ITEM, "VER");
    double VERSION = DATA->valuedouble;

    // Initialize window, title will be set once resolution properly assigned
    InitWindow(WIDTH, HEIGHT, "");
    // Initialize audio device
    InitAudioDevice();

    // Change window size based on target resolution
    int DISP_WIDTH = GetScreenWidth();

    // For 2560 x 1440 displays (my laptop)
    if (DISP_WIDTH == 2560) {
        WIDTH = cJSON_GetArrayItem(ITEM, 0)->valueint;
        HEIGHT = cJSON_GetArrayItem(ITEM, 1)->valueint;
    }
    // For 1920 x 1080 displays (average display)
    if (DISP_WIDTH == 1920) {
        WIDTH = cJSON_GetArrayItem(ITEM, 2)->valueint;
        HEIGHT = cJSON_GetArrayItem(ITEM, 3)->valueint;
    }

    // Format window title string
    snprintf(BUFF, 256, MSG, WIDTH, HEIGHT, VERSION, PLATFORM);

    // Set window size and title based on scaled resolution
    SetWindowSize(WIDTH, HEIGHT);
    SetWindowTitle(BUFF);

    return 0;
}

// Update and draw next frame
void UpdateDrawFrame() {
    switch (currentScreen) {
        case INIT:
            UpdateInitScreen();
            if (FinishInitScreen()) ChangeToScreen(MAIN);
            break;

        case MAIN:
            UpdateMainScreen();
            break;
        
        default:
            break;
    }

    BeginDrawing();
        switch (currentScreen) {
            case INIT: DrawInitScreen(); break;
            case MAIN: DrawMainScreen(); break;
            default:
                break;
        }

    EndDrawing();

}

void ChangeToScreen(ScreenPhase screen) {
    // Unload current screen
    switch (currentScreen) {
        case INIT: UnloadInitScreen(); break;
        case MAIN: UnloadMainScreen(); break;
        default: break;
    }

    // Init next screen
    switch (screen) {
        case INIT: InitInitScreen(); break;
        case MAIN: InitMainScreen(); break;
        default: break;
    }

    currentScreen = screen;

}

void DrawTextExMultiLine(Font drawfont, const char *text, Vector2 position, float fontSize, float spacing, Color tint, float lineSpacing) {
    char* LINE = NULL;
    char* TEXT = (char*)text;

    int n = 1;
    
    
    float X = position.x;
    float Y = position.y;
    
    LINE = strtok(TEXT, "\n");

    printf("LINE\n\n\n\n\n\n");

    while (LINE != NULL) {
        DrawTextEx(drawfont, LINE, (Vector2){ X, Y + (n * lineSpacing)}, fontSize, spacing, tint);
        LINE = strtok(NULL, "\n");
        ++n;
    }
}


void PALETTE_LOAD(PALETTE** PALETTE_LIST) {
    // TODO: move palette/color preset to graphics init binary
}

// Nametable mirror set functions

// 32x60 tilemap
void NT_MIRROR_HORIZONTAL();

// 64x30 tilemap
void NT_MIRROR_VERTICAL();

// 
void NT_MIRROR_SINGLE_SCREEN();

// 
void NT_MIRROR_FOUR_SCREEN();

// Adds sprite to OAM
void SPRITE_PUSH();

// Removes last sprite added to OAM
void SPRITE_POP();

// Edit sprite attributes in OAM
// Will update in nametable+on screen next cycle
void SPRITE_POKE();

// Clears OAM, removing all sprites from screen
void OAM_CLEAR();