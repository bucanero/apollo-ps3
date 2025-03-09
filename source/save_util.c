/*
 * main.c : Load/Save game sample
 *
 * Copyright (C) Youness Alaoui (KaKaRoTo)
 *
 * This software is distributed under the terms of the MIT License
 *
 */

#include <sys/stat.h>
#include <sys/thread.h>
#include <sys/memory.h>
#include <sysutil/video.h>
#include <sysutil/sysutil.h>
#include <sysutil/save.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>

#include "saves.h"
#include "util.h"
#include "settings.h"
#include "common.h"
#include "sfo.h"

#define THREAD_STACK_SIZE 16*1024
#define THREAD_PRIORITY 1000

/* Allow 100 save games */
#define SAVE_LIST_MAX_DIRECTORIES 10
/* Max 3 files : icon, screenshot, save data */
#define SAVE_LIST_MAX_FILES 3

/* This should actually be the maximum between
 * (MAX_FILES * sizeof(sysSaveFileStatus)) and
 * (MAX_DIRECTORIES * sizeof(sysSaveDirectoryList))
 */
#define BUFFER_SETTINGS_BUFSIZE (SAVE_LIST_MAX_DIRECTORIES * sizeof(sysSaveDirectoryList))
#define MEMORY_CONTAINER_SIZE (5*1024*1024)

#define SAVE_DATA_FOLDER   "NP0APOLLO-OPTIONS"
#define SAVE_DATA_FILENAME "SETTINGS"
#define SAVE_DATA_TITLE    "Apollo Save Tool"
#define SAVE_DATA_SUBTITLE "Settings"
#define SAVE_DATA_DETAIL   "www.bucanero.com.ar"

#define SAVE_UTIL_LOADING   1
#define SAVE_UTIL_SAVING    2

#define save_game_thread(folder, name, t, s, d)    _create_thread(SAVE_UTIL_SAVING, folder, name, t, s, d)
#define load_game_thread(folder, name)             _create_thread(SAVE_UTIL_LOADING, folder, name, NULL, NULL, NULL)

enum SaveDataMode {
	PS3_SAVE_MODE_ICON,
	PS3_SAVE_MODE_SCREENSHOT,
	PS3_SAVE_MODE_DATA,
	PS3_SAVE_MODE_DONE,
};

typedef struct SaveData {
	char *folder;
	char *filename;
	char *title;
	char *subtitle;
	char *detail;
	int flags;
	enum SaveDataMode mode;
	u8 *icon_data;
	u64 icon_size;
	u8 *screenshot_data;
	u64 screenshot_size;
	s32 result;
} save_metadata_t;

static sys_ppu_thread_t save_tid = 0;
static save_metadata_t* save_data;
static u8* file_data;
static u64 file_size;

