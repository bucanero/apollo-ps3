#include <apollo.h>
#include <dbglogger.h>
#define LOG dbglogger_log

#define PS3_TMP_PATH			"/dev_hdd0/tmp/"
#define APOLLO_PATH				"/dev_hdd0/game/NP0APOLLO/USRDIR/"
#define APOLLO_TMP_PATH			PS3_TMP_PATH "apollo/"
#define APOLLO_DATA_PATH		APOLLO_PATH "DATA/"
#define APOLLO_LOCAL_CACHE		APOLLO_PATH "CACHE/"
#define APOLLO_UPDATE_URL		"https://api.github.com/repos/bucanero/apollo-ps3/releases/latest"

#define MAX_USB_DEVICES         6
#define USB0_PATH               "/dev_usb000/"
#define USB1_PATH               "/dev_usb001/"
#define USB_PATH                "/dev_usb%03d/"
#define FAKE_USB_PATH           PS3_TMP_PATH "fakeusb/"
#define USER_PATH_HDD			"/dev_hdd0/home/%08d/"

#define PS3_EXPORT_PATH         "PS3/EXPORT/"
#define PS3_SAVES_PATH_USB      "PS3/SAVEDATA/"
#define PSP_SAVES_PATH_USB      "PSP/SAVEDATA/"
#define PS2_SAVES_PATH_USB      PS3_EXPORT_PATH "PS2SD/"
#define PSV_SAVES_PATH_USB      PS3_EXPORT_PATH "PSV/"
#define TROPHIES_PATH_USB       PS3_EXPORT_PATH "TROPHY/"

#define PS3_LICENSE_PATH        "exdata/"
#define PS3_SAVES_PATH_HDD      "savedata/"
#define PS2_SAVES_PATH_HDD      "ps2emu2_savedata/"
#define PSP_SAVES_PATH_HDD      "minis_savedata/"

#define PS1_IMP_PATH_USB        "PS1/SAVEDATA/"
#define PS2_IMP_PATH_USB        "PS2/SAVEDATA/"

#define SAVES_PATH_HDD          USER_PATH_HDD PS3_SAVES_PATH_HDD
#define TROPHY_PATH_HDD         USER_PATH_HDD "trophy/"
#define EXDATA_PATH_HDD         USER_PATH_HDD PS3_LICENSE_PATH

#define EXPORT_RAP_PATH_USB     USB_PATH PS3_LICENSE_PATH
#define EXPORT_RAP_PATH_HDD     "/dev_hdd0/" PS3_LICENSE_PATH

#define VMC_PS1_PATH_USB        "PS1/VMC/"
#define VMC_PS2_PATH_USB        "PS2/VMC/"
#define VMC_PS2_PATH_HDD        "/dev_hdd0/savedata/vmc/"

#define IMP_PS2VMC_PATH_USB     USB_PATH VMC_PS2_PATH_USB
#define IMPORT_RAP_PATH_USB     USB_PATH PS3_LICENSE_PATH

#define PS2ISO_PATH             "PS2ISO/"
#define PS2ISO_PATH_USB         USB_PATH PS2ISO_PATH
#define PS2ISO_PATH_HDD         "/dev_hdd0/" PS2ISO_PATH

#define ONLINE_URL				"https://bucanero.github.io/apollo-saves/"
#define ONLINE_PATCH_URL		"https://bucanero.github.io/apollo-patches/PS3/"
#define ONLINE_CACHE_TIMEOUT    24*3600     // 1-day local cache

#define OWNER_XML_FILE          "owners.xml"

enum storage_enum
{
    STORAGE_USB0,
    STORAGE_USB1,
    STORAGE_USB2,
    STORAGE_USB3,
    STORAGE_USB4,
    STORAGE_USB5,
    STORAGE_USB6,
    STORAGE_HDD = 0x10,
};

