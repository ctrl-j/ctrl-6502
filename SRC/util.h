#ifndef __UTIL_H__
#define __UTIL_H__

#define _BLACK "\033[30m"
#define _RED "\033[31m"
#define _GREEN "\033[32m"
#define _YELLOW "\033[33m"
#define _BLUE "\033[34m"
#define _PURPLE "\033[35m"

#define _WHITE "\033[37m"
#define _DEFAULT "\033[0m"

// BG colors
#define _BG_WHITE_PURPLE_TEXT "\033[45;1;7m"
#define _BG_BRIGHTCYAN "\033[46;1m"
#define _BG_BRIGHTPURPLE "\033[45;1m"
#define _BG_BRIGHTBLUE "\033[44;1;7m"
#define _BG_RED "\033[41;1m"
#define _BG_GREEN "\033[42;1m"
#define _BG_YELLOW "\033[48;5;214;1m"
#define _BG_BLUE "\033[44;1m"

// Flashing
#define _FLASH_BG_PURPLE "\033[45;1;5m"

#define _SCR_START "\033[0;0H"
#define _CLR_SCR "\033[2J"

//Log string "msg" to the given fp. Defaults to stderr
//Depending on "type" changes color of text for stderr/stdout
//"type" options:
//     -1: clear whole screen
//      0: error message/failure, red text
//      1: log message, white text
//      2: warning, yellow text
//      3: success, green text
//      4: flashing white text on purple BG
//      5: red BG, white text
//      6: cyan BG, white text
//      7: purple BG, white text
//      8: white BG, blue text
//      9: red BG, white text
//     10: green BG, white text
//     11: Yellow BG, white text
//     12: Blue BG, white text
void logMsg(FILE *fp, char *msg, int type);

#endif