static void saveload_game_status_cb (sysSaveCallbackResult *result, sysSaveStatusIn *in, sysSaveStatusOut *out)
{
	int i;

	LOG("saveload_game_status_cb called");

	LOG("Free space : %d\n"
		"New save game? : %d\n"
		" -dirname : %s\n"
		" -title : %s\n"
		"  subtitle : %s\n"
		"  detail : %s\n"
		"  copy protected? : %d\n"
		"  parental level : %d\n"
		"  listParameter : %s\n"
		"binding information : %d\n"
		"size of save data : %d\n"
		"size of system file : %d\n"
		"total files : %d\n"
		"number of files : %d\n",
		in->freeSpaceKB,
		in->isNew,
		in->directoryStatus.directoryName,
		in->getParam.title,
		in->getParam.subtitle,
		in->getParam.detail,
		in->getParam.copyProtected,
		in->getParam.parentalLevel,
		in->getParam.listParameter,
		in->bindingInformation,
		in->sizeKB,
		in->systemSizeKB,
		in->totalFiles,
		in->numFiles);

	for (i = 0; i < in->numFiles; i++)
	{
		LOG("  -File type : %d\n"
			"   File size : %lu\n"
			"   File name : %s\n",
			in->fileList[i].fileType,
			in->fileList[i].fileSize,
			in->fileList[i].filename);
	}

	result->result = SYS_SAVE_CALLBACK_RESULT_CONTINUE;
	out->setParam = &in->getParam;

	if (save_data->flags & SAVE_UTIL_LOADING)
	{
		/* Do not tell it to delete the files if we're loading!!! */
		out->recreateMode = SYS_SAVE_RECREATE_MODE_OVERWRITE_NOT_CORRUPTED;
		/* We'll only load the data */
		save_data->mode = PS3_SAVE_MODE_DATA;
		file_size = 0;
		for (i = 0; i < in->numFiles; i++)
		{
			switch (in->fileList[i].fileType)
			{
			case SYS_SAVE_FILETYPE_STANDARD_FILE:
				file_size = in->fileList[i].fileSize;
				break;
			case SYS_SAVE_FILETYPE_CONTENT_ICON0:
				save_data->icon_size = in->fileList[i].fileSize;
				break;
			case SYS_SAVE_FILETYPE_CONTENT_PIC1:
				save_data->screenshot_size = in->fileList[i].fileSize;
				break;
			default:
				break;
			}
		}
		if (file_size == 0)
		{
			LOG("Couldn't find the save data.. !");
			result->result = SYS_SAVE_CALLBACK_RESULT_CORRUPTED;
			return;
		} else {
			LOG("Found save game data of size : %lu", file_size);
			file_data = malloc (file_size);
		}
	} else {
		/* Delete */
		out->recreateMode = SYS_SAVE_RECREATE_MODE_DELETE;
		save_data->mode = PS3_SAVE_MODE_ICON;

		/* Check for free space... don't forget the system file's size, and to check
		* for existing files that would get deleted if you overwrite a save game.
		* Let's assume we need 1MB of data...
		*/
		if ((in->freeSpaceKB + in->sizeKB) < (1024 + in->systemSizeKB))
		{
			result->result = SYS_SAVE_CALLBACK_RESULT_NO_SPACE_LEFT;
			result->missingSpaceKB  = (1024 + in->systemSizeKB) - (in->freeSpaceKB + in->sizeKB);
		}

		strncpy(in->getParam.title, save_data->title, SYS_SAVE_MAX_TITLE);
		strncpy(in->getParam.subtitle, save_data->subtitle, SYS_SAVE_MAX_SUBTITLE);
		strncpy(in->getParam.detail, save_data->detail, SYS_SAVE_MAX_DETAIL);
	}
}

static void saveload_game_file_cb (sysSaveCallbackResult *result, sysSaveFileIn *in, sysSaveFileOut *out)
{
	LOG("saveload_game_file_cb called");

	LOG("Last operation %s %d bytes", (save_data->flags & SAVE_UTIL_SAVING ? "wrote" : "read"),
		in->previousOperationResultSize);

	memset (out, 0, sizeof(sysSaveFileOut));
	switch (save_data->mode)
	{
	case PS3_SAVE_MODE_ICON:
		{
		LOG("Saving icon");

		out->fileOperation = SYS_SAVE_FILE_OPERATION_WRITE;
		out->fileType = SYS_SAVE_FILETYPE_CONTENT_ICON0;
		out->size = save_data->icon_size;
		out->bufferSize = save_data->icon_size;
		out->buffer = save_data->icon_data;

		result->result = SYS_SAVE_CALLBACK_RESULT_CONTINUE;
		result->incrementProgress = 30;
		save_data->mode = save_data->filename ? PS3_SAVE_MODE_DATA : PS3_SAVE_MODE_DONE;
//        save_data->mode = PS3_SAVE_MODE_SCREENSHOT;
		break;
		}
	/*
	case PS3_SAVE_MODE_SCREENSHOT:
		{
		LOG("Saving screenshot");
		out->fileOperation = SYS_SAVE_FILE_OPERATION_WRITE;
		out->fileType = SYS_SAVE_FILETYPE_CONTENT_PIC1;
		out->size = save_data->screenshot_size;
		out->bufferSize = save_data->screenshot_size;
		out->buffer = save_data->screenshot_data;

		result->result = SYS_SAVE_CALLBACK_RESULT_CONTINUE;
		result->incrementProgress = 30;
		save_data->mode = PS3_SAVE_MODE_DATA;
		break;
		}
	*/
	case PS3_SAVE_MODE_DATA:
		{
		if (save_data->flags & SAVE_UTIL_SAVING)
		{
			LOG("Writing game data");
			out->fileOperation = SYS_SAVE_FILE_OPERATION_WRITE;
		} else {
			LOG("Reading game data");
			out->fileOperation = SYS_SAVE_FILE_OPERATION_READ;
		}

		out->filename = save_data->filename;
		out->fileType = SYS_SAVE_FILETYPE_STANDARD_FILE;
		out->size = file_size;
		out->bufferSize = file_size;
		out->buffer = file_data;

		result->result = SYS_SAVE_CALLBACK_RESULT_CONTINUE;
		result->incrementProgress = 100;
		save_data->mode = PS3_SAVE_MODE_DONE;
		break;
		}
	case PS3_SAVE_MODE_DONE:
	default:
		result->result = SYS_SAVE_CALLBACK_RESULT_DONE;
		if (save_data->flags & SAVE_UTIL_LOADING) {
			if (in->previousOperationResultSize != file_size) {
				result->result = SYS_SAVE_CALLBACK_RESULT_CORRUPTED;
				LOG("ERROR Reading data!");
			}
		}
		break;
	}

	LOG("saveload_game_file_cb exit");
}

