#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "saves.h"
#include "menu.h"
#include "menu_gui.h"

#include <tiny3d.h>
#include <libfont.h>

#define MAX_LINES 19


static void _draw_HexEditor(const hexedit_data_t* hex, u8 alpha)
{
    char msgout[64];
    char ascii[32];
    int i, y_off = 74;

    SetCurrentFont(font_console_16x32);
    SetFontSize(9, 18);
    SetFontColor(0x00000000 | alpha, 0);

    memset(msgout, 32, sizeof(msgout));
    sprintf(msgout + (hex->pos % 16)*3 + hex->low_nibble, "%c", 0xDB);
    DrawFormatStringMono(MENU_ICON_OFF + MENU_TITLE_OFF, y_off + (hex->pos - hex->start)/16*MAX_LINES, "        %s", msgout);
    DrawSelector(0, y_off-2 + (hex->pos - hex->start)/16*MAX_LINES, 0, 0, 0, alpha);

    SetFontColor(APP_FONT_COLOR | alpha, 0);
    for (i = hex->start; i < hex->size && (i - hex->start) < (MAX_LINES*16); i++)
    {
        if (i != hex->start && !(i % 16))
        {
            DrawFormatStringMono(MENU_ICON_OFF + MENU_TITLE_OFF, y_off, "%06X: %s \xB3 %s", i-0x10, msgout, ascii);
            y_off += 19;
        }

        sprintf(msgout + (i % 16)*3, "%02X ", hex->data[i]);
        sprintf(ascii  + (i % 16), "%c", hex->data[i] ? hex->data[i] : '.');
    }
    DrawFormatStringMono(MENU_ICON_OFF + MENU_TITLE_OFF, y_off, "%06X: %-48s \xB3 %s", (i-1) & ~15, msgout, ascii);

    SetCurrentFont(font_adonais_regular);
}

void Draw_HexEditor_Ani(const hexedit_data_t* hex)
{
    for (int ani = 0; ani < MENU_ANI_MAX; ani++)
    {
        tiny3d_Clear(0xff000000, TINY3D_CLEAR_ALL);
        tiny3d_AlphaTest(1, 0x0, TINY3D_ALPHA_FUNC_GEQUAL);
        tiny3d_BlendFunc(1, TINY3D_BLEND_FUNC_SRC_RGB_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_ALPHA_SRC_ALPHA,
            TINY3D_BLEND_FUNC_SRC_RGB_ONE_MINUS_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_RGB_ZERO,
            TINY3D_BLEND_RGB_FUNC_ADD | TINY3D_BLEND_ALPHA_FUNC_ADD);

        tiny3d_Project2D();

        DrawHeader_Ani(cat_opt_png_index, "Hex Editor", strrchr(hex->filepath, '/')+1, APP_FONT_TITLE_COLOR, 0xffffffff, ani, 12);

        u8 icon_a = (u8)(((ani * 2) > 0xFF) ? 0xFF : (ani * 2));
        int _game_a = (int)(icon_a - (MENU_ANI_MAX / 2)) * 2;
        if (_game_a > 0xFF)
            _game_a = 0xFF;
        u8 game_a = (u8)(_game_a < 0 ? 0 : _game_a);

        if (game_a > 0)
        	_draw_HexEditor(hex, game_a);

        tiny3d_Flip();

        if (game_a == 0xFF)
            return;
    }
}

void Draw_HexEditor(const hexedit_data_t* hex)
{
    DrawHeader(cat_opt_png_index, 0, "Hex Editor", strrchr(hex->filepath, '/')+1, APP_FONT_TITLE_COLOR | 0xFF, 0xffffffff, 0);
    _draw_HexEditor(hex, 0xFF);
}
