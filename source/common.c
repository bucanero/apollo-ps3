#include <stdio.h>
#include <string.h>
#include <ppu-lv2.h>
#include <sys/systime.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <dirent.h>
#include <zlib.h>
#include <sysutil/sysutil.h>

#include "common.h"

#define TMP_BUFF_SIZE 0x20000

//----------------------------------------
//String Utils
//----------------------------------------
int is_char_integer(char c)
{
	if (c >= '0' && c <= '9')
		return SUCCESS;
	return FAILED;
}

int is_char_letter(char c)
{
	if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
		return SUCCESS;
	return FAILED;
}

//----------------------------------------
//FILE UTILS
//----------------------------------------

int file_exists(const char *path)
{
    struct stat sb;
    if ((stat(path, &sb) == 0) && S_ISREG(sb.st_mode)) {
    	return SUCCESS;
    }
    
	return FAILED;
}

int dir_exists(const char *path)
{
    struct stat sb;
    if ((stat(path, &sb) == 0) && S_ISDIR(sb.st_mode)) {
        return SUCCESS;
    }
    return FAILED;
}

int unlink_secure(const char *path)
{   
    if(file_exists(path)==SUCCESS)
    {
        sysLv2FsChmod(path, S_IFMT | 0777);
        return sysLv2FsUnlink(path);
		//return remove(path);
    }
    else if (dir_exists(path)==SUCCESS)
    {
        sysLv2FsChmod(path, 0777);
        return sysLv2FsRmdir(path);
    }
    return FAILED;
}

/*
* Creates all the directories in the provided path. (can include a filename)
* (directory must end with '/')
*/
int mkdirs(const char* dir)
{
    char path[256];
    snprintf(path, sizeof(path), "%s", dir);

    char* ptr = strrchr(path, '/');
    *ptr = 0;
    ptr = path;
    ptr++;
    while (*ptr)
    {
        while (*ptr && *ptr != '/')
            ptr++;

        char last = *ptr;
        *ptr = 0;

        if (dir_exists(path) == FAILED)
        {
            if (mkdir(path, 0777) < 0)
                return FAILED;
            else
                sysLv2FsChmod(path, S_IFDIR | 0777);
        }
        
        *ptr++ = last;
        if (last == 0)
            break;

    }

    return SUCCESS;
}

int copy_file(const char* input, const char* output)
{
    u64 read, written;
    s32 fd, fd2;

    if (mkdirs(output) != SUCCESS)
        return FAILED;

    if(sysLv2FsOpen(input, 0, &fd, SYS_O_RDONLY, NULL, 0) != SUCCESS)
        return FAILED;

    if(sysLv2FsOpen(output, SYS_O_WRONLY | SYS_O_CREAT | SYS_O_TRUNC, &fd2, 0777, NULL, 0) != SUCCESS)
    {
        sysLv2FsClose(fd);
        return FAILED;
    }

    char* buffer = malloc(TMP_BUFF_SIZE);

    if (!buffer)
        return FAILED;

    do
    {
        sysLv2FsRead(fd, buffer, TMP_BUFF_SIZE, &read);
        sysLv2FsWrite(fd2, buffer, read, &written);
    }
    while ((read == written) && (read == TMP_BUFF_SIZE));

    free(buffer);
    sysLv2FsClose(fd);
    sysLv2FsClose(fd2);
    sysLv2FsChmod(output, S_IFMT | 0777);

    return (read - written);
}

int file_chmod(const char* path)
{
    return sysLv2FsChmod(path, S_IFMT | 0777);
}

uint32_t file_crc32(const char* input)
{
    Bytef *buffer;
    uLong crc = crc32_z(0L, Z_NULL, 0);
    size_t read;

    FILE* in = fopen(input, "rb");
    if (!in)
        return FAILED;

    buffer = malloc(TMP_BUFF_SIZE);
    do
    {
        read = fread(buffer, 1, TMP_BUFF_SIZE, in);
        crc = crc32_z(crc, buffer, read);
    }
    while (read == TMP_BUFF_SIZE);

    free(buffer);
    fclose(in);

    return crc;
}

int copy_directory(const char* startdir, const char* inputdir, const char* outputdir)
{
	char fullname[256];
    char out_name[256];
	struct dirent *dirp;
	int len = strlen(startdir);
	DIR *dp = opendir(inputdir);

	if (!dp) {
		return FAILED;
	}

	while ((dirp = readdir(dp)) != NULL) {
		if ((strcmp(dirp->d_name, ".")  != 0) && (strcmp(dirp->d_name, "..") != 0)) {
  			snprintf(fullname, sizeof(fullname), "%s%s", inputdir, dirp->d_name);

  			if (dirp->d_type == DT_DIR) {
                strcat(fullname, "/");
    			copy_directory(startdir, fullname, outputdir);
  			} else {
  			    snprintf(out_name, sizeof(out_name), "%s%s", outputdir, &fullname[len]);
    			if (copy_file(fullname, out_name) != SUCCESS) {
     				return FAILED;
    			}
  			}
		}
	}
	closedir(dp);

    return SUCCESS;
}

