#include <unistd.h>
#include <string.h>
#include <pngdec/pngdec.h>

#include "saves.h"
#include "menu.h"
#include "menu_about.h"

#include <tiny3d.h>
#include <libfont.h>

void _draw_AboutMenu(u8 alpha)
{
	int cnt = 0;
    
    //------------- About Menu Contents
    SetFontSize(APP_FONT_SIZE_SUBTITLE);
    SetFontColor(APP_FONT_COLOR | alpha, 0);
    SetFontAlign(1);
	SetCurrentFont(font_comfortaa_bold);
	DrawString(0, 70, "Thanks for using Apollo!");
	SetCurrentFont(font_comfortaa_regular);
    SetFontSize(APP_FONT_SIZE_SUBTEXT);
	DrawString(0, 95, "an open source save game tool");

	SetCurrentFont(font_comfortaa_bold);
	SetFontSize(APP_FONT_SIZE_DESCRIPTION);
	DrawString(0, 140, "PlayStation 3® version:");
    
    for (cnt = 0; menu_about_strings[cnt] != NULL; cnt += 2)
    {
		SetFontSize(APP_FONT_SIZE_ABOUT);

		SetCurrentFont(font_comfortaa_regular);
        SetFontAlign(2);
		DrawString((848 / 2) - 10, 170 + (cnt * 12), (char *)menu_about_strings[cnt]);
        
		SetCurrentFont(font_comfortaa_light);
		SetFontAlign(0);
		DrawString((848 / 2) + 10, 170 + (cnt * 12), (char *)menu_about_strings[cnt + 1]);
    }

	SetFontAlign(1);
	SetCurrentFont(font_comfortaa_bold);
	SetFontSize(APP_FONT_SIZE_DESCRIPTION);
	DrawString(0, 170 + ((cnt + 3) * 12), "Console details:");

	int off = cnt + 5;
	for (cnt = 0; menu_about_strings_project[cnt] != NULL; cnt += 2)
	{
		SetFontSize(APP_FONT_SIZE_ABOUT);

		SetCurrentFont(font_comfortaa_regular);
		SetFontAlign(2);
		DrawString((848 / 2) - 10, 175 + ((cnt + off) * 12), (char *)menu_about_strings_project[cnt]);

		SetCurrentFont(font_comfortaa_light);
		SetFontAlign(0);
		DrawString((848 / 2) + 10, 175 + ((off + cnt) * 12), (char *)menu_about_strings_project[cnt + 1]);
	}

	SetFontAlign(1);
	SetCurrentFont(font_comfortaa_regular);
	DrawString(0, 420, "http://apollo.psdev.tk/");
	SetFontAlign(0);
}

void Draw_AboutMenu_Ani()
{
	int div = 12, ani = 0;
	for (ani = 0; ani < MENU_ANI_MAX; ani++)
	{
		tiny3d_Clear(0xff000000, TINY3D_CLEAR_ALL);
		tiny3d_AlphaTest(1, 0x0, TINY3D_ALPHA_FUNC_GEQUAL);
		tiny3d_BlendFunc(1, TINY3D_BLEND_FUNC_SRC_RGB_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_ALPHA_SRC_ALPHA,
			0x00000303 | 0x00000000,
			TINY3D_BLEND_RGB_FUNC_ADD | TINY3D_BLEND_ALPHA_FUNC_ADD);

		tiny3d_Project2D();

		DrawHeader_Ani(menu_textures[header_ico_abt_png_index], "About", "v" APOLLO_VERSION, APP_FONT_TITLE_COLOR, 0xffffffff, ani, div);

		//------------- About Menu Contents

		int rate = (0x100 / (MENU_ANI_MAX - 0x60));
		u8 about_a = (u8)((((ani - 0x60) * rate) > 0xFF) ? 0xFF : ((ani - 0x60) * rate));
		if (ani < 0x60)
			about_a = 0;

		_draw_AboutMenu(about_a);

		tiny3d_Flip();

		if (about_a == 0xFF)
			return;
	}
}

void Draw_AboutMenu()
{
	DrawHeader(menu_textures[header_ico_abt_png_index], 0, "About", "v" APOLLO_VERSION, APP_FONT_TITLE_COLOR | 0xFF, 0xffffffff, 0);
	_draw_AboutMenu(0xFF);
}
