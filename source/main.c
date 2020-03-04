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
//#include <pngdec/pngdec.h>

#include <tiny3d.h>
#include "libfont.h"
#include "saves.h"
#include "sfo.h"
#include "pfd.h"
#include "util.h"

//From NzV's MAMBA PRX Loader (https://github.com/NzV/MAMBA_PRX_Loader)
#include "common.h"
#include "lv2_utils.h"

//Menus
#include "menu.h"
#include "menu_about.h"
#include "menu_options.h"
#include "menu_cheats.h"

enum menu_screen_ids
{
	MENU_MAIN_SCREEN,
	MENU_USB_SAVES,
	MENU_HDD_SAVES,
	MENU_ONLINE_DB,
	MENU_USER_BACKUP,
	MENU_SETTINGS,
	MENU_CREDITS,
	MENU_PATCHES,
	MENU_PATCH_VIEW,
	MENU_CODE_OPTIONS,
	MENU_SAVE_DETAILS,
	TOTAL_MENU_IDS
};

//Font
#include "ttf_render.h"

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
	   LoadTexture(name##_##type##_index); \
	})

#define show_message(msg)	show_dialog(0, msg)

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
void clearcache_callback(int sel);
void up_appdata_callback(int sel);

void update_usb_path(char *p);
void update_hdd_path(char *p);

app_config_t apollo_config = {
    .music = 1,
    .doSort = 1,
    .doAni = 1,
    .update = 1,
    .marginH = 0,
    .marginV = 0,
    .user_id = 0,
    .psid = {0, 0},
    .account_id = 0,
};

const menu_option_t menu_options[] = {
	{ .name = "Background Music", .options = NULL, .type = APP_OPTION_BOOL, .value = &apollo_config.music, .callback = music_callback },
	{ .name = "Sort Saves", .options = NULL, .type = APP_OPTION_BOOL, .value = &apollo_config.doSort, .callback = sort_callback },
	{ .name = "Menu Animations", .options = NULL, .type = APP_OPTION_BOOL, .value = &apollo_config.doAni, .callback = ani_callback },
	{ .name = "Screen Horizontal Margin", .options = NULL, .type = APP_OPTION_INC, .value = &apollo_config.marginH, .callback = horm_callback },
	{ .name = "Screen Vertical Margin", .options = NULL, .type = APP_OPTION_INC, .value = &apollo_config.marginV, .callback = verm_callback },
	{ .name = "Version Update Check", .options = NULL, .type = APP_OPTION_BOOL, .value = &apollo_config.update, .callback = update_callback },
	{ .name = "Clear Local Cache", .options = NULL, .type = APP_OPTION_CALL, .value = NULL, .callback = clearcache_callback },
	{ .name = "Update Application Data", .options = NULL, .type = APP_OPTION_CALL, .value = NULL, .callback = up_appdata_callback },
	{ .name = NULL }
};

int menu_options_maxopt = 0;
int * menu_options_maxsel;

int close_app = 0;

png_texture * menu_textures;                // png_texture array for main menu, initialized in LoadTexture

int idle_time = 0;                          // Set by readPad

#define MENU_MAIN_DESCRIPTION   "Apollo Save Tool"
#define MENU_MAIN_FOOTER        "www.bucanero.com.ar"

const char * menu_about_strings[] = { "Bucanero", "Developer",
									"Berion", "GUI design",
									"Dnawrkshp", "Artemis code",
									"flatz", "PFD/SFO tools",
									NULL, NULL };

char user_id_str[9] = "00000000";
char psid_str1[SFO_PSID_SIZE+1] = "0000000000000000";
char psid_str2[SFO_PSID_SIZE+1] = "0000000000000000";
char account_id_str[SFO_ACCOUNT_ID_SIZE+1] = "0000000000000000";

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
int menu_old_sel[TOTAL_MENU_IDS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };		// Previous menu_sel for each menu
int last_menu_id[TOTAL_MENU_IDS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };		// Last menu id called (for returning)

const char * menu_pad_help[TOTAL_MENU_IDS] = { NULL,												//Main
								"\x10 Select    \x13 Back    \x12 Details    \x11 Refresh",			//USB list
								"\x10 Select    \x13 Back    \x12 Details    \x11 Refresh",			//HDD list
								"\x10 Select    \x13 Back    \x11 Refresh",							//Online list
								"\x10 Select    \x13 Back",											//User backup
								"\x10 Select    \x13 Back",											//Options
								"\x13 Back",														//About
								"\x10 Select    \x12 View Code    \x13 Back",						//Select Cheats
								"\x13 Back",														//View Cheat
								"\x10 Select    \x13 Back",											//Cheat Option
								"\x13 Back",														//View Details
								};

/*
* HDD save list
*/
save_list_t hdd_saves = {
	.icon_id = cat_hdd_png_index,
	.title = "HDD Saves",
    .list = NULL,
    .count = 0,
    .path = "",
    .ReadList = &ReadUserList,
    .ReadCodes = &ReadCodesHDD,
    .UpdatePath = &update_hdd_path,
};

/*
* USB save list
*/
save_list_t usb_saves = {
	.icon_id = cat_usb_png_index,
	.title = "USB Saves",
    .list = NULL,
    .count = 0,
    .path = "",
    .ReadList = &ReadUserList,
    .ReadCodes = &ReadCodesUSB,
    .UpdatePath = &update_usb_path,
};

/*
* Online code list
*/
save_list_t online_saves = {
	.icon_id = cat_db_png_index,
	.title = "Online Database",
    .list = NULL,
    .count = 0,
    .path = ONLINE_URL,
    .ReadList = &ReadOnlineList,
    .ReadCodes = &ReadOnlineSaves,
    .UpdatePath = NULL,
};

/*
* Online code list
*/
save_entry_t user_backup = {
	.name = "User Data Backup",
	.codes = NULL,
};

