/* 
	Apollo PS3 main.c
*/

#include <ppu-lv2.h>
#include <sys/spu.h>
#include <lv2/spu.h>

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>
#include <sysutil/video.h>
#include <lv2/process.h>
#include <time.h>
#include <dirent.h>

#include <errno.h>

#include <zlib.h>
#include <io/pad.h>

#include <sysmodule/sysmodule.h>
#include <pngdec/pngdec.h>

#include <tiny3d.h>
#include "libfont.h"
#include "saves.h"
#include "sfo.h"
#include "pfd.h"

//From NzV's MAMBA PRX Loader (https://github.com/NzV/MAMBA_PRX_Loader)
#include "common.h"
#include "lv2_utils.h"

//Menus
#include "menu.h"
#include "menu_about.h"
#include "menu_options.h"
#include "menu_cheats.h"

#define MENU_MAIN_SCREEN    0
#define MENU_USB_SAVES      1
#define MENU_HDD_SAVES      2
#define MENU_ONLINE_DB      3
#define MENU_SETTINGS       4
#define MENU_CREDITS        5
#define MENU_PATCHES        6
#define MENU_PATCH_VIEW     7
#define MENU_CODE_OPTIONS   8
#define MENU_SAVE_DETAILS   9

//Font
#include "comfortaa_bold_ttf.h"
#include "comfortaa_light_ttf.h"
#include "comfortaa_regular_ttf.h"

//Sound
#include "spu_soundmodule_bin.h"
#include "spu_soundlib.h"
#include "audioplayer.h"
#include "background_music_mp3.h"

// SPU
u32 inited;
u32 spu = 0;
sysSpuImage spu_image;

#define INITED_CALLBACK     1
#define INITED_SPU          2
#define INITED_SOUNDLIB     4
#define INITED_AUDIOPLAYER  8

#define SPU_SIZE(x) (((x)+127) & ~127)

