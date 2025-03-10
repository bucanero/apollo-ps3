/* 
	Apollo PS3 main.c
*/

#include <sys/spu.h>
#include <lv2/spu.h>

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

#include "saves.h"
#include "pfd.h"
#include "util.h"
#include "common.h"

//Menus
#include "menu.h"
#include "menu_gui.h"

//Font
#include "libfont.h"
#include "ttf_render.h"
#include "font_adonais.h"
#include "font-10x20.h"

//Sound
#include "spu_soundmodule_bin.h"
#include <soundlib/spu_soundlib.h>
#include <libxmp-lite/xmp.h>

#define SAMPLING_FREQ       48000 /* 48khz. */
#define AUDIO_SAMPLES       SAMPLING_FREQ * 2 /* audio buffer to decode (for 48000 samples x 0.5 seconds and 16 bit stereo as reference) */

extern const uint8_t haiku_s3m[];
extern const uint32_t haiku_s3m_size;

static short *background_music[2] = {NULL, NULL};
static xmp_context xmp = NULL;

// SPU
static u32 inited;
static u32 spu = 0;
static sysSpuImage spu_image;

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
	   LoadTexture_##type(name##_##type##_index); \
	})

void update_usb_path(char *p);
void update_hdd_path(char *p);
void update_trophy_path(char *p);
void update_db_path(char *p);
void update_vmc_path(char *p);

app_config_t apollo_config = {
    .app_name = "APOLLO",
    .app_ver = APOLLO_VERSION,
    .save_db = ONLINE_URL,
    .ftp_server = "",
    .music = 1,
    .doSort = 1,
    .doAni = 1,
    .update = 1,
    .dbglog = 0,
    .user_id = 0,
    .idps = {0, 0},
    .psid = {0, 0},
    .account_id = 0,
};

int close_app = 0;
int idle_time = 0;                          // Set by readPad

png_texture * menu_textures;                // png_texture array for main menu, initialized in LoadTexture

const char * menu_pad_help[TOTAL_MENU_IDS] = { NULL,												//Main
								"\x10 Select    \x13 Back    \x12 Details    \x11 Refresh",			//Trophy list
								"\x10 Select    \x13 Back    \x12 Details    \x11 Refresh",			//USB list
								"\x10 Select    \x13 Back    \x12 Details    \x11 Refresh",			//HDD list
								"\x10 Select    \x13 Back    \x12 Details    \x11 Refresh",			//Online list
								"\x10 Select    \x13 Back    \x11 Refresh",							//User backup
								"\x10 Select    \x13 Back",											//Options
								"\x13 Back",														//About
								"\x10 Select    \x12 View Code    \x13 Back",						//Select Cheats
								"\x13 Back",														//View Cheat
								"\x10 Select    \x13 Back",											//Cheat Option
								"\x13 Back",														//View Details
								"\x10 Value Up  \x11 Value Down   \x13 Exit",						//Hex Editor
								};

/*
* HDD save list
*/
save_list_t hdd_saves = {
	.icon_id = cat_hdd_png_index,
	.title = "HDD Saves",
    .list = NULL,
    .path = "",
    .ReadList = &ReadUserList,
    .ReadCodes = &ReadCodes,
    .UpdatePath = &update_hdd_path,
};

/*
* USB save list
*/
save_list_t usb_saves = {
	.icon_id = cat_usb_png_index,
	.title = "USB Saves",
    .list = NULL,
    .path = "",
    .ReadList = &ReadUsbList,
    .ReadCodes = &ReadCodes,
    .UpdatePath = &update_usb_path,
};

/*
* Trophy list
*/
save_list_t trophies = {
	.icon_id = cat_warning_png_index,
	.title = "Trophies",
    .list = NULL,
    .path = "",
    .ReadList = &ReadTrophyList,
    .ReadCodes = &ReadTrophies,
    .UpdatePath = &update_trophy_path,
};

/*
* Online code list
*/
save_list_t online_saves = {
	.icon_id = cat_db_png_index,
	.title = "Online Database",
    .list = NULL,
    .path = ONLINE_URL,
    .ReadList = &ReadOnlineList,
    .ReadCodes = &ReadOnlineSaves,
    .UpdatePath = &update_db_path,
};

