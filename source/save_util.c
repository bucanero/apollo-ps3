/*
 * main.c : Load/Save game sample
 *
 * Copyright (C) Youness Alaoui (KaKaRoTo)
 *
 * This software is distributed under the terms of the MIT License
 *
 */

#include <sys/thread.h>
#include <sys/memory.h>
#include <sysutil/video.h>
#include <sysutil/sysutil.h>
#include <sysutil/save.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>

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


enum SaveDataMode {
  PS3_SAVE_MODE_ICON,
  PS3_SAVE_MODE_SCREENSHOT,
  PS3_SAVE_MODE_DATA,
  PS3_SAVE_MODE_DONE,
};

static struct SaveData {
  sys_ppu_thread_t save_tid;
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
  u8 *save_data;
  u64 save_size;
  s32 result;
} *save_data;


void saveload_game_status_cb (sysSaveCallbackResult *result, sysSaveStatusIn *in, sysSaveStatusOut *out)
{
  int i;

  dbglogger_log("saveload_game_status_cb called");

  dbglogger_log("Free space : %d\n"
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
    dbglogger_log("  -File type : %d\n"
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
    save_data->save_size = 0;
    for (i = 0; i < in->numFiles; i++) {
      switch (in->fileList[i].fileType) {
        case SYS_SAVE_FILETYPE_STANDARD_FILE:
          save_data->save_size = in->fileList[i].fileSize;
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
    if (save_data->save_size == 0) {
      dbglogger_log("Couldn't find the save data.. !");
      result->result = SYS_SAVE_CALLBACK_RESULT_CORRUPTED;
      return;
    } else {
      dbglogger_log("Found save game data of size : %lu\n", save_data->save_size);
      save_data->save_data = malloc (save_data->save_size);
    }
  } else {
//    char subtitle[SYS_SAVE_MAX_SUBTITLE];

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

  dbglogger_log("saveload_game_file_cb called");

  dbglogger_log("Last operation %s %d bytes", save_data->saving ? "wrote" : "read",
      in->previousOperationResultSize);

  memset (out, 0, sizeof(sysSaveFileOut));
  switch (save_data->mode) {
    case PS3_SAVE_MODE_ICON:
      {
        dbglogger_log("Saving icon");

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
    case PS3_SAVE_MODE_SCREENSHOT:
      {
        dbglogger_log("Saving screenshot");
/*
        out->fileOperation = SYS_SAVE_FILE_OPERATION_WRITE;
        out->fileType = SYS_SAVE_FILETYPE_CONTENT_PIC1;
        out->size = save_data->screenshot_size;
        out->bufferSize = save_data->screenshot_size;
        out->buffer = save_data->screenshot_data;

        result->result = SYS_SAVE_CALLBACK_RESULT_CONTINUE;
        result->incrementProgress = 30;
        save_data->mode = PS3_SAVE_MODE_DATA;
*/
        break;
      }
    case PS3_SAVE_MODE_DATA:
      {
        if (save_data->saving) {
          dbglogger_log("Writing game data");
          out->fileOperation = SYS_SAVE_FILE_OPERATION_WRITE;
        } else {
          dbglogger_log("Reading game data");
          out->fileOperation = SYS_SAVE_FILE_OPERATION_READ;
        }

        out->filename = SAVE_DATA_FILENAME;
        out->fileType = SYS_SAVE_FILETYPE_STANDARD_FILE;
        out->size = save_data->save_size;
        out->bufferSize = save_data->save_size;
        out->buffer = save_data->save_data;

        result->result = SYS_SAVE_CALLBACK_RESULT_CONTINUE;
        result->incrementProgress = 100;
        save_data->mode = PS3_SAVE_MODE_DONE;
        break;
      }
    case PS3_SAVE_MODE_DONE:
    default:
      result->result = SYS_SAVE_CALLBACK_RESULT_DONE;
      if (save_data->loading) {
        if (in->previousOperationResultSize != save_data->save_size) {
          result->result = SYS_SAVE_CALLBACK_RESULT_CORRUPTED;
        } else {
          dbglogger_log("READ data is : `%s`\n", save_data->save_data);
        }
      }
      break;
  }

  dbglogger_log("saveload_game_file_cb exit");
}

static int load_file (const char *filename, u8 **data, u64 *size)
{
  FILE *fd = fopen (filename, "rb");

  if (fd == NULL)
    return FALSE;

  fseek (fd, 0, SEEK_END);
  *size = ftell (fd);
  fseek (fd, 0, SEEK_SET);

  *data = malloc (*size);
  fread (*data, *size, 1, fd);
  fclose (fd);

  return TRUE;
}


void saveload_game_thread(void *user_data)
{
//  sysSaveListSettings listSettings;
  sysSaveBufferSettings bufferSettings;
  sys_mem_container_t container;
  char *prefix = save_data->prefix;
  s32 ret;

  dbglogger_log("saveload_thread started");

  /* Directory name must be all upper case */
  strncpy (prefix, "NP0APOLLO-OPTIONS", SYS_SAVE_MAX_DIRECTORY_NAME);

/*
  memset (&listSettings, 0, sizeof (sysSaveListSettings));
  listSettings.sortType = SYS_SAVE_SORT_TYPE_TIMESTAMP;
  listSettings.sortOrder = SYS_SAVE_SORT_ORDER_DESCENDING;
  listSettings.pathPrefix = save_data->prefix;
  listSettings.reserved = NULL;
*/

  memset (&bufferSettings, 0, sizeof (sysSaveBufferSettings));
  bufferSettings.maxDirectories = SAVE_LIST_MAX_DIRECTORIES;
  bufferSettings.maxFiles = SAVE_LIST_MAX_FILES;
  bufferSettings.bufferSize = BUFFER_SETTINGS_BUFSIZE;
  bufferSettings.buffer = malloc (bufferSettings.bufferSize);

  if (sysMemContainerCreate (&container, MEMORY_CONTAINER_SIZE) != 0 ) {
    dbglogger_log("Unable to create memory container");
    goto end;
  }

  if (save_data->saving) {
    dbglogger_log("Loading icon");

    load_file ("/dev_hdd0/game/NP0APOLLO/ICON0.PNG", &save_data->icon_data, &save_data->icon_size);

    dbglogger_log("Loading screenshot");
    load_file ("data/screenshot.png", &save_data->screenshot_data, &save_data->screenshot_size);

    save_data->save_data = malloc (1024);
    snprintf ((char *) save_data->save_data, 1024,
        "This is the content of save data : %d",  4321);

    save_data->save_size = strlen ((char *) save_data->save_data);
    dbglogger_log("Save data is : `%s`\n", save_data->save_data);

    ret = sysSaveAutoSave2 (SYS_SAVE_CURRENT_VERSION, prefix, SYS_SAVE_ERROR_DIALOG_NONE,
            &bufferSettings, saveload_game_status_cb, saveload_game_file_cb, container, NULL);
  } else {
    ret = sysSaveAutoLoad2 (SYS_SAVE_CURRENT_VERSION, prefix, SYS_SAVE_ERROR_DIALOG_NONE,
            &bufferSettings, saveload_game_status_cb, saveload_game_file_cb, container, NULL);
  }

  save_data->result = ret;

  dbglogger_log("sysSaveListLoad2/Save2 returned : %d", ret);
  sysMemContainerDestroy (container);

end:
  if (bufferSettings.buffer)
    free (bufferSettings.buffer);
  if (save_data->new_save.directoryName)
    free (save_data->new_save.directoryName);
  if (save_data->save_data)
    free (save_data->save_data);
  if (save_data->icon_data)
    free (save_data->icon_data);
  if (save_data->screenshot_data)
    free (save_data->screenshot_data);

  dbglogger_log("saveload_thread exiting");
  save_data->save_tid = 0;
  sysThreadExit (0);
}

int _create_thread (int saving, char *thread_name)
{
  s32 ret;

  if (save_data->save_tid != 0) {
    dbglogger_log("Save/Load thread already running");
    return FALSE;
  }

  memset (save_data, 0, sizeof(struct SaveData));
  save_data->saving = saving;
  save_data->loading = !saving;
  ret = sysThreadCreate (&save_data->save_tid, saveload_game_thread, NULL,
      THREAD_PRIORITY, THREAD_STACK_SIZE, 0, thread_name);

  if (ret < 0) {
    dbglogger_log("Failed to create %s : %d\n", thread_name, ret);
    return FALSE;
  }

  return TRUE;
}

int test_save_game ()
{
  return _create_thread (TRUE, "save_thread");
}

int test_load_game ()
{
  return _create_thread (FALSE, "load_thread");
}

int main_loop_iterate (int f)
{

  if (!save_data) {
      save_data = malloc (sizeof(struct SaveData));
      memset (save_data, 0, sizeof(struct SaveData));
  }

  /* If we just started and we're doing nothing, then save */

//  if (save_data->saving == 0 && save_data->loading == 0)
  if (f == 1)
    test_save_game ();

  /* Check if we finished loading, then start saving */
//  if (save_data->loading && save_data->save_tid == 0)
//    test_save_game ();

  /* Check if we finished saving, then start loading */

//  if (save_data->saving && save_data->save_tid == 0) {
  if (f == 2) {
    test_load_game ();
  }

  /* We need to poll for events */
  sysUtilCheckCallback ();

  return TRUE;
}
