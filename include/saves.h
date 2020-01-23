#define APOLLO_ENABLE_LOGGING 1

#ifdef APOLLO_ENABLE_LOGGING
#include <dbglogger.h>
#define LOG dbglogger_log
#else
#define LOG(...)
#define dbglogger_init(...)
#endif

#define APOLLO_PATH				"/dev_hdd0/game/NP0APOLLO/USRDIR/"
#define APOLLO_DATA_PATH		APOLLO_PATH "DATA/"

#define SAVES_PATH_USB0			"/dev_usb000/apollo/PS3/SAVEDATA/"
#define SAVES_PATH_USB1			"/dev_usb001/apollo/PS3/SAVEDATA/"
#define SAVES_PATH_HDD			"/dev_hdd0/tmp/apollo/"   // /dev_hdd0/home/0000000x/savedata

#define ONLINE_URL				"http://apollo.psdev.tk/"
#define ONLINE_LOCAL_CACHE		APOLLO_PATH "CACHE/"
#define ONLINE_CACHE_TIMEOUT    24*3600     // 1-day local cache

#define CODE_RESIGN_SAVE        "RESIGN_SAVE"
#define CODE_UNLOCK_COPY        "UNLOCK_COPY"

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
    char * name;
    char * file;
    int cwrite;
    int activated;
    int options_count;
    char * codes;
    option_entry_t * options;
} code_entry_t;

typedef struct save_entry
{
    char * name;
	char * title_id;
	char * path;
	char * account_id;
	unsigned char locked;
    int code_count;
    int code_sorted;
    code_entry_t * codes;
} save_entry_t;

typedef struct game_entry_s
{
    char * name;
	char * title_id;
	char * path;
    int save_count;
    save_entry_t * saves;
} game_entry_t;

save_entry_t * ReadUserList(int * gmc);
save_entry_t * ReadOnlineList(int * gmc);
void UnloadGameList(save_entry_t * list, int count);
int isGameActivated(save_entry_t game);
char * ParseActivatedGameList(save_entry_t * list, int count);
void writeFile(const char * path, char * a, char * b);
char * readFile(const char * path);
void readFileBuffered(const char * path, char * buffer);
void BubbleSortGameList(save_entry_t * games, int count);
save_entry_t BubbleSortCodeList(save_entry_t game);
int isCodeLineValid(char * line);
long getFileSize(const char * path);
option_entry_t * ReadOptions(code_entry_t code, int * count);
code_entry_t * ReadCodes(const char * title_id, int * _code_count);
code_entry_t * ReadOnlineNCL(const char * path, int * _code_count);

int http_init(void);
void http_end(void);
int http_download(const char* url, const char* filename, const char* local_dst, int show_progress);

int extract_zip(const char* zip_file, const char* dest_path);

int show_dialog(int tdialog, const char * str);
void init_progress_bar(const char* progress_bar_title, const char* msg);
void update_progress_bar(long unsigned int* progress, const long unsigned int total_size, const char* msg);
void end_progress_bar(void);
