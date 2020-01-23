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

//Options
const char * options_path = APOLLO_PATH "opt.ini";

void music_callback(int index, int sel);
void sort_callback(int index, int sel);
void ani_callback(int index, int sel);
void horm_callback(int index, int sel);
void verm_callback(int index, int sel);
void update_callback(int index, int sel);

const option menu_options_options[] = {
	{ .name = "Music", .options = NULL, .type = ARTEMIS_OPTION_BOOL, .callback = music_callback },
	{ .name = "Sort Saves", .options = NULL, .type = ARTEMIS_OPTION_BOOL, .callback = sort_callback },
	{ .name = "Menu Animations", .options = NULL, .type = ARTEMIS_OPTION_BOOL, .callback = ani_callback },
	{ .name = "Horizontal Margin", .options = NULL, .type = ARTEMIS_OPTION_INC, .callback = horm_callback },
	{ .name = "Vertical Margin", .options = NULL, .type = ARTEMIS_OPTION_INC, .callback = verm_callback },
	{ .name = "Update Local Cheats", .options = NULL, .type = ARTEMIS_OPTION_CALL, .callback = update_callback },
	{ .name = NULL }
};

int doSort = 1;
int doAni = 1;
int marginHorizontal = 0;
int marginVertical = 0;

int menu_options_maxopt = 0;
int * menu_options_maxsel;
int * menu_options_selections;

const char * VERSION = "v0.5.0";            //Apollo PS3 version (about menu)
const int MENU_TITLE_OFF = 30;              //Offset of menu title text from menu mini icon
const int MENU_ICON_OFF = 70;               //X Offset to start printing menu mini icon
const int MENU_ANI_MAX = 0x80;              //Max animation number
const int MENU_MAIN_ICON_WIDTH = 80;		//Width of main menu icons
const int MENU_SPLIT_OFF = 200;				//Offset from left of sub/split menu to start drawing

int close_art = 0;

png_texture * menu_textures;           // png_texture array for main menu, initialized in LoadTexture
const int menu_size = 34;              // Size of menu png_texture array

int screen_width = 0, screen_height = 0;    // Set to dimensions of the screen in main()

int highlight_alpha = 128;                  // Alpha of the selected
int highlight_pulse = 1;                    // Whether the increment the highlight
int highlight_amount = 6;                   // Amount of alpha to inc/dec each time
int pause_pulse = 0;                        // Counter that holds how long alpha is held in place
int idle_time = 0;                          // Set by readPad

const char * menu_main_description = "PlayStation 3 Save Game Tool";

const char * menu_about_strings[] = { "Bucanero", "Developer",
									"Dnawrkshp", "GUI code",
									"Berion", "GUI design",
									"flatz", "PFD/SFO tools",
									NULL, NULL };

const char * menu_about_strings_project[] = { "WebSite", "http://apollo.psdev.tk/",
											"GitHub", "github.com/bucanero/apollo-ps3",
											NULL, NULL };

//Game filtering
#define GAMES_MOUNT_PATH		"/dev_hdd0/GAMES/"
#define GAMES_PSN_PATH			"/dev_hdd0/game/"
#define GAMES_DISC_PATH			"/app_home/PS3_GAME/PARAM.SFO"

char * * user_installed_titleids;
int user_installed_titleids_count = 0;

/*
* 0 - Main Menu
* 1 - Code Menu (User List)
* 2 - Code Menu (Online List)
* 3 - About Menu
* 4 - Options Menu
* 5 - Code Menu (Select Cheats)
* 6 - Code Menu (View Cheat)
* 7 - Code Menu (View Cheat Options)
*/
int menu_id = 0;																					// Menu currently in
int menu_sel = 0;																					// Index of selected item (use varies per menu)
int menu_old_sel[] = { 0, 0, 0, 0, 0, 0, 0, 0 };													// Previous menu_sel for each menu
int last_menu_id[] = { 0, 0, 0, 0, 0, 0, 0, 0 };													// Last menu id called (for returning)
const char * menu_pad_help[] = { NULL,																//Main
								"\x10 Select    \x13 Back    \x11 Refresh",							//User list   
								"\x10 Select    \x13 Back    \x11 Refresh",							//Online list
								"\x13 Back",														//About
								"\x10 Select    \x13 Back",											//Options
								"\x10 Enable    \x11 Toggle Mode    \x12 View Code    \x13 Back",	//Select Cheats
								"\x13 Back",														//View Cheat
								"\x10 Select    \x13 Back"											//Cheat Option
								};

/*
* User code list
*/
save_entry_t * user_game_list = NULL;
int user_game_count = 0;

