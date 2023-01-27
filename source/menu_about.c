#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pngdec/pngdec.h>

#include "saves.h"
#include "menu.h"
#include "menu_gui.h"

#include <tiny3d.h>
#include <libfont.h>

static char user_id_str[9] = "00000000";
static char idps_str[] = "0000000000000000 0000000000000000";
static char psid_str[] = "0000000000000000 0000000000000000";
static char account_id_str[] = "0000000000000000";

const char * menu_about_strings[] = { "Bucanero", "Developer",
									"Berion", "GUI design",
									"Dnawrkshp", "Artemis code",
									"flatz", "PFD/SFO tools",
									"aldostools", "Bruteforce Save Data",
									NULL };

const char * menu_about_strings_project[] = { "User ID", user_id_str,
											"Account ID", account_id_str,
											idps_str, psid_str, NULL };

static void _setIdValues()
{
	// set to display the PSID on the About menu
	snprintf(idps_str, sizeof(idps_str), "%016lX %016lX", apollo_config.idps[0], apollo_config.idps[1]);
	snprintf(psid_str, sizeof(psid_str), "%016lX %016lX", apollo_config.psid[0], apollo_config.psid[1]);
	snprintf(user_id_str, sizeof(user_id_str), "%08d", apollo_config.user_id);
	snprintf(account_id_str, sizeof(account_id_str), "%016lx", apollo_config.account_id);
}

void _draw_AboutMenu(u8 alpha)
{
	int cnt = 0;
    
    //------------- About Menu Contents
	DrawTextureCenteredX(&menu_textures[logo_text_png_index], 424, 70, 0, 245, 40, 0xFFFFFF00 | alpha);

    SetFontAlign(FONT_ALIGN_SCREEN_CENTER);
	SetCurrentFont(font_adonais_regular);
	SetFontColor(APP_FONT_MENU_COLOR | 0xFF, 0);
	SetFontSize(APP_FONT_SIZE_DESCRIPTION);
	DrawStringMono(0, 120, "PlayStation 3 version:");
	SetFontSize(APP_FONT_SIZE_ABOUT);
    
    for (cnt = 0; menu_about_strings[cnt] != NULL; cnt += 2)
    {
        SetFontAlign(FONT_ALIGN_RIGHT);
		DrawStringMono((848 / 2) - 10, 150 + (cnt * 12), menu_about_strings[cnt]);
        
		SetFontAlign(FONT_ALIGN_LEFT);
		DrawStringMono((848 / 2) + 10, 150 + (cnt * 12), menu_about_strings[cnt + 1]);
    }

	DrawTexture(&menu_textures[help_png_index], help_png_x, 300, 0, help_png_w, 110, 0xFFFFFF00 | 0xFF);

	SetFontAlign(FONT_ALIGN_SCREEN_CENTER);
	SetFontColor(APP_FONT_COLOR | 0xFF, 0);
	SetFontSize(APP_FONT_SIZE_DESCRIPTION);
	DrawString(0, 150 + ((cnt + 3) * 12), "Console details:");
	SetFontSize(APP_FONT_SIZE_SELECTION);

	int off = cnt + 5;
	for (cnt = 0; menu_about_strings_project[cnt] != NULL; cnt += 2)
	{
		SetFontAlign(FONT_ALIGN_RIGHT);
		DrawString((848 / 2) - 10, 155 + ((cnt + off) * 12), menu_about_strings_project[cnt]);

		SetFontAlign(FONT_ALIGN_LEFT);
		DrawString((848 / 2) + 10, 155 + ((off + cnt) * 12), menu_about_strings_project[cnt + 1]);
	}

	SetFontAlign(FONT_ALIGN_SCREEN_CENTER);
	SetCurrentFont(font_adonais_regular);
	SetFontColor(APP_FONT_MENU_COLOR | 0xFF, 0);
	SetFontSize(APP_FONT_SIZE_DESCRIPTION);
	DrawStringMono(0, 430, "www.bucanero.com.ar");
	SetFontAlign(FONT_ALIGN_LEFT);
}

void Draw_AboutMenu_Ani()
{
	_setIdValues();
	for (int ani = 0; ani < MENU_ANI_MAX; ani++)
	{
		tiny3d_Clear(0xff000000, TINY3D_CLEAR_ALL);
		tiny3d_AlphaTest(1, 0x0, TINY3D_ALPHA_FUNC_GEQUAL);
		tiny3d_BlendFunc(1, TINY3D_BLEND_FUNC_SRC_RGB_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_ALPHA_SRC_ALPHA,
			TINY3D_BLEND_FUNC_SRC_RGB_ONE_MINUS_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_RGB_ZERO,
			TINY3D_BLEND_RGB_FUNC_ADD | TINY3D_BLEND_ALPHA_FUNC_ADD);

		tiny3d_Project2D();

		DrawHeader_Ani(cat_about_png_index, "About", "v" APOLLO_VERSION, APP_FONT_TITLE_COLOR, 0xffffffff, ani, 12);

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
	_setIdValues();
	DrawHeader(cat_about_png_index, 0, "About", "v" APOLLO_VERSION, APP_FONT_TITLE_COLOR | 0xFF, 0xffffffff, 0);
	_draw_AboutMenu(0xFF);
}