#define load_menu_texture(name, type) \
	({ extern const uint8_t name##_##type []; \
	   extern const uint32_t name##_##type##_size; \
	   menu_textures[name##_##type##_index].buffer = (const void*) name##_##type; \
	   menu_textures[name##_##type##_index].size = name##_##type##_size; \
	})

//Pad stuff
padInfo padinfo;
padData paddata[MAX_PADS];
padData padA[MAX_PADS];
padData padB[MAX_PADS];

void music_callback(int sel);
void sort_callback(int sel);
void ani_callback(int sel);
void horm_callback(int sel);
void verm_callback(int sel);
void update_callback(int sel);

app_config_t apollo_config = {
    .music = 1,
    .doSort = 1,
    .doAni = 1,
    .marginH = 0,
    .marginV = 0,
    .user_id = 0,
    .psid = {0, 0},
    .account_id = 0,
};

const menu_option_t menu_options[] = {
	{ .name = "Music", .options = NULL, .type = APP_OPTION_BOOL, .value = &apollo_config.music, .callback = music_callback },
	{ .name = "Sort Saves", .options = NULL, .type = APP_OPTION_BOOL, .value = &apollo_config.doSort, .callback = sort_callback },
	{ .name = "Menu Animations", .options = NULL, .type = APP_OPTION_BOOL, .value = &apollo_config.doAni, .callback = ani_callback },
	{ .name = "Horizontal Margin", .options = NULL, .type = APP_OPTION_INC, .value = &apollo_config.marginH, .callback = horm_callback },
	{ .name = "Vertical Margin", .options = NULL, .type = APP_OPTION_INC, .value = &apollo_config.marginV, .callback = verm_callback },
//	{ .name = "Update Check", .options = NULL, .type = APP_OPTION_CALL, .callback = update_callback },
	{ .name = NULL }
};

int menu_options_maxopt = 0;
int * menu_options_maxsel;

int close_app = 0;

png_texture * menu_textures;                // png_texture array for main menu, initialized in LoadTexture

int screen_width = 0, screen_height = 0;    // Set to dimensions of the screen in main()

int highlight_alpha = 128;                  // Alpha of the selected
int highlight_pulse = 1;                    // Whether the increment the highlight
int highlight_amount = 6;                   // Amount of alpha to inc/dec each time
int pause_pulse = 0;                        // Counter that holds how long alpha is held in place
int idle_time = 0;                          // Set by readPad

#define MENU_MAIN_DESCRIPTION   "PlayStation 3 Save Game Tool"
#define MENU_MAIN_FOOTER        "www.bucanero.com.ar"

const char * menu_about_strings[] = { "Bucanero", "Developer",
									"Dnawrkshp", "GUI code",
									"Berion", "GUI design",
									"flatz", "PFD/SFO tools",
									NULL, NULL };

char user_id_str[9] = "00000000";
char psid_str1[17] = "0000000000000000";
char psid_str2[17] = "0000000000000000";
char account_id_str[17] = "0000000000000000";

const char * menu_about_strings_project[] = { psid_str1, psid_str2,
											"Account ID", account_id_str,
											"User ID", user_id_str,
											NULL, NULL };

/*
* 0 - Main Menu
* 1 - USB Menu (User List)
* 2 - HDD Menu (User List)
* 3 - Online Menu (Online List)
* 4 - Options Menu
* 5 - About Menu
* 6 - Code Menu (Select Cheats)
* 7 - Code Menu (View Cheat)
* 8 - Code Menu (View Cheat Options)
*/
int menu_id = 0;												// Menu currently in
int menu_sel = 0;												// Index of selected item (use varies per menu)
int menu_old_sel[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };			// Previous menu_sel for each menu
int last_menu_id[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };			// Last menu id called (for returning)

const char * menu_pad_help[] = { NULL,																//Main
								"\x10 Select    \x13 Back    \x12 Details    \x11 Refresh",			//USB list
								"\x10 Select    \x13 Back    \x12 Details    \x11 Refresh",			//HDD list
								"\x10 Select    \x13 Back    \x11 Refresh",							//Online list
								"\x10 Select    \x13 Back",											//Options
								"\x13 Back",														//About
								"\x10 Enable    \x12 View Code    \x13 Back",						//Select Cheats
								"\x13 Back",														//View Cheat
								"\x10 Select    \x13 Back",											//Cheat Option
								"\x13 Back",														//View Details
								};

/*
* HDD save list
*/
save_list_t hdd_saves = {
    .list = NULL,
    .count = 0,
    .path = "",
};

/*
* USB save list
*/
save_list_t usb_saves = {
    .list = NULL,
    .count = 0,
    .path = "",
};

/*
* Online code list
*/
save_list_t online_saves = {
    .list = NULL,
    .count = 0,
    .path = ONLINE_URL,
};

save_entry_t selected_entry;
code_entry_t selected_centry;
int option_index = 0;

// Set to dimensions of the screen in main()
int screen_width;
int screen_height;

void release_all() {
	
	if(inited & INITED_CALLBACK)
		sysUtilUnregisterCallback(SYSUTIL_EVENT_SLOT0);

	if(inited & INITED_AUDIOPLAYER)
		StopAudio();
	
	if(inited & INITED_SOUNDLIB)
		SND_End();

	if(inited & INITED_SPU) {
		sysSpuRawDestroy(spu);
		sysSpuImageClose(&spu_image);
	}

	inited=0;
}

static void sys_callback(uint64_t status, uint64_t param, void* userdata) {

	 switch (status) {
		case SYSUTIL_EXIT_GAME: //0x0101
				
			release_all();
			sysProcessExit(1);
			break;
	  
		case SYSUTIL_MENU_OPEN: //0x0131

			break;
		case SYSUTIL_MENU_CLOSE: //0x0132

			break;
	   default:
		   break;
		 
	}
}

int isAppLoaded()
{
	char buf[100];
	readFileBuffered("/dev_hdd0/tmp/apollo", (char *)buf);
	if (buf[0] == 0)
		return 0;
	if (strstr(buf, "on") != NULL)
		return 1;
	
	return 0;
}

void DeleteBootHistory(void)
{
	DIR *d;
	struct dirent *dir;
	d = opendir("/dev_hdd0/home/");
	char fullPath[256];
	
	//Delete home boot history files first
	while ((dir = readdir(d)) != NULL)
	{
		if (strstr(dir->d_name, ".") == NULL && strstr(dir->d_name, "..") == NULL)
		{
			snprintf(fullPath, sizeof(fullPath), "/dev_hdd0/home/%s/etc/boot_history.dat", dir->d_name);
			unlink_secure(fullPath);
		}
	}
	
	//Delete the other boot history files
	unlink_secure("/dev_hdd0/vsh/pushlist/game.dat");
	unlink_secure("/dev_hdd0/vsh/pushlist/patch.dat");
}

int pad_time = 0, rest_time = 0, pad_held_time = 0, rest_held_time = 0;
u16 oldPad = 0;

int readPad(int port)
{
	ioPadGetInfo(&padinfo);
	int off = 2;
	int retDPAD = 0, retREST = 0;
	u8 dpad = 0, _dpad = 0;
	u16 rest = 0, _rest = 0;
	
	if(padinfo.status[port])
	{
		ioPadGetData(port, &padA[port]);
		
		//new
		dpad = ((char)*(&padA[port].zeroes + off) << 8) >> 12;
		rest = ((((char)*(&padA[port].zeroes + off) & 0xF) << 8) | ((char)*(&padA[port].zeroes + off + 1) << 0));
		
		//old
		_dpad = ((char)*(&padB[port].zeroes + off) << 8) >> 12;
		_rest = ((((char)*(&padB[port].zeroes + off) & 0xF) << 8) | ((char)*(&padB[port].zeroes + off + 1) << 0));
		
		if (dpad == 0 && rest == 0)
			idle_time++;
		else
			idle_time = 0;
		
		//Copy new to old
		//memcpy(&_paddata[port].zeroes + off, &paddata[port].zeroes + off, size);
		memcpy(paddata, padA, sizeof(padData));
		memcpy(padB, padA, sizeof(padData));
		
		//DPad has 3 mode input delay
		if (dpad == _dpad && dpad != 0)
		{
			if (pad_time > 0) //dpad delay
			{
				pad_held_time++;
				pad_time--;
				retDPAD = 0;
			}
			else
			{
				//So the pad speeds up after a certain amount of time being held
				if (pad_held_time > 180)
				{
					pad_time = 2;
				}
				else if (pad_held_time > 60)
				{
					pad_time = 5;
				}
				else
					pad_time = 20;
				
				retDPAD = 1;
			}
		}
		else
		{
			pad_held_time = 0;
			pad_time = 0;
		}
		
		//rest has its own delay
		if (rest == _rest && rest != 0)
		{
			if (rest_time > 0) //rest delay
			{
				rest_held_time++;
				rest_time--;
				retREST = 0;
			}
			else
			{
				//So the pad speeds up after a certain amount of time being held
				if (rest_held_time > 180)
				{
					rest_time = 2;
				}
				else if (rest_held_time > 60)
				{
					rest_time = 5;
				}
				else
					rest_time = 20;
				
				retREST = 1;
			}
		}
		else
		{
			rest_held_time = 0;
			rest_time = 0;
		}
		
	}
	
	if (!retDPAD && !retREST)
		return 0;
	
	if (!retDPAD)
	{
		paddata[port].BTN_LEFT = 0;
		paddata[port].BTN_RIGHT = 0;
		paddata[port].BTN_UP = 0;
		paddata[port].BTN_DOWN = 0;
	}
	
	if (!retREST)
	{
		paddata[port].BTN_L2 = 0;
		paddata[port].BTN_R2 = 0;
		paddata[port].BTN_L1 = 0;
		paddata[port].BTN_R1 = 0;
		paddata[port].BTN_TRIANGLE = 0; 
		paddata[port].BTN_CIRCLE = 0;
		paddata[port].BTN_CROSS = 0;
		paddata[port].BTN_SQUARE = 0;
		paddata[port].BTN_SELECT = 0;
		paddata[port].BTN_L3 = 0;
		paddata[port].BTN_R3 = 0;
		paddata[port].BTN_START = 0;
	}
	
	return 1;
}

void Draw_MainMenu_Ani()
{
	int w = 0, h = 0;
	
	int max = MENU_ANI_MAX, ani = 0;
	for (ani = 0; ani < max; ani++)
	{
		tiny3d_Clear(0xff000000, TINY3D_CLEAR_ALL);
		
		tiny3d_AlphaTest(1, 0x10, TINY3D_ALPHA_FUNC_GEQUAL);
		
		tiny3d_BlendFunc(1, TINY3D_BLEND_FUNC_SRC_RGB_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_ALPHA_SRC_ALPHA,
			TINY3D_BLEND_FUNC_SRC_ALPHA_ONE_MINUS_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_RGB_ONE_MINUS_SRC_ALPHA,
			TINY3D_BLEND_RGB_FUNC_ADD | TINY3D_BLEND_ALPHA_FUNC_ADD);
		
		tiny3d_Project2D();
		
		DrawBackground2D(0xFFFFFFFF);
		
		//------------ Backgrounds
		u8 bg_a = (u8)(ani * 2);
		if (bg_a < 0x20)
			bg_a = 0x20;
		int logo_a_t = ((ani < 0x30) ? 0 : ((ani - 0x20)*3));
		if (logo_a_t > 0xFF)
			logo_a_t = 0xFF;
		u8 logo_a = (u8)logo_a_t;
		
		//Background
		DrawBackgroundTexture(0, bg_a);
		
		//App logo
		w = 500, h = 140;
		DrawTexture(menu_textures[titlescr_logo_png_index], 424 - (w / 2), 256 - (h / 2) - 120, 0, w, h, 0xFFFFFF00 | logo_a);
		
		//App description
		SetFontAlign(1);
		SetCurrentFont(font_comfortaa_light);
		SetFontSize(APP_FONT_SIZE_DESCRIPTION);
		SetFontColor(APP_FONT_COLOR | logo_a, 0);
		DrawString(screen_width / 2, 210, MENU_MAIN_DESCRIPTION);
		
		tiny3d_Flip();
	}
	
	max = MENU_ANI_MAX / 2;
	int rate = (0x100 / max);
	for (ani = 0; ani < max; ani++)
	{
		tiny3d_Clear(0xff000000, TINY3D_CLEAR_ALL);
		
		tiny3d_AlphaTest(1, 0x10, TINY3D_ALPHA_FUNC_GEQUAL);
		
		tiny3d_BlendFunc(1, TINY3D_BLEND_FUNC_SRC_RGB_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_ALPHA_SRC_ALPHA,
			TINY3D_BLEND_FUNC_SRC_ALPHA_ONE_MINUS_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_RGB_ONE_MINUS_SRC_ALPHA,
			TINY3D_BLEND_RGB_FUNC_ADD | TINY3D_BLEND_ALPHA_FUNC_ADD);
		
		tiny3d_Project2D();
		
		DrawBackground2D(0xFFFFFFFF);
		
		u8 icon_a = (u8)(((ani * rate) > 32) ? 32 : (ani * rate));
		
		//------------ Backgrounds
		
		//Background
		DrawBackgroundTexture(0, 0xFF);
		
		//App logo
		w = 500; h = 140;
		DrawTexture(menu_textures[titlescr_logo_png_index], 424 - (w / 2), 256 - (h / 2) - 120, 0, w, h, 0xFFFFFFFF);

		//App description
		SetFontAlign(1);
		SetCurrentFont(font_comfortaa_light);
		SetFontSize(APP_FONT_SIZE_DESCRIPTION);
		SetFontColor(APP_FONT_COLOR | 0xFF, 0);
		DrawString(screen_width / 2, 210, MENU_MAIN_DESCRIPTION);

		SetFontSize(APP_FONT_SIZE_SUBTEXT);
		DrawString(screen_width / 2, 480, MENU_MAIN_FOOTER);
		
		//------------ Icons
		
		//Start game
		DrawTexture(menu_textures[titlescr_ico_xmb_png_index], 200, 320, 0, MENU_MAIN_ICON_WIDTH, 64, 0xffffff00 | icon_a);
		
		//Cheats
		DrawTexture(menu_textures[titlescr_ico_cht_png_index], 300, 320, 0, MENU_MAIN_ICON_WIDTH, 64, 0xffffff00 | icon_a);

		//Online cheats
		DrawTexture(menu_textures[titlescr_ico_net_png_index], 400, 320, 0, MENU_MAIN_ICON_WIDTH, 64, 0xffffff00 | icon_a);
		
		//Options
		DrawTexture(menu_textures[titlescr_ico_opt_png_index], 500, 320, 0, MENU_MAIN_ICON_WIDTH, 64, 0xffffff00 | icon_a);
		
		//About
		DrawTexture(menu_textures[titlescr_ico_abt_png_index], 600, 320, 0, MENU_MAIN_ICON_WIDTH, 64, 0xffffff00 | icon_a);
		
		tiny3d_Flip();

		if (icon_a == 32)
			break;
	}
	
	highlight_alpha = 32;
}

void Draw_MainMenu()
{
	int c = bgimg_png_index, w = 0, h = 0;
	
	//------------ Backgrounds
	
	//Background
	DrawBackgroundTexture(0, 0xff);
	
	//App logo
	c = titlescr_logo_png_index;
	w = 500; h = 140;
	DrawTexture(menu_textures[c], 424 - (w / 2), 256 - (h / 2) - 120, 0, w, h, 0xffffffff);
	
	//App description
	SetFontAlign(1);
	SetCurrentFont(font_comfortaa_light);
	SetFontSize(APP_FONT_SIZE_DESCRIPTION);
	SetFontColor(APP_FONT_COLOR | 0xFF, 0);
	DrawString(screen_width / 2, 210, MENU_MAIN_DESCRIPTION);

	SetFontSize(APP_FONT_SIZE_SUBTEXT);
	DrawString(screen_width / 2, 480, MENU_MAIN_FOOTER);

	//------------ Icons
	SetFontAlign(3);
	SetFontSize(APP_FONT_SIZE_SUBTEXT);
	SetCurrentFont(font_comfortaa_regular);

	//USB saves
	c = titlescr_ico_xmb_png_index;
	DrawTexture(menu_textures[c], 200, 320, 0, MENU_MAIN_ICON_WIDTH, 64, 0xffffff00 | ((menu_sel == 0) ? 0xFF : 32));
	SetFontColor(APP_FONT_COLOR | ((menu_sel == 0) ? 0xFF : 32), 0);
	DrawString(200 + (MENU_MAIN_ICON_WIDTH / 2), 390, "USB Saves");

	//HDD saves
	c = titlescr_ico_cht_png_index;
	DrawTexture(menu_textures[c], 300, 320, 0, MENU_MAIN_ICON_WIDTH, 64, 0xffffff00 | ((menu_sel == 1) ? 0xFF : 32));
	SetFontColor(APP_FONT_COLOR | ((menu_sel == 1) ? 0xFF : 32), 0);
	DrawString(300 + (MENU_MAIN_ICON_WIDTH / 2), 390, "HDD Saves");

	//Online Cheats
	c = titlescr_ico_net_png_index;
	DrawTexture(menu_textures[c], 400, 320, 0, MENU_MAIN_ICON_WIDTH, 64, 0xffffff00 | ((menu_sel == 2) ? 0xFF : 32));
	SetFontColor(APP_FONT_COLOR | ((menu_sel == 2) ? 0xFF : 32), 0);
	DrawString(400 + (MENU_MAIN_ICON_WIDTH / 2), 390, "Online DB");

	//Options
	c = titlescr_ico_opt_png_index;
	DrawTexture(menu_textures[c], 500, 320, 0, MENU_MAIN_ICON_WIDTH, 64, 0xffffff00 | ((menu_sel == 3) ? 0xFF : 32));
	SetFontColor(APP_FONT_COLOR | ((menu_sel == 3) ? 0xFF : 32), 0);
	DrawString(500 + (MENU_MAIN_ICON_WIDTH / 2) + 5, 390, "Options");

	//About
	c = titlescr_ico_abt_png_index;
	DrawTexture(menu_textures[c], 600, 320, 0, MENU_MAIN_ICON_WIDTH, 64, 0xffffff00 | ((menu_sel == 4) ? 0xFF : 32));
	SetFontColor(APP_FONT_COLOR | ((menu_sel == 4) ? 0xFF : 32), 0);
	DrawString(600 + (MENU_MAIN_ICON_WIDTH / 2), 390, "About");

	SetFontAlign(0);

}

// Used only in initialization. Allocates 64 mb for textures and loads the font
void LoadTexture()
{
	texture_mem = tiny3d_AllocTexture(64*1024*1024); // alloc 64MB of space for textures (this pointer can be global)
	
	if(!texture_mem) return; // fail!

	u32 * texture_pointer = texture_mem; // use to asign texture space without changes texture_mem
	
	ResetFont();
	
	TTFLoadFont(NULL, (void *)comfortaa_regular_ttf, comfortaa_regular_ttf_size);
	texture_pointer = (u32 *) AddFontFromTTF((u8 *) texture_pointer, 32, 255, 32, 32, TTF_to_Bitmap);
	TTFLoadFont(NULL, (void *)comfortaa_bold_ttf, comfortaa_bold_ttf_size);
	texture_pointer = (u32 *) AddFontFromTTF((u8 *) texture_pointer, 32, 255, 32, 32, TTF_to_Bitmap);
	TTFLoadFont(NULL, (void *)comfortaa_light_ttf, comfortaa_light_ttf_size);
	texture_pointer = (u32 *) AddFontFromTTF((u8 *) texture_pointer, 32, 255, 32, 32, TTF_to_Bitmap);
	
	font_mem = texture_pointer;
	
	TTFUnloadFont();
	
	if (!menu_textures && TOTAL_MENU_TEXTURES)
		menu_textures = (png_texture *)malloc(sizeof(png_texture) * TOTAL_MENU_TEXTURES);
	
	//Init Main Menu textures
	load_menu_texture(bgimg, png);
	load_menu_texture(cheat, png);
	load_menu_texture(circle_error_dark, png);
	load_menu_texture(circle_error_light, png);
	load_menu_texture(circle_loading_bg, png);
	load_menu_texture(circle_loading_seek, png);
	load_menu_texture(edit_ico_add, png);
	load_menu_texture(edit_ico_del, png);
	load_menu_texture(edit_shadow, png);
	load_menu_texture(footer_ico_circle, png);
	load_menu_texture(footer_ico_cross, png);
	load_menu_texture(footer_ico_lt, png);
	load_menu_texture(footer_ico_rt, png);
	load_menu_texture(footer_ico_square, png);
	load_menu_texture(footer_ico_triangle, png);
	load_menu_texture(header_dot, png);
	load_menu_texture(header_ico_abt, png);
	load_menu_texture(header_ico_cht, png);
	load_menu_texture(header_ico_opt, png);
	load_menu_texture(header_ico_xmb, png);
	load_menu_texture(header_line, png);
	load_menu_texture(help, png);
	load_menu_texture(mark_arrow, png);
	load_menu_texture(mark_line, png);
	load_menu_texture(opt_off, png);
	load_menu_texture(opt_on, png);
	load_menu_texture(scroll_bg, png);
	load_menu_texture(scroll_lock, png);
	load_menu_texture(titlescr_ico_abt, png);
	load_menu_texture(titlescr_ico_cht, png);
	load_menu_texture(titlescr_ico_opt, png);
	load_menu_texture(titlescr_ico_xmb, png);
	load_menu_texture(titlescr_ico_net, png);
	load_menu_texture(titlescr_logo, png);
}

void LoadTextures_Menu()
{
	if (!font_mem) return; // fail!

	u32 * texture_pointer = font_mem; // use to assign texture space without changes texture_mem
	int cnt = 0;

	//Main Menu
	for (cnt = 0; cnt < TOTAL_MENU_TEXTURES; cnt++)
	{
		pngLoadFromBuffer(menu_textures[cnt].buffer, menu_textures[cnt].size, &menu_textures[cnt].texture);

		// copy texture datas from PNG to the RSX memory allocated for textures
		if (menu_textures[cnt].texture.bmp_out)
		{
			memcpy(texture_pointer, menu_textures[cnt].texture.bmp_out, menu_textures[cnt].texture.pitch * menu_textures[cnt].texture.height);
			free(menu_textures[cnt].texture.bmp_out); // free the PNG because i don't need this datas
			menu_textures[cnt].texture_off = tiny3d_TextureOffset(texture_pointer);      // get the offset (RSX use offset instead address)
			texture_pointer += ((menu_textures[cnt].texture.pitch * menu_textures[cnt].texture.height + 15) & ~15) / 4; // aligned to 16 bytes (it is u32) and update the pointer
		}
	}

	u32 tBytes = texture_pointer - texture_mem;
	LOG("LoadTextures_Menu() :: Allocated %db (%.02fkb, %.02fmb) for textures", tBytes, tBytes / (float)1024, tBytes / (float)(1024 * 1024));
}

short *background_music = NULL;
int background_music_size = 48000*72*4; // initial size of buffer to decode (for 48000 samples x 72 seconds and 16 bit stereo as reference)
int effect_freq;
int effect_is_stereo;

void LoadSounds()
{
	//Initialize SPU
	u32 entry = 0;
	u32 segmentcount = 0;
	sysSpuSegment* segments;
	
	sysSpuInitialize(6, 5);
	sysSpuRawCreate(&spu, NULL);
	sysSpuElfGetInformation(spu_soundmodule_bin, &entry, &segmentcount);
	
	size_t segmentsize = sizeof(sysSpuSegment) * segmentcount;
	segments = (sysSpuSegment*)memalign(128, SPU_SIZE(segmentsize)); // must be aligned to 128 or it break malloc() allocations
	memset(segments, 0, segmentsize);

	sysSpuElfGetSegments(spu_soundmodule_bin, segments, segmentcount);
	sysSpuImageImport(&spu_image, spu_soundmodule_bin, 0);
	sysSpuRawImageLoad(spu, &spu_image);
	
	inited |= INITED_SPU;
	if(SND_Init(spu)==0)
		inited |= INITED_SOUNDLIB;
	
	background_music   = (short *) malloc(background_music_size);

	//printf("Decoding Effect\n");

	// decode the mp3 effect file included to memory. It stops by EOF or when samples exceed size_effects_samples
	DecodeAudio( (void *) background_music_mp3, background_music_mp3_size, background_music, &background_music_size, &effect_freq, &effect_is_stereo);

	// adjust the sound buffer sample correctly to the background_music_size
	{
		// SPU dma works aligned to 128 bytes. SPU module is designed to read unaligned buffers and it is better thing aligned buffers)
		short *temp = (short *)memalign(128, SPU_SIZE(background_music_size));
		memcpy((void *) temp, (void *) background_music, background_music_size);
		free(background_music);

		background_music = temp;
	}
	
	SND_Pause(0);
}

void music_callback(int sel)
{
	apollo_config.music = !sel;

	if (apollo_config.music)
		//on
		SND_PauseVoice(2, 0);
	else
		//off
		SND_PauseVoice(2, 1);
}

void sort_callback(int sel)
{
	apollo_config.doSort = !sel;
}

void ani_callback(int sel)
{
	apollo_config.doAni = !sel;
}

void horm_callback(int sel)
{
	if (sel == 255)
		sel = 0;
	if (sel > 100)
		sel = 100;
	apollo_config.marginH = sel;
}

void verm_callback(int sel)
{
	if (sel == 255)
		sel = 0;
	if (sel > 100)
		sel = 100;
	apollo_config.marginV = sel;
}

void update_callback(int sel)
{
	if (sel)
	{
		if (http_download(ONLINE_URL, "cheatdb.zip", ONLINE_LOCAL_CACHE "tmp.zip", 1))
		{
			if (extract_zip(ONLINE_LOCAL_CACHE "tmp.zip", APOLLO_DATA_PATH))
				show_dialog(0, "Successfully updated local cheat database");

			unlink_secure(ONLINE_LOCAL_CACHE "tmp.zip");
		}
	}
}

void set_usb_path(save_list_t* save_list)
{
	if (dir_exists(SAVES_PATH_USB0) == SUCCESS)
		strcpy(save_list->path, SAVES_PATH_USB0);
	else if (dir_exists(SAVES_PATH_USB1) == SUCCESS)
		strcpy(save_list->path, SAVES_PATH_USB1);
	else
		strcpy(save_list->path, "");
}

void ReloadUserSaves(save_list_t* save_list)
{
    init_loading_screen();

	if (save_list->list)
	{
		UnloadGameList(save_list->list, save_list->count);
		save_list->count = 0;
		save_list->list = NULL;
	}
	
	save_list->list = ReadUserList(save_list->path, &(save_list->count));
	if (apollo_config.doSort)
		BubbleSortGameList(save_list->list, save_list->count);

    stop_loading_screen();
}

void ReloadOnlineCheats()
{
	if (online_saves.list)
	{
		UnloadGameList(online_saves.list, online_saves.count);
		online_saves.count = 0;
		online_saves.list = NULL;
	}

	online_saves.list = ReadOnlineList(&online_saves.count);
	if (apollo_config.doSort)
		BubbleSortGameList(online_saves.list, online_saves.count);
}

void SetMenu(int id)
{   
	switch (menu_id) //Leaving menu
	{
		case MENU_MAIN_SCREEN: //Main Menu
		case MENU_USB_SAVES: //USB Saves Menu
		case MENU_HDD_SAVES: //HHD Saves Menu
		case MENU_ONLINE_DB: //Cheats Online Menu
		case MENU_SETTINGS: //Options Menu
		case MENU_CREDITS: //About Menu
		case MENU_PATCHES: //Cheat Selection Menu			
			break;

		case MENU_SAVE_DETAILS:
		case MENU_PATCH_VIEW: //Cheat View Menu
			Draw_CheatsMenu_View_Ani_Exit();
			break;

		case MENU_CODE_OPTIONS: //Cheat Option Menu
			Draw_CheatsMenu_Options_Ani_Exit();
			break;
	}
	
	switch (id) //going to menu
	{
		case MENU_MAIN_SCREEN: //Main Menu
			if (apollo_config.doAni || menu_id == MENU_MAIN_SCREEN) //if load animation
				Draw_MainMenu_Ani();
			break;

		case MENU_USB_SAVES: //USB saves Menu
			if (!usb_saves.list)
			{
			    set_usb_path(&usb_saves);
				ReloadUserSaves(&usb_saves);
			}
			
			if (apollo_config.doAni)
				Draw_UserCheatsMenu_Ani(usb_saves.list, usb_saves.count);
			break;

		case MENU_HDD_SAVES: //HDD saves Menu
			if (!hdd_saves.list)
			{
				snprintf(hdd_saves.path, sizeof(hdd_saves.path), SAVES_PATH_HDD, apollo_config.user_id);
				ReloadUserSaves(&hdd_saves);
			}
			
			if (apollo_config.doAni)
				Draw_UserCheatsMenu_Ani(hdd_saves.list, hdd_saves.count);
			break;

		case MENU_ONLINE_DB: //Cheats Online Menu
			if (!online_saves.list)
			{
				ReloadOnlineCheats();
			}

			if (apollo_config.doAni)
				Draw_UserCheatsMenu_Ani(online_saves.list, online_saves.count);
			break;

		case MENU_CREDITS: //About Menu
			if (apollo_config.doAni)
				Draw_AboutMenu_Ani();
			break;

		case MENU_SETTINGS: //Options Menu
			if (apollo_config.doAni)
				Draw_OptionsMenu_Ani();
			break;

		case MENU_PATCHES: //Cheat Selection Menu
			if (menu_id == MENU_USB_SAVES || menu_id == MENU_HDD_SAVES) //if entering from game list, don't keep index, otherwise keep
				menu_old_sel[MENU_PATCHES] = 0;
			if (apollo_config.doAni && menu_id != MENU_PATCH_VIEW && menu_id != MENU_CODE_OPTIONS)
				Draw_CheatsMenu_Selection_Ani();
			break;

		case MENU_PATCH_VIEW: //Cheat View Menu
			menu_old_sel[MENU_PATCH_VIEW] = 0;
			if (apollo_config.doAni)
				Draw_CheatsMenu_View_Ani();
			break;

		case MENU_SAVE_DETAILS: //Save Detail View Menu
			if (apollo_config.doAni)
				Draw_CheatsMenu_View_Ani();
			break;

		case MENU_CODE_OPTIONS: //Cheat Option Menu
			menu_old_sel[MENU_CODE_OPTIONS] = 0;
			if (apollo_config.doAni)
				Draw_CheatsMenu_Options_Ani();
			break;
	}
	
	menu_old_sel[menu_id] = menu_sel;
	if (last_menu_id[menu_id] != id)
		last_menu_id[id] = menu_id;
	menu_id = id;
	
	menu_sel = menu_old_sel[menu_id];
}

void move_letter_back(save_entry_t * games, int game_count)
{
	int i;
	char ch = games[menu_sel].name[0];

	if ((ch > '\x29') && (ch < '\x40'))
	{
		menu_sel = 0;
		return;
	}

	for (i = menu_sel; (i > 0) && (ch == games[i].name[0]); i--) {}

	menu_sel = i;
}

void move_letter_fwd(save_entry_t * games, int game_count)
{
	int i;
	char ch = games[menu_sel].name[0];

	if (ch == 'Z')
	{
		menu_sel = game_count - 1;
		return;
	}
	
	for (i = menu_sel; (i < game_count - 2) && (ch == games[i].name[0]); i++) {}

	menu_sel = i;
}

void move_selection_back(int game_count, int steps)
{
	menu_sel -= steps;
	if ((menu_sel == -1) && (steps == 1))
		menu_sel = game_count - 1;
	else if (menu_sel < 0)
		menu_sel = 0;
}

void move_selection_fwd(int game_count, int steps)
{
	menu_sel += steps;
	if ((menu_sel == game_count) && (steps == 1))
		menu_sel = 0;
	else if (menu_sel >= game_count)
		menu_sel = game_count - 1;
}

int moveMenuSelection(padData pad_data, save_list_t * save_list, int(*read_codes)(save_entry_t*) )
{
	if(pad_data.BTN_UP)
	{
		move_selection_back(save_list->count, 1);
	}
	else if(pad_data.BTN_DOWN)
	{
		move_selection_fwd(save_list->count, 1);
	}
	else if (pad_data.BTN_LEFT)
	{
		move_selection_back(save_list->count, 5);
	}
	else if (pad_data.BTN_L1)
	{
		move_selection_back(save_list->count, 25);
	}
	else if (pad_data.BTN_L2)
	{
		move_letter_back(save_list->list, save_list->count);
	}
	else if (pad_data.BTN_RIGHT)
	{
		move_selection_fwd(save_list->count, 5);
	}
	else if (pad_data.BTN_R1)
	{
		move_selection_fwd(save_list->count, 25);
	}
	else if (pad_data.BTN_R2)
	{
		move_letter_fwd(save_list->list, save_list->count);
	}
	else if (pad_data.BTN_CIRCLE)
	{
		SetMenu(MENU_MAIN_SCREEN);
		return 1;
	}
	else if (pad_data.BTN_CROSS)
	{
		if (!save_list->list[menu_sel].codes)
			read_codes(&save_list->list[menu_sel]);

		if (apollo_config.doSort)
			save_list->list[menu_sel] = BubbleSortCodeList(save_list->list[menu_sel]);
		selected_entry = save_list->list[menu_sel];
		SetMenu(MENU_PATCHES);
		return 1;
	}

	return 0;
}

void build_patch(sfo_patch_t* patch)
{
    int j;

	for (j = 0; j < selected_entry.code_count; j++)
	{
		if (!selected_entry.codes[j].activated)
		    continue;
		    
    	LOG("Active: [%s]", selected_entry.codes[j].name);

		if (strcmp(selected_entry.codes[j].codes, CODE_UNLOCK_COPY) == 0)
		    patch->flags = SFO_PATCH_FLAG_REMOVE_COPY_PROTECTION;

		if (strcmp(selected_entry.codes[j].codes, CODE_REMOVE_ACCOUNT_ID) == 0)
		    strcpy(patch->account_id, "0000000000000000");

		selected_entry.codes[j].activated = 0;
	}
}

// Resets new frame
void drawScene()
{   
	tiny3d_Project2D(); // change to 2D context (remember you it works with 848 x 512 as virtual coordinates)
	
//	char * userc, * onlinec;
	int max = 0;
	
	switch (menu_id)
	{
		case MENU_MAIN_SCREEN:
			
			// Check the pads.
			if (readPad(0))
			{
				if(paddata[0].BTN_LEFT)
				{
					move_selection_back(5, 1);
				}
				else if(paddata[0].BTN_RIGHT)
				{
					move_selection_fwd(5, 1);
				}
				else if (paddata[0].BTN_CROSS)
				{
				    SetMenu(menu_sel+1);
				}
				else if(paddata[0].BTN_CIRCLE && show_dialog(1, "Exit to XMB?"))
				{
					close_app = 1;
				}

				else if(paddata[0].BTN_SELECT)
				{
					LOG("Screen");
//					dbglogger_screenshot_tmp(0);
					LOG("Shot");
				}
			}
			
			Draw_MainMenu();
			break;

		case MENU_USB_SAVES: //USB Saves Menu
			// Check the pads.
			if (readPad(0))
			{
				if(moveMenuSelection(paddata[0], &usb_saves, ReadCodes))
				{
					return;
				}
            	else if (paddata[0].BTN_TRIANGLE)
            	{
            		char sfoPath[256];
            		selected_entry = usb_saves.list[menu_sel];

            		snprintf(sfoPath, sizeof(sfoPath), "%s" "PARAM.SFO", selected_entry.path);
        			LOG("Save Details :: Reading %s...", sfoPath);
        
        			sfo_context_t* sfo = sfo_alloc();
        			if (sfo_read(sfo, sfoPath) < 0) {
        				LOG("Unable to read from '%s'", sfoPath);
        				sfo_free(sfo);
        				return;
        			}
        
        			sfo_params_ids_t* param_ids = (sfo_params_ids_t*)(sfo_get_param_value(sfo, "PARAMS") + 0x1C);
        			param_ids->user_id = ES32(param_ids->user_id);

            	    asprintf(&selected_centry.name, "Details");
            	    asprintf(&selected_centry.codes, "%s\n\n"
            	    "Name: %s\n"
            	    "Title: %s\n"
            	    "User ID: %08d\n"
            	    "Account ID: %s\n"
            	    "PSID: %016lX %016lX\n", selected_entry.path, selected_entry.name, selected_entry.title_id,
            	    param_ids->user_id, param_ids->account_id, param_ids->psid[0], param_ids->psid[1]);
        			LOG(selected_centry.codes);

        			sfo_free(sfo);

            		SetMenu(MENU_SAVE_DETAILS);
            		return;
            	}
				else if (paddata[0].BTN_SQUARE)
				{
				    set_usb_path(&usb_saves);
					ReloadUserSaves(&usb_saves);
				}
			}
			
			Draw_UserCheatsMenu(usb_saves.list, usb_saves.count, menu_sel, 0xFF);
			break;

		case MENU_HDD_SAVES: //HDD Saves Menu
			// Check the pads.
			if (readPad(0))
			{
				if(moveMenuSelection(paddata[0], &hdd_saves, ReadCodes))
				{
					return;
				}
				else if (paddata[0].BTN_SQUARE)
				{
					snprintf(hdd_saves.path, sizeof(hdd_saves.path), SAVES_PATH_HDD, apollo_config.user_id);
					ReloadUserSaves(&hdd_saves);
				}
			}
			
			Draw_UserCheatsMenu(hdd_saves.list, hdd_saves.count, menu_sel, 0xFF);
			break;

		case MENU_ONLINE_DB: //Online Cheats Menu
			// Check the pads.
			if (readPad(0))
			{
				if(moveMenuSelection(paddata[0], &online_saves, ReadOnlineSaves))
				{
					return;
				}
				else if (paddata[0].BTN_SQUARE)
				{
					ReloadOnlineCheats();
				}
			}

			Draw_UserCheatsMenu(online_saves.list, online_saves.count, menu_sel, 0xFF);
			break;

		case MENU_CREDITS: //About Menu
			// Check the pads.
			if (readPad(0))
			{
				if (paddata[0].BTN_CIRCLE)
				{
					SetMenu(MENU_MAIN_SCREEN);
					return;
				}
			}
			
			Draw_AboutMenu();
			break;

		case MENU_SETTINGS: //Options Menu
			// Check the pads.
			if (readPad(0))
			{
				if(paddata[0].BTN_UP)
				{
					move_selection_back(menu_options_maxopt, 1);
				}
				else if(paddata[0].BTN_DOWN)
				{
					move_selection_fwd(menu_options_maxopt, 1);
				}
				else if (paddata[0].BTN_CIRCLE)
				{
					save_app_settings(&apollo_config);
					SetMenu(MENU_MAIN_SCREEN);
					return;
				}
				else if (paddata[0].BTN_LEFT)
				{
					if (menu_options[menu_sel].type == APP_OPTION_LIST)
					{
						if (*menu_options[menu_sel].value > 0)
							(*menu_options[menu_sel].value)--;
						else
							*menu_options[menu_sel].value = menu_options_maxsel[menu_sel] - 1;
					}
					else if (menu_options[menu_sel].type == APP_OPTION_INC)
						(*menu_options[menu_sel].value)--;
					
					menu_options[menu_sel].callback(*menu_options[menu_sel].value);
				}
				else if (paddata[0].BTN_RIGHT)
				{
					if (menu_options[menu_sel].type == APP_OPTION_LIST)
					{
						if (*menu_options[menu_sel].value < (menu_options_maxsel[menu_sel] - 1))
							*menu_options[menu_sel].value += 1;
						else
							*menu_options[menu_sel].value = 0;
					}
					else if (menu_options[menu_sel].type == APP_OPTION_INC)
						*menu_options[menu_sel].value += 1;

					menu_options[menu_sel].callback(*menu_options[menu_sel].value);
				}
				else if (paddata[0].BTN_CROSS)
				{
					if ((menu_options[menu_sel].type == APP_OPTION_BOOL) || (menu_options[menu_sel].type == APP_OPTION_CALL))
						menu_options[menu_sel].callback(*menu_options[menu_sel].value);
				}
			}
			
			Draw_OptionsMenu();
			break;

		case MENU_PATCHES: //Cheats Selection Menu
			// Check the pads.
			if (readPad(0))
			{
				if(paddata[0].BTN_UP)
				{
					move_selection_back(selected_entry.code_count, 1);
				}
				else if(paddata[0].BTN_DOWN)
				{
					move_selection_fwd(selected_entry.code_count, 1);
				}
				else if (paddata[0].BTN_LEFT)
				{
					move_selection_back(selected_entry.code_count, 5);
				}
				else if (paddata[0].BTN_RIGHT)
				{
					move_selection_fwd(selected_entry.code_count, 5);
				}
				else if (paddata[0].BTN_CIRCLE)
				{
					for (int j = 0; j < selected_entry.code_count; j++)
						selected_entry.codes[j].activated = 0;

					SetMenu(last_menu_id[MENU_PATCHES]);
					return;
				}
				else if (paddata[0].BTN_CROSS)
				{
					selected_entry.codes[menu_sel].activated = !selected_entry.codes[menu_sel].activated;

					if (strcmp(selected_entry.codes[menu_sel].codes, CODE_RESIGN_SAVE) == 0)
					{
						char in_file_path[256];
						sfo_patch_t patch = {
					        .flags = 0,
					        .user_id = apollo_config.user_id,
					        .account_id = NULL,
					        .psid = (u8*) &(apollo_config.psid[0]),
						};

						asprintf(&patch.account_id, "%016lx", apollo_config.account_id);
						build_patch(&patch);

						snprintf(in_file_path, sizeof(in_file_path), "%s" "PARAM.SFO", selected_entry.path);
						LOG("Applying SFO patches '%s'...", in_file_path);

		                if (patch_sfo(in_file_path, &patch) == SUCCESS)
			            {
    						LOG("Resigning save '%s'...", selected_entry.name);
    						if (pfd_util_init(selected_entry.title_id, selected_entry.path))
    						{
    							if (pfd_util_process(PFD_CMD_UPDATE, 0) == SUCCESS)
        		                    show_dialog(0, "Save file successfully resigned!");
    	    	                else
    		                        show_dialog(0, "Error! Save file couldn't be resigned");
    
    							pfd_util_process(PFD_CMD_CHECK, 0);
    						}
                            else
                            {
    		                    show_dialog(0, "Error! Save file couldn't be resigned");
                            }
    
    						pfd_util_end();
						}

						if(patch.account_id)
						    free(patch.account_id);

						selected_entry.codes[menu_sel].activated = 0;
					}
					
					if (selected_entry.codes[menu_sel].activated)
					{
						//Check if option code
						if (!selected_entry.codes[menu_sel].options)
						{
							int size[1];
							selected_entry.codes[menu_sel].options = ReadOptions(selected_entry.codes[menu_sel], (int *)size);
							selected_entry.codes[menu_sel].options_count = size[0];
						}
						
						if (selected_entry.codes[menu_sel].options)
						{
							selected_centry = selected_entry.codes[menu_sel];
							option_index = 0;
							SetMenu(MENU_CODE_OPTIONS);
							return;
						}
					}
				}
/*
				else if (paddata[0].BTN_TRIANGLE)
				{
					selected_centry = selected_entry.codes[menu_sel];
					SetMenu(MENU_PATCH_VIEW);
					return;
				}
*/
			}
			
			Draw_CheatsMenu_Selection(menu_sel, 0xFFFFFFFF);
			break;

		case MENU_PATCH_VIEW: //Cheat View Menu
			//Calc max
			max = 0;
			const char * str;
			for(str = selected_centry.codes; *str; ++str)
				max += *str == '\n';
			//max += -((512 - (120*2))/18) + 1; //subtract the max per page
			if (max <= 0)
				max = 1;
			
			// Check the pads.
			if (readPad(0))
			{
				if(paddata[0].BTN_UP)
				{
					move_selection_back(max, 1);
				}
				else if(paddata[0].BTN_DOWN)
				{
					move_selection_fwd(max, 1);
				}
				else if (paddata[0].BTN_CIRCLE)
				{
					SetMenu(last_menu_id[MENU_PATCH_VIEW]);
					return;
				}
			}
			
			Draw_CheatsMenu_View();
			break;

		case MENU_CODE_OPTIONS: //Cheat Option Menu
			// Check the pads.
			if (readPad(0))
			{
				if(paddata[0].BTN_UP)
				{
					move_selection_back(selected_centry.options[option_index].size, 1);
				}
				else if(paddata[0].BTN_DOWN)
				{
					move_selection_fwd(selected_centry.options[option_index].size, 1);
				}
				else if (paddata[0].BTN_CIRCLE)
				{
					selected_entry.codes[menu_old_sel[last_menu_id[MENU_CODE_OPTIONS]]].activated = 0;
					SetMenu(last_menu_id[MENU_CODE_OPTIONS]);
					return;
				}
				else if (paddata[0].BTN_CROSS)
				{
					selected_entry.codes[menu_old_sel[last_menu_id[MENU_CODE_OPTIONS]]].options[option_index].sel = menu_sel;
					option_index++;
					
					if (option_index >= selected_entry.codes[menu_old_sel[last_menu_id[MENU_CODE_OPTIONS]]].options_count)
					{
						SetMenu(last_menu_id[MENU_CODE_OPTIONS]);
						return;
					}
					else
						menu_sel = 0;
				}
			}
			
			Draw_CheatsMenu_Options();
			break;

		case MENU_SAVE_DETAILS: //Cheat View Menu
			// Check the pads.
			if (readPad(0))
			{
				if(paddata[0].BTN_UP)
				{
					move_selection_back(7, 1);
				}
				else if(paddata[0].BTN_DOWN)
				{
					move_selection_fwd(7, 1);
				}
				if (paddata[0].BTN_CIRCLE)
				{
					if (selected_centry.name)
						free(selected_centry.name);
					if (selected_centry.codes)
						free(selected_centry.codes);

					SetMenu(last_menu_id[MENU_SAVE_DETAILS]);
					return;
				}
			}
			
			Draw_CheatsMenu_View();
			break;
	}
}

void exiting()
{
	http_end();
	sysModuleUnload(SYSMODULE_PNGDEC);
}

/*
	Program start
*/
s32 main(s32 argc, const char* argv[])
{
	dbglogger_init_str("tcp:192.168.1.102:18999");
//	dbglogger_init_str("file:/dev_hdd0/tmp/apollo.log");

	http_init();

	load_app_settings(&apollo_config);

	// set to display the PSID on the About menu
    sprintf(psid_str1, "%016lX", apollo_config.psid[0]);
    sprintf(psid_str2, "%016lX", apollo_config.psid[1]);
    sprintf(user_id_str, "%08d", apollo_config.user_id);
    sprintf(account_id_str, "%016lX", apollo_config.account_id);

	pfd_util_setup_keys((u8*) &(apollo_config.psid[0]), apollo_config.user_id);

	tiny3d_Init(1024*1024);

	ioPadInit(7);
	
	sysModuleLoad(SYSMODULE_PNGDEC);

	atexit(exiting); // Tiny3D register the event 3 and do exit() call when you exit  to the menu

	// register exit callback
	if(sysUtilRegisterCallback(SYSUTIL_EVENT_SLOT0, sys_callback, NULL)==0) inited |= INITED_CALLBACK;
	
	// Load texture
	LoadTexture();
	LoadTextures_Menu();
	LoadSounds();
	
	// Setup font
	SetExtraSpace(5);
	SetCurrentFont(0);
	RegisterSpecialCharacter(0x10,
		menu_textures[footer_ico_cross_png_index].texture.width / 1,
		4,
		1.2,
		1.2,
		menu_textures[footer_ico_cross_png_index]);
	RegisterSpecialCharacter(0x11,
		menu_textures[footer_ico_square_png_index].texture.width / 1,
		4,
		1.2,
		1.2,
		menu_textures[footer_ico_square_png_index]);
	RegisterSpecialCharacter(0x12,
		menu_textures[footer_ico_triangle_png_index].texture.width / 1,
		4,
		1.2,
		1.2,
		menu_textures[footer_ico_triangle_png_index]);
	RegisterSpecialCharacter(0x13,
		menu_textures[footer_ico_circle_png_index].texture.width / 1,
		4,
		1.2,
		1.2,
		menu_textures[footer_ico_circle_png_index]);

	menu_options_maxopt = 0;
	while (menu_options[menu_options_maxopt].name)
		menu_options_maxopt++;
	
	int selSize = menu_options_maxopt * sizeof(int);
	menu_options_maxsel = (int *)calloc(1, selSize);
	
	int i = 0;
	for (i = 0; i < menu_options_maxopt; i++)
	{
		menu_options_maxsel[i] = 0;
		if (menu_options[i].type == APP_OPTION_LIST)
		{
			while (menu_options[i].options[menu_options_maxsel[i]])
				menu_options_maxsel[i]++;
		}
	}

	videoState state;
	assert(videoGetState(0, 0, &state) == 0); // Get the state of the display
	assert(state.state == 0); // Make sure display is enabled
	
	videoResolution res;
	assert(videoGetResolution(state.displayMode.resolution, &res) == 0);
	LOG("Resolution: %dx%d", res.width, res.height);
	screen_width = res.width;
	screen_height = res.height;
	
	SND_SetInfiniteVoice(2, (effect_is_stereo) ? VOICE_STEREO_16BIT : VOICE_MONO_16BIT, effect_freq, 0, background_music, background_music_size, 255, 255);
	
	//Set options
	music_callback(!apollo_config.music);

	SetMenu(MENU_MAIN_SCREEN);
	
	while (!close_app)
	{       
		tiny3d_Clear(0xff000000, TINY3D_CLEAR_ALL);

		// Enable alpha Test
		tiny3d_AlphaTest(1, 0x10, TINY3D_ALPHA_FUNC_GEQUAL);

		// Enable alpha blending.
		tiny3d_BlendFunc(1, TINY3D_BLEND_FUNC_SRC_RGB_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_ALPHA_SRC_ALPHA,
			TINY3D_BLEND_FUNC_SRC_ALPHA_ONE_MINUS_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_RGB_ONE_MINUS_SRC_ALPHA,
			TINY3D_BLEND_RGB_FUNC_ADD | TINY3D_BLEND_ALPHA_FUNC_ADD);
		
		drawScene();
		
		//Draw help
		if (menu_pad_help[menu_id])
		{
			u8 alpha = 0xFF;
			if (idle_time > 80)
			{
				int dec = (idle_time - 80) * 4;
				if (dec > alpha)
					dec = alpha;
				alpha -= dec;
			}
			
			SetFontSize(APP_FONT_SIZE_DESCRIPTION);
			SetCurrentFont(0);
			SetFontAlign(1);
			SetFontColor(APP_FONT_COLOR | alpha, 0);
			DrawString(0, 480, (char *)menu_pad_help[menu_id]);
			SetFontAlign(0);
		}
		
		tiny3d_Flip();
		
		/* Alpha calculation to control Highlight pulse */
		if (highlight_pulse) {
			highlight_alpha += highlight_amount;
			if (highlight_alpha > 128) {
				highlight_alpha = 128;
				pause_pulse++;
				if (pause_pulse >= 12) {
					highlight_amount = -3;
					pause_pulse = 0;
				}
			} else if (highlight_alpha < 32) {
				highlight_amount = 3;
				highlight_alpha = 32;
			}
		}
		else {
			highlight_alpha = 128;  
		}
	}
	
	return 0;
}