/*
* Online code list
*/
save_entry_t * online_game_list = NULL;
int online_game_count = 0;

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
		//tiny3d_Clear(0xff000000, TINY3D_CLEAR_ALL);
		//SetFontSize(12, 24);
		//SetFontColor(0xffffffff, 0x00000000);
		//DrawFormatString(0, 0, "Destroying SPU... ");
		//tiny3d_Flip();
		//sleep(1);
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

char ** LoadGames_ReadDirectory(char * path, const char * param, int * ret_count)
{
	DIR *d;
	struct dirent *dir;

	char fullPath[1024];

	char * * files = (char**)malloc(1000 * sizeof(char*));

	int count = 0;

	if ((d = opendir(path)))
	{
		while ((dir = readdir(d)) != NULL && dir->d_name != NULL)
		{
			if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0)
			{
				snprintf(fullPath, sizeof(fullPath)-1, "%s%s%s", path, dir->d_name, param);
				if (file_exists(fullPath) == SUCCESS)
				{
					files[count] = (char*)malloc(strlen(fullPath));
					strcpy(files[count], (char*)fullPath);
					count++;
				}
			}
		}
		closedir(d);
	}

	*ret_count = count;
	return files;
}

void LoadGames()
{
	int id_count[3];
	char ** id_path[3];
	int count = 0, x = 0, y = 0;

	//Free old array if exists
	if (user_installed_titleids)
	{
		for (x = 0; x < user_installed_titleids_count; x++)
			if (user_installed_titleids[x])
				free(user_installed_titleids);

		free(user_installed_titleids);
	}
	user_installed_titleids_count = 0;
	
	id_path[0] = LoadGames_ReadDirectory(GAMES_MOUNT_PATH, "/PS3_GAME/PARAM.SFO", &id_count[0]);
	id_path[1] = LoadGames_ReadDirectory(GAMES_PSN_PATH, "/PARAM.SFO", &id_count[1]);
	id_path[2] = (char **)malloc(1 * sizeof(char*));
	id_path[2][0] = (char*)malloc(512);
	strcpy(id_path[2][0], GAMES_DISC_PATH);
	if (file_exists(id_path[2][0]) == SUCCESS)
		id_count[2] = 1;
	else
	{
		id_count[2] = 0;
		free(id_path[2][0]);
		id_path[2][0] = NULL;
	}

	count = id_count[0] + id_count[1] + id_count[2];
	if (count > 0)
	{
		user_installed_titleids = (char **)malloc(count * sizeof(char*));
		for (x = 0; x < 3; x++)
		{
			if (id_path[x])
			{
				for (y = 0; y < id_count[x]; y++)
				{
					if (id_path[x][y])
					{
						user_installed_titleids[user_installed_titleids_count] = (char*)malloc(10);

						int fSize = getFileSize(id_path[x][y]);
						char * fullFile = readFile(id_path[x][y]);
						strcpy((char*)user_installed_titleids[user_installed_titleids_count], (char*)&fullFile[fSize - 0x18]);
						free(fullFile);
						fullFile = NULL;
						user_installed_titleids_count++;

						free(id_path[x][y]);
					}
				}
				free(id_path[x]);
			}
		}
	}

}

char * ParseOptionName(char * buffer, char * ret)
{
	strcpy(ret, buffer);
	int i = 0;
	for (i = 0; i < strlen(ret); i++)
		if (ret[i] == ' ')
			ret[i] = '_';
	
	return ret;
}

void LoadOptions()
{
	int fsize = getFileSize(options_path);
	if (fsize > 0)
	{
		FILE *f = fopen(options_path, "rb");
		if (f == NULL)
			return;
		fseek(f, 0, SEEK_SET);
		
		char * name = malloc(256);
		int val = 0, i = 0, r = 0, line = 0;
		
		r = fscanf(f, " %s %d\n", name, &val);
		while (r != EOF)
		{
			line++;
			for (i = 0; i < menu_options_maxopt; i++)
			{
				char tName[strlen(menu_options_options[i].name) + 1];
				if (strcmp(ParseOptionName(menu_options_options[i].name, (char *)tName), name) == 0)
				{
					LOG("LoadOptions() :: %s = %d", name, val);
					menu_options_selections[i] = val;
					break;
				}
			}
			
			r = fscanf(f, " %s %d\n", name, &val);
		}
		
		free(name);
		fclose(f);
	}
}

void SaveOptions()
{
	FILE * fp = fopen(options_path, "w");
	if(fp == NULL)
		return;
	
	int i = 0;
	for (i = 0; i < menu_options_maxopt; i++)
	{
		char tName[strlen(menu_options_options[i].name) + 1];
		fprintf(fp, "%s %d\n", ParseOptionName(menu_options_options[i].name, (char *)tName), menu_options_selections[i]);
	}
	
	fclose(fp);
}