/*
* User Backup code list
*/
save_list_t user_backup = {
    .icon_id = cat_bup_png_index,
    .title = "User Tools",
    .list = NULL,
    .path = "",
    .ReadList = &ReadBackupList,
    .ReadCodes = &ReadBackupCodes,
    .UpdatePath = NULL,
};

/*
* PS1 VMC list
*/
save_list_t vmc1_saves = {
    .icon_id = cat_usb_png_index,
    .title = "PS1 Virtual Memory Card",
    .list = NULL,
    .path = "",
    .ReadList = &ReadVmc1List,
    .ReadCodes = &ReadVmc1Codes,
    .UpdatePath = &update_vmc_path,
};

/*
* PS2 VMC list
*/
save_list_t vmc2_saves = {
    .icon_id = cat_usb_png_index,
    .title = "PS2 Virtual Memory Card",
    .list = NULL,
    .path = "",
    .ReadList = &ReadVmc2List,
    .ReadCodes = &ReadVmc2Codes,
    .UpdatePath = &update_vmc_path,
};

static void release_all(void)
{	
	if(inited & INITED_CALLBACK)
		sysUtilUnregisterCallback(SYSUTIL_EVENT_SLOT0);

	if(inited & INITED_SOUNDLIB)
		SND_End();

	if(inited & INITED_AUDIOPLAYER) {
		xmp_end_player(xmp);
		xmp_release_module(xmp);
		xmp_free_context(xmp);
	}

	if(inited & INITED_SPU) {
		sysSpuRawDestroy(spu);
		sysSpuImageClose(&spu_image);
	}

	http_end();
	wait_save_thread();
	sysModuleUnload(SYSMODULE_PNGDEC);

	inited=0;
}

static void sys_callback(uint64_t status, uint64_t param, void* userdata)
{
	switch (status) {
		case SYSUTIL_EXIT_GAME:
			release_all();
			if (file_exists("/dev_hdd0/mms/db.err") == SUCCESS)
				sys_reboot();

			sysProcessExit(1);
			break;

		case SYSUTIL_MENU_OPEN:
		case SYSUTIL_MENU_CLOSE:
			break;

		default:
			break;
	}
}

static void LoadTexture_png(int idx)
{
	pngLoadFromBuffer(menu_textures[idx].buffer, menu_textures[idx].size, &menu_textures[idx].texture);
	copyTexture(idx);
}

static void LoadTexture_jpg(int idx)
{
	jpgLoadFromBuffer(menu_textures[idx].buffer, menu_textures[idx].size, (jpgData*) &menu_textures[idx].texture);
	copyTexture(idx);
}

static void LoadImageFontTexture(const u8* rawData, uint16_t unicode, int idx)
{
	menu_textures[idx].size = LoadImageFontEntry(rawData, unicode, &menu_textures[idx].texture);
	copyTexture(idx);
}