save_entry_t* selected_entry;
code_entry_t* selected_centry;
int option_index = 0;

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
		DrawTexture(&menu_textures[logo_png_index], logo_png_x, logo_png_y, 0, logo_png_w, logo_png_h, 0xFFFFFF00 | logo_a);
		
		//App description
		SetFontAlign(1);
		SetCurrentFont(font_comfortaa_light);
		SetFontSize(APP_FONT_SIZE_DESCRIPTION);
		SetFontColor(APP_FONT_COLOR | logo_a, 0);
		DrawString(0, 210, MENU_MAIN_DESCRIPTION);
		
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
		
		u8 icon_a = (u8)(((ani * rate) > 0xFF) ? 0xFF : (ani * rate));
		
		//------------ Backgrounds
		
		//Background
		DrawBackgroundTexture(0, 0xFF);
		
		//App logo
		DrawTexture(&menu_textures[logo_png_index], logo_png_x, logo_png_y, 0, logo_png_w, logo_png_h, 0xFFFFFFFF);

		//App description
		SetFontAlign(1);
		SetCurrentFont(font_comfortaa_light);
		SetFontSize(APP_FONT_SIZE_DESCRIPTION);
		SetFontColor(APP_FONT_COLOR | 0xFF, 0);
		DrawString(0, 210, MENU_MAIN_DESCRIPTION);

		drawColumns(icon_a);

		SetFontSize(APP_FONT_SIZE_SUBTEXT);
		DrawString(0, 490 + apollo_config.marginV, MENU_MAIN_FOOTER);
		
		//------------ Icons
		
		//Empty
		drawJar(jar_empty_png_index, jar_empty_png_x, jar_empty_png_y, "", icon_a);

		//USB save
		drawJar(jar_usb_png_index, jar_usb_png_x, jar_usb_png_y, "", icon_a);
		
		//HDD save
		drawJar(jar_hdd_png_index, jar_hdd_png_x, jar_hdd_png_y, "", icon_a);

		//Online cheats
		drawJar(jar_db_png_index, jar_db_png_x, jar_db_png_y, "", icon_a);
		
		//Online cheats
		drawJar(jar_bup_png_index, jar_bup_png_x, jar_bup_png_y, "", icon_a);

		//Options
		drawJar(jar_opt_png_index, jar_opt_png_x, jar_opt_png_y, "", icon_a);
		
		//About
		drawJar(jar_about_png_index, jar_about_png_x, jar_about_png_y, "", icon_a);
		
		tiny3d_Flip();

		if (icon_a == 32)
			break;
	}
}

void Draw_MainMenu()
{
	//------------ Backgrounds
	
	//Background
	DrawBackgroundTexture(0, 0xff);
	
	//App logo
	DrawTexture(&menu_textures[logo_png_index], logo_png_x, logo_png_y, 0, logo_png_w, logo_png_h, 0xffffffff);
	
	//App description
	SetFontAlign(1);
	SetCurrentFont(font_comfortaa_light);
	SetFontSize(APP_FONT_SIZE_DESCRIPTION);
	SetFontColor(APP_FONT_COLOR | 0xFF, 0);
	DrawString(0, 210, MENU_MAIN_DESCRIPTION);

	drawColumns(0xFF);

	SetFontSize(APP_FONT_SIZE_SUBTEXT);
	DrawString(0, 490 + apollo_config.marginV, MENU_MAIN_FOOTER);

	//------------ Icons
	SetFontAlign(3);
	SetFontSize(APP_FONT_SIZE_MENU);
	SetCurrentFont(font_comfortaa_regular);

//	drawColumns(0xFF);

	//Empty
	drawJar(jar_empty_png_index, jar_empty_png_x, jar_empty_png_y, "", 0xFF);

	//USB saves
	drawJar(jar_usb_png_index, jar_usb_png_x, jar_usb_png_y, "USB Saves", 0xFF);

	//HDD saves
	drawJar(jar_hdd_png_index, jar_hdd_png_x, jar_hdd_png_y, "HDD Saves", 0xFF);

	//Online Cheats
	drawJar(jar_db_png_index, jar_db_png_x, jar_db_png_y, "Online DB", 0xFF);

	//User Backup
	drawJar(jar_bup_png_index, jar_bup_png_x, jar_bup_png_y, "User Backup", 0xFF);

	//Options
	drawJar(jar_opt_png_index, jar_opt_png_x, jar_opt_png_y, "Settings", 0xFF);

	//About
	drawJar(jar_about_png_index, jar_about_png_x, jar_about_png_y, "About", 0xFF);

	SetFontAlign(0);
}

void LoadTexture(int cnt)
{
	pngLoadFromBuffer(menu_textures[cnt].buffer, menu_textures[cnt].size, &menu_textures[cnt].texture);

	// copy texture datas from PNG to the RSX memory allocated for textures
	if (menu_textures[cnt].texture.bmp_out)
	{
		memcpy(free_mem, menu_textures[cnt].texture.bmp_out, menu_textures[cnt].texture.pitch * menu_textures[cnt].texture.height);
		free(menu_textures[cnt].texture.bmp_out); // free the PNG because i don't need this datas
		menu_textures[cnt].texture_off = tiny3d_TextureOffset(free_mem);      // get the offset (RSX use offset instead address)
		free_mem += ((menu_textures[cnt].texture.pitch * menu_textures[cnt].texture.height + 15) & ~15) / 4; // aligned to 16 bytes (it is u32) and update the pointer
	}
}

