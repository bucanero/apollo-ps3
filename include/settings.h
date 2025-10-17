#define APOLLO_PLATFORM			"PS3"		//Apollo platform
#define APOLLO_VERSION			"2.2.0"		//Apollo PS3 version (about menu)

#define MENU_TITLE_OFF			30			//Offset of menu title text from menu mini icon
#define MENU_ICON_OFF 			70          //X Offset to start printing menu mini icon
#define MENU_ANI_MAX 			0x80        //Max animation number
#define MENU_SPLIT_OFF			200			//Offset from left of sub/split menu to start drawing
#define OPTION_ITEM_OFF         730         //Offset from left of settings item/value

enum app_option_type
{
    APP_OPTION_NONE,
    APP_OPTION_BOOL,
    APP_OPTION_LIST,
    APP_OPTION_INC,
    APP_OPTION_CALL,
};

typedef struct
{
	const char * name;
	const char * * options;
	int type;
	uint8_t * value;
	void(*callback)(int);
} menu_option_t;

typedef struct
{
    char app_name[8];
    char app_ver[8];
    uint8_t music;
    uint8_t doSort;
    uint8_t doAni;
    uint8_t update;
    uint8_t online_opt;
    uint8_t dbglog;
    uint8_t usb_dev;
    uint32_t user_id;
    uint64_t idps[2];
    uint64_t psid[2];
    uint64_t account_id;
    char save_db[256];
    char ftp_url[512];
} app_config_t;

extern menu_option_t menu_options[];
extern app_config_t apollo_config;

void music_callback(int sel);
void update_callback(int sel);
void unzip_app_data(const char* zip_file);
void wait_save_thread(void);

int save_xml_owner(const char *xmlfile);