// Used only in initialization. Allocates 64 mb for textures and loads the font
static void LoadTextures_Menu(void)
{
	texture_mem = tiny3d_AllocTexture(64*1024*1024); // alloc 64MB of space for textures (this pointer can be global)
	menu_textures = (png_texture *)calloc(TOTAL_MENU_TEXTURES, sizeof(png_texture));
	
	if(!texture_mem || !menu_textures)
		return; // fail!
	
	ResetFont();
	free_mem = (u32 *) AddFontFromBitmapArray((u8 *) data_font_Adonais, (u8 *) texture_mem, 0x20, 0x7e, 32, 31, 1, BIT7_FIRST_PIXEL);
	free_mem = (u32 *) AddFontFromBitmapArray((u8 *) console_font_10x20, (u8 *) free_mem, 0, 0xFF, 10, 20, 1, BIT7_FIRST_PIXEL);
	
	TTFUnloadFont();
	TTFLoadFont(0, "/dev_flash/data/font/SCE-PS3-SR-R-LATIN2.TTF", NULL, 0);
	TTFLoadFont(1, "/dev_flash/data/font/SCE-PS3-DH-R-CGB.TTF", NULL, 0);
	TTFLoadFont(2, "/dev_flash/data/font/SCE-PS3-SR-R-JPN.TTF", NULL, 0);
	TTFLoadFont(3, "/dev_flash/data/font/SCE-PS3-YG-R-KOR.TTF", NULL, 0);

	free_mem = (u32*) init_ttf_table((u16*) free_mem);
	
	set_ttf_window(0, 0, 848, 512, WIN_SKIP_LF);
//	TTFUnloadFont();

	//Init Main Menu textures
	load_menu_texture(leon_luna, jpg);
	load_menu_texture(bgimg, jpg);
	load_menu_texture(cheat, png);

	load_menu_texture(circle_loading_bg, png);
	load_menu_texture(circle_loading_seek, png);
	load_menu_texture(edit_shadow, png);

	load_menu_texture(footer_ico_circle, png);
	load_menu_texture(footer_ico_cross, png);
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
	load_menu_texture(buk_scr, png);
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
	load_menu_texture(jar_trophy, png);
	load_menu_texture(jar_trophy_hover, png);
	load_menu_texture(jar_hdd, png);
	load_menu_texture(jar_hdd_hover, png);
	load_menu_texture(jar_opt, png);
	load_menu_texture(jar_opt_hover, png);
	load_menu_texture(jar_usb, png);
	load_menu_texture(jar_usb_hover, png);
	load_menu_texture(logo, png);
	load_menu_texture(logo_text, png);
	load_menu_texture(tag_lock, png);
	load_menu_texture(tag_own, png);
	load_menu_texture(tag_vmc, png);
	load_menu_texture(tag_ps1, png);
	load_menu_texture(tag_ps2, png);
	load_menu_texture(tag_ps3, png);
	load_menu_texture(tag_psp, png);
	load_menu_texture(tag_psv, png);
	load_menu_texture(tag_warning, png);
	load_menu_texture(tag_net, png);
	load_menu_texture(tag_zip, png);
	load_menu_texture(tag_apply, png);
	load_menu_texture(tag_transfer, png);

	u8* imagefont;
	if (read_buffer("/dev_flash/vsh/resource/imagefont.bin", &imagefont, NULL) == SUCCESS)
	{
		LoadImageFontTexture(imagefont, 0xF888, footer_ico_lt_png_index);
		LoadImageFontTexture(imagefont, 0xF88B, footer_ico_rt_png_index);
		LoadImageFontTexture(imagefont, 0xF6AD, trp_sync_img_index);
		LoadImageFontTexture(imagefont, 0xF8AC, trp_bronze_img_index);
		LoadImageFontTexture(imagefont, 0xF8AD, trp_silver_img_index);
		LoadImageFontTexture(imagefont, 0xF8AE, trp_gold_img_index);
		LoadImageFontTexture(imagefont, 0xF8AF, trp_platinum_img_index);

		free(imagefont);
	}

	menu_textures[icon_png_file_index].buffer = free_mem;
	menu_textures[icon_png_file_index].size = 1;
	menu_textures[icon_png_file_index].texture.height = 176;
	menu_textures[icon_png_file_index].texture.pitch = (320*4);
	menu_textures[icon_png_file_index].texture.bmp_out = calloc(320 * 176, sizeof(u32));

	copyTexture(icon_png_file_index);
	free_mem = (u32*) menu_textures[icon_png_file_index].buffer;

	u32 tBytes = free_mem - texture_mem;
	LOG("LoadTextures_Menu() :: Allocated %db (%.02fkb, %.02fmb) for textures", tBytes, tBytes / (float)1024, tBytes / (float)(1024 * 1024));
}

static void xmp_audio_callback(int voice)
{
	static int music_buffer = 0;

	music_buffer ^= 1;
	xmp_play_buffer(xmp, background_music[music_buffer], AUDIO_SAMPLES, 0);
	SND_AddVoice(voice, background_music[music_buffer], AUDIO_SAMPLES);
}