// Used only in initialization. Allocates 64 mb for textures and loads the font
void LoadTextures_Menu()
{
	texture_mem = tiny3d_AllocTexture(64*1024*1024); // alloc 64MB of space for textures (this pointer can be global)
	
	if(!texture_mem) return; // fail!
	
	ResetFont();
	
	TTFUnloadFont();
	TTFLoadFont(0, "/dev_flash/data/font/SCE-PS3-SR-R-LATIN2.TTF", NULL, 0);
	TTFLoadFont(1, "/dev_flash/data/font/SCE-PS3-DH-R-CGB.TTF", NULL, 0);
	TTFLoadFont(2, "/dev_flash/data/font/SCE-PS3-SR-R-JPN.TTF", NULL, 0);
	TTFLoadFont(3, "/dev_flash/data/font/SCE-PS3-YG-R-KOR.TTF", NULL, 0);

	free_mem = (u32*) init_ttf_table((u16*) texture_mem);
	
	set_ttf_window(0, 0, 848 + apollo_config.marginH, 512 + apollo_config.marginV, 0);
//	TTFUnloadFont();
	
	if (!menu_textures)
		menu_textures = (png_texture *)malloc(sizeof(png_texture) * TOTAL_MENU_TEXTURES);
	
	//Init Main Menu textures
	load_menu_texture(bgimg, png);
	load_menu_texture(cheat, png);

	load_menu_texture(circle_loading_bg, png);
	load_menu_texture(circle_loading_seek, png);
	load_menu_texture(edit_shadow, png);

	load_menu_texture(footer_ico_circle, png);
	load_menu_texture(footer_ico_cross, png);
	load_menu_texture(footer_ico_lt, png);
	load_menu_texture(footer_ico_rt, png);
	load_menu_texture(footer_ico_square, png);
	load_menu_texture(footer_ico_triangle, png);
	load_menu_texture(header_dot, png);
	load_menu_texture(header_line, png);

	load_menu_texture(mark_arrow, png);
	load_menu_texture(mark_line, png);
	load_menu_texture(opt_off, png);
	load_menu_texture(opt_on, png);
	load_menu_texture(scroll_bg, png);
	load_menu_texture(scroll_lock, png);
	load_menu_texture(help, png);

	load_menu_texture(cat_about, png);
	load_menu_texture(cat_cheats, png);
	load_menu_texture(cat_opt, png);
	load_menu_texture(cat_usb, png);
	load_menu_texture(cat_bup, png);
	load_menu_texture(cat_db, png);
	load_menu_texture(cat_hdd, png);
	load_menu_texture(cat_sav, png);
	load_menu_texture(cat_warning, png);
	load_menu_texture(column_1, png);
	load_menu_texture(column_2, png);
	load_menu_texture(column_3, png);
	load_menu_texture(column_4, png);
	load_menu_texture(column_5, png);
	load_menu_texture(column_6, png);
	load_menu_texture(column_7, png);
	load_menu_texture(jar_about, png);
	load_menu_texture(jar_about_hover, png);
	load_menu_texture(jar_bup, png);
	load_menu_texture(jar_bup_hover, png);
	load_menu_texture(jar_db, png);
	load_menu_texture(jar_db_hover, png);
	load_menu_texture(jar_empty, png);
	load_menu_texture(jar_hdd, png);
	load_menu_texture(jar_hdd_hover, png);
	load_menu_texture(jar_opt, png);
	load_menu_texture(jar_opt_hover, png);
	load_menu_texture(jar_usb, png);
	load_menu_texture(jar_usb_hover, png);
	load_menu_texture(logo, png);
	load_menu_texture(tag_lock, png);
	load_menu_texture(tag_own, png);
	load_menu_texture(tag_pce, png);
	load_menu_texture(tag_ps1, png);
	load_menu_texture(tag_ps2, png);
	load_menu_texture(tag_ps3, png);
	load_menu_texture(tag_psp, png);
	load_menu_texture(tag_psv, png);
	load_menu_texture(tag_warning, png);
	load_menu_texture(tag_zip, png);
	load_menu_texture(tag_apply, png);
	load_menu_texture(tag_transfer, png);

	u32 tBytes = free_mem - texture_mem;
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

void clearcache_callback(int sel)
{
	DIR *d;
	struct dirent *dir;
	char dataPath[256];

	d = opendir(ONLINE_LOCAL_CACHE);
	if (!d)
		return;

	LOG("Cleaning folder '%s'...", ONLINE_LOCAL_CACHE);

	while ((dir = readdir(d)) != NULL)
	{
		if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0)
		{
			snprintf(dataPath, sizeof(dataPath), "%s" "%s", ONLINE_LOCAL_CACHE, dir->d_name);
			LOG("Removing %s", dataPath);
			unlink_secure(dataPath);
		}
	}
	closedir(d);
}

void up_appdata_callback(int sel)
{
	if (http_download(ONLINE_URL, "appdata.zip", ONLINE_LOCAL_CACHE "tmpdata.zip", 1))
	{
		if (extract_zip(ONLINE_LOCAL_CACHE "tmpdata.zip", APOLLO_DATA_PATH))
			show_message("Successfully updated local application data");

		unlink_secure(ONLINE_LOCAL_CACHE "tmpdata.zip");
	}
}

void update_callback(int sel)
{
    apollo_config.update = !sel;

    if (!apollo_config.update)
        return;

	LOG("checking latest Apollo version at %s", APOLLO_UPDATE_URL);

	if (http_download(APOLLO_UPDATE_URL, "", ONLINE_LOCAL_CACHE "ver.check", 0))
	{
		char *buffer;
		long size = 0;

		buffer = readFile(ONLINE_LOCAL_CACHE "ver.check", &size);

		if (!buffer)
			return;

		LOG("received %u bytes", size);
		buffer[size-1] = 0;

		static const char find[] = "\"name\":\"Apollo Save Tool v";
		const char* start = strstr(buffer, find);
		if (start != NULL)
		{
			LOG("found name");
			start += sizeof(find) - 1;

			char* end = strstr(start, "\"");
			if (end != NULL)
			{
				*end = 0;
				LOG("latest version is %s", start);

				if (stricmp(APOLLO_VERSION, start) != 0)
				{
					if (show_dialog(1, "New version available! Download update?"))
					{
						if (http_download(ONLINE_URL, "apollo-ps3.pkg", "/dev_hdd0/packages/apollo-ps3.pkg", 1))
							show_message("Update downloaded!");
						else
							show_message("Download error!");
					}
				}
			}
			else
			{
				LOG("no end of name found");
			}
		}
		else
		{
			LOG("no name found");
		}

	}
	else
	{
		LOG("http request to %s failed", APOLLO_UPDATE_URL);
	}
}

void update_usb_path(char* path)
{
	if (dir_exists(SAVES_PATH_USB0) == SUCCESS)
		strcpy(path, SAVES_PATH_USB0);
	else if (dir_exists(SAVES_PATH_USB1) == SUCCESS)
		strcpy(path, SAVES_PATH_USB1);
	else
		strcpy(path, "");
}

