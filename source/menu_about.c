#include <unistd.h>
#include <string.h>
#include <pngdec/pngdec.h>

#include "saves.h"
#include "menu.h"
#include "menu_about.h"

#include <tiny3d.h>
#include <libfont.h>

void Draw_AboutMenu_Ani()
{
	int div = 12, max = MENU_ANI_MAX, ani = 0, cnt = 0;
	for (ani = 0; ani < max; ani++)
	{
		tiny3d_Clear(0xff000000, TINY3D_CLEAR_ALL);

		tiny3d_AlphaTest(1, 0x0, TINY3D_ALPHA_FUNC_GEQUAL);

		tiny3d_BlendFunc(1, TINY3D_BLEND_FUNC_SRC_RGB_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_ALPHA_SRC_ALPHA,
			0x00000303 | 0x00000000,
			TINY3D_BLEND_RGB_FUNC_ADD | TINY3D_BLEND_ALPHA_FUNC_ADD);

		tiny3d_Project2D();

		DrawHeader_Ani(menu_textures[header_ico_abt_png_index], "About", APOLLO_VERSION, 0, 0xffffffff, ani, div);

		//------------- About Menu Contents

		int rate = (0x100 / (MENU_ANI_MAX - 0x60));
		u8 about_a = (u8)((((ani - 0x60) * rate) > 0xFF) ? 0xFF : ((ani - 0x60) * rate));
		if (ani < 0x60)
			about_a = 0;

		SetFontSize(20, 20);
		SetFontColor(APP_FONT_COLOR | about_a, 0);
		SetFontAlign(1);
		SetCurrentFont(font_comfortaa_bold);
		DrawString(848 / 2, 70, "Thank you for using Apollo!");
		SetCurrentFont(font_comfortaa_regular);
		SetFontSize(12, 12);
		DrawString(848 / 2, 95, "an open source save game tool");

		SetFontAlign(0);
		SetCurrentFont(font_comfortaa_bold);
		SetFontSize(18, 16);
		float dx = DrawString((848 - (u32)WidthFromStr((u8*)"PlayStation 3® version:")) / 2, 140, "PlayStation 3");
		//Make the ® small
		SetFontSize(9, 8);
		dx = DrawString(dx, 139, "® ");
		SetFontSize(18, 16);
		DrawString(dx, 140, "version:");

		for (cnt = 0; menu_about_strings[cnt] != NULL; cnt += 2)
		{
			SetFontSize(14, 14);

			SetCurrentFont(font_comfortaa_regular);
			SetFontAlign(2);
			DrawString((848 / 2) - 10, 170 + (cnt * 12), (char *)menu_about_strings[cnt]);

			SetCurrentFont(font_comfortaa_light);
			SetFontAlign(0);
			DrawString((848 / 2) + 10, 170 + (cnt * 12), (char *)menu_about_strings[cnt + 1]);
		}

		SetFontAlign(1);
		SetCurrentFont(font_comfortaa_bold);
		SetFontSize(18, 16);
		DrawString(848 / 2, 170 + ((cnt + 3) * 12), "Project Apollo:");

		int off = cnt + 5;
		for (cnt = 0; menu_about_strings_project[cnt] != NULL; cnt += 2)
		{
			SetFontSize(14, 14);

			SetCurrentFont(font_comfortaa_regular);
			SetFontAlign(2);
			DrawString((848 / 2) - 10, 175 + ((cnt + off) * 12), (char *)menu_about_strings_project[cnt]);

			SetCurrentFont(font_comfortaa_light);
			SetFontAlign(0);
			DrawString((848 / 2) + 10, 175 + ((off + cnt) * 12), (char *)menu_about_strings_project[cnt + 1]);
		}

		tiny3d_Flip();

		if (about_a == 0xFF)
			return;
	}
}

