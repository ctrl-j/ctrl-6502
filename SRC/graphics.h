#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include <raylib.h>

extern int SCR_RES[2];
extern int CANV_RES[2];

/********************************************
* RAYLIB Function declarations
********************************************/
void ChangeToScreen(ScreenPhase screen);
void UpdateDrawFrame();

// INIT SCREEN DRAW+UPDATE FUNCTIONS
//////////////////////////
void InitInitScreen();
void UpdateInitScreen();
void DrawInitScreen();
void UnloadInitScreen();
int FinishInitScreen();

// MAIN SCREEN DRAW+UPDATE FUNCTIONS
//////////////////////////
void InitMainScreen();
void UpdateMainScreen();
void DrawMainScreen();
void UnloadMainScreen();
int FinishMainScreen();
// Drawing within DrawMainScreen()
void DrawMEM();
void DrawCONSOLE();
void DrawCANVAS();
// Updating within UpdateMainScreen()
void UpdateMEM();
void UpdateCONSOLE();
void UpdateCANVAS();

void DrawTextExMultiLine(Font drawfont, const char *text, Vector2 position, float fontSize, float spacing, Color tint, float lineSpacing);

// 2 byte color type, 4 fields separated by nibbles
// Little endian (LLHH)
// RRRR_GGGG BBBB_AAAA
typedef uint16_t COLOR_TETRA;

typedef uint8_t PIXEL;

// 16-color palette
// 4 color sets of 4 colors each
// 32B total
typedef struct _PALETTE {
    COLOR_TETRA SET[4][4];
} PALETTE; 

// 8x8 pixel color data
// Pixel color determined by concatenating {PLANE1[i], PLANE0[i]}
// Each index of array = 1 byte = 8 pixels (half of color data)
// One bit defines half of the color data for that pixel location
typedef struct _TILE {
    PIXEL PLANE0[8];
    PIXEL PLANE1[8];
} TILE;

// Attribute table
// 64B total
// 8x8 byte array
// Each byte controls color set for 4 quadrants of 2x2 tile groups
//
//  Singular byte:
// ,---+---+---+---.
// |   |   |   |   |    
// + D1-D0 + D3-D2 +
// |   |   |   |   |
// +---+---+---+---+
// |   |   |   |   |
// + D5-D4 + D7-D6 +
// |   |   |   |   |
// `---+---+---+---'
// (diagram from nesdev.org)

typedef struct _ATTRIB_TABLE {
    PIXEL TILE16[8][8];
} ATTRIB_TABLE;

// 32x16 tile pattern table
// Two halves of 16x16 tiles, 
typedef struct _PATT_TABLE {
    TILE LEFT[16][16];
    TILE RIGHT[16][16];
} PATT_TABLE;

// Sprite object to represent currently rendered sprites
typedef struct _SPRITE {
    PIXEL YPOS;
    PIXEL TILE_INDEX;
    PIXEL ATTRIBUTES;
    PIXEL XPOS;
} SPRITE;

// Object Attribute Memory
// List of currently rendered sprites
typedef struct _OAM {
    SPRITE RENDER_LIST[64];
    uint8_t STACK_TOP;
} OAM;

// Nametable
typedef struct _NAMETABLE {
    TILE IMAGE[32][30];
    OAM* OBJ_ATTRIBUTE_MEM;
} NAMETABLE;

HBYTE GRAPHICS_INIT(cJSON* GRAPHICS_CONFIG, char* PLATFORM);

/////////////////////////
// Color and Palette
/////////////////////////

// Loads 8 palettes from data in CONFIG
// Will eventually write to bin file to be loaded in at
// graphics init
void PALETTE_LOAD(PALETTE** PALETTE_LIST);

// Nametable mirror set functions

// 32x60 tilemap
void NT_MIRROR_HORIZONTAL();

// 64x30 tilemap
void NT_MIRROR_VERTICAL();

// 
void NT_MIRROR_SINGLE_SCREEN();

// 
void NT_MIRROR_FOUR_SCREEN();


////////////////////////
// OAM/Sprite operations
////////////////////////

// Adds sprite to OAM
void SPRITE_PUSH();

// Removes last sprite added to OAM
void SPRITE_POP();

// Edit sprite attributes in OAM
// Will update in nametable+on screen next cycle
void SPRITE_POKE();

// Clears OAM, removing all sprites from screen
void OAM_CLEAR();



#endif