void update_hdd_path(char* path)
{
	sprintf(path, SAVES_PATH_HDD, apollo_config.user_id);
}

void ReloadUserSaves(save_list_t* save_list)
{
    init_loading_screen("Loading save games...");

	if (save_list->list)
	{
		UnloadGameList(save_list->list, save_list->count);
		save_list->count = 0;
		save_list->list = NULL;
	}

	if (save_list->UpdatePath)
		save_list->UpdatePath(save_list->path);

	save_list->list = save_list->ReadList(save_list->path, &(save_list->count));
	if (apollo_config.doSort)
		qsort(save_list->list, save_list->count, sizeof(save_entry_t), &qsortSaveList_Compare);

    stop_loading_screen();
}

code_entry_t* LoadSaveDetails()
{
	char sfoPath[256];
	code_entry_t* centry = calloc(1, sizeof(code_entry_t));

	snprintf(sfoPath, sizeof(sfoPath), "%s" "PARAM.SFO", selected_entry->path);
	LOG("Save Details :: Reading %s...", sfoPath);

	sfo_context_t* sfo = sfo_alloc();
	if (sfo_read(sfo, sfoPath) < 0) {
		LOG("Unable to read from '%s'", sfoPath);
		sfo_free(sfo);
		return centry;
	}

	char* subtitle = (char*) sfo_get_param_value(sfo, "SUB_TITLE");
	sfo_params_ids_t* param_ids = (sfo_params_ids_t*)(sfo_get_param_value(sfo, "PARAMS") + 0x1C);
	param_ids->user_id = ES32(param_ids->user_id);

    asprintf(&centry->name, selected_entry->title_id);
    asprintf(&centry->codes, "%s\n\n"
        "Title: %s\n"
        "Sub-Title: %s\n"
        "Lock: %s\n\n"
        "User ID: %08d\n"
        "Account ID: %s (%s)\n"
        "PSID: %016lX %016lX\n", selected_entry->path, selected_entry->name, subtitle, 
        (selected_entry->flags & SAVE_FLAG_LOCKED ? "Copying Prohibited" : "Unlocked"),
        param_ids->user_id, param_ids->account_id, 
        (selected_entry->flags & SAVE_FLAG_OWNER ? "Owner" : "Not Owner"),
		param_ids->psid[0], param_ids->psid[1]);
	LOG(centry->codes);

	sfo_free(sfo);
	return (centry);
}