static void LoadSounds(void)
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
	
	// decode the mp3 effect file included to memory. It stops by EOF or when samples exceed size_effects_samples
	xmp = xmp_create_context();

	// Decode a s3m file to play
	if (xmp_load_module_from_memory(xmp, (void*) haiku_s3m, haiku_s3m_size) < 0)
	{
		LOG("[ERROR] Failed to decode audio file");
		return;
	}

	xmp_set_player(xmp, XMP_PLAYER_VOLUME, 100);
	xmp_set_player(xmp, XMP_PLAYER_INTERP, XMP_INTERP_SPLINE);
	xmp_start_player(xmp, SAMPLING_FREQ, 0);

	// Calculate the sample count and allocate a buffer for the sample data accordingly
	// adjust the sound buffer sample correctly to the background_music_size
	// SPU dma works aligned to 128 bytes. SPU module is designed to read unaligned buffers and it is better thing aligned buffers)
	background_music[0] = (short *)memalign(128, SPU_SIZE(AUDIO_SAMPLES));
	background_music[1] = (short *)memalign(128, SPU_SIZE(AUDIO_SAMPLES));

	// Decode the audio into pSampleData
	if (xmp_play_buffer(xmp, background_music[0], AUDIO_SAMPLES, 0) == SUCCESS)
		inited |= INITED_AUDIOPLAYER;

	SND_Pause(1);
}

void update_usb_path(char* path)
{
	for (int i = 0; i <= MAX_USB_DEVICES; i++)
	{
		sprintf(path, USB_PATH, i);

		if (dir_exists(path) == SUCCESS)
			return;
	}

	sprintf(path, FAKE_USB_PATH);
	if (dir_exists(path) == SUCCESS)
		return;

	path[0] = 0;
}

void update_hdd_path(char* path)
{
	sprintf(path, USER_PATH_HDD, apollo_config.user_id);
}

void update_trophy_path(char* path)
{
	sprintf(path, TROPHY_PATH_HDD, apollo_config.user_id);
}

void update_db_path(char* path)
{
	if (apollo_config.ftp_server[0] && show_dialog(DIALOG_TYPE_YESNO, "Do you want to connect to your FTP server?"))
	{
		sprintf(path, "%s%016lX/", apollo_config.ftp_server, apollo_config.account_id);
		return;
	}

	strcpy(path, apollo_config.save_db);
}

void update_vmc_path(char* path)
{
	if (file_exists(path) == SUCCESS)
		return;

	path[0] = 0;
}

static void registerSpecialChars(void)
{
	// Register save tags
	RegisterSpecialCharacter(CHAR_TAG_PS1, 2, 1.5, &menu_textures[tag_ps1_png_index]);
	RegisterSpecialCharacter(CHAR_TAG_PS2, 2, 1.5, &menu_textures[tag_ps2_png_index]);
	RegisterSpecialCharacter(CHAR_TAG_PS3, 2, 1.5, &menu_textures[tag_ps3_png_index]);
	RegisterSpecialCharacter(CHAR_TAG_PSP, 2, 1.5, &menu_textures[tag_psp_png_index]);
	RegisterSpecialCharacter(CHAR_TAG_PSV, 2, 1.5, &menu_textures[tag_psv_png_index]);
	RegisterSpecialCharacter(CHAR_TAG_VMC, 2, 1.0, &menu_textures[tag_vmc_png_index]);
	RegisterSpecialCharacter(CHAR_TAG_LOCKED, 0, 1.5, &menu_textures[tag_lock_png_index]);
	RegisterSpecialCharacter(CHAR_TAG_OWNER, 0, 1.5, &menu_textures[tag_own_png_index]);
	RegisterSpecialCharacter(CHAR_TAG_WARNING, 0, 1.5, &menu_textures[tag_warning_png_index]);
	RegisterSpecialCharacter(CHAR_TAG_APPLY, 2, 1.1, &menu_textures[tag_apply_png_index]);
	RegisterSpecialCharacter(CHAR_TAG_ZIP, 0, 1.2, &menu_textures[tag_zip_png_index]);
	RegisterSpecialCharacter(CHAR_TAG_TRANSFER, 0, 1.2, &menu_textures[tag_transfer_png_index]);
	RegisterSpecialCharacter(CHAR_TAG_NET, 1, 1.2, &menu_textures[tag_net_png_index]);

	// Register button icons
	RegisterSpecialCharacter(ps3PadCrossOk() ? CHAR_BTN_X : CHAR_BTN_O, 0, 1.2, &menu_textures[footer_ico_cross_png_index]);
	RegisterSpecialCharacter(CHAR_BTN_S, 0, 1.2, &menu_textures[footer_ico_square_png_index]);
	RegisterSpecialCharacter(CHAR_BTN_T, 0, 1.2, &menu_textures[footer_ico_triangle_png_index]);
	RegisterSpecialCharacter(ps3PadCrossOk() ? CHAR_BTN_O : CHAR_BTN_X, 0, 1.2, &menu_textures[footer_ico_circle_png_index]);

	// Register trophy icons
	RegisterSpecialCharacter(CHAR_TRP_BRONZE, 2, 1.0, &menu_textures[trp_bronze_img_index]);
	RegisterSpecialCharacter(CHAR_TRP_SILVER, 2, 1.0, &menu_textures[trp_silver_img_index]);
	RegisterSpecialCharacter(CHAR_TRP_GOLD, 2, 1.0, &menu_textures[trp_gold_img_index]);
	RegisterSpecialCharacter(CHAR_TRP_PLATINUM, 0, 1.2, &menu_textures[trp_platinum_img_index]);
	RegisterSpecialCharacter(CHAR_TRP_SYNC, 0, 1.2, &menu_textures[trp_sync_img_index]);
}