static void saveload_settings_thread(void *user_data)
{
	sysSaveBufferSettings bufferSettings;
	sys_mem_container_t container;
	s32 ret;

	LOG("saveload_thread started");

	/* Directory name must be all upper case */

	memset (&bufferSettings, 0, sizeof (sysSaveBufferSettings));
	bufferSettings.maxDirectories = SAVE_LIST_MAX_DIRECTORIES;
	bufferSettings.maxFiles = SAVE_LIST_MAX_FILES;
	bufferSettings.bufferSize = BUFFER_SETTINGS_BUFSIZE;
	bufferSettings.buffer = malloc (bufferSettings.bufferSize);

	if (sysMemContainerCreate (&container, MEMORY_CONTAINER_SIZE) != 0 ) {
		LOG("Unable to create memory container");
		goto end;
	}

	if (save_data->flags & SAVE_UTIL_SAVING)
	{
		LOG("Loading icon");
		read_buffer(APOLLO_PATH "../ICON0.PNG", &save_data->icon_data, &save_data->icon_size);

//    LOG("Loading screenshot");
//    load_file ("data/screenshot.png", &save_data->screenshot_data, &save_data->screenshot_size);

		ret = sysSaveAutoSave2 (SYS_SAVE_CURRENT_VERSION, save_data->folder, SYS_SAVE_ERROR_DIALOG_NONE,
				&bufferSettings, saveload_game_status_cb, saveload_game_file_cb, container, NULL);
	} else {
		ret = sysSaveAutoLoad2 (SYS_SAVE_CURRENT_VERSION, save_data->folder, SYS_SAVE_ERROR_DIALOG_NONE,
				&bufferSettings, saveload_game_status_cb, saveload_game_file_cb, container, NULL);
	}

	save_data->result = ret;

	LOG("sysSaveAutoLoad2/Save2 returned: %X", ret);
	sysMemContainerDestroy (container);

end:
	if (bufferSettings.buffer)
		free (bufferSettings.buffer);
	if (save_data->icon_data)
		free (save_data->icon_data);
	if (save_data->screenshot_data)
		free (save_data->screenshot_data);

	free (save_data);

	LOG("saveload_thread exiting");
	save_tid = 0;
	sysThreadExit (0);
}

static int _create_thread (int flags, const char *folder, const char *filename, char* title, char* subtitle, char* detail)
{
	s32 ret;

	if (save_tid != 0)
	{
		LOG("Save/Load thread already running");
		return FALSE;
	}

	save_data = calloc (1, sizeof(save_metadata_t));
	save_data->flags = flags;
	save_data->folder = (char*) folder;
	save_data->filename = (char*) filename;
	save_data->title = title ? title : "";
	save_data->subtitle = subtitle ? subtitle : "";
	save_data->detail = detail ? detail : "";
	ret = sysThreadCreate (&save_tid, saveload_settings_thread, NULL, THREAD_PRIORITY, THREAD_STACK_SIZE, 0, "save_thread");

	if (ret < 0)
	{
		LOG("Failed to create thread: %X", ret);
		return FALSE;
	}

	return TRUE;
}

void wait_save_thread(void)
{
    while (save_tid != 0) {
        usleep(100*1000);
    }
}