enum cmd_code_enum
{
    CMD_CODE_NULL,

// Trophy commands
    CMD_RESIGN_TROPHY,
    CMD_EXP_TROPHY_USB,
    CMD_COPY_TROPHIES_USB,
    CMD_COPY_ALL_TROP_USB,
    CMD_ZIP_TROPHY_USB,
    CMD_IMP_TROPHY_HDD,

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
    CMD_VIEW_RAW_PATCH,
    CMD_RESIGN_PSV,
    CMD_CONVERT_TO_PSV,
    CMD_IMPORT_DATA_FILE,
    CMD_HEX_EDIT_FILE,
    CMD_DELETE_SAVE,
    CMD_UPLOAD_SAVE,

// Bulk commands
    CMD_RESIGN_SAVES,
    CMD_RESIGN_ALL_SAVES,
    CMD_COPY_SAVES_USB,
    CMD_COPY_SAVES_HDD,
    CMD_COPY_ALL_SAVES_USB,
    CMD_COPY_ALL_SAVES_HDD,
    CMD_SAVE_WEB_SERVER,
    CMD_RESIGN_VMP,
    CMD_EXP_SAVES_VMC,
    CMD_EXP_ALL_SAVES_VMC,

// Export commands
    CMD_EXP_EXDATA_USB,
    CMD_EXP_LICS_RAPS,
    CMD_EXP_FLASH2_USB,
    CMD_EXP_PS2_BINENC,
    CMD_EXP_PS2_VM2,
    CMD_EXP_SAVE_PSV,
    CMD_EXP_VM2_RAW,
    CMD_EXP_VMC1SAVE,
    CMD_EXP_VMC2SAVE,
    CMD_EXP_VMP2MCR,

// Import commands
    CMD_IMP_EXDATA_USB,
    CMD_IMP_PS2_VM2,
    CMD_IMP_PS2_ISO,
    CMD_IMP_PS2_CONFIG,
    CMD_IMP_PS2VMC_USB,
    CMD_IMP_VMC1SAVE,
    CMD_IMP_VMC2SAVE,
    CMD_IMP_MCR2VMP,
    CMD_CREATE_ACT_DAT,
    CMD_EXTRACT_ARCHIVE,
    CMD_URL_DOWNLOAD,
    CMD_NET_WEBSERVER,

// SFO patches
    SFO_UNLOCK_COPY,
    SFO_CHANGE_ACCOUNT_ID,
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
#define SAVE_FLAG_HDD           64
#define SAVE_FLAG_TROPHY        128
#define SAVE_FLAG_ONLINE        256
#define SAVE_FLAG_SELECTED      512
#define SAVE_FLAG_VMC           1024
#define SAVE_FLAG_UPDATED       2048

enum save_type_enum
{
    FILE_TYPE_NULL,
    FILE_TYPE_PS1,
    FILE_TYPE_PS2,
    FILE_TYPE_PS3,
    FILE_TYPE_PSV,
    FILE_TYPE_TRP,
    FILE_TYPE_VMC,
    FILE_TYPE_MENU,

    // PS1 File Types
    FILE_TYPE_PSX,
    FILE_TYPE_MCS,

    // PS2 File Types
    FILE_TYPE_PSU,
    FILE_TYPE_MAX,
    FILE_TYPE_CBS,
    FILE_TYPE_XPS,
    FILE_TYPE_PS2RAW,

    // License Files
    FILE_TYPE_RIF,
    FILE_TYPE_RAP,
    FILE_TYPE_ACT,

    FILE_TYPE_ZIP,
    FILE_TYPE_NET,
    // ISO Files
    FILE_TYPE_ISO,
    FILE_TYPE_BINENC,
};

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
    CHAR_TAG_NET,
    CHAR_RES_LF,
    CHAR_TAG_VMC,
    CHAR_TAG_ZIP,
    CHAR_RES_CR,
    CHAR_TAG_TRANSFER,
    CHAR_TAG_WARNING,
    CHAR_BTN_X,
    CHAR_BTN_S,
    CHAR_BTN_T,
    CHAR_BTN_O,
    CHAR_TRP_BRONZE,
    CHAR_TRP_SILVER,
    CHAR_TRP_GOLD,
    CHAR_TRP_PLATINUM,
    CHAR_TRP_SYNC,
};

enum code_type_enum
{
    PATCH_NULL,
    PATCH_GAMEGENIE = APOLLO_CODE_GAMEGENIE,
    PATCH_BSD = APOLLO_CODE_BSD,
    PATCH_COMMAND,
    PATCH_SFO,
    PATCH_TROP_UNLOCK,
    PATCH_TROP_LOCK,
};

enum save_sort_enum
{
    SORT_DISABLED,
    SORT_BY_NAME,
    SORT_BY_TITLE_ID,
    SORT_BY_TYPE,
};

typedef struct save_entry
{
    char * name;
    char * title_id;
    char * dir_name;
    char * path;
    uint32_t blocks;
    uint16_t flags;
    uint16_t type;
    list_t * codes;
} save_entry_t;

typedef struct
{
    list_t * list;
    char path[128];
    char* title;
    uint8_t icon_id;
    void (*UpdatePath)(char *);
    int (*ReadCodes)(save_entry_t *);
    list_t* (*ReadList)(const char*);
} save_list_t;

