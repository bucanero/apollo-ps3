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
#include "sfo.h"
#include "settings.h"

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

#define SAVE_DATA_FILENAME "SETTINGS"

typedef struct {
	uint64_t psid[2];
	uint32_t user_id;
	char account_id[17];
} params_ids_t;

enum SaveDataMode {
  PS3_SAVE_MODE_ICON,
  PS3_SAVE_MODE_SCREENSHOT,
  PS3_SAVE_MODE_DATA,
  PS3_SAVE_MODE_DONE,
};

typedef struct SaveData {
  int saving;
  int loading;
  char prefix[SYS_SAVE_MAX_DIRECTORY_NAME];
  sysSaveNewSaveGame new_save;
  sysSaveNewSaveGameIcon new_save_icon;
  enum SaveDataMode mode;
  u8 *icon_data;
  u64 icon_size;
  u8 *screenshot_data;
  u64 screenshot_size;
  s32 result;
} save_data_t;

sys_ppu_thread_t save_tid = 0;
save_data_t* save_data;
u8* file_data;
u64 file_size;

void saveload_game_status_cb (sysSaveCallbackResult *result, sysSaveStatusIn *in, sysSaveStatusOut *out)
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
    LOG("  -File type : %d\n"
        "   File size : %lu\n"
        "   filename : %s\n",
        in->fileList[i].fileType,
        in->fileList[i].fileSize,
        in->fileList[i].filename);

  result->result = SYS_SAVE_CALLBACK_RESULT_CONTINUE;
  out->setParam = &in->getParam;

  if (save_data->loading) {
    /* Do not tell it to delete the files if we're loading!!! */
    out->recreateMode = SYS_SAVE_RECREATE_MODE_OVERWRITE_NOT_CORRUPTED;
    /* We'll only load the data */
    save_data->mode = PS3_SAVE_MODE_DATA;
    file_size = 0;
    for (i = 0; i < in->numFiles; i++) {
      switch (in->fileList[i].fileType) {
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
    if (file_size == 0) {
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
    if ((in->freeSpaceKB + in->sizeKB) < (1024 + in->systemSizeKB)) {
      result->result = SYS_SAVE_CALLBACK_RESULT_NO_SPACE_LEFT;
      result->missingSpaceKB  = (1024 + in->systemSizeKB) -
          (in->freeSpaceKB + in->sizeKB);
    }

    strncpy (in->getParam.title, "Apollo Save Tool", SYS_SAVE_MAX_TITLE);
    strncpy (in->getParam.subtitle, "Settings", SYS_SAVE_MAX_SUBTITLE);
    strncpy (in->getParam.detail, "www.bucanero.com.ar", SYS_SAVE_MAX_DETAIL);
  }

}

void saveload_game_file_cb (sysSaveCallbackResult *result, sysSaveFileIn *in, sysSaveFileOut *out)
{

  LOG("saveload_game_file_cb called");

  LOG("Last operation %s %d bytes", save_data->saving ? "wrote" : "read",
      in->previousOperationResultSize);

  memset (out, 0, sizeof(sysSaveFileOut));
  switch (save_data->mode) {
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
        save_data->mode = PS3_SAVE_MODE_DATA;
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
        if (save_data->saving) {
          LOG("Writing game data");
          out->fileOperation = SYS_SAVE_FILE_OPERATION_WRITE;
        } else {
          LOG("Reading game data");
          out->fileOperation = SYS_SAVE_FILE_OPERATION_READ;
        }

        out->filename = SAVE_DATA_FILENAME;
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
      if (save_data->loading) {
        if (in->previousOperationResultSize != file_size) {
          result->result = SYS_SAVE_CALLBACK_RESULT_CORRUPTED;
          LOG("ERROR Reading data!");
        }
      }
      break;
  }

  LOG("saveload_game_file_cb exit");
}

void saveload_game_thread(void *user_data)
{
  sysSaveBufferSettings bufferSettings;
  sys_mem_container_t container;
  char *prefix = save_data->prefix;
  s32 ret;

  LOG("saveload_thread started");

  /* Directory name must be all upper case */
  strncpy (prefix, "NP0APOLLO-OPTIONS", SYS_SAVE_MAX_DIRECTORY_NAME);

  memset (&bufferSettings, 0, sizeof (sysSaveBufferSettings));
  bufferSettings.maxDirectories = SAVE_LIST_MAX_DIRECTORIES;
  bufferSettings.maxFiles = SAVE_LIST_MAX_FILES;
  bufferSettings.bufferSize = BUFFER_SETTINGS_BUFSIZE;
  bufferSettings.buffer = malloc (bufferSettings.bufferSize);

  if (sysMemContainerCreate (&container, MEMORY_CONTAINER_SIZE) != 0 ) {
    LOG("Unable to create memory container");
    goto end;
  }

  if (save_data->saving) {
    LOG("Loading icon");

    read_buffer("/dev_hdd0/game/NP0APOLLO/ICON0.PNG", &save_data->icon_data, &save_data->icon_size);

//    LOG("Loading screenshot");
//    load_file ("data/screenshot.png", &save_data->screenshot_data, &save_data->screenshot_size);

    ret = sysSaveAutoSave2 (SYS_SAVE_CURRENT_VERSION, prefix, SYS_SAVE_ERROR_DIALOG_NONE,
            &bufferSettings, saveload_game_status_cb, saveload_game_file_cb, container, NULL);
  } else {
    ret = sysSaveAutoLoad2 (SYS_SAVE_CURRENT_VERSION, prefix, SYS_SAVE_ERROR_DIALOG_NONE,
            &bufferSettings, saveload_game_status_cb, saveload_game_file_cb, container, NULL);
  }

  save_data->result = ret;

  LOG("sysSaveAutoLoad2/Save2 returned : %d", ret);
  sysMemContainerDestroy (container);

end:
  if (bufferSettings.buffer)
    free (bufferSettings.buffer);
  if (save_data->new_save.directoryName)
    free (save_data->new_save.directoryName);
  if (save_data->icon_data)
    free (save_data->icon_data);
  if (save_data->screenshot_data)
    free (save_data->screenshot_data);
  free (save_data);

  LOG("saveload_thread exiting");
  save_tid = 0;
  sysThreadExit (0);
}

int _create_thread (int saving, char *thread_name)
{
  s32 ret;

  if (save_tid != 0) {
    LOG("Save/Load thread already running");
    return FALSE;
  }

  save_data = calloc (1, sizeof(save_data_t));
  save_data->saving = saving;
  save_data->loading = !saving;
  ret = sysThreadCreate (&save_tid, saveload_game_thread, NULL,
      THREAD_PRIORITY, THREAD_STACK_SIZE, 0, thread_name);

  if (ret < 0) {
    LOG("Failed to create %s : %d\n", thread_name, ret);
    return FALSE;
  }

  return TRUE;
}

int save_game_thread ()
{
  return _create_thread (TRUE, "save_thread");
}

int load_game_thread ()
{
  return _create_thread (FALSE, "load_thread");
}

void wait_save_thread() {
    while (save_tid != 0) {
        usleep(10*1000);
    }
}

uint32_t get_userid_dir(uint32_t tid)
{
	char path[128];
	int found = 0;
    struct stat sb;

    tid++;	
	while (!found) {
	    snprintf(path, sizeof(path), SAVES_PATH_HDD "NP0APOLLO-OPTIONS", tid);
        if ((stat(path, &sb) == 0) && S_ISDIR(sb.st_mode)) {
		    found = 1;
		} else {
		    tid++;
		}
        // just to avoid an endless search
        if (tid > 200)
            return 0;
    }

	return tid;
}

int save_app_settings(app_config_t* config)
{
    file_data = (u8*) config;
    file_size = sizeof(app_config_t);
    
    return save_game_thread();
}

int load_app_settings(app_config_t* config)
{
    file_size = 0;

    // Check if we finished loading, then load data
    load_game_thread();
    wait_save_thread();

    if (file_size == sizeof(app_config_t)) {
        memcpy(config, file_data, file_size);

        LOG("SETTINGS: uid %d (%016lX) PSID %016lX %016lX", config->user_id, config->account_id, config->psid[0], config->psid[1]);
        return TRUE;
    }

    uint32_t uid = 0;
    char tmp_path[256];
    u8 canary[10] = "123456789";
    u8 verify[10] = "000000000";

    file_data = canary;
    file_size = 10;
    
    save_game_thread();
    wait_save_thread();

    LOG("Hunting file...");

    while (memcmp(canary, verify, 9) != 0) {
        uid = get_userid_dir(uid);
        LOG("GET UID = %d", uid);
        if (uid == 0)
        {
            LOG("[!] ERROR: Couldn't find save data folder");
            return FALSE;
        }

        snprintf(tmp_path, sizeof(tmp_path), SAVES_PATH_HDD "NP0APOLLO-OPTIONS/SETTINGS", uid);
        read_file(tmp_path, verify, 9);
    }

    LOG("Canary found at '%s'", tmp_path);

    snprintf(tmp_path, sizeof(tmp_path), SAVES_PATH_HDD "NP0APOLLO-OPTIONS/PARAM.SFO", uid);
	sfo_context_t* sfo = sfo_alloc();
	sfo_read(sfo, tmp_path);

    LOG("Reading '%s'...", tmp_path);

	params_ids_t* param_ids = (params_ids_t*)(sfo_get_param_value(sfo, "PARAMS") + 0x1C);

    config->user_id = uid;
    config->psid[0] = param_ids->psid[0];
    config->psid[1] = param_ids->psid[1];
    sscanf(param_ids->account_id, "%lx", &(config->account_id));

	sfo_free(sfo);

    save_app_settings(config);

    return TRUE;
}