int isArtemisLoaded()
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
			snprintf(fullPath, sizeof(fullPath)-1, "%s%s%s", "/dev_hdd0/home/", dir->d_name, "/etc/boot_history.dat");
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
		
		//Artemis logo
		w = 500, h = 140;
		DrawTexture(menu_textures[titlescr_logo_png_index], 424 - (w / 2), 256 - (h / 2) - 100, 0, w, h, 0xFFFFFF00 | logo_a);
		
		//Artemis description
		SetFontAlign(1);
		SetCurrentFont(font_comfortaa_light);
		SetFontSize(18, 16);
		SetFontColor(0x00000000 | logo_a, 0x00000000);
		DrawString(screen_width / 2, 210, (char*)menu_main_description);
		
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
		
		//Artemis logo
		w = 500; h = 140;
		DrawTexture(menu_textures[titlescr_logo_png_index], 424 - (w / 2), 256 - (h / 2) - 100, 0, w, h, 0xFFFFFFFF);

		//Artemis description
		SetFontAlign(1);
		SetCurrentFont(font_comfortaa_light);
		SetFontSize(18, 16);
		SetFontColor(0x000000FF, 0x00000000);
		DrawString(screen_width / 2, 210, (char*)menu_main_description);

		SetFontSize(12, 12);
		DrawString(screen_width / 2, 480, "apollo.psdev.tk");
		
		DrawTextureRotated(menu_textures[circle_loading_seek_png_index], screen_width / 2, screen_height / 2, 1, 89, 89, 0xFFFFFFFF, ani);
		//------------ Info
		//c = 11;
		//w = (2 * menu_textures[c].texture.width) / 4; h = (2 * menu_textures[c].texture.height) / 4;
		//DrawTexture(menu_textures[c], 424 - (w / 2), 450, 0, w, h, 0xffffff00 | icon_a);
		
		//------------ Icons
		
		//Start game
		DrawTexture(menu_textures[titlescr_ico_xmb_png_index], 50 + 150, 320, 0, MENU_MAIN_ICON_WIDTH, 64, 0xffffff00 | icon_a);
		
		//Cheats
		DrawTexture(menu_textures[titlescr_ico_cht_png_index], 150 + 150, 320, 0, MENU_MAIN_ICON_WIDTH, 64, 0xffffff00 | icon_a);

		//Online cheats
		DrawTexture(menu_textures[titlescr_ico_net_png_index], 250 + 150, 320, 0, MENU_MAIN_ICON_WIDTH, 64, 0xffffff00 | icon_a);
		
		//Options
		DrawTexture(menu_textures[titlescr_ico_opt_png_index], 350 + 150 + 5, 320, 0, MENU_MAIN_ICON_WIDTH, 64, 0xffffff00 | icon_a);
		
		//About
		DrawTexture(menu_textures[titlescr_ico_abt_png_index], 450 + 150, 320, 0, MENU_MAIN_ICON_WIDTH, 64, 0xffffff00 | icon_a);
		
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
	
	//Artemis logo
	c = titlescr_logo_png_index;
	w = 500; h = 140;
	DrawTexture(menu_textures[c], 424 - (w / 2), 256 - (h / 2) - 100, 0, w, h, 0xffffffff);
	
	//Artemis description
	SetFontAlign(1);
	SetCurrentFont(font_comfortaa_light);
	SetFontSize(18, 16);
	SetFontColor(0x000000FF, 0x00000000);
	DrawString(screen_width / 2, 210, (char*)menu_main_description);

	SetFontSize(12, 12);
	DrawString(screen_width / 2, 480, "apollo.psdev.tk");

	//------------ Info
	//c = 11;
	//w = (2 * menu_textures[c].texture.width) / 4; h = (2 * menu_textures[c].texture.height) / 4;
	//DrawTexture(menu_textures[c], 424 - (w / 2), 450, 0, w, h, 0xffffffff);
	
	//------------ Icons
	SetFontAlign(3);
	SetFontSize(12, 12);
	SetCurrentFont(font_comfortaa_regular);

	//Start game
	c = titlescr_ico_xmb_png_index;
	DrawTexture(menu_textures[c], 50 + 150, 320, 0, MENU_MAIN_ICON_WIDTH, 64, 0xffffff00 | ((menu_sel == 0) ? 0xFF : 32));
	SetFontColor(0x00000000 | ((menu_sel == 0) ? 0xFF : 32), 0x00000000);
	DrawString(50 + 150 + (MENU_MAIN_ICON_WIDTH / 2), 390, "Start game");

	//Cheats
	c = titlescr_ico_cht_png_index;
	DrawTexture(menu_textures[c], 150 + 150, 320, 0, MENU_MAIN_ICON_WIDTH, 64, 0xffffff00 | ((menu_sel == 1) ? 0xFF : 32));
	SetFontColor(0x00000000 | ((menu_sel == 1) ? 0xFF : 32), 0x00000000);
	DrawString(150 + 150 + (MENU_MAIN_ICON_WIDTH / 2) + 5, 390, "Saves");

	//Online Cheats
	c = titlescr_ico_net_png_index;
	DrawTexture(menu_textures[c], 250 + 150, 320, 0, MENU_MAIN_ICON_WIDTH, 64, 0xffffff00 | ((menu_sel == 2) ? 0xFF : 32));
	SetFontColor(0x00000000 | ((menu_sel == 2) ? 0xFF : 32), 0x00000000);
	DrawString(250 + 150 + (MENU_MAIN_ICON_WIDTH / 2) + 5, 390, "Online DB");

	//Options
	c = titlescr_ico_opt_png_index;
	DrawTexture(menu_textures[c], 350 + 150 + 5, 320, 0, MENU_MAIN_ICON_WIDTH, 64, 0xffffff00 | ((menu_sel == 3) ? 0xFF : 32));
	SetFontColor(0x00000000 | ((menu_sel == 3) ? 0xFF : 32), 0x00000000);
	DrawString(350 + 150 + (MENU_MAIN_ICON_WIDTH / 2) + 14, 390, "Options");

	//About
	c = titlescr_ico_abt_png_index;
	DrawTexture(menu_textures[c], 450 + 150, 320, 0, MENU_MAIN_ICON_WIDTH, 64, 0xffffff00 | ((menu_sel == 4) ? 0xFF : 32));
	SetFontColor(0x00000000 | ((menu_sel == 4) ? 0xFF : 32), 0x00000000);
	DrawString(450 + 150 + (MENU_MAIN_ICON_WIDTH / 2), 390, "About");

	SetFontAlign(0);

}

