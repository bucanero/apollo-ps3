
#define APOLLO_VERSION			"v0.0.1"	//Apollo PS3 version (about menu)
#define MENU_TITLE_OFF			30			//Offset of menu title text from menu mini icon
#define MENU_ICON_OFF 			70          //X Offset to start printing menu mini icon
#define MENU_ANI_MAX 			0x80        //Max animation number
#define MENU_SPLIT_OFF			200			//Offset from left of sub/split menu to start drawing
#define MENU_MAIN_ICON_WIDTH 	80			//Width of main menu icons

#define APP_OPTION_BOOL			1
#define APP_OPTION_LIST			2
#define APP_OPTION_INC			3
#define APP_OPTION_CALL			4

typedef struct
{
	char * name;
	char * * options;
	int type;
	uint8_t * value;
	void(*callback)(int,int);
} menu_option_t;

typedef struct
{
    uint8_t music;
    uint8_t doSort;
    uint8_t doAni;
    uint8_t marginH;
    uint8_t marginV;
    uint32_t user_id;
    uint64_t psid[2];
    uint64_t account_id;
} app_config_t;

extern const menu_option_t menu_options[];

extern app_config_t apollo_config;

extern int menu_options_maxopt;
extern int * menu_options_maxsel;
