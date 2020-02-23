#include <unistd.h>
#include <string.h>
#include <pngdec/pngdec.h>
#include <stdio.h>

#include "saves.h"
#include "menu.h"
#include "menu_options.h"

#include <tiny3d.h>
#include <libfont.h>

void _draw_OptionsMenu(u8 alpha)
{
	int c = 0, w = 0, h = 0;
	char APP_OPTION_INC_TEMP[16];

    SetFontSize(APP_FONT_SIZE_SELECTION);
    int ind = 0, y_off = 120;
	SetCurrentFont(font_comfortaa_regular);
    while (menu_options[ind].name)
    {
        SetFontColor(APP_FONT_COLOR | alpha, 0);
        float dx = DrawString(MENU_ICON_OFF + MENU_TITLE_OFF, y_off, menu_options[ind].name);
        
		switch (menu_options[ind].type)
		{
			case APP_OPTION_BOOL:
				c = (*menu_options[ind].value == 1) ? opt_on_png_index : opt_off_png_index;
				w = (int)(menu_textures[c].texture.width / 1.8);
				h = (int)(menu_textures[c].texture.height / 1.8);
				DrawTexture(&menu_textures[c], MENU_ICON_OFF - 29, y_off, 0, w, h, 0xFFFFFF00 | alpha);
				break;
			case APP_OPTION_CALL:
				w = (int)(menu_textures[mark_arrow_png_index].texture.width / 1.8);
				h = (int)(menu_textures[mark_arrow_png_index].texture.height / 1.8);
				DrawTexture(&menu_textures[mark_arrow_png_index], MENU_ICON_OFF - 26, y_off-3, 0, w, h, 0xFFFFFF00 | alpha);
				break;
			case APP_OPTION_LIST:
				SetFontAlign(2);
				DrawString(dx + 40, y_off, menu_options[ind].options[*menu_options[ind].value]);
				SetFontAlign(0);
				break;
			case APP_OPTION_INC:
				SetFontAlign(0);
				sprintf(APP_OPTION_INC_TEMP, "- %d +", *menu_options[ind].value);
				int inc_width = WidthFromStr(APP_OPTION_INC_TEMP);
				DrawString((MENU_ICON_OFF - 18) - (inc_width / 2), y_off, APP_OPTION_INC_TEMP);
				break;
		}
        
        if (menu_sel == ind)
        {
            int i = 0;
            for (i = 0; i < 848; i++)
				DrawTexture(&menu_textures[mark_line_png_index], i, y_off, 0, menu_textures[mark_line_png_index].texture.width, menu_textures[mark_line_png_index].texture.height, 0xFFFFFF00 | alpha);
        }
        
        y_off += 20;
        ind++;
    }
}

void Draw_OptionsMenu_Ani()
{
    int ani = 0;
    for (ani = 0; ani < MENU_ANI_MAX; ani++)
    {
        tiny3d_Clear(0xff000000, TINY3D_CLEAR_ALL);
        tiny3d_AlphaTest(1, 0x0, TINY3D_ALPHA_FUNC_GEQUAL);
        tiny3d_BlendFunc(1, TINY3D_BLEND_FUNC_SRC_RGB_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_ALPHA_SRC_ALPHA,
            0x00000303 | 0x00000000,
            TINY3D_BLEND_RGB_FUNC_ADD | TINY3D_BLEND_ALPHA_FUNC_ADD);
        
        tiny3d_Project2D();
        
		DrawHeader_Ani(cat_opt_png_index, "Options", NULL, APP_FONT_TITLE_COLOR, 0xffffffff, ani, 12);
        
		u8 icon_a = (u8)(((ani * 2) > 0xFF) ? 0xFF : (ani * 2));
        int _game_a = (int)(icon_a - (MENU_ANI_MAX / 2)) * 2;
        if (_game_a > 0xFF)
            _game_a = 0xFF;
        u8 game_a = (u8)(_game_a < 0 ? 0 : _game_a);
        
        if (game_a > 0)
        	_draw_OptionsMenu(game_a);
        
        tiny3d_Flip();
        
        if (game_a == 0xFF)
            return;
    }
}

void Draw_OptionsMenu()
{
	DrawHeader(cat_opt_png_index, 0, "Options", NULL, APP_FONT_TITLE_COLOR | 0xFF, 0xffffffff, 0);
    _draw_OptionsMenu(0xFF);
}
