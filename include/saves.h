#ifdef APOLLO_ENABLE_LOGGING
#include <dbglogger.h>
#define LOG dbglogger_log
#else
#define LOG(...)
#define dbglogger_init(...)
#endif

#define APOLLO_PATH				"/dev_hdd0/game/NP0APOLLO/USRDIR/"
#define APOLLO_DATA_PATH		APOLLO_PATH "DATA/"
#define APOLLO_UPDATE_URL		"https://api.github.com/repos/bucanero/apollo-ps3/releases/latest"

#define USB0_PATH               "/dev_usb000/PS3/"
#define USB1_PATH               "/dev_usb001/PS3/"
#define USER_PATH_HDD			"/dev_hdd0/home/%08d/"
#define SAVES_PATH_USB0			USB0_PATH "SAVEDATA/"
#define SAVES_PATH_USB1			USB1_PATH "SAVEDATA/"
#define SAVES_PATH_HDD			USER_PATH_HDD "savedata/"
#define TROPHY_PATH_HDD			USER_PATH_HDD "trophy/"
#define EXDATA_PATH_HDD			USER_PATH_HDD "exdata/"

#define EXPORT_PATH_USB0        USB0_PATH "EXPORT/"
#define EXPORT_PATH_USB1        USB1_PATH "EXPORT/"

#define ONLINE_URL				"http://apollo-db.psdev.tk/"
#define ONLINE_LOCAL_CACHE		APOLLO_PATH "CACHE/"
#define ONLINE_CACHE_TIMEOUT    24*3600     // 1-day local cache

// Save commands
#define CMD_APPLY_CHEATS        "APPLY_CHEAT"
#define CMD_RESIGN_SAVE         "RESIGN_SAVE"
#define CMD_DOWNLOAD_USB        "DNLOAD_USB"
#define CMD_DOWNLOAD_HDD        "DNLOAD_HDD0"
#define CMD_COPY_SAVE_USB       "COPYSG_USB"
#define CMD_EXPORT_ZIP_USB      "EXPORT_ZIP"

// Export commands
#define CMD_EXP_EXDATA_USB      "EXP_EXDATA"
#define CMD_EXP_TROPHY_USB      "EXP_TROPHY"
#define CMD_EXP_SAVES_USB       "EXP_ASAVES"

// Import commands
#define CMD_IMP_EXDATA_USB      "IMP_EXDATA"
#define CMD_IMP_TROPHY_USB      "IMP_TROPHY"

// SFO patches
#define SFO_UNLOCK_COPY         "SFO_UNLOCK"
#define SFO_REMOVE_ACCOUNT_ID   "SFO_RM_ACT"
#define SFO_REMOVE_PSID         "SFO_RM_PSD"

// Save flags
#define SAVE_FLAG_LOCKED        1
#define SAVE_FLAG_OWNER         2
#define SAVE_FLAG_PS3           4
#define SAVE_FLAG_PS1           8
#define SAVE_FLAG_PS2           16
#define SAVE_FLAG_PSP           32
#define SAVE_FLAG_PSV           64

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
    CHAR_TAG_PCE,
    CHAR_RES_LF,
    CHAR_TAG_TRANSFER,
    CHAR_TAG_ZIP,
    CHAR_TAG_WARNING,
    CHAR_BTN_X = 0x10,
    CHAR_BTN_S = 0x11,
    CHAR_BTN_T = 0x12,
    CHAR_BTN_O = 0x13,
};

enum code_type_enum
{
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
    unsigned char type;
    char * name;
    char * file;
    unsigned char activated;
    int options_count;
    char * codes;
    option_entry_t * options;
} code_entry_t;

typedef struct save_entry
{
    char * name;
	char * title_id;
	char * path;
	unsigned int flags;
    int code_count;
    code_entry_t * codes;
} save_entry_t;

typedef struct
{
    save_entry_t * list;
    int count;
    char path[128];
    char* title;
    unsigned char icon_id;
    void (*UpdatePath)(char *);
    int (*ReadCodes)(save_entry_t *);
    save_entry_t* (*ReadList)(const char*, int *);
} save_list_t;

save_entry_t * ReadUserList(const char* userPath, int * gmc);
save_entry_t * ReadOnlineList(const char* urlPath, int * gmc);
void UnloadGameList(save_entry_t * list, int count);
int isGameActivated(save_entry_t game);
char * ParseActivatedGameList(save_entry_t * list, int count);
void writeFile(const char * path, char * a, char * b);
char * readFile(const char * path, long* size);
void readFileBuffered(const char * path, char * buffer);
int qsortSaveList_Compare(const void* A, const void* B);
int qsortCodeList_Compare(const void* A, const void* B);
int isCodeLineValid(char * line);
long getFileSize(const char * path);
option_entry_t * ReadOptions(code_entry_t code, int * count);
int ReadCodesHDD(save_entry_t * save);
int ReadCodesUSB(save_entry_t * save);
int ReadOnlineSaves(save_entry_t * game);
int LoadBackupCodes(save_entry_t * bup);

int http_init(void);
void http_end(void);
int http_download(const char* url, const char* filename, const char* local_dst, int show_progress);

int extract_zip(const char* zip_file, const char* dest_path);
int zip_directory(const char* basedir, const char* inputdir, const char* output_zipfile);

int show_dialog(int dialog_type, const char * str);
void init_progress_bar(const char* progress_bar_title, const char* msg);
void update_progress_bar(long unsigned int* progress, const long unsigned int total_size, const char* msg);
void end_progress_bar(void);

int init_loading_screen(const char* msg);
void stop_loading_screen();

int apply_cheat_patch_code(const char* fpath, code_entry_t* code);
