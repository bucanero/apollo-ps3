#ifdef APOLLO_ENABLE_LOGGING
#include <dbglogger.h>
#define LOG dbglogger_log
#else
#define LOG(...)
#define dbglogger_init(...)
#endif

#define APOLLO_PATH				"/dev_hdd0/game/NP0APOLLO/USRDIR/"
#define APOLLO_DATA_PATH		APOLLO_PATH "DATA/"
#define APOLLO_LOCAL_CACHE		APOLLO_PATH "CACHE/"
#define APOLLO_UPDATE_URL		"https://api.github.com/repos/bucanero/apollo-ps3/releases/latest"

#define USB0_PATH               "/dev_usb000/"
#define USB1_PATH               "/dev_usb001/"
#define USER_PATH_HDD			"/dev_hdd0/home/%08d/"
#define SAVES_PATH_USB0			USB0_PATH "PS3/SAVEDATA/"
#define SAVES_PATH_USB1			USB1_PATH "PS3/SAVEDATA/"
#define SAVES_PATH_HDD			USER_PATH_HDD "savedata/"
#define TROPHY_PATH_HDD			USER_PATH_HDD "trophy/"
#define EXDATA_PATH_HDD			USER_PATH_HDD "exdata/"

#define EXPORT_PATH_USB0        USB0_PATH "PS3/EXPORT/"
#define EXPORT_PATH_USB1        USB1_PATH "PS3/EXPORT/"
#define EXPORT_RAP_PATH_USB0    EXPORT_PATH_USB0 "exdata/"
#define EXPORT_RAP_PATH_USB1    EXPORT_PATH_USB1 "exdata/"
#define EXPORT_RAP_PATH_HDD     "/dev_hdd0/exdata/"

#define IMPORT_RAP_PATH_USB0    USB0_PATH "exdata/"
#define IMPORT_RAP_PATH_USB1    USB1_PATH "exdata/"

#define ONLINE_URL				"http://apollo-db.psdev.tk/"
#define ONLINE_CACHE_TIMEOUT    24*3600     // 1-day local cache

#define OWNER_XML_FILE          "owners.xml"

enum cmd_code_enum
{
    CMD_CODE_NULL,

// Save commands
    CMD_DECRYPT_FILE,
    CMD_RESIGN_SAVE,
    CMD_DOWNLOAD_USB,
    CMD_DOWNLOAD_HDD,
    CMD_COPY_SAVE_USB,
    CMD_COPY_SAVE_HDD,
    CMD_EXPORT_ZIP_USB,
    CMD_EXPORT_ZIP_HDD,
    CMD_VIEW_DETAILS,

// Bulk commands
    CMD_RESIGN_SAVES_USB,
    CMD_COPY_SAVES_USB,
    CMD_COPY_SAVES_HDD,

// Export commands
    CMD_EXP_EXDATA_USB,
    CMD_EXP_TROPHY_USB,
    CMD_EXP_RAPS_USB,
    CMD_EXP_RAPS_HDD,
    CMD_EXP_FLASH2_USB,

// Import commands
    CMD_IMP_EXDATA_USB,
    CMD_IMP_TROPHY_USB,

// SFO patches
    SFO_UNLOCK_COPY,
    SFO_REMOVE_ACCOUNT_ID,
    SFO_REMOVE_PSID,
    SFO_CHANGE_TITLE_ID,
};

// Save flags
#define SAVE_FLAG_LOCKED        1
#define SAVE_FLAG_OWNER         2
#define SAVE_FLAG_PS3           4
#define SAVE_FLAG_PS1           8
#define SAVE_FLAG_PS2           16
#define SAVE_FLAG_PSP           32
#define SAVE_FLAG_PSV           64
#define SAVE_FLAG_RIF           128
#define SAVE_FLAG_RAP           256

enum char_flag_enum
{
    CHAR_TAG_NULL,
    CHAR_TAG_PS1,
    CHAR_TAG_PS2,
    CHAR_TAG_PS3,
    CHAR_TAG_PSP,
    CHAR_TAG_PSV,
    CHAR_TAG_APPLY,
    CHAR_TAG_OWNER,
    CHAR_TAG_LOCKED,
    CHAR_RES_TAB,
    CHAR_RES_LF,
    CHAR_TAG_TRANSFER,
    CHAR_TAG_ZIP,
    CHAR_RES_CR,
    CHAR_TAG_PCE,
    CHAR_TAG_WARNING,
    CHAR_BTN_X,
    CHAR_BTN_S,
    CHAR_BTN_T,
    CHAR_BTN_O,
};

enum code_type_enum
{
    PATCH_NULL,
    PATCH_COMMAND,
    PATCH_SFO,
    PATCH_GAMEGENIE,
    PATCH_BSD,
};

typedef struct option_entry
{
    char * line;
    char * * name;
    char * * value;
    int id;
    int size;
    int sel;
} option_entry_t;

typedef struct code_entry
{
    uint8_t type;
    char * name;
    char * file;
    uint8_t activated;
    int options_count;
    char * codes;
    option_entry_t * options;
} code_entry_t;

typedef struct save_entry
{
    char * name;
	char * title_id;
	char * path;
	uint32_t flags;
    int code_count;
    code_entry_t * codes;
} save_entry_t;

typedef struct
{
    save_entry_t * list;
    int count;
    char path[128];
    char* title;
    uint8_t icon_id;
    void (*UpdatePath)(char *);
    int (*ReadCodes)(save_entry_t *);
    save_entry_t* (*ReadList)(const char*, int *);
} save_list_t;

save_entry_t * ReadUserList(const char* userPath, int * gmc);
save_entry_t * ReadOnlineList(const char* urlPath, int * gmc);
save_entry_t * ReadBackupList(const char* userPath, int * gmc);
void UnloadGameList(save_entry_t * list, int count);
int isGameActivated(save_entry_t game);
char * ParseActivatedGameList(save_entry_t * list, int count);
char * readFile(const char * path, long* size);
int qsortSaveList_Compare(const void* A, const void* B);
int qsortCodeList_Compare(const void* A, const void* B);
long getFileSize(const char * path);
int ReadCodes(save_entry_t * save);
int ReadOnlineSaves(save_entry_t * game);
int ReadBackupCodes(save_entry_t * bup);

int http_init(void);
void http_end(void);
int http_download(const char* url, const char* filename, const char* local_dst, int show_progress);

int extract_zip(const char* zip_file, const char* dest_path);
int zip_directory(const char* basedir, const char* inputdir, const char* output_zipfile);

int show_dialog(int dialog_type, const char * str);
void init_progress_bar(const char* progress_bar_title, const char* msg);
void update_progress_bar(uint64_t* progress, const uint64_t total_size, const char* msg);
void end_progress_bar(void);
#define show_message(msg)	show_dialog(0, msg)

int init_loading_screen(const char* msg);
void stop_loading_screen();

int apply_cheat_patch_code(const char* fpath, const char* title_id, code_entry_t* code);
void execCodeCommand(code_entry_t* code, const char* codecmd);

int rif2rap(const uint8_t* idps_key, const char* lic_path, const char* rifFile, const char* rap_path);
int rap2rif(const uint8_t* idps_key, const char* exdata_path, const char* rap_file, const char *rif_path);

int create_savegame_folder(const char* folder);
