/*
    Copyright (C) 2024  Joshua Jackson
    [Full GPL-3.0 license can be found in LICENSE, in the root folder]
*/

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
    int WIDTH = 0;
    int HEIGHT = 0;
    
    // Initialize window, title will be set once resolution properly assigned
    InitWindow(1, 1, "");
    // Initialize audio device
    InitAudioDevice();

    // Load resolution from CONFIG
    // Default is 256W x 240H
    cJSON* ITEM = cJSON_GetObjectItem(GRAPHICS_CONFIG, "RES");
    
    // Parse window
    // Change window size based on target resolution
    int DISP_WIDTH = GetMonitorWidth(0);
    printf("DISP_WIDTH: %d\n", DISP_WIDTH);

    // For 2560 x 1440 displays (my laptop)
    if (DISP_WIDTH == 2560) {
        ITEM = cJSON_GetObjectItem(ITEM, "Large");
        ITEM = cJSON_GetObjectItem(ITEM, "APP");

        WIDTH = cJSON_GetArrayItem(ITEM, 0)->valueint;
        HEIGHT = cJSON_GetArrayItem(ITEM, 1)->valueint;
    }
    // For 1920 x 1080 displays (average display)
    if (DISP_WIDTH == 1920) {
        ITEM = cJSON_GetObjectItem(ITEM, "Normal");
        ITEM = cJSON_GetObjectItem(ITEM, "APP");

        WIDTH = cJSON_GetArrayItem(ITEM, 0)->valueint;
        HEIGHT = cJSON_GetArrayItem(ITEM, 1)->valueint;
    }
    
    printf("Width: %d, Height: %d\n", WIDTH, HEIGHT);

    // Format window title from CONFIG
    ITEM = cJSON_GetObjectItem(GRAPHICS_CONFIG, "TITLE");
    // Parse platform-dependent description
    ITEM = cJSON_GetObjectItem(ITEM, "DESC");
    char BUFF[256];
    char* MSG = ITEM->valuestring;
    // Parse version number
    ITEM = cJSON_GetObjectItem(GRAPHICS_CONFIG, "TITLE");
    ITEM = cJSON_GetObjectItem(ITEM, "VER");
    double VERSION = ITEM->valuedouble;

    // Format window title string
    snprintf(BUFF, 256, MSG, WIDTH, HEIGHT, VERSION, PLATFORM);

    // Set window size and title based on scaled resolution
    SetWindowSize(WIDTH, HEIGHT);
    SetWindowTitle(BUFF);

    return 0;
}

// Update and draw next frame
void UpdateDrawFrame(CONSOLE_BUFF* CLI_BUFFER) {
    switch (currentScreen) {
        case INIT:
            UpdateInitScreen();
            if (FinishInitScreen()) ChangeToScreen(MAIN, CLI_BUFFER);
            break;

        case MAIN:
            UpdateMainScreen(CLI_BUFFER);
            break;
        
        default:
            break;
    }

    BeginDrawing();
        switch (currentScreen) {
            case INIT: DrawInitScreen(); break;
            case MAIN: DrawMainScreen(CLI_BUFFER); break;
            default:
                break;
        }

    EndDrawing();

}

void ChangeToScreen(ScreenPhase screen, CONSOLE_BUFF* CLI_BUFFER) {
    // Unload current screen
    switch (currentScreen) {
        case INIT: UnloadInitScreen(); break;
        case MAIN: UnloadMainScreen(CLI_BUFFER); break;
        default: break;
    }

    // Init next screen
    switch (screen) {
        case INIT: InitInitScreen(); break;
        case MAIN: InitMainScreen(CLI_BUFFER); break;
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