// Used only in initialization. Allocates 64 mb for textures and loads the font
void LoadTexture()
{
	texture_mem = tiny3d_AllocTexture(64*1024*1024); // alloc 64MB of space for textures (this pointer can be global)
	
	u32 * texture_pointer; // use to asign texture space without changes texture_mem

	if(!texture_mem) return; // fail!

	texture_pointer = texture_mem;
	
	ResetFont();
	
	TTFLoadFont(NULL, (void *)comfortaa_regular_ttf, comfortaa_regular_ttf_size);
	texture_pointer = (u32 *) AddFontFromTTF((u8 *) texture_pointer, 32, 255, 32, 32, TTF_to_Bitmap);
	TTFLoadFont(NULL, (void *)comfortaa_bold_ttf, comfortaa_bold_ttf_size);
	texture_pointer = (u32 *) AddFontFromTTF((u8 *) texture_pointer, 32, 255, 32, 32, TTF_to_Bitmap);
	TTFLoadFont(NULL, (void *)comfortaa_light_ttf, comfortaa_light_ttf_size);
	texture_pointer = (u32 *) AddFontFromTTF((u8 *) texture_pointer, 32, 255, 32, 32, TTF_to_Bitmap);
	
	font_mem = texture_pointer;
	
	TTFUnloadFont();
	
	if (!menu_textures && menu_size)
		menu_textures = (png_texture *)malloc(sizeof(png_texture) * menu_size);
	
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
	u32 * texture_pointer; // use to assign texture space without changes texture_mem

	if (!font_mem) return; // fail!

	texture_pointer = font_mem;

	int cnt = 0;

	//Main Menu
	for (cnt = 0; cnt < menu_size; cnt++)
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

		background_music =  temp;
	}
	
	SND_Pause(0);
}

void music_callback(int index, int sel)
{
	switch (sel)
	{
		case 0: //on
			SND_PauseVoice(2, 0);
			break;
		case 1: //off
			SND_PauseVoice(2, 1);
	}
}

void sort_callback(int index, int sel)
{
	doSort = !sel;
}

void ani_callback(int index, int sel)
{
	doAni = !sel;
}

void horm_callback(int index, int sel)
{
	if (sel < 0)
		sel = 0;
	if (sel > 100)
		sel = 100;
	marginHorizontal = sel;
	menu_options_selections[index] = sel;
}

void verm_callback(int index, int sel)
{
	if (sel < 0)
		sel = 0;
	if (sel > 100)
		sel = 100;
	marginVertical = sel;
	menu_options_selections[index] = sel;
}