int clean_directory(const char* inputdir, const char* filter)
{
	DIR *d;
	struct dirent *dir;
	char dataPath[256];

	d = opendir(inputdir);
	if (!d)
		return FAILED;

	while ((dir = readdir(d)) != NULL)
	{
		if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0 && strstr(dir->d_name, filter) != NULL)
		{
			snprintf(dataPath, sizeof(dataPath), "%s" "%s", inputdir, dir->d_name);
			unlink_secure(dataPath);
		}
	}
	closedir(d);

    return SUCCESS;
}

//----------------------------------------
//POWER UTILS
//----------------------------------------

int sys_shutdown(void)
{   
    unlink_secure("/dev_hdd0/tmp/turnoff");
    
    lv2syscall4(379,0x1100,0,0,0);
    return_to_user_prog(int);
}

int sys_reboot(void)
{
    unlink_secure("/dev_hdd0/tmp/turnoff");

    lv2syscall4(379,0x1200,0,0,0);
    return_to_user_prog(int);
}

const char * get_user_language(void)
{
    int language;

    if(sysUtilGetSystemParamInt(SYSUTIL_SYSTEMPARAM_ID_LANG, &language) < 0)
        return "en";

    switch (language)
    {
    case SYSUTIL_LANG_JAPANESE:             //  0   Japanese
        return "ja";

    case SYSUTIL_LANG_ENGLISH_US:           //  1   English (United States)
    case SYSUTIL_LANG_ENGLISH_GB:           // 18   English (United Kingdom)
        return "en";

    case SYSUTIL_LANG_FRENCH:               //  2   French
        return "fr";

    case SYSUTIL_LANG_SPANISH:              //  3   Spanish
        return "es";

    case SYSUTIL_LANG_GERMAN:               //  4   German
        return "de";

    case SYSUTIL_LANG_ITALIAN:              //  5   Italian
        return "it";

    case SYSUTIL_LANG_DUTCH:                //  6   Dutch
        return "nl";

    case SYSUTIL_LANG_RUSSIAN:              //  8   Russian
        return "ru";

    case SYSUTIL_LANG_KOREAN:               //  9   Korean
        return "ko";

    case SYSUTIL_LANG_CHINESE_T:            // 10   Chinese (traditional)
    case SYSUTIL_LANG_CHINESE_S:            // 11   Chinese (simplified)
        return "zh";

    case SYSUTIL_LANG_FINNISH:              // 12   Finnish
        return "fi";

    case SYSUTIL_LANG_SWEDISH:              // 13   Swedish
        return "sv";

    case SYSUTIL_LANG_DANISH:               // 14   Danish
        return "da";

    case SYSUTIL_LANG_NORWEGIAN:            // 15   Norwegian
        return "no";

    case SYSUTIL_LANG_POLISH:               // 16   Polish
        return "pl";

    case SYSUTIL_LANG_PORTUGUESE_PT:        //  7   Portuguese (Portugal)
    case SYSUTIL_LANG_PORTUGUESE_BR:        // 17   Portuguese (Brazil)
        return "pt";

    case SYSUTIL_LANG_TURKISH:              // 19   Turkish
        return "tr";

    default:
        break;
    }

    return "en";
}

//----------------------------------------
//CONSOLE ID UTILS
//----------------------------------------

#define SYS_SS_APPLIANCE_INFO_MANAGER                  867
#define SYS_SS_GET_OPEN_PSID                           872
#define AIM_GET_DEVICE_ID                              0x19003
#define AIM_GET_OPEN_PSID                              0x19005
#define SYSCALL8_OPCODE_IS_HEN                         0x1337

int sys_ss_get_open_psid(uint64_t psid[2])
{
	lv2syscall1(SYS_SS_GET_OPEN_PSID, (uint64_t) psid);
	return_to_user_prog(int);
}

static int sys_ss_appliance_info_manager(u32 packet_id, u64 arg)
{
	lv2syscall2(SYS_SS_APPLIANCE_INFO_MANAGER, (uint64_t)packet_id, (uint64_t)arg);
	return_to_user_prog(int);
}

int ss_aim_get_device_id(uint8_t *idps)
{
	return sys_ss_appliance_info_manager(AIM_GET_DEVICE_ID, (uint64_t)idps);
}

int ss_aim_get_open_psid(uint8_t *psid)
{
	return sys_ss_appliance_info_manager(AIM_GET_OPEN_PSID, (uint64_t)psid);
}

static int sys8_get_hen(void)
{
    lv2syscall1(8, SYSCALL8_OPCODE_IS_HEN);
    return_to_user_prog(int);
}

int is_ps3hen(void)
{
    return (sys8_get_hen() == 0x1337);
}
