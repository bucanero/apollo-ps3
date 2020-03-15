#include <stdio.h>
#include <string.h>
#include <ppu-lv2.h>
#include <sys/systime.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <dirent.h>
#include <zlib.h>

#include "common.h"

#define FS_S_IFMT 0170000

#define TMP_BUFF_SIZE 64*1024

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
//COBRA/MAMBA
//----------------------------------------

#define SYSCALL8_OPCODE_GET_VERSION         0x7000ULL
#define SYSCALL8_OPCODE_GET_MAMBA           0x7FFFULL

int sys8_get_version(u32 *version)
{
    lv2syscall2(8, SYSCALL8_OPCODE_GET_VERSION, (u64)version);
    return_to_user_prog(int);
}

int sys8_get_mamba(void)
{
    lv2syscall1(8, SYSCALL8_OPCODE_GET_MAMBA);
    return_to_user_prog(int);
}

int is_cobra(void)
{
    u32 version = 0x99999999;
    if (sys8_get_version(&version) < 0) return FAILED;
    if (version != 0x99999999 && sys8_get_mamba() != 0x666) return SUCCESS;
    return FAILED;
}

int is_mamba(void)
{
    u32 version = 0x99999999;
    if (sys8_get_version(&version) < 0) return FAILED;
    if (version != 0x99999999 && sys8_get_mamba() == 0x666) return SUCCESS;
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

int unlink_secure(void *path)
{   
    if(file_exists(path)==SUCCESS)
    {
        sysLv2FsChmod(path, FS_S_IFMT | 0777);
        return sysLv2FsUnlink(path);
		//return remove(path);
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
            if (mkdir(path, 0777) < 0)
                return FAILED;
        
        *ptr++ = last;
        if (last == 0)
            break;

    }

    return SUCCESS;
}

int copy_file(const char* input, const char* output)
{
    size_t read;

    mkdirs(output);
    FILE* in = fopen(input, "rb");
    FILE* out = fopen(output, "wb");
    
    if (!in || !out)
        return FAILED;

    char* buffer = malloc(TMP_BUFF_SIZE);

    do
    {
        read = fread(buffer, 1, TMP_BUFF_SIZE, in);
        if (fwrite(buffer, read, 1, out) == 0)
            break;
    }
    while (read == TMP_BUFF_SIZE);

    free(buffer);
    fclose(out);
    fclose(in);

    return SUCCESS;
}

uint32_t file_crc32(const char* input)
{
    char buffer[TMP_BUFF_SIZE];
    uLong crc = crc32_z(0L, Z_NULL, 0);
    size_t read;

    FILE* in = fopen(input, "rb");
    
    if (!in)
        return FAILED;

    do
    {
        read = fread(buffer, 1, TMP_BUFF_SIZE, in);
        crc = crc32_z(crc, (u8*)buffer, read);
    }
    while (read == TMP_BUFF_SIZE);

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

  			if (dir_exists(fullname) == SUCCESS) {
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

//----------------------------------------
//POWER UTILS
//----------------------------------------


int sys_shutdown()
{   
    unlink_secure("/dev_hdd0/tmp/turnoff");
    
    lv2syscall4(379,0x1100,0,0,0);
    return_to_user_prog(int);
}

int sys_reboot()
{
    unlink_secure("/dev_hdd0/tmp/turnoff");

    lv2syscall4(379,0x1200,0,0,0);
    return_to_user_prog(int);
}
