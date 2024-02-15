/*
    Copyright (C) 2024  Joshua Jackson
    [Full GPL-3.0 license can be found in LICENSE, in the root folder]
*/

#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#include <stdint.h>

#include "cpu.h"
#include "graphics.h"

static int finishInitScreen = 0;
static int finishMainScreen = 0;

Color DOSBLUE = {.r = 0, .g = 0, .b = 170, .a = 200};


void InitInitScreen() {
    finishInitScreen = 0;
    
}

void UpdateInitScreen() {
    if (IsKeyPressed(KEY_ENTER)) {
        finishInitScreen = 1;
        printf("FUCK\n\n\n\n\n");
    }

}

void DrawInitScreen() {
    // TODO: 
    //      -animated "intro" on init screen, with lines being typed out character by character
    //       and animated elipses while waiting for [enter] input
    ClearBackground(DOSBLUE);
    DrawTextEx(font, "> CTRL-6502:", (Vector2){ 100.0f, 60.0f }, 30, 2, WHITE);
    DrawTextEx(font, "> \t\tA C-based MOS 6502 Emulator,", (Vector2){ 100.0f, 90.0f }, 30, 2, WHITE);
    DrawTextEx(font, "> \t\twith I/O, graphics, and memory monitoring", (Vector2){ 100.0f, 120.0f }, 30, 2, WHITE);

    DrawTextEx(font, "> Press [ENTER] to begin", (Vector2){ 100.0f, 150.0f }, 30, 2, WHITE);

    DrawTextEx(font, "                           .,,uod8B8bou,,.\n\n              ..,uod8BBBBBBBBBBBBBBBBRPFT?l!i:.\n\n         ,=m8BBBBBBBBBBBBBBBRPFT?!||||||||||||||\n\n         !...:!TVBBBRPFT||||||||||!!^^\"\"\'   ||||\n\n         !.......:!?|||||!!^^\"\"\'            ||||\n\n         !.........||||                     ||||\n\n         !.........||||  ## ctrl-6502       ||||\n\n         !.........||||  ## by ctrl-j 2024  ||||\n\n         !.........||||                     ||||\n\n         !.........||||                     ||||\n\n         !.........||||                     ||||\n\n         `.........||||                    ,||||\n\n          .;.......||||               _.-!!|||||\n\n   .,uodWBBBBb.....||||       _.-!!|||||||||!:\'\n\n!YBBBBBBBBBBBBBBb..!|||:..-!!|||||||!iof68BBBBBb....\n\n!..YBBBBBBBBBBBBBBb!!||||||||!iof68BBBBBBRPFT?!::   `.\n\n!....YBBBBBBBBBBBBBBbaaitf68BBBBBBRPFT?!:::::::::     `.\n\n!......YBBBBBBBBBBBBBBBBBBBRPFT?!::::::;:!^\"`;:::       `.\n\n!........YBBBBBBBBBBRPFT?!::::::::::^\'\'...::::::;         iBBbo.\n\n`..........YBRPFT?!::::::::::::::::::::::::;iof68bo.      WBBBBbo.\n\n  `..........:::::::::::::::::::::::;iof688888888888b.     `YBBBP^\'\n\n    `........::::::::::::::::;iof688888888888888888888b.     `\n\n      `......:::::::::;iof688888888888888888888888888888b.\n\n        `....:::;iof688888888888888888888888888888888899fT!\n\n          `..::!8888888888888888888888888888888899fT|!^\"\'\n\n            `\' !!988888888888888888888888899fT|!^\"\'\n\n                `!!8888888888888888899fT|!^\"\'\n\n                  `!988888888899fT|!^\"\'\n\n                    `!9899fT|!^\"\'\n\n                      `!^\"\'", (Vector2){ 100.0f, 230.0f }, 30, 2, WHITE);
}

void UnloadInitScreen() {
    // Unload Init screen variables
}

int FinishInitScreen() {
    return finishInitScreen;
}



void InitMainScreen(CONSOLE_BUFF* CLI_BUFFER) {
    finishMainScreen = 0;
    int i = 0;
    int j = 0;

    // Initialize command buffer structure
    CLI_BUFFER->CHAR_WIDTH = 80;
    CLI_BUFFER->CMD_BUFF = malloc(sizeof(char[40][80]));
    for (j = 0; j < 40; j++) {
        i = 0;
        printf("i: %d\n", i);
        for (i = 0; i < 80; i++) {
            CLI_BUFFER->CMD_BUFF[j][i] = (i % 26) + 65;
            fputc(CLI_BUFFER->CMD_BUFF[j][i], stdout);
            
        }
        
        //printf("j: %d\n", j);
    }
    CLI_BUFFER->PREV_INPUT = malloc(sizeof(char[80]) * 16);

}

void UpdateMainScreen(CONSOLE_BUFF* CLI_BUFFER) {
    if (IsKeyPressed(KEY_ENTER)) {
        printf("FUCK\n\n\n\n\n");
    }

    // Draw console text


}

void DrawMainScreen(CONSOLE_BUFF* CLI_BUFFER) {
    ClearBackground(WHITE);

    // Draw console area 
    DrawRectangle(768, 0, 1032, 1200, DOSBLUE);
    // TODO: buffer to store past lines of output/input
    
    // Draw memory area
    DrawRectangle(0, 720, 768, 480, RAYWHITE);

    // Draw screen area
    // 3x (256, 240) pixels
    DrawRectangle(0, 0, 768, 720, BLACK);
}

void UnloadMainScreen(CONSOLE_BUFF* CLI_BUFFER) {
    // Unload Main screen variables
}

int FinishMainScreen() {
    return finishMainScreen;
}