void Draw_AboutMenu_Ani__()
{
	int cnt = 0;
    
    int div = 12, max = MENU_ANI_MAX, ani = 0;
    for (ani = 0; ani < max; ani++)
    {
        tiny3d_Clear(0xff000000, TINY3D_CLEAR_ALL);
        
        tiny3d_AlphaTest(1, 0x0, TINY3D_ALPHA_FUNC_GEQUAL);
        
        tiny3d_BlendFunc(1, TINY3D_BLEND_FUNC_SRC_RGB_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_ALPHA_SRC_ALPHA,
            0x00000303 | 0x00000000,
            TINY3D_BLEND_RGB_FUNC_ADD | TINY3D_BLEND_ALPHA_FUNC_ADD);
        
        tiny3d_Project2D();
        
		DrawHeader_Ani(menu_textures[header_ico_opt_png_index], "About", APOLLO_VERSION, 0x00000000, 0xffffffff, ani, div);
        
        //------------- About Menu Contents
        
        int rate = (0x100 / (MENU_ANI_MAX - 0x60));
        u8 about_a = (u8)((((ani - 0x60) * rate) > 0xFF) ? 0xFF : ((ani - 0x60) * rate));
        if (ani < 0x60)
            about_a = 0;
        
		SetFontSize(20, 20);
		SetFontColor(APP_FONT_COLOR | about_a, 0);
		SetFontAlign(1);
		SetCurrentFont(font_comfortaa_bold);
		DrawString(848 / 2, 70, "Thank you for using Apollo!");
		SetCurrentFont(font_comfortaa_regular);
		SetFontSize(12, 12);
		DrawString(848 / 2, 95, "an open source save game tool");
        
		SetFontAlign(0);
		SetCurrentFont(font_comfortaa_bold);
		SetFontSize(18, 16);
		float dx = DrawString((848 - (u32)WidthFromStr((u8*)"PlayStation 3® version:")) / 2, 140, "PlayStation 3");
		//Make the ® small
		SetFontSize(9, 8);
		dx = DrawString(dx, 139, "® ");
		SetFontSize(18, 16);
		DrawString(dx, 140, "version:");

		for (cnt = 0; menu_about_strings[cnt] != NULL; cnt += 2)
		{
			SetFontSize(14, 14);

			SetCurrentFont(font_comfortaa_regular);
			SetFontAlign(2);
			DrawString((848 / 2) - 10, 170 + (cnt * 12), (char *)menu_about_strings[cnt]);

			SetCurrentFont(font_comfortaa_light);
			SetFontAlign(0);
			DrawString((848 / 2) + 10, 170 + (cnt * 12), (char *)menu_about_strings[cnt + 1]);
		}

		SetFontAlign(1);
		SetCurrentFont(font_comfortaa_bold);
		SetFontSize(18, 16);
		DrawString(848 / 2, 170 + ((cnt + 3) * 12), "Project Apollo:");

		int off = cnt + 5;
		for (cnt = 0; menu_about_strings_project[cnt] != NULL; cnt += 2)
		{
			SetFontSize(14, 14);

			SetCurrentFont(font_comfortaa_regular);
			SetFontAlign(2);
			DrawString((848 / 2) - 10, 175 + ((cnt + off) * 12), (char *)menu_about_strings_project[cnt]);

			SetCurrentFont(font_comfortaa_light);
			SetFontAlign(0);
			DrawString((848 / 2) + 10, 175 + ((off + cnt) * 12), (char *)menu_about_strings_project[cnt + 1]);
		}
        
        tiny3d_Flip();
        
        if (about_a == 0xFF)
            return;
    }
}

void Draw_AboutMenu()
{
	int cnt = 0;

	DrawHeader(menu_textures[header_ico_abt_png_index], 0, "About", APOLLO_VERSION, 0x000000ff, 0xffffffff, 0);
    
    //------------- About Menu Contents
    SetFontSize(20, 20);
    SetFontColor(APP_FONT_COLOR | 0xFF, 0);
    SetFontAlign(1);
	SetCurrentFont(font_comfortaa_bold);
	DrawString(848 / 2, 70, "Thank you for using Apollo!");
	SetCurrentFont(font_comfortaa_regular);
    SetFontSize(12, 12);
	DrawString(848 / 2, 95, "an open source save game tool");

	SetFontAlign(0);
	SetCurrentFont(font_comfortaa_bold);
	SetFontSize(18, 16);
	float dx = DrawString((848 - (u32)WidthFromStr((u8*)"PlayStation 3® version:")) / 2, 140, "PlayStation 3");
	//Make the ® small
	SetFontSize(9, 8);
	dx = DrawString(dx, 139, "® ");
	SetFontSize(18, 16);
	DrawString(dx, 140, "version:");
    
    for (cnt = 0; menu_about_strings[cnt] != NULL; cnt += 2)
    {
		SetFontSize(14, 14);

		SetCurrentFont(font_comfortaa_regular);
        SetFontAlign(2);
		DrawString((848 / 2) - 10, 170 + (cnt * 12), (char *)menu_about_strings[cnt]);
        
		SetCurrentFont(font_comfortaa_light);
		SetFontAlign(0);
		DrawString((848 / 2) + 10, 170 + (cnt * 12), (char *)menu_about_strings[cnt + 1]);
    }

	SetFontAlign(1);
	SetCurrentFont(font_comfortaa_bold);
	SetFontSize(18, 16);
	DrawString(848 / 2, 170 + ((cnt + 3) * 12), "Project Apollo:");

	int off = cnt + 5;
	for (cnt = 0; menu_about_strings_project[cnt] != NULL; cnt += 2)
	{
		SetFontSize(14, 14);

		SetCurrentFont(font_comfortaa_regular);
		SetFontAlign(2);
		DrawString((848 / 2) - 10, 175 + ((cnt + off) * 12), (char *)menu_about_strings_project[cnt]);

		SetCurrentFont(font_comfortaa_light);
		SetFontAlign(0);
		DrawString((848 / 2) + 10, 175 + ((off + cnt) * 12), (char *)menu_about_strings_project[cnt + 1]);
	}

    
}
