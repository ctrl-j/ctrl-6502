/*
    Copyright (C) 2024  Joshua Jackson
    [Full GPL-3.0 license can be found in LICENSE, in the root folder]
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "util.h"


void logMsg(FILE *fp, char *msg, int type){
    //Default msg routing, stderr, when *fp == NULL
    if (fp == NULL){
        fp = stderr;
    }

    //Output going to stderr or stdout, color settings so that file output is not cluttered
    if ((fp == stderr) || (fp == stdout)){
        switch (type){
            case -1:
                fprintf(fp, _SCR_START);
                fprintf(fp, _CLR_SCR);
                fprintf(fp, _DEFAULT);
            break;
            case 0:
                fprintf(fp, _RED);
                fprintf(fp, "%s", msg);
                fprintf(fp, _DEFAULT);
            break;
            case 1:
                fprintf(fp, _WHITE);
                fprintf(fp, "%s", msg);
                fprintf(fp, _DEFAULT);
            break;
            case 2:
                fprintf(fp, _YELLOW);
                fprintf(fp, "%s", msg);
                fprintf(fp, _DEFAULT);
            break;
            case 4:
                fprintf(fp, _FLASH_BG_PURPLE);
                fprintf(fp, "%s", msg);
                fprintf(fp, _DEFAULT);
            break;
            case 5:
                fprintf(fp, _BG_WHITE_PURPLE_TEXT);
                fprintf(fp, "%s", msg);
                fprintf(fp, _DEFAULT);
            break;
            case 6:
                fprintf(fp, _BG_BRIGHTCYAN);
                fprintf(fp, "%s", msg);
                fprintf(fp, _DEFAULT);
            break;
            case 7:
                fprintf(fp, _BG_BRIGHTPURPLE);
                fprintf(fp, "%s", msg);
                fprintf(fp, _DEFAULT);
            break;
            case 8:
                fprintf(fp, _BG_BRIGHTBLUE);
                fprintf(fp, "%s", msg);
                fprintf(fp, _DEFAULT);
            break;
            case 9:
                fprintf(fp, _BG_RED);
                fprintf(fp, "%s", msg);
                fprintf(fp, _DEFAULT);
            break;
            case 10:
                fprintf(fp, _BG_GREEN);
                fprintf(fp, "%s", msg);
                fprintf(fp, _DEFAULT);
            break;
            case 11:
                fprintf(fp, _BG_YELLOW);
                fprintf(fp, "%s", msg);
                fprintf(fp, _DEFAULT);
            break;
            case 12:
                fprintf(fp, _BG_BLUE);
                fprintf(fp, "%s", msg);
                fprintf(fp, _DEFAULT);
            break;
            default:
                fprintf(fp, _DEFAULT);
                fprintf(fp, "%s", msg);
                fprintf(fp, _DEFAULT);
            break;
        }
    }
    //Output going to text file, no color settings
    else{
        fp = fopen("log.txt", "a");
        //If fails to open log file, do nothing
        if (fp == NULL){
            return;
        }

        //Log file format:

        //Header (to help separate entries)
        fprintf(fp, "\n====================\n");

        /*//Message type
        switch (type){
            case 0:
                fprintf(fp, "***ERROR*** ");
                break;
            case 1:
                fprintf(fp, "***MESSAGE*** ");
                break;
            case 2:
                fprintf(fp, "***WARNING*** ");
                break;
            case 3:
                fprintf(fp, "***SUCCESS*** ");
                break;
            case 4:
                fprintf(fp, "***INFO*** ");
                break;
        }
        */

        //Message content
        fprintf(fp, "%s\n", msg);

        //Timestamp
        time_t now = time(NULL);
        struct tm * timeInfo;

        time(&now);
        timeInfo = localtime(&now);

        char* now_local = asctime(timeInfo);
        fprintf(fp, "%s\n", now_local);

        //Footer (to help separate entries)
        fprintf(fp, "====================\n");

        fclose(fp);
    }
}