/*
	Program start
*/
s32 main(s32 argc, const char* argv[])
{
#ifdef APOLLO_ENABLE_LOGGING
	dbglogger_init();
	dbglogger_failsafe("9999");
#endif

	http_init();

	tiny3d_Init(1024*1024);
	tiny3d_UserViewport(1, 0, 0, // 2D position
		(float) (Video_Resolution.width / 848.0f),  (float) (Video_Resolution.height / 512.0f),   // 2D scale
		(float) (Video_Resolution.width / 1920.0f), (float) (Video_Resolution.height / 1080.0f)); // 3D scale

	ps3PadInit();
	
	sysModuleLoad(SYSMODULE_PNGDEC);
	sysModuleLoad(SYSMODULE_JPGDEC);

	// register exit callback
	if(sysUtilRegisterCallback(SYSUTIL_EVENT_SLOT0, sys_callback, NULL)==0) inited |= INITED_CALLBACK;
	
	// Load texture
	LoadTextures_Menu();
	LoadSounds();

	// Unpack application data on first run
	if (file_exists(APOLLO_LOCAL_CACHE "appdata.zip") == SUCCESS)
	{
		clean_directory(APOLLO_DATA_PATH);
		unzip_app_data(APOLLO_LOCAL_CACHE "appdata.zip");
	}

	// Splash screen logo (fade-in)
	drawSplashLogo(1);

	// Load application settings
	load_app_settings(&apollo_config);

	if (apollo_config.dbglog)
		dbglogger_init_mode(FILE_LOGGER, "/dev_hdd0/tmp/apollo.log", 0);

	if (file_exists(APOLLO_PATH OWNER_XML_FILE) == SUCCESS)
		save_xml_owner(APOLLO_PATH OWNER_XML_FILE, NULL);

	menu_options[OWNER_SETTING].options = get_xml_owners(APOLLO_PATH OWNER_XML_FILE);
 
	// Set PFD keys from loaded settings
	pfd_util_setup_keys();

	// Setup font
	SetExtraSpace(5);
	SetCurrentFont(font_adonais_regular);

	registerSpecialChars();
	initMenuOptions();

	// Splash screen logo (fade-out)
	drawSplashLogo(-1);

	SND_SetVoice(2, VOICE_STEREO_16BIT, SAMPLING_FREQ, 0, background_music[0], AUDIO_SAMPLES, MAX_VOLUME, MAX_VOLUME, xmp_audio_callback);
	SND_Pause(!apollo_config.music);
	
	//Set options
	update_callback(!apollo_config.update);

	Draw_MainMenu_Ani();
	
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

		//Draw help
		if (menu_pad_help[menu_id])
		{
			u8 alpha = 0xFF;
			if (idle_time > 80)
			{
				int dec = (idle_time - 80) * 4;
				alpha = (dec > alpha) ? 0 : (alpha - dec);
			}
			
			SetFontSize(APP_FONT_SIZE_DESCRIPTION);
			SetCurrentFont(font_adonais_regular);
			SetFontAlign(FONT_ALIGN_SCREEN_CENTER);
			SetFontColor(APP_FONT_COLOR | alpha, 0);
			DrawString(0, 470, (char *)menu_pad_help[menu_id]);
			SetFontAlign(FONT_ALIGN_LEFT);
		}
		
		tiny3d_Flip();
	}

	release_all();
	if (file_exists("/dev_hdd0/mms/db.err") == SUCCESS)
		sys_reboot();

	return 0;
}