void SetMenu(int id)
{   
	switch (menu_id) //Leaving menu
	{
		case MENU_MAIN_SCREEN: //Main Menu
		case MENU_USB_SAVES: //USB Saves Menu
		case MENU_HDD_SAVES: //HHD Saves Menu
		case MENU_ONLINE_DB: //Cheats Online Menu
		case MENU_USER_BACKUP: //Backup Menu
		case MENU_SETTINGS: //Options Menu
		case MENU_CREDITS: //About Menu
		case MENU_PATCHES: //Cheat Selection Menu
			break;

		case MENU_SAVE_DETAILS:
		case MENU_PATCH_VIEW: //Cheat View Menu
			if (apollo_config.doAni)
				Draw_CheatsMenu_View_Ani_Exit();
			break;

		case MENU_CODE_OPTIONS: //Cheat Option Menu
			if (apollo_config.doAni)
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
				ReloadUserSaves(&usb_saves);
			
			if (apollo_config.doAni)
				Draw_UserCheatsMenu_Ani(&usb_saves);
			break;

		case MENU_HDD_SAVES: //HDD saves Menu
			if (!hdd_saves.list)
				ReloadUserSaves(&hdd_saves);
			
			if (apollo_config.doAni)
				Draw_UserCheatsMenu_Ani(&hdd_saves);
			break;

		case MENU_ONLINE_DB: //Cheats Online Menu
			if (!online_saves.list)
				ReloadUserSaves(&online_saves);

			if (apollo_config.doAni)
				Draw_UserCheatsMenu_Ani(&online_saves);
			break;

		case MENU_CREDITS: //About Menu
			// set to display the PSID on the About menu
			sprintf(psid_str1, "%016lX", apollo_config.psid[0]);
			sprintf(psid_str2, "%016lX", apollo_config.psid[1]);
			sprintf(user_id_str, "%08d", apollo_config.user_id);
			sprintf(account_id_str, "%016lx", apollo_config.account_id);

			if (apollo_config.doAni)
				Draw_AboutMenu_Ani();
			break;

		case MENU_SETTINGS: //Options Menu
			if (apollo_config.doAni)
				Draw_OptionsMenu_Ani();
			break;

		case MENU_USER_BACKUP: //User Backup Menu
			if (!user_backup.codes)
			{
				LoadBackupCodes(&user_backup);
    			qsort(user_backup.codes, user_backup.code_count, sizeof(code_entry_t), &qsortCodeList_Compare);
			}
			selected_entry = &user_backup;
// ---------
			last_menu_id[MENU_PATCHES] = 0;

			if (apollo_config.doAni)
				Draw_CheatsMenu_Selection_Ani();
			break;

		case MENU_PATCHES: //Cheat Selection Menu
			//if entering from game list, don't keep index, otherwise keep
			if (menu_id == MENU_USB_SAVES || menu_id == MENU_HDD_SAVES || menu_id == MENU_ONLINE_DB)
				menu_old_sel[MENU_PATCHES] = 0;

			if (apollo_config.doAni && menu_id != MENU_PATCH_VIEW && menu_id != MENU_CODE_OPTIONS)
				Draw_CheatsMenu_Selection_Ani();
			break;

		case MENU_PATCH_VIEW: //Cheat View Menu
			menu_old_sel[MENU_PATCH_VIEW] = 0;
			if (apollo_config.doAni)
				Draw_CheatsMenu_View_Ani("Patch view");
			break;

		case MENU_SAVE_DETAILS: //Save Detail View Menu
    	    selected_centry = LoadSaveDetails();
			if (apollo_config.doAni)
				Draw_CheatsMenu_View_Ani(selected_entry->name);
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

void doSaveMenu(save_list_t * save_list)
{
    if(readPad(0))
    {
    	if(paddata[0].BTN_UP)
    		move_selection_back(save_list->count, 1);
    
    	else if(paddata[0].BTN_DOWN)
    		move_selection_fwd(save_list->count, 1);
    
    	else if (paddata[0].BTN_LEFT)
    		move_selection_back(save_list->count, 5);
    
    	else if (paddata[0].BTN_L1)
    		move_selection_back(save_list->count, 25);
    
    	else if (paddata[0].BTN_L2)
    		move_letter_back(save_list->list, save_list->count);
    
    	else if (paddata[0].BTN_RIGHT)
    		move_selection_fwd(save_list->count, 5);
    
    	else if (paddata[0].BTN_R1)
    		move_selection_fwd(save_list->count, 25);
    
    	else if (paddata[0].BTN_R2)
    		move_letter_fwd(save_list->list, save_list->count);
    
    	else if (paddata[0].BTN_CIRCLE)
    	{
    		SetMenu(MENU_MAIN_SCREEN);
    		return;
    	}
    	else if (paddata[0].BTN_CROSS)
    	{
    		if (!save_list->list[menu_sel].codes)
    			save_list->ReadCodes(&save_list->list[menu_sel]);
    
    		if (apollo_config.doSort)
    			qsort(save_list->list[menu_sel].codes, save_list->list[menu_sel].code_count, sizeof(code_entry_t), &qsortCodeList_Compare);
    		selected_entry = &save_list->list[menu_sel];
    		SetMenu(MENU_PATCHES);
    		return;
    	}
    	else if (paddata[0].BTN_TRIANGLE && save_list->UpdatePath)
    	{
    		selected_entry = &save_list->list[menu_sel];
    		SetMenu(MENU_SAVE_DETAILS);
    		return;
    	}
		else if (paddata[0].BTN_SQUARE)
		{
			ReloadUserSaves(save_list);
		}
	}

	Draw_UserCheatsMenu(save_list, menu_sel, 0xFF);
}

void doMainMenu()
{
	// Check the pads.
	if (readPad(0))
	{
		if(paddata[0].BTN_LEFT)
			move_selection_back(6, 1);

		else if(paddata[0].BTN_RIGHT)
			move_selection_fwd(6, 1);

		else if (paddata[0].BTN_CROSS)
		    SetMenu(menu_sel+1);

		else if(paddata[0].BTN_CIRCLE && show_dialog(1, "Exit to XMB?"))
			close_app = 1;
	}
	
	Draw_MainMenu();
}

void doAboutMenu()
{
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
}

void doOptionsMenu()
{
	// Check the pads.
	if (readPad(0))
	{
		if(paddata[0].BTN_UP)
			move_selection_back(menu_options_maxopt, 1);

		else if(paddata[0].BTN_DOWN)
			move_selection_fwd(menu_options_maxopt, 1);

		else if (paddata[0].BTN_CIRCLE)
		{
			save_app_settings(&apollo_config);
			set_ttf_window(0, 0, 848 + apollo_config.marginH, 512 + apollo_config.marginV, 0);
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
			if (menu_options[menu_sel].type == APP_OPTION_BOOL)
				menu_options[menu_sel].callback(*menu_options[menu_sel].value);

			else if (menu_options[menu_sel].type == APP_OPTION_CALL)
				menu_options[menu_sel].callback(0);
		}
	}
	
	Draw_OptionsMenu();
}

void doPatchViewMenu()
{
	//Calc max
	int max = 0;
	const char * str;

	for(str = selected_centry->codes; *str; ++str)
		max += (*str == '\n');
	//max += -((512 - (120*2))/18) + 1; //subtract the max per page
	if (max <= 0)
		max = 1;
	
	// Check the pads.
	if (readPad(0))
	{
		if(paddata[0].BTN_UP)
			move_selection_back(max, 1);

		else if(paddata[0].BTN_DOWN)
			move_selection_fwd(max, 1);

		else if (paddata[0].BTN_CIRCLE)
		{
			SetMenu(last_menu_id[MENU_PATCH_VIEW]);
			return;
		}
	}
	
	Draw_CheatsMenu_View("Patch view");
}

void downloadSave(const char* file, const char* path)
{
	if (http_download(ONLINE_URL, file, ONLINE_LOCAL_CACHE "tmpsave.zip", 1))
	{
		if (extract_zip(ONLINE_LOCAL_CACHE "tmpsave.zip", path))
			show_message("Save game successfully downloaded");
		else
			show_message("Error extracting save game!");

		unlink_secure(ONLINE_LOCAL_CACHE "tmpsave.zip");
	}
	else
		show_message("Error downloading save game!");
}

void _saveOwnerData(const char* path)
{
	FILE* f = fopen(path, "w");
	if (!f)
		return;

	fprintf(f, "%016lX %016lX\n", apollo_config.psid[0], apollo_config.psid[1]);
	fprintf(f, "%016lX\n", apollo_config.account_id);
	fprintf(f, "%08d\n", apollo_config.user_id);
	fclose(f);
}

uint32_t get_filename_id(const char* dir)
{
	char path[128];
	uint32_t tid = 0;
	
	do
	{
	    tid++;
	    snprintf(path, sizeof(path), "%s%08d.zip", dir, tid);
	}
	while (file_exists(path) == SUCCESS);

	return tid;
}

void zipSave(const char* save_path, const char* exp_path)
{
	char* export_file;
	char* tmp;
	uint32_t fid;

	if (mkdirs(exp_path) != SUCCESS)
		return;

    init_loading_screen("Exporting save game...");

	fid = get_filename_id(exp_path);
	asprintf(&export_file, "%s%08d.zip", exp_path, fid);

	asprintf(&tmp, save_path);
	*strrchr(tmp, '/') = 0;
	*strrchr(tmp, '/') = 0;

	zip_directory(tmp, save_path, export_file);

	sprintf(export_file, "%s" "saves.txt", exp_path);
	FILE* f = fopen(export_file, "a");
	if (f)
	{
		fprintf(f, "%08d.zip=[%s]%s\n", fid, selected_entry->title_id, selected_entry->name);
		fclose(f);
	}

	sprintf(export_file, "%s" "owner.txt", exp_path);
	_saveOwnerData(export_file);

	free(export_file);
	free(tmp);

    stop_loading_screen();
}

void copySave(const char* save_path, const char* exp_path)
{
	char* copy_path;
	char* tmp;

	if ((strncmp(save_path, exp_path, strlen(exp_path)) == 0) || (mkdirs(exp_path) != SUCCESS))
		return;

    init_loading_screen("Copying save game...");

	asprintf(&tmp, save_path);
	*strrchr(tmp, '/') = 0;
	asprintf(&copy_path, "%s%s/", exp_path, strrchr(tmp, '/')+1);

    LOG("Copying <%s> to %s...", save_path, copy_path);
	copy_directory(save_path, save_path, copy_path);

	free(copy_path);
	free(tmp);

    stop_loading_screen();
}

void exportLicenses(const char* exp_path)
{
	char* export_file;
	char* lic_path;
	char* tmp;

	if (mkdirs(exp_path) != SUCCESS)
		return;

    init_loading_screen("Exporting user licenses...");

	asprintf(&export_file, "%s" "licenses.zip", exp_path);
	asprintf(&lic_path, EXDATA_PATH_HDD, apollo_config.user_id);

	asprintf(&tmp, lic_path);
	*strrchr(tmp, '/') = 0;

	zip_directory(tmp, lic_path, export_file);

	sprintf(export_file, "%s" "owner.txt", exp_path);
	_saveOwnerData(export_file);

	free(export_file);
	free(lic_path);
	free(tmp);

    stop_loading_screen();
}

void exportSaves(const char* exp_path)
{
	char* save_path;

	if (mkdirs(exp_path) != SUCCESS)
		return;

    init_loading_screen("Transfering all saves...");

	asprintf(&save_path, SAVES_PATH_HDD, apollo_config.user_id);

    LOG("Copying <%s> to %s...", save_path, exp_path);
	copy_directory(save_path, save_path, exp_path);

	free(save_path);

    stop_loading_screen();
}

void doCodeOptionsMenu()
{
    code_entry_t* code = &selected_entry->codes[menu_old_sel[last_menu_id[MENU_CODE_OPTIONS]]];
	// Check the pads.
	if (readPad(0))
	{
		if(paddata[0].BTN_UP)
			move_selection_back(selected_centry->options[option_index].size, 1);

		else if(paddata[0].BTN_DOWN)
			move_selection_fwd(selected_centry->options[option_index].size, 1);

		else if (paddata[0].BTN_CIRCLE)
		{
			code->activated = 0;
			SetMenu(last_menu_id[MENU_CODE_OPTIONS]);
			return;
		}
		else if (paddata[0].BTN_CROSS)
		{
			code->options[option_index].sel = menu_sel;
			const char* codecmd = code->options[option_index].value[menu_sel];

			if (strncmp(codecmd, CMD_DOWNLOAD_USB, 10) == 0)
			{
				downloadSave(code->codes, codecmd[10] ? SAVES_PATH_USB1 : SAVES_PATH_USB0);
        		code->activated = 0;
			}

			if (strncmp(codecmd, CMD_EXPORT_ZIP_USB, 10) == 0)
			{
				zipSave(selected_entry->path, codecmd[10] ? EXPORT_PATH_USB1 : EXPORT_PATH_USB0);
        		code->activated = 0;
			}

			if (strncmp(codecmd, CMD_COPY_SAVE_USB, 10) == 0)
			{
				copySave(selected_entry->path, codecmd[10] ? SAVES_PATH_USB1 : SAVES_PATH_USB0);
        		code->activated = 0;
			}

			if (strncmp(codecmd, CMD_EXP_EXDATA_USB, 10) == 0)
			{
				exportLicenses(codecmd[10] ? EXPORT_PATH_USB1 : EXPORT_PATH_USB0);
        		code->activated = 0;
			}

			if (strncmp(codecmd, CMD_EXP_TROPHY_USB, 10) == 0)
			{
//				exportLicenses(codecmd[10] ? EXPORT_PATH_USB1 : EXPORT_PATH_USB0);
        		code->activated = 0;
			}

			if (strncmp(codecmd, CMD_EXP_SAVES_USB, 10) == 0)
			{
				exportSaves(codecmd[10] ? SAVES_PATH_USB1 : SAVES_PATH_USB0);
        		code->activated = 0;
			}

			option_index++;
			
			if (option_index >= code->options_count)
			{
				SetMenu(last_menu_id[MENU_CODE_OPTIONS]);
				return;
			}
			else
				menu_sel = 0;
		}
	}
	
	Draw_CheatsMenu_Options();
}

void doSaveDetailsMenu()
{
	// Check the pads.
	if (readPad(0))
	{
		if(paddata[0].BTN_UP)
		{
			move_selection_back(9, 1);
		}
		else if(paddata[0].BTN_DOWN)
		{
			move_selection_fwd(9, 1);
		}
		if (paddata[0].BTN_CIRCLE)
		{
			if (selected_centry->name)
				free(selected_centry->name);
			if (selected_centry->codes)
				free(selected_centry->codes);
			free(selected_centry);

			SetMenu(last_menu_id[MENU_SAVE_DETAILS]);
			return;
		}
	}
	
	Draw_CheatsMenu_View(selected_entry->name);
}

void build_sfo_patch(sfo_patch_t* patch)
{
    int j;

	for (j = 0; j < selected_entry->code_count; j++)
	{
		if (!selected_entry->codes[j].activated || selected_entry->codes[j].type != PATCH_SFO)
		    continue;
		    
    	LOG("Active: [%s]", selected_entry->codes[j].name);

		if (strcmp(selected_entry->codes[j].codes, CMD_UNLOCK_COPY) == 0)
		    patch->flags = SFO_PATCH_FLAG_REMOVE_COPY_PROTECTION;

		if (strcmp(selected_entry->codes[j].codes, CMD_REMOVE_ACCOUNT_ID) == 0)
		    bzero(patch->account_id, SFO_ACCOUNT_ID_SIZE);

		if (strcmp(selected_entry->codes[j].codes, CMD_REMOVE_PSID) == 0)
		{
		    bzero(psid_str1, SFO_PSID_SIZE);
			patch->psid = (u8*) psid_str1;
		}

		selected_entry->codes[j].activated = 0;
	}
}

int _is_decrypted(list_t* list, const char* fname) {
	list_node_t *node = list->head;

	while (node) {
		if (strcmp(list_get(node), fname) == 0)
			return 1;

		node = node->next;
	}

	return 0;
}

int apply_cheat_patch()
{
    int j;
	char tmpfile[256];
	char* filename;
	code_entry_t* code;
	savedata_file_t save_file;
	list_t* decrypted_files = list_alloc();

    init_loading_screen("Applying cheats...");

	for (j = 0; j < selected_entry->code_count; j++)
	{
		code = &selected_entry->codes[j];

		if (!code->activated || code->type != PATCH_GAMEGENIE)
		    continue;

    	LOG("Code Active: [%s]", code->name);

		if (strrchr(code->file, '\\'))
			filename = strrchr(code->file, '\\')+1;
		else
			filename = code->file;

		snprintf(tmpfile, sizeof(tmpfile), ONLINE_LOCAL_CACHE "%s", filename);

		if (!_is_decrypted(decrypted_files, filename))
		{
			LOG("Decrypting '%s'...", filename);

			strcpy(save_file.filename, filename);
			strcpy(save_file.folder, selected_entry->folder);
			*strrchr(save_file.folder, '/') = 0;
			save_file.protected_file_id = get_secure_file_id(selected_entry->title_id, filename);

			if (load_game_file(&save_file))
			{
				write_buffer(tmpfile, save_file.data, save_file.size);
				list_append(decrypted_files, filename);

				free(save_file.data);
			}
			else
			{
				LOG("Error: failed to decrypt (%s)", filename);
				continue;
			}

		}

		if (!apply_ggenie_patch_code(tmpfile, code))
		{
			LOG("Error: failed to apply (%s)", code->name);
		}

		code->activated = 0;
	}

	list_node_t *node = list_head(decrypted_files);

	while (node) {
		filename = list_get(node);
		snprintf(tmpfile, sizeof(tmpfile), ONLINE_LOCAL_CACHE "%s", filename);

		LOG("Encrypting '%s'...", tmpfile);

		strcpy(save_file.filename, filename);
		strcpy(save_file.folder, selected_entry->folder);
		*strrchr(save_file.folder, '/') = 0;
		save_file.protected_file_id = get_secure_file_id(selected_entry->title_id, filename);
		
		read_buffer(tmpfile, &save_file.data, &save_file.size);

		if (save_game_file(&save_file))
		{
			free(save_file.data);
		}
		else
		{
			LOG("Error: failed to encrypt files.");
		}

		unlink_secure(tmpfile);
		node = node->next;
	}

	list_free(decrypted_files);
	stop_loading_screen();

	return 1;
}

void doPatchMenu()
{
	// Check the pads.
	if (readPad(0))
	{
		if(paddata[0].BTN_UP)
			move_selection_back(selected_entry->code_count, 1);

		else if(paddata[0].BTN_DOWN)
			move_selection_fwd(selected_entry->code_count, 1);

		else if (paddata[0].BTN_LEFT)
			move_selection_back(selected_entry->code_count, 5);

		else if (paddata[0].BTN_RIGHT)
			move_selection_fwd(selected_entry->code_count, 5);

		else if (paddata[0].BTN_CIRCLE)
		{
			for (int j = 0; j < selected_entry->code_count; j++)
				selected_entry->codes[j].activated = 0;

			SetMenu(last_menu_id[MENU_PATCHES]);
			return;
		}
		else if (paddata[0].BTN_CROSS)
		{
			selected_entry->codes[menu_sel].activated = !selected_entry->codes[menu_sel].activated;

			if (strcmp(selected_entry->codes[menu_sel].codes, CMD_RESIGN_SAVE) == 0)
			{
				char in_file_path[256];
				sfo_patch_t patch = {
			        .flags = 0,
			        .user_id = apollo_config.user_id,
			        .account_id = NULL,
			        .psid = (u8*) &(apollo_config.psid[0]),
				};

				asprintf(&patch.account_id, "%016lx", apollo_config.account_id);
				build_sfo_patch(&patch);

				snprintf(in_file_path, sizeof(in_file_path), "%s" "PARAM.SFO", selected_entry->path);
				LOG("Applying SFO patches '%s'...", in_file_path);

                if (patch_sfo(in_file_path, &patch) == SUCCESS)
	            {
					LOG("Resigning save '%s'...", selected_entry->name);
					if (pfd_util_init(selected_entry->title_id, selected_entry->path))
					{
						if (pfd_util_process(PFD_CMD_UPDATE, 1) == SUCCESS)
		                    show_message("Save file successfully resigned!");
    	                else
	                        show_message("Error! Save file couldn't be resigned");

//						pfd_util_process(PFD_CMD_CHECK, 0);
					}
                    else
                    {
	                    show_message("Error! Save file couldn't be resigned");
                    }

					pfd_util_end();
				}

				if(patch.account_id)
				    free(patch.account_id);

				selected_entry->codes[menu_sel].activated = 0;
			}

			if (strcmp(selected_entry->codes[menu_sel].codes, CMD_APPLY_CHEATS) == 0)
			{
				LOG("Applying cheats to '%s'...", selected_entry->name);

				if (apply_cheat_patch())
					show_message("Cheat codes successfully applied!");
				else
					show_message("Error! Cheat codes couldn't be applied");

				selected_entry->codes[menu_sel].activated = 0;
			}

			if (selected_entry->codes[menu_sel].activated)
			{
				//Check if option code
				if (!selected_entry->codes[menu_sel].options)
				{
					int size;
					selected_entry->codes[menu_sel].options = ReadOptions(selected_entry->codes[menu_sel], &size);
					selected_entry->codes[menu_sel].options_count = size;
				}
				
				if (selected_entry->codes[menu_sel].options)
				{
					selected_centry = &selected_entry->codes[menu_sel];
					option_index = 0;
					SetMenu(MENU_CODE_OPTIONS);
					return;
				}
			}
		}
		else if (paddata[0].BTN_TRIANGLE)
		{
			selected_centry = &selected_entry->codes[menu_sel];

			if (selected_centry->type == PATCH_GAMEGENIE || selected_centry->type == PATCH_BSD)
			{
				SetMenu(MENU_PATCH_VIEW);
				return;
			}
		}
	}
	
	Draw_CheatsMenu_Selection(menu_sel, 0xFFFFFFFF);
}

void doUserBackupMenu()
{
	doPatchMenu();
}

// Resets new frame
void drawScene()
{
	switch (menu_id)
	{
		case MENU_MAIN_SCREEN:
			doMainMenu();
			break;

		case MENU_USB_SAVES: //USB Saves Menu
			doSaveMenu(&usb_saves);
			break;

		case MENU_HDD_SAVES: //HDD Saves Menu
			doSaveMenu(&hdd_saves);
			break;

		case MENU_ONLINE_DB: //Online Cheats Menu
			doSaveMenu(&online_saves);
			break;

		case MENU_CREDITS: //About Menu
			doAboutMenu();
			break;

		case MENU_SETTINGS: //Options Menu
			doOptionsMenu();
			break;

		case MENU_USER_BACKUP: //User Backup Menu
			doUserBackupMenu();
			break;

		case MENU_PATCHES: //Cheats Selection Menu
			doPatchMenu();
			break;

		case MENU_PATCH_VIEW: //Cheat View Menu
			doPatchViewMenu();
			break;

		case MENU_CODE_OPTIONS: //Cheat Option Menu
			doCodeOptionsMenu();
			break;

		case MENU_SAVE_DETAILS: //Save Details Menu
			doSaveDetailsMenu();
			break;
	}
}

void exiting()
{
	http_end();
	sysModuleUnload(SYSMODULE_PNGDEC);
}

void registerSpecialChars()
{
	// Register save tags
	RegisterSpecialCharacter(CHAR_TAG_PS1, 2, 1.5, &menu_textures[tag_ps1_png_index]);
	RegisterSpecialCharacter(CHAR_TAG_PS2, 2, 1.5, &menu_textures[tag_ps2_png_index]);
	RegisterSpecialCharacter(CHAR_TAG_PS3, 2, 1.5, &menu_textures[tag_ps3_png_index]);
	RegisterSpecialCharacter(CHAR_TAG_PSP, 2, 1.5, &menu_textures[tag_psp_png_index]);
	RegisterSpecialCharacter(CHAR_TAG_PSV, 2, 1.5, &menu_textures[tag_psv_png_index]);
	RegisterSpecialCharacter(CHAR_TAG_PCE, 2, 1.5, &menu_textures[tag_pce_png_index]);
	RegisterSpecialCharacter(CHAR_TAG_LOCKED, 0, 1.5, &menu_textures[tag_lock_png_index]);
	RegisterSpecialCharacter(CHAR_TAG_OWNER, 0, 1.5, &menu_textures[tag_own_png_index]);
	RegisterSpecialCharacter(CHAR_TAG_WARNING, 0, 1.5, &menu_textures[tag_warning_png_index]);
	RegisterSpecialCharacter(CHAR_TAG_APPLY, 0, 1.0, &menu_textures[tag_apply_png_index]);
	RegisterSpecialCharacter(CHAR_TAG_ZIP, 0, 1.2, &menu_textures[tag_zip_png_index]);
	RegisterSpecialCharacter(CHAR_TAG_TRANSFER, 0, 1.2, &menu_textures[tag_transfer_png_index]);

	// Register button icons
	RegisterSpecialCharacter(CHAR_BTN_X, 0, 1.2, &menu_textures[footer_ico_cross_png_index]);
	RegisterSpecialCharacter(CHAR_BTN_S, 0, 1.2, &menu_textures[footer_ico_square_png_index]);
	RegisterSpecialCharacter(CHAR_BTN_T, 0, 1.2, &menu_textures[footer_ico_triangle_png_index]);
	RegisterSpecialCharacter(CHAR_BTN_O, 0, 1.2, &menu_textures[footer_ico_circle_png_index]);
}

/*
	Program start
*/
s32 main(s32 argc, const char* argv[])
{
	dbglogger_init();

	http_init();

	load_app_settings(&apollo_config);

	pfd_util_setup_keys((u8*) &(apollo_config.psid[0]), apollo_config.user_id);

	tiny3d_Init(1024*1024);

	ioPadInit(7);
	
	sysModuleLoad(SYSMODULE_PNGDEC);

	atexit(exiting); // Tiny3D register the event 3 and do exit() call when you exit  to the menu

	// register exit callback
	if(sysUtilRegisterCallback(SYSUTIL_EVENT_SLOT0, sys_callback, NULL)==0) inited |= INITED_CALLBACK;
	
	// Load texture
	LoadTextures_Menu();
	LoadSounds();
	
	// Setup font
	SetExtraSpace(5);
	SetCurrentFont(0);

	registerSpecialChars();

	menu_options_maxopt = 0;
	while (menu_options[menu_options_maxopt].name)
		menu_options_maxopt++;
	
	menu_options_maxsel = (int *)calloc(1, menu_options_maxopt * sizeof(int));
	
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
	
	SND_SetInfiniteVoice(2, (effect_is_stereo) ? VOICE_STEREO_16BIT : VOICE_MONO_16BIT, effect_freq, 0, background_music, background_music_size, 255, 255);
	
	//Set options
	music_callback(!apollo_config.music);
	update_callback(!apollo_config.update);

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
		
		// change to 2D context (remember you it works with 848 x 512 as virtual coordinates)
		tiny3d_Project2D();

		drawScene();

#ifdef APOLLO_ENABLE_LOGGING
		if(paddata[0].BTN_SELECT)
		{
			LOG("Screen");
			dbglogger_screenshot_tmp(0);
			LOG("Shot");
		}
#endif

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

	}
	
	return 0;
}