void update_callback(int index, int sel)
{
	if (sel)
	{
		if (http_download(ONLINE_URL, "cheatdb.zip", ONLINE_LOCAL_CACHE "tmp.zip", 1))
		{
			if (extract_zip(ONLINE_LOCAL_CACHE "tmp.zip", USERLIST_PATH_HDD))
				show_dialog(0, "Successfully updated local cheat database");

			unlink_secure(ONLINE_LOCAL_CACHE "tmp.zip");
		}
		menu_options_selections[index] = 0;
	}
}

void ReloadUserCheats()
{
	if (user_game_list)
	{
		UnloadGameList(user_game_list, user_game_count);
		user_game_count = 0;
		user_game_list = NULL;
	}
	
	int gmc[1];
	user_game_list = ReadUserList((int *)gmc);
	user_game_count = *gmc;
	if (doSort)
		BubbleSortGameList(user_game_list, user_game_count);
}

void ReloadOnlineCheats()
{
	if (online_game_list)
	{
		UnloadGameList(online_game_list, online_game_count);
		online_game_count = 0;
		online_game_list = NULL;
	}

	int gmc[1];
	online_game_list = ReadOnlineList((int *)gmc);
	online_game_count = *gmc;
	if (doSort)
		BubbleSortGameList(online_game_list, online_game_count);
}

