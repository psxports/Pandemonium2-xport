#include <stdio.h>
#include <string.h>

#include "movie.h"
#include "game.h"
#include "hd.h"

void Movie::Play(const char *path)
{
    static const char *names[] = {"LOGO.STR", "MIDWAY.STR", "INTRO.STR", "NOUTRO.STR", "FOUTRO.STR", "CREDITS.STR"};
    char filename[64];
    uint32 selection;
    if (path == NULL)
        return;
    if (*path == '\\' || *path == '/')
        ++path;
    for (selection = 0; selection < sizeof(names) / sizeof(names[0]); ++selection)
    {
        size_t length = strlen(names[selection]);
        if (strncmp(path, names[selection], length) == 0 &&
            (path[length] == 0 || strcmp(path + length, ";1") == 0))
            break;
    }
    if (selection == sizeof(names) / sizeof(names[0]))
        return;
#if defined(AP_WEB)
    if (selection < 2)
        return;
#endif
    sprintf(filename, "DATA/%s", names[selection]);
    hd_reset();
    hd_level_reset();
    hd_menu_image_changed();
    if (xport_fmv_play(filename) < 0)
        xport_message_error("Movie playback", filename);
    hd_level_reset();
    game_field = CurrentField() & ~(uint64)1;
}