static uint32_t get_userid_dir(uint32_t tid)
{
	char path[128];
	struct stat sb;

	do
	{
		// just to avoid an endless search
		if (tid++ > 10000)
			return 0;

		snprintf(path, sizeof(path), SAVES_PATH_HDD SAVE_DATA_FOLDER, tid);

	} while (stat(path, &sb) || !S_ISDIR(sb.st_mode));

	return tid;
}

int create_savegame_folder(const char* folder, const char* path)
{
	char sfoPath[256];

	snprintf(sfoPath, sizeof(sfoPath), "%sPARAM.SFO", path);
	LOG("Reading %s...", sfoPath);

	sfo_context_t* sfo = sfo_alloc();
	if (sfo_read(sfo, sfoPath) < 0) {
		LOG("Unable to read from '%s'", sfoPath);
		sfo_free(sfo);
		return FALSE;
	}

	file_data = (u8*) sfoPath;
	file_size = sizeof(sfoPath);

	if (!save_game_thread(folder, NULL,
			(char*) sfo_get_param_value(sfo, "TITLE"),
			(char*) sfo_get_param_value(sfo, "SUB_TITLE"),
			(char*) sfo_get_param_value(sfo, "DETAIL")))
	{
		sfo_free(sfo);
		return FALSE;
	}
 
	wait_save_thread();
	sfo_free(sfo);

	return TRUE;
}

static void _log_settings(app_config_t* config)
{
    LOG("Apollo Save Tool v%s - Patch Engine v%s", APOLLO_VERSION, APOLLO_LIB_VERSION);
    LOG("User Settings: UserID (%08d) AccountID (%016lX)", config->user_id, config->account_id);
    LOG("PSID %016lX %016lX", config->psid[0], config->psid[1]);
    LOG("IDPS %016lX %016lX", config->idps[0], config->idps[1]);
    LOG("Music [%d] Sort [%d] Update [%d] Animation [%d]", config->music, config->doSort, config->update, config->doAni);
}

int save_app_settings(app_config_t* config)
{
    file_data = (u8*) config;
    file_size = sizeof(app_config_t);
    _log_settings(config);

    return save_game_thread(SAVE_DATA_FOLDER, SAVE_DATA_FILENAME, SAVE_DATA_TITLE, SAVE_DATA_SUBTITLE, SAVE_DATA_DETAIL);
}

int load_app_settings(app_config_t* config)
{
    file_size = 0;

    // Check if we finished loading, then load data
    if (!load_game_thread(SAVE_DATA_FOLDER, SAVE_DATA_FILENAME))
        return FALSE;

    wait_save_thread();

    if (file_size == sizeof(app_config_t))
    {
        memcpy(config, file_data, file_size);
        _log_settings(config);
        return TRUE;
    }

    uint32_t uid = 0;
    char tmp_path[256];
    u8 canary[10] = "123456789";
    u8 verify[10] = "000000000";

    file_data = canary;
    file_size = 10;

    config->account_id = 0;
    ss_aim_get_device_id((u8*) config->idps);
    ss_aim_get_open_psid((u8*) config->psid);

    if (!save_game_thread(SAVE_DATA_FOLDER, SAVE_DATA_FILENAME, SAVE_DATA_TITLE, SAVE_DATA_SUBTITLE, SAVE_DATA_DETAIL))
        return FALSE;

    wait_save_thread();

    LOG("Hunting file...");

    while (memcmp(canary, verify, 9) != 0)
    {
        uid = get_userid_dir(uid);
        LOG("GET UID = %d", uid);
        if (uid == 0)
        {
            LOG("[!] ERROR: Couldn't find save data folder");
            return FALSE;
        }

        snprintf(tmp_path, sizeof(tmp_path), SAVES_PATH_HDD SAVE_DATA_FOLDER "/" SAVE_DATA_FILENAME, uid);
        read_file(tmp_path, verify, 9);
    }

    config->user_id = uid;
    config->account_id = get_account_id(uid);

    save_app_settings(config);

    return TRUE;
}

int reset_app_settings(app_config_t* config)
{
	char tmp_path[256];

	snprintf(tmp_path, sizeof(tmp_path), SAVES_PATH_HDD SAVE_DATA_FOLDER "/" SAVE_DATA_FILENAME, config->user_id);
	write_buffer(tmp_path, (u8*) tmp_path, 1);

	return (load_app_settings(config));
}