void SetMenu(int id)
{   
	switch (menu_id) //Leaving menu
	{
		case 0: //Main Menu
			
			break;
		case 1: //Cheats Offline Menu
			
			break;
		case 2: //Cheats Online Menu
			
			break;
		case 3: //About Menu
			
			break;
		case 4: //Options Menu
			
			break;
		case 5: //Cheat Selection Menu
			
			break;
		case 6: //Cheat View Menu
			Draw_CheatsMenu_View_Ani_Exit();
			break;
		case 7: //Cheat Option Menu
			Draw_CheatsMenu_Options_Ani_Exit();
			break;
	}
	
	switch (id) //going to menu
	{
		case 0: //Main Menu
			if (doAni || menu_id == 0) //if load animation
				Draw_MainMenu_Ani();
			break;
		case 1: //Cheats Offline Menu
			if (!user_game_list)
			{
				int gmc[1];
				user_game_list = ReadUserList((int *)gmc);
				user_game_count = *gmc;
				if (doSort)
					BubbleSortGameList(user_game_list, user_game_count);
			}
			
			if (doAni)
				Draw_UserCheatsMenu_Ani(user_game_list, user_game_count);
			break;
		case 2: //Cheats Online Menu
			if (!online_game_list)
			{
				int gmc[1];
				online_game_list = ReadOnlineList((int *)gmc);
				online_game_count = *gmc;
				if (doSort)
					BubbleSortGameList(online_game_list, online_game_count);
			}

			if (doAni)
				Draw_UserCheatsMenu_Ani(online_game_list, online_game_count);
			break;
		case 3: //About Menu
			if (doAni)
				Draw_AboutMenu_Ani();
			break;
		case 4: //Options Menu
			if (doAni)
				Draw_OptionsMenu_Ani();
			break;
		case 5: //Cheat Selection Menu
			if (menu_id == 1 || menu_id == 2) //if entering from game list, don't keep index, otherwise keep
				menu_old_sel[5] = 0;
			if (doAni && menu_id != 7 && menu_id != 6)
				Draw_CheatsMenu_Selection_Ani();
			break;
		case 6: //Cheat View Menu
			menu_old_sel[6] = 0;
			if (doAni)
				Draw_CheatsMenu_View_Ani();
			break;
		case 7: //Cheat Option Menu
			menu_old_sel[7] = 0;
			if (doAni)
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

u8 IDPS[0x10];

u8 lv2peek8(u64 addr) {
	u8 ret = (u8) (lv2peek(addr) >> 56ULL);
	return ret;
}

#define OFFSET_2_IDPS_484C         0x8000000000474AF4ULL
void peek_IDPS()
{
	u8 i;
	u64 OFFSET_2_IDPS = OFFSET_2_IDPS_484C;
	for(i=0; i<0x10; i++) {
		IDPS[i]=lv2peek8(OFFSET_2_IDPS + i);
	}
}



// Resets new frame
void drawScene()
{   
	tiny3d_Project2D(); // change to 2D context (remember you it works with 848 x 512 as virtual coordinates)
	
	char * userc, * onlinec;
	int max = 0;
	
	switch (menu_id)
	{
		case 0:
			
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
					switch (menu_sel)
					{
						case 0: //start (install MAMBA, load artemis_ps3.sprx, write cheats to /dev_hdd0/tmp/art.txt)
							userc = ParseActivatedGameList(user_game_list, user_game_count);
							onlinec = ParseActivatedGameList(online_game_list, online_game_count);
							
							writeFile("/dev_hdd0/tmp/art.txt", userc, onlinec);
							
							free (userc);
							free (onlinec);
							
							//
							char plugin_name[30];
							char plugin_filename[256];
							memset(plugin_name, 0, sizeof(plugin_name));
							memset(plugin_filename, 0, sizeof(plugin_filename));
							
							
							//So we know art is loaded if we boot up later
							writeFile("/dev_hdd0/tmp/artstate", "on", "");
							
							//Clear boot history
							DeleteBootHistory();
							
							close_art = 1;
							return;
						case 1: //Cheats menu
							SetMenu(1);
							return;
						case 2: //Online Cheats menu
							SetMenu(2);
							return;
						case 3: //Options menu
							SetMenu(4);
							return;
						case 4: //About menu
							SetMenu(3);
							return;
					}
				}
/*
				else if(paddata[0].BTN_SELECT)
				{
					LOG("Screen");
					dbglogger_screenshot_tmp(0);
					LOG("Shot");
				}
*/
				else if(paddata[0].BTN_CIRCLE && show_dialog(1, "Exit to XMB?"))
				{
					close_art = 1;
				}
				else if(paddata[0].BTN_SQUARE && show_dialog(1, "Remove Artemis plugin from memory?"))
				{
					//
					char plugin_name[30];
					char plugin_filename[256];
					memset(plugin_name, 0, sizeof(plugin_name));
					memset(plugin_filename, 0, sizeof(plugin_filename));
				}
			}
			
			Draw_MainMenu();
			
			break;
		case 1: //Offline Cheats Menu
			
			// Check the pads.
			if (readPad(0))
			{
				if(paddata[0].BTN_UP)
				{
					move_selection_back(user_game_count, 1);
				}
				else if(paddata[0].BTN_DOWN)
				{
					move_selection_fwd(user_game_count, 1);
				}
				else if (paddata[0].BTN_LEFT)
				{
					move_selection_back(user_game_count, 5);
				}
				else if (paddata[0].BTN_L1)
				{
					move_selection_back(user_game_count, 25);
				}
				else if (paddata[0].BTN_L2)
				{
					move_letter_back(user_game_list, user_game_count);
				}
				else if (paddata[0].BTN_RIGHT)
				{
					move_selection_fwd(user_game_count, 5);
				}
				else if (paddata[0].BTN_R1)
				{
					move_selection_fwd(user_game_count, 25);
				}
				else if (paddata[0].BTN_R2)
				{
					move_letter_fwd(user_game_list, user_game_count);
				}
				else if (paddata[0].BTN_CIRCLE)
				{
					SetMenu(0);
					return;
				}
				else if (paddata[0].BTN_CROSS)
				{
					if (!user_game_list[menu_sel].codes)
					{
						int sz = 0;
						user_game_list[menu_sel].codes = ReadCodes(user_game_list[menu_sel].title_id, &sz);
						user_game_list[menu_sel].code_count = sz;
					}
					if (doSort)
						user_game_list[menu_sel] = BubbleSortCodeList(user_game_list[menu_sel]);
					selected_entry = user_game_list[menu_sel];
					SetMenu(5);
					return;
				}
				else if (paddata[0].BTN_SQUARE)
				{
					ReloadUserCheats();
				}
			}
			
			Draw_UserCheatsMenu(user_game_list, user_game_count, menu_sel, 0xFF);
			break;
		case 2: //Online Cheats Menu
			
			// Check the pads.
			if (readPad(0))
			{
				if(paddata[0].BTN_UP)
				{
					move_selection_back(online_game_count, 1);
				}
				else if(paddata[0].BTN_DOWN)
				{
					move_selection_fwd(online_game_count, 1);
				}
				else if (paddata[0].BTN_LEFT)
				{
					move_selection_back(online_game_count, 5);
				}
				else if (paddata[0].BTN_L1)
				{
					move_selection_back(online_game_count, 25);
				}
				else if (paddata[0].BTN_L2)
				{
					move_letter_back(online_game_list, online_game_count);
				}
				else if (paddata[0].BTN_RIGHT)
				{
					move_selection_fwd(online_game_count, 5);
				}
				else if (paddata[0].BTN_R1)
				{
					move_selection_fwd(online_game_count, 25);
				}
				else if (paddata[0].BTN_R2)
				{
					move_letter_fwd(online_game_list, online_game_count);
				}
				else if (paddata[0].BTN_CIRCLE)
				{
					SetMenu(0);
					return;
				}
				else if (paddata[0].BTN_CROSS)
				{
					if (!online_game_list[menu_sel].codes)
					{
						int sz = 0;
						online_game_list[menu_sel].codes = ReadOnlineNCL(online_game_list[menu_sel].path, &sz);
						online_game_list[menu_sel].code_count = sz;
					}
					if (doSort)
						online_game_list[menu_sel] = BubbleSortCodeList(online_game_list[menu_sel]);
					selected_entry = online_game_list[menu_sel];
					SetMenu(5);
					return;
				}
				else if (paddata[0].BTN_SQUARE)
				{
					ReloadOnlineCheats();
				}
			}

			Draw_UserCheatsMenu(online_game_list, online_game_count, menu_sel, 0xFF);
			break;
		case 3: //About Menu
			
			// Check the pads.
			if (readPad(0))
			{
				if (paddata[0].BTN_CIRCLE)
				{
					SetMenu(0);
					return;
				}
			}
			
			Draw_AboutMenu();
			break;
		case 4: //Options Menu
			
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
					SaveOptions();
					SetMenu(0);
					return;
				}
				else if (paddata[0].BTN_LEFT)
				{
					if (menu_options_options[menu_sel].type == ARTEMIS_OPTION_BOOL)
						menu_options_selections[menu_sel] = !menu_options_selections[menu_sel];
					else if (menu_options_options[menu_sel].type == ARTEMIS_OPTION_LIST)
					{
						if (menu_options_selections[menu_sel] > 0)
							menu_options_selections[menu_sel]--;
						else
							menu_options_selections[menu_sel] = menu_options_maxsel[menu_sel] - 1;
					}
					else if (menu_options_options[menu_sel].type == ARTEMIS_OPTION_INC)
						menu_options_selections[menu_sel]--;
					
					menu_options_options[menu_sel].callback(menu_sel, menu_options_selections[menu_sel]);
				}
				else if (paddata[0].BTN_CROSS || paddata[0].BTN_RIGHT)
				{
					if (menu_options_options[menu_sel].type == ARTEMIS_OPTION_BOOL)
						menu_options_selections[menu_sel] = !menu_options_selections[menu_sel];
					else if (menu_options_options[menu_sel].type == ARTEMIS_OPTION_LIST)
					{
						if (menu_options_selections[menu_sel] < (menu_options_maxsel[menu_sel] - 1))
							menu_options_selections[menu_sel]++;
						else
							menu_options_selections[menu_sel] = 0;
					}
					else if (menu_options_options[menu_sel].type == ARTEMIS_OPTION_INC)
						menu_options_selections[menu_sel]++;
					else if (menu_options_options[menu_sel].type == ARTEMIS_OPTION_CALL)
						menu_options_selections[menu_sel] = 1;
					

					menu_options_options[menu_sel].callback(menu_sel, menu_options_selections[menu_sel]);
				}
			}
			
			Draw_OptionsMenu();
			break;
		case 5: //Cheats Selection Menu
			
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
					SetMenu(last_menu_id[5]);
					return;
				}
				else if (paddata[0].BTN_CROSS)
				{
					selected_entry.codes[menu_sel].activated = !selected_entry.codes[menu_sel].activated;

/*
					LOG("act %d", selected_entry.codes[menu_sel].activated);
					LOG("nam %s", selected_entry.codes[menu_sel].name);
					LOG("cod %s", selected_entry.codes[menu_sel].codes);
*/
					
					if (selected_entry.codes[menu_sel].activated)
					{

						if (strcmp(selected_entry.codes[menu_sel].codes, CODE_RESIGN_SAVE) == 0)
						{
							LOG("Resigning save '%s'...", selected_entry.path);
							if (pfd_util_init(selected_entry.title_id, selected_entry.path, 0) != 0)
								LOG("ERROR INIT!");

							LOG("init OK");
							
							if (pfd_util_process(PFD_CMD_UPDATE) != 0)
								LOG("ERROR PROCESS");

							LOG("PFD process OK");
							
							pfd_util_end();

							LOG("PFD end OK");

							selected_entry.codes[menu_sel].activated = !selected_entry.codes[menu_sel].activated;
						}

						if (strcmp(selected_entry.codes[menu_sel].codes, CODE_UNLOCK_COPY) == 0)
						{
							unsigned int patch_flags = 0;
							char in_file_path[256];
							snprintf(in_file_path, sizeof(in_file_path)-1, "%s" "PARAM.SFO", selected_entry.path);

							LOG("Unlocking save '%s'...", in_file_path);
							patch_flags |= SFO_PATCH_FLAG_REMOVE_COPY_PROTECTION;
			                if (patch_sfo(in_file_path, in_file_path, patch_flags) == 0)
			                    show_dialog(0, "Save file successfully unlocked!");
			                else
			                    show_dialog(0, "Error! Save file couldn't be unlocked");

							selected_entry.codes[menu_sel].activated = !selected_entry.codes[menu_sel].activated;
						}

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
							SetMenu(7);
							return;
						}
					}
				}
				else if (paddata[0].BTN_SQUARE)
				{
					if (selected_entry.codes[menu_sel].activated)
					{
						selected_entry.codes[menu_sel].cwrite = !selected_entry.codes[menu_sel].cwrite;
					}
				}
				else if (paddata[0].BTN_TRIANGLE)
				{
					selected_centry = selected_entry.codes[menu_sel];
					SetMenu(6);
					return;
				}
			}
			
			Draw_CheatsMenu_Selection(menu_sel, 0xFFFFFFFF);
			break;
		case 6: //Cheat View Menu
			
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
					SetMenu(last_menu_id[6]);
					return;
				}
			}
			
			Draw_CheatsMenu_View();
			break;
		case 7: //Cheat Option Menu
			
			//Calc max
			max = selected_centry.options[option_index].size;
			
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
					selected_entry.codes[menu_old_sel[last_menu_id[7]]].activated = 0;
					SetMenu(last_menu_id[7]);
					return;
				}
				else if (paddata[0].BTN_CROSS)
				{
					selected_entry.codes[menu_old_sel[last_menu_id[7]]].options[option_index].sel = menu_sel;
					option_index++;
					
					if (option_index >= selected_entry.codes[menu_old_sel[last_menu_id[7]]].options_count)
					{
						SetMenu(last_menu_id[7]);
						return;
					}
					else
						menu_sel = 0;
				}
			}
			
			Draw_CheatsMenu_Options();
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

	http_init();

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
	while (menu_options_options[menu_options_maxopt].name)
		menu_options_maxopt++;
	
	int selSize = menu_options_maxopt * sizeof(int);
	menu_options_maxsel = (int *)calloc(1, selSize);
	menu_options_selections = (int *)calloc(1, selSize);
	
	int i = 0;
	for (i = 0; i < menu_options_maxopt; i++)
	{
		menu_options_maxsel[i] = 0;
		if (menu_options_options[i].type == ARTEMIS_OPTION_LIST)
		{
			while (menu_options_options[i].options[menu_options_maxsel[i]])
				menu_options_maxsel[i]++;
		}
	}
	
	LoadOptions();

	//LoadGames();
	
	//texture_mem = tiny3d_AllocTexture(64*1024*1024);

	videoState state;
	assert(videoGetState(0, 0, &state) == 0); // Get the state of the display
	assert(state.state == 0); // Make sure display is enabled
	
	videoResolution res;
	assert(videoGetResolution(state.displayMode.resolution, &res) == 0);
	LOG("Resolution: %d by %d", res.width, res.height);
	screen_width = res.width;
	screen_height = res.height;
	
	//SND_SetVoice( 2, (effect_is_stereo) ? VOICE_STEREO_16BIT : VOICE_MONO_16BIT, effect_freq, 0, background_music, background_music_size, 255, 255, NULL);
	SND_SetInfiniteVoice(2, (effect_is_stereo) ? VOICE_STEREO_16BIT : VOICE_MONO_16BIT, effect_freq, 0, background_music, background_music_size, 255, 255);
	
	//Set options
	for (i = 0; i < menu_options_maxopt; i++)
		menu_options_options[i].callback(i, menu_options_selections[i]);

	SetMenu(0);
	
	while (1)
	{       
		tiny3d_Clear(0xff000000, TINY3D_CLEAR_ALL);

		// Enable alpha Test
		tiny3d_AlphaTest(1, 0x10, TINY3D_ALPHA_FUNC_GEQUAL);

		// Enable alpha blending.
		tiny3d_BlendFunc(1, TINY3D_BLEND_FUNC_SRC_RGB_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_ALPHA_SRC_ALPHA,
			TINY3D_BLEND_FUNC_SRC_ALPHA_ONE_MINUS_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_RGB_ONE_MINUS_SRC_ALPHA,
			TINY3D_BLEND_RGB_FUNC_ADD | TINY3D_BLEND_ALPHA_FUNC_ADD);
					
		// Check the pads.
		if (close_art)
			return 0;
		
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
			
			SetFontSize(18, 16);
			SetCurrentFont(0);
			SetFontAlign(1);
			SetFontColor(0x00000000 | alpha, 0);
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