list_t * ReadUserList(const char* userPath);
list_t * ReadUsbList(const char* userPath);
list_t * ReadOnlineList(const char* urlPath);
list_t * ReadBackupList(const char* userPath);
list_t * ReadTrophyList(const char* userPath);
list_t * ReadVmc1List(const char* userPath);
list_t * ReadVmc2List(const char* userPath);
void UnloadGameList(list_t * list);
char * readTextFile(const char * path, long* size);
int sortSaveList_Compare(const void* A, const void* B);
int sortSaveList_Compare_Type(const void* A, const void* B);
int sortSaveList_Compare_TitleID(const void* A, const void* B);
int sortCodeList_Compare(const void* A, const void* B);
int ReadCodes(save_entry_t * save);
int ReadTrophies(save_entry_t * game);
int ReadOnlineSaves(save_entry_t * game);
int ReadBackupCodes(save_entry_t * bup);
int ReadVmc1Codes(save_entry_t * save);
int ReadVmc2Codes(save_entry_t * save);

int http_init(void);
void http_end(void);
int http_download(const char* url, const char* filename, const char* local_dst, int show_progress);
int ftp_upload(const char* local_file, const char* url, const char* filename, int show_progress);

int extract_rar(const char* rar_file, const char* dest_path);
int extract_7zip(const char* zip_file, const char* dest_path);
int extract_zip(const char* zip_file, const char* dest_path);
int zip_directory(const char* basedir, const char* inputdir, const char* output_zipfile);
int zip_savegame(const char* basedir, const char* inputdir, const char* output_zipfile);
int zip_file(const char* input, const char* output_zipfile);
int extract_sfo(const char* zip_file, const char* dest_path);

int show_dialog(int dialog_type, const char * format, ...);
int osk_dialog_get_text(const char* title, char* text, uint32_t size);
void init_progress_bar(const char* progress_bar_title, const char* msg);
void update_progress_bar(uint64_t progress, const uint64_t total_size, const char* msg);
void end_progress_bar(void);
#define show_message(...)	show_dialog(DIALOG_TYPE_OK, __VA_ARGS__)

int init_loading_screen(const char* msg);
void stop_loading_screen(void);

void execCodeCommand(code_entry_t* code, const char* codecmd);

int patch_trophy_account(const char* trp_path, const char* account_id);
int apply_trophy_patch(const char* trp_path, uint32_t trophy_id, int unlock);

int rif2rap(const uint8_t* idps_key, const char* lic_path, const char* rifFile, const char* rap_path);
int rap2rif(const uint8_t* idps_key, const char* exdata_path, const char* rap_file, const char *rif_path);
int rif2klicensee(const uint8_t* idps_key, const char* exdata_path, const char* rif_file, uint8_t* klic);
int create_actdat(const char* exdata_path, uint64_t account_id);
uint64_t create_fake_account(uint32_t user_id, uint64_t account_id);
uint64_t get_account_id(uint32_t user_id);

int create_savegame_folder(const char* folder, const char* path);

void ps2_encrypt_image(uint8_t cfg_file, const char* image_name, const char* data_file);
void ps2_decrypt_image(uint8_t dex_mode, const char* image_name, const char* data_file);
void ps2_crypt_vmc(uint8_t dex_mode, const char* vmc_path, const char* vmc_out, int crypt_mode);
int ps2_add_vmc_ecc(const char* src, const char* dst);
int psv_resign(const char *src_psv);
int vmp_resign(const char *src_vmp);

int ps1_mcs2psv(const char* save, const char* psv_path);
int ps1_psx2psv(const char* save, const char* psv_path);
int ps2_psu2psv(const char *save, const char* psv_path);
int ps2_max2psv(const char *save, const char* psv_path);
int ps2_cbs2psv(const char *save, const char *psv_path);
int ps2_xps2psv(const char *save, const char *psv_path);
int ps1_psv2mcs(const char* save, const char* mcs_path);
int ps2_psv2psu(const char *save, const char* psu_path);

int vmc_export_psv(const char* save, const char* out_path);
int vmc_export_psu(const char* path, const char* output);
int vmc_import_psv(const char *input);
int vmc_import_psu(const char *input);
int vmc_delete_save(const char* path);

char* get_xml_title_name(const char *xmlfile);
char* sjis2utf8(char* input);
uint8_t* getIconPS2(const char* folder, const char* iconfile);
