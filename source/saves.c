#include "saves.h"
#include "common.h"
#include "sfo.h"
#include "settings.h"
#include "util.h"

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>
#include <sysutil/video.h>
#include <time.h>
#include <dirent.h>

#include <sys/stat.h>


/*
 * Function:		stripExt()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Removes the extension from a filename
 * Arguments:		
 *	filename:		String	
 * Return:			Pointer (as char *) to the resulting string
 */
char * stripExt(const char * filename)
{
	int len = strlen(filename);
	
	while (filename[len] != '.' && len > 0)
		len--;
	
	if (len == 0)
	{
		char * r = (char *)malloc(strlen(filename));
		memcpy(r, filename, strlen(filename));
		r[strlen(filename)] = 0;
		return r;
	}
	else
	{
		char * b = (char *)malloc(len);
		memcpy(b, filename, len);
		b[len] = 0;
		return b;
	}
}

/*
 * Function:		isExist()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Whether a path exists as a file or directory
 * Arguments:
 *	path:			Path to file/dir
 * Return:			1 if true, 0 if false
 */
int isExist(const char* path)
{
	int isf = file_exists(path) == SUCCESS;
	int isd = dir_exists(path) == SUCCESS;
	
	return isf | isd;
}

/*
 * Function:		getFileSize()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Removes the extension from a filename
 * Arguments:
 *	path:			Path to file
 * Return:			Size of file as long (== 0 if failed)
 */
long getFileSize(const char * path)
{
	FILE *f = fopen(path, "rb");
	if (f == NULL)
		return 0;
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);
	
	fclose(f);
	
	return fsize;
}

/*
 * Function:		EndsWith()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Checks to see if a ends with b
 * Arguments:
 *	a:				String
 *	b:				Potential end
 * Return:			1 if true, 0 if false
 */
int EndsWith(char * a, char * b)
{
	int al = strlen(a), bl = strlen(b);
	
	if (al < bl)
		return 0;
	if (al == bl)
		return !strcmp(a, b);
	
	int x = al - bl;
	for (; x < al; x++)
		if (a[x] != b[(x + bl) - al])
			return 0;
	
	return 1;
}

/*
 * Function:		getDirListSize()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Calculates the number of save files in a directory
 * Arguments:
 *	path:			path to directory
 * Return:			Number of results
 */
long getDirListSize(const char * path)
{
	DIR *d;
	struct dirent *dir;
	d = opendir(path);
	
	char sfoPath[256];
	int count = 0;
	
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0)
			{
				snprintf(sfoPath, sizeof(sfoPath), "%s%s/PARAM.SFO", path, dir->d_name);
				if (file_exists(sfoPath) == SUCCESS)
				{
					count++;
				}
			}
		}
		closedir(d);
	}
	
	return count;
}

/*
 * Function:		removeAtIndex()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		removes a single char from a string at index i
 * Arguments:
 *	str:			String
 *	i:				Index
 * Return:			void
 */
void removeAtIndex(char * str, int i)
{
	memmove(str+i, str+i+1, strlen(str) - i);
}

/*
 * Function:		readFileBuffered()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Reads the specified file into a buffer
 * Arguments:
 *	path:			Path to file
 *	buffer:			Buffer to store file in (file size should be grabbed and allocated before calling readFileBuffered())
 * Return:			void
 */
void readFileBuffered(const char * path, char * buffer)
{
	FILE *f = fopen(path, "rb");
	if (f == NULL)
		return;
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);
	if (fsize <= 0)
		return;
	
	char * string = malloc(fsize + 1);
	fread(string, fsize, 1, f);
	fclose(f);

	string[fsize] = 0;
	
	memcpy(buffer, string, fsize);
	free(string);
	
	//printf("\nBefore:\n%s\n\nAfter:\n", buffer);
	
	//Remove all '\r'
	int c = 0, len = fsize;
	for (c = 0; c < len; c++)
	{
		if (buffer[c] == '\r')
		{
			//printf ("\\r found at %d\n", c);
			removeAtIndex(buffer, c);
			c--; len--;
			//buffer[c] = '\n';
		}
		if (buffer[c] == '\n' && buffer[c+1] == '\n')
		{
			removeAtIndex(buffer, c);
			c--; len--;
		}
	}
	
	//writeFile("/dev_hdd0/tmp/art2.txt", "\r\r\n\n", buffer);
	
	buffer[len] = 0;
	//printf("%s\n\n", buffer);
}

/*
 * Function:		writeFile()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Writes the contents or a and b to the specified file
 * Arguments:
 *	path:			Path to file
 *	a:				File to be written
 *	b:				Written right after a (no newlines or anything)
 * Return:			void
 */
void writeFile(const char * path, char * a, char * b)
{
	FILE *fp;

	fp = fopen(path, "w");
	if(fp == NULL)
		return;
	fprintf(fp, "%s%s", a, b);
	fclose(fp);
}

/*
 * Function:		readFile()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		reads the contents of a file into a new buffer
 * Arguments:
 *	path:			Path to file
 * Return:			Pointer to the newly allocated buffer
 */
char * readFile(const char * path, long* size)
{
	FILE *f = fopen(path, "rb");

	if (!f)
		return NULL;

	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);
	if (fsize <= 0)
	{
		fclose(f);
		return NULL;
	}

	char * string = malloc(fsize + 1);
	fread(string, fsize, 1, f);
	fclose(f);

	*size = fsize;
	return string;
}

/*
 * Function:		findNextLine()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Finds the next instance of '\n' and returns the index
 * Arguments:
 *	buffer:			String
 *	start:			Index to begin searching at
 * Return:			Index of found '\n' or, if none found, the strlen of buffer
 */
int findNextLine(char * buffer, int start)
{
	int len = 0, max = strlen(buffer);
	for (len = start; len < max; len++)
	{
        if (buffer[len] == '\n' || buffer[len] == '\r')
			break;
	}
	return len;
}

/*
 * Function:		findNextTag()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Finds the next instance of "[Z" and returns the index
 * Arguments:
 *	buffer:			String
 *	start:			Index to begin searching at
 * Return:			Index of found "[Z" or, if none found, the strlen of buffer
 */
int findNextTag(char * buffer, int start)
{
	int len = 0;
	for (len = start; len < strlen(buffer); len++)
	{
		if (buffer[len] == '[' && buffer[len+1] == 'Z')
			break;
	}
	return len;
}

/*
 * Function:		findNextEndTag()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Finds the next instance of "[/Z" and returns the index
 * Arguments:
 *	buffer:			String
 *	start:			Index to begin searching at
 * Return:			Index of found "[/Z" or, if none found, the strlen of buffer
 */
int findNextEndTag(char * buffer, int start)
{
	int len = 0;
	for (len = start; len < strlen(buffer); len++)
	{
		if (buffer[len] == '[' && buffer[len+1] == '/' && buffer[len+2] == 'Z')
			break;
	}
	return len;
}

/*
 * Function:		countTags()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Counts the number of tags in a string
 * Arguments:
 *	buffer:			String
 * Return:			Returns the number of tags in a string
 */
int countTags(char * buffer)
{
	int len = strlen(buffer);
	int cnt = 0, x = -1;
	
	while (x < len)
	{
		x = findNextTag(buffer, x+1);
		x = findNextEndTag(buffer, x);
		if (x < len)
			cnt++;
	}
	
	return cnt;
}

/*
 * Function:		findNextBreak()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Finds the next instance of '#' or '}' and returns the index
 * Arguments:
 *	buffer:			String
 *	start:			Index to begin searching at
 * Return:			Index of found '#'/'}' or, if none found, the strlen of buffer
 */
int findNextBreak(char * buffer, int start)
{
	int len = 0, max = strlen(buffer);
	for (len = start; len < max; len++)
	{
        if (buffer[len] == ':' || buffer[len] == '[')
			break;
	}
	return len;
}

/*
 * Function:		ReadOptionFromLine()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Reads all the options (characterized by the [Z..] [/Z..] tags from a given line
 * Arguments:
 *	line:			String
 *	options:		Allocated option_entry struct that will hold the resulting option after the function processes line
 *	ind:			Index of which option to write to within options
 * Return:			Returns the number of options (and the options themselves within the arg options)
 */
int ReadOptionFromLine(char * line, option_entry_t * options, int ind)
{
	options[ind].line = line;
	
	int x = 0, len = strlen(line), c = 0, i = 0;
	
	while (x < len)
	{
		if (line[x] == '=')
			c++;
		x++;
	}
	
	options[ind].value = malloc (sizeof(char *) * c);
	options[ind].name = malloc (sizeof(char *) * c);
	memset(options[ind].value, 0, (sizeof(char *) * c));
	memset(options[ind].name, 0, (sizeof(char *) * c));
	
	x = 0;
	while (x < len)
	{
		if (x >= len)
			break;
		int oldX = x;
		
		while (x < len && line[x] != '=' && line[x] != ';')
			x++;
		
		if (line[x] == '=')
			options[ind].value[i] = (char*)&line[oldX];
		else
		{
			options[ind].name[i] = (char*)&line[oldX];
			i++;
		}
		
		line[x] = 0;
		
		x++;
	}
	
	return c;
}

/*
 * Function:		ReadOptions()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Reads the options (characterized by [Z..] [/Z..] tags from a given code
 * Arguments:
 *	code:			Code to process
 *	count:			Pointer to int (set to the number of tags within the code)
 * Return:			Returns an array of option_entry and the count at *count
 */
option_entry_t * ReadOptions(code_entry_t code, int * count)
{
	if (code.name == NULL || code.codes == NULL)
		return NULL;
	
	int len = strlen(code.codes);
	int x = 0, y = 0, cnt = 0, c = 0;
	char * buffer = code.codes;
	
	cnt = countTags(buffer);
	
	if (cnt <= 0)
		return NULL;
	
	option_entry_t * ret = (option_entry_t *)malloc(sizeof(option_entry_t) * cnt);
	memset(ret, 0, sizeof(option_entry_t) * cnt);
	
	while (x < len && c < cnt)
	{
		//Find tags
		x = findNextTag(buffer, x) + 1;
		int id = 0;
		while (x < len && buffer[x] == 'Z') { id++; x++; }
		
		if (id <= 0)
			break;
		
		y = findNextEndTag(buffer, x);
		
		int xOff = (x + id - 1);
		char * line = malloc(y - xOff + 1);
		memcpy(line, &buffer[xOff], y - xOff);
		line[y - xOff] = 0;
		ret[c].size = ReadOptionFromLine(line, ret, c);
		ret[c].id = id;
		
		//free(line);
		
		c++;
	}
	
	*count = cnt;
	return ret;
}

void _setManualCode(code_entry_t* entry, uint8_t type, const char* name, const char* code)
{
	entry->type = type;
	entry->activated = 0;
	entry->options_count = 0;
	entry->options = NULL;
	asprintf(&entry->name, name);
	asprintf(&entry->codes, code);
}

option_entry_t* _initOptions(int count)
{
	option_entry_t* options = (option_entry_t*)malloc(sizeof(option_entry_t));

	options->id = 0;
	options->sel = -1;
	options->size = count;
	options->line = NULL;
	options->value = malloc (sizeof(char *) * count);
	options->name = malloc (sizeof(char *) * count);

	return options;
}

option_entry_t* _createOptions(int count, const char* name, const char* value)
{
	option_entry_t* options = _initOptions(count);

	asprintf(&options->name[0], "%s %d", name, 0);
	asprintf(&options->value[0], "%s%c", value, 0);
	asprintf(&options->name[1], "%s %d", name, 1);
	asprintf(&options->value[1], "%s%c", value, 1);

	return options;
}

void _remove_char(char * str, int len, char seek)
{
	int x;
	for (x = 0; x < len; x++)
		if (str[x] == seek)
			str[x] = '\n';
}

int _count_codes(char* buffer)
{
	int i=0;
    char *line = strtok(buffer, "\n");
        
    while (line)
    {
    	if (wildcard_match(line, "[*]"))
    		i++;

    	line = strtok(NULL, "\n");
    }
	return i;
}

// Expects buffer without CR's (\r)
void get_patch_code(char* buffer, int code_id, code_entry_t* entry)
{
	int i=0;
    char *tmp = NULL;
    char *res = calloc(1, 1);
    char *line = strtok(buffer, "\n");

    while (line)
    {
    	if (wildcard_match(line, "[*]") && (i++ == code_id))
    	{
			LOG("Reading patch code for '%s'...", line);
	    	line = strtok(NULL, "\n");

		    while (line)
		    {
		    	if ((wildcard_match(line, "; --- * ---")) 	||
		    		(wildcard_match(line, ":*"))			||
		    		(wildcard_match(line, "[*]"))			||
		    		(wildcard_match_icase(line, "PATH:*"))	||
		    		(wildcard_match_icase(line, "GROUP:*")))
		    	{
						break;
			    }

		    	if (!wildcard_match(line, ";*"))
		    	{
					asprintf(&tmp, "%s%s\n", res, line);
					free(res);
					res = tmp;

//			    	LOG("%s", line);
					if (!wildcard_match(line, "\?\?\?\?\?\?\?\? \?\?\?\?\?\?\?\?*"))
						entry->type = PATCH_BSD;
			    }
		    	line = strtok(NULL, "\n");
		    }
    	}
    	line = strtok(NULL, "\n");
    }

//	LOG("Result (%s)", res);
	entry->codes = res;
}

#define MENU_COPY_CMDS	3
void _add_commands(code_entry_t * code)
{
	int count = 0;

	_setManualCode(&code[count], PATCH_COMMAND, "\x0b Copy save game to USB", "");
	code[count].options_count = 1;
	code[count].options = _createOptions(2, "Copy to USB", CMD_COPY_SAVE_USB);
	count++;

	_setManualCode(&code[count], PATCH_COMMAND, "\x0c Export save game to Zip", "");
	code[count].options_count = 1;
	code[count].options = _createOptions(2, "Export Zip to USB", CMD_EXPORT_ZIP_USB);
	count++;

	_setManualCode(&code[count], PATCH_COMMAND, "\x0b Decrypt save game files", CMD_DECRYPT_FILE);
	code[count].options_count = 1;
	code[count].options = NULL;
	count++;
}

option_entry_t* _getFileOptions(const char* save_path, const char* mask)
{
	DIR *d;
	struct dirent *dir;
	int i = 0;
	option_entry_t* opt;

	d = opendir(save_path);
	if (!d)
		return NULL;

	LOG("Loading filenames {%s} from '%s'...", mask, save_path);

	while ((dir = readdir(d)) != NULL)
	{
		if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0 &&
			strcmp(dir->d_name, "SND0.AT3") != 0 &&
			strcmp(dir->d_name, "PIC1.PNG") != 0 &&
			strcmp(dir->d_name, "ICON0.PNG") != 0 &&
			strcmp(dir->d_name, "ICON1.PAM") != 0 &&
			strcmp(dir->d_name, "PARAM.PFD") != 0 &&
			strcmp(dir->d_name, "PARAM.SFO") != 0 &&
			wildcard_match_icase(dir->d_name, mask))
		{
			i++;
		}
	}
	closedir(d);

	if (i == 0)
		return NULL;

	opt = _initOptions(i);
	i = 0;

	d = opendir(save_path);
	while ((dir = readdir(d)) != NULL)
	{
		if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0 &&
			strcmp(dir->d_name, "SND0.AT3") != 0 &&
			strcmp(dir->d_name, "PIC1.PNG") != 0 &&
			strcmp(dir->d_name, "ICON0.PNG") != 0 &&
			strcmp(dir->d_name, "ICON1.PAM") != 0 &&
			strcmp(dir->d_name, "PARAM.PFD") != 0 &&
			strcmp(dir->d_name, "PARAM.SFO") != 0 &&
			wildcard_match_icase(dir->d_name, mask))
		{
			LOG("Adding '%s' (%s)", dir->d_name, mask);

			asprintf(&opt->name[i], "%s", dir->d_name);
			asprintf(&opt->value[i], "%s", mask);
			i++;
		}
	}

	closedir(d);

	return opt;
}

/*
 * Function:		ReadLocalCodes()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Reads an entire NCL file into an array of code_entry
 * Arguments:
 *	path:			Path to ncl
 *	_count_count:	Pointer to int (set to the number of codes within the ncl)
 * Return:			Returns an array of code_entry, null if failed to load
 */
int ReadCodes(save_entry_t * save)
{
    int code_count = 0, cheat_count = 0, cur_count = 0;
	code_entry_t * ret;
	option_entry_t * file_opt = NULL;
	char filePath[256];
	char group[128] = "";
	long bufferLen;
	char * buffer = NULL;

	snprintf(filePath, sizeof(filePath), APOLLO_DATA_PATH "%s.ps3savepatch", save->title_id);

	if (file_exists(filePath) == SUCCESS)
	{
		LOG("Loading BSD codes '%s'...", filePath);
		buffer = readFile(filePath, &bufferLen);
		buffer[bufferLen]=0;

		_remove_char(buffer, bufferLen, '\r');
		cheat_count = _count_codes(buffer);
	}

	code_count = 3 + cheat_count + (save->flags & SAVE_FLAG_LOCKED) + MENU_COPY_CMDS;
	ret = (code_entry_t *)calloc(1, sizeof(code_entry_t) * (code_count));

	save->code_count = code_count;
	save->codes = ret;

	_setManualCode(&ret[cur_count++], PATCH_COMMAND, "\x06 Apply changes & Resign", CMD_RESIGN_SAVE);
	_setManualCode(&ret[cur_count++], PATCH_SFO, "\x07 Remove Account ID", SFO_REMOVE_ACCOUNT_ID);
	_setManualCode(&ret[cur_count++], PATCH_SFO, "\x07 Remove Console ID", SFO_REMOVE_PSID);
	if (save->flags & SAVE_FLAG_LOCKED)
		_setManualCode(&ret[cur_count++], PATCH_SFO, "\x08 Remove copy protection", SFO_UNLOCK_COPY);

	_add_commands(&ret[cur_count]);
	cur_count += 2;
	ret[cur_count++].options = _getFileOptions(save->path, "*");

	if (cheat_count == 0)
	{
		if (buffer)
			free(buffer);

		return cur_count;
	}

	// remove 0x00 from previous strtok(...)
	_remove_char(buffer, bufferLen, '\0');
	char *line = strtok(buffer, "\n");
		
	while (line)
	{
		if (wildcard_match(line, ";*"))
		{
			if (wildcard_match(line, "; --- * ---"))
			{
				strcpy(group, line+6);
				group[strlen(group)-4] = 0;
				LOG("GROUP: %s\n", group);
			}
		}
		else if (wildcard_match(line, ":*"))
		{
			char* tmp_mask;

			strcpy(filePath, line+1);
			LOG("FILE: %s\n", filePath);

			if (strrchr(filePath, '\\'))
				tmp_mask = strrchr(filePath, '\\')+1;
			else
				tmp_mask = filePath;

			if (strchr(tmp_mask, '*'))
				file_opt = _getFileOptions(save->path, tmp_mask);
			else
				file_opt = NULL;

		}
		else if (wildcard_match(line, "?=*") ||
					wildcard_match(line, "\?\?=*") ||
					wildcard_match(line, "\?\?\?\?=*"))
		{
			// options
		}
		else if (wildcard_match_icase(line, "PATH:*"))
		{
			//
		}
		else if (wildcard_match_icase(line, "GROUP:*"))
		{
			strcpy(group, line+6);
			LOG("GROUP: %s\n", group);
		}
		else if (wildcard_match(line, "[*]"))
		{
			//LOG("Line: '%s'\n", line);
			if (wildcard_match_icase(line, "[DEFAULT:*"))
			{
				line += 6;
				line[1] = CHAR_TAG_WARNING;
				line[2] = ' ';
			}
			if (wildcard_match_icase(line, "[INFO:*"))
			{
				line += 3;
				line[1] = CHAR_TAG_WARNING;
				line[2] = ' ';
			}

			ret[cur_count].type = PATCH_GAMEGENIE;
			ret[cur_count].activated = wildcard_match_icase(line, "*(REQUIRED)*");
			ret[cur_count].codes = NULL;
			ret[cur_count].options = file_opt;
			ret[cur_count].options_count = (file_opt ? 1 : 0);
			asprintf(&ret[cur_count].file, "%s", filePath);

			if (strlen(group))
				asprintf(&ret[cur_count].name, "%s: %s", group, line+1);
			else
				asprintf(&ret[cur_count].name, "%s", line+1);

			*strrchr(ret[cur_count].name, ']') = 0;

			cur_count++;
		}

		line = strtok(NULL, "\n");
	}

	for (int x = code_count - cheat_count; x < cur_count; x++)
	{
		// remove 0x00 from previous strtok(...)
		_remove_char(buffer, bufferLen, '\0');
		get_patch_code(buffer, x - code_count + cheat_count, &ret[x]);

		LOG("Name: %s\nFile: %s\nCode (%d): %s\n", ret[x].name, ret[x].file, ret[x].type, ret[x].codes);
	}

	free (buffer);

	LOG("cur_count=%d,code_count=%d", cur_count, code_count);

	return cur_count;
}

/*
 * Function:		ReadOnlineSaves()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Downloads an entire NCL file into an array of code_entry
 * Arguments:
 *	filename:		File name ncl
 *	_count_count:	Pointer to int (set to the number of codes within the ncl)
 * Return:			Returns an array of code_entry, null if failed to load
 */
int ReadOnlineSaves(save_entry_t * game)
{ 
	char path[256], url[256];
	snprintf(path, sizeof(path), ONLINE_LOCAL_CACHE "%s.txt", game->title_id);
	snprintf(url, sizeof(url), ONLINE_URL "PS3/%s/", game->title_id);

	if (isExist(path))
	{
		struct stat stats;
		stat(path, &stats);
		// re-download if file is +1 day old
		if ((stats.st_mtime + ONLINE_CACHE_TIMEOUT) < time(NULL))
			http_download(url, "saves.txt", path, 0);
	}
	else
	{
		if (!http_download(url, "saves.txt", path, 0))
			return -1;
	}

	long fsize;
	char *data = readFile(path, &fsize);
	data[fsize] = 0;
	
	char *ptr = data;
	char *end = data + fsize + 1;

	int game_count = 0;

	while (ptr < end && *ptr)
	{
		if (*ptr == '\n')
		{
			game_count++;
		}
		ptr++;
	}
	
	if (!game_count)
		return -1;

	code_entry_t * ret = (code_entry_t *)malloc(sizeof(code_entry_t) * game_count);
	game->code_count = game_count;

	int cur_count = 0;
	ptr = data;
	
	while (ptr < end && *ptr && cur_count < game_count)
	{
		char* content = ptr;

		while (ptr < end && *ptr != '\n' && *ptr != '\r')
		{
			ptr++;
		}
		*ptr++ = 0;

        LOG("ReadUserList() :: Reading %s...", content);
		if (content[12] == '=')
		{
			ret[cur_count].activated = 0;
			asprintf(&ret[cur_count].codes, "%s/%.12s", game->title_id, content);

			content += 13;
			asprintf(&ret[cur_count].name, "%s", content);

			ret[cur_count].options_count = 1;
			ret[cur_count].options = _createOptions(2, "Download to USB", CMD_DOWNLOAD_USB);

			LOG("%d - [%s] %s", cur_count, ret[cur_count].codes, ret[cur_count].name);
			cur_count++;
		}

		if (ptr < end && *ptr == '\r')
		{
			ptr++;
		}
		if (ptr < end && *ptr == '\n')
		{
			ptr++;
		}
	}

	if (data) free(data);

	game->codes = ret;
	return cur_count;
}

int LoadBackupCodes(save_entry_t * bup)
{
	int bup_count = 3;

	code_entry_t * ret = (code_entry_t *)malloc(sizeof(code_entry_t) * bup_count);
	bup->code_count = bup_count;

	bup_count = 0;
	LOG("Loading backup commands...");

	_setManualCode(&ret[bup_count], PATCH_COMMAND, "\x0c Export Licenses to .Zip", "");
	ret[bup_count].options_count = 1;
	ret[bup_count].options = _createOptions(2, "Save .Zip to USB", CMD_EXP_EXDATA_USB);
	bup_count++;

	_setManualCode(&ret[bup_count], PATCH_COMMAND, "\x0b Export Trophies to USB", "");
	ret[bup_count].options_count = 1;
	ret[bup_count].options = _createOptions(2, "Save trophies to USB", CMD_EXP_TROPHY_USB);
	bup_count++;

	_setManualCode(&ret[bup_count], PATCH_COMMAND, "\x0b Copy all HDD saves to USB", "");
	ret[bup_count].options_count = 1;
	ret[bup_count].options = _createOptions(2, "Copy saves to USB", CMD_EXP_SAVES_USB);
	bup_count++;

/*
	_setManualCode(&ret[bup_count], "Import Licenses from USB", "");
	ret[bup_count].options_count = 1;
	ret[bup_count].options = _createOptions(2, "Import licenses.zip from USB", CMD_IMP_EXDATA_USB);
	bup_count++;

	_setManualCode(&ret[bup_count], "Import Trophies from USB", "");
	ret[bup_count].options_count = 1;
	ret[bup_count].options = _createOptions(2, "Import trophies from USB", CMD_IMP_TROPHY_USB);
	bup_count++;
*/

	LOG("%d commands loaded", bup_count);

	bup->codes = ret;
	return bup_count;
}

/*
 * Function:		UnloadGameList()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Free entire array of game_entry
 * Arguments:
 *	list:			Array of game_entry to free
 *	count:			number of game entries
 * Return:			void
 */
void UnloadGameList(save_entry_t * list, int count)
{
	if (list)
	{
		int x = 0, y = 0, z = 0;
		for (x = 0; x < count; x++)
		{
			if (list[x].name)
			{
				free(list[x].name);
				list[x].name = NULL;
			}

			if (list[x].path)
			{
				free(list[x].path);
				list[x].path = NULL;
			}

			if (list[x].title_id)
			{
				free(list[x].title_id);
				list[x].title_id = NULL;
			}
			
			if (list[x].codes)
			{
				for (y = 0; y < list[x].code_count; y++)
				{
					if (list[x].codes[y].codes)
					{
						free (list[x].codes[y].codes);
						list[x].codes[y].codes = NULL;
					}
					if (list[x].codes[y].name)
					{
						free (list[x].codes[y].name);
						list[x].codes[y].name = NULL;
					}
					if (list[x].codes[y].options && list[x].codes[y].options_count > 0)
					{
						for (z = 0; z < list[x].codes[y].options_count; z++)
						{
							if (list[x].codes[y].options[z].line)
								free(list[x].codes[y].options[z].line);
							if (list[x].codes[y].options[z].name)
								free(list[x].codes[y].options[z].name);
							if (list[x].codes[y].options[z].value)
								free(list[x].codes[y].options[z].value);
						}
						
						free (list[x].codes[y].options);
					}
				}
				
				free(list[x].codes);
				list[x].codes = NULL;
			}
		}
		
		free (list);
		list = NULL;
	}
	
	LOG("UnloadGameList() :: Successfully unloaded game list");
}

int qsortCodeList_Compare(const void* itemA, const void* itemB)
{
	code_entry_t* a = (code_entry_t*) itemA;
	code_entry_t* b = (code_entry_t*) itemB;

	if (!a->name || !b->name)
		return 0;

	return strcasecmp(a->name, b->name);
}

/*
 * Function:		qsortSaveList_Compare()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Compares two game_entry for QuickSort
 * Arguments:
 *	a:				First code
 *	b:				Second code
 * Return:			1 if greater, -1 if less, or 0 if equal
 */
int qsortSaveList_Compare(const void* a, const void* b)
{
	return strcasecmp(((save_entry_t*) a)->name, ((save_entry_t*) b)->name);
}

/*
 * Function:		ReadUserList()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Reads the entire userlist folder into a game_entry array
 * Arguments:
 *	gmc:			Set as the number of games read
 * Return:			Pointer to array of game_entry, null if failed
 */
save_entry_t * ReadUserList(const char* userPath, int * gmc)
{
	int save_count = getDirListSize(userPath);
	*gmc = save_count;

	if (!save_count)
		return NULL;

	DIR *d;
	struct dirent *dir;
	d = opendir(userPath);
	
	if (!d)
		return NULL;

	save_entry_t * ret = (save_entry_t *)malloc(sizeof(save_entry_t) * save_count);

	char sfoPath[256];
	int cur_count = 0;

	while ((dir = readdir(d)) != NULL)
	{
		if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
			continue;

		snprintf(sfoPath, sizeof(sfoPath), "%s%s/PARAM.SFO", userPath, dir->d_name);
		if (file_exists(sfoPath) == SUCCESS)
		{
			LOG("ReadUserList() :: Reading %s...", dir->d_name);

			sfo_context_t* sfo = sfo_alloc();
			if (sfo_read(sfo, sfoPath) < 0) {
				LOG("Unable to read from '%s'", sfoPath);
				sfo_free(sfo);
				continue;
			}

			ret[cur_count].codes = NULL;
			ret[cur_count].code_count = 0;
			ret[cur_count].flags = SAVE_FLAG_PS3;

			asprintf(&ret[cur_count].path, "%s%s/", userPath, dir->d_name);
			asprintf(&ret[cur_count].title_id, "%.9s", dir->d_name);

			char *sfo_data = (char*) sfo_get_param_value(sfo, "TITLE");
			asprintf(&ret[cur_count].name, "%s", sfo_data);

			sfo_data = (char*) sfo_get_param_value(sfo, "ATTRIBUTE");
			ret[cur_count].flags |=	(sfo_data[0] ? SAVE_FLAG_LOCKED : 0);

			sprintf(sfoPath, "%016lx", apollo_config.account_id);
			sfo_data = (char*) sfo_get_param_value(sfo, "ACCOUNT_ID");
			if (strncmp(sfo_data, sfoPath, 16) == 0)
				ret[cur_count].flags |=	SAVE_FLAG_OWNER;

			sfo_free(sfo);
				
			LOG("[%s] F(%d) name '%s'", ret[cur_count].title_id, ret[cur_count].flags, ret[cur_count].name);
				
			//printf("Successfully read %d codes\n", ret[cur_count].code_count);

			cur_count++;
		}
	}
		
	closedir(d);
	
	return ret;
}

/*
 * Function:		ReadOnlineList()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Downloads the entire gamelist file into a game_entry array
 * Arguments:
 *	gmc:			Set as the number of games read
 * Return:			Pointer to array of game_entry, null if failed
 */
save_entry_t * ReadOnlineList(const char* urlPath, int * gmc)
{
	const char* path = ONLINE_LOCAL_CACHE "games.txt";

	if (isExist(path))
	{
		struct stat stats;
		stat(path, &stats);
		// re-download if file is +1 day old
		if ((stats.st_mtime + ONLINE_CACHE_TIMEOUT) < time(NULL))
			http_download(urlPath, "games.txt", path, 0);
	}
	else
	{
		if (!http_download(urlPath, "games.txt", path, 0))
			return NULL;
	}
	
	long fsize;
	char *data = readFile(path, &fsize);
	data[fsize] = 0;
	
	char *ptr = data;
	char *end = data + fsize + 1;

	int game_count = 0;

	while (ptr < end && *ptr)
	{
		if (*ptr == '\n')
		{
			game_count++;
		}
		ptr++;
	}
	
	if (!game_count)
		return NULL;

	save_entry_t * ret = (save_entry_t *)malloc(sizeof(save_entry_t) * game_count);
	*gmc = game_count;

	int cur_count = 0;    
	ptr = data;
	
	while (ptr < end && *ptr && cur_count < game_count)
	{
		char* content = ptr;

		while (ptr < end && *ptr != '\n' && *ptr != '\r')
		{
			ptr++;
		}
		*ptr++ = 0;

//        LOG("ReadUserList() :: Reading %s...", content);
		if (content[9] == '=')
		{
			ret[cur_count].path = NULL;
			ret[cur_count].codes = NULL;
			ret[cur_count].flags = SAVE_FLAG_PS3;
			ret[cur_count].code_count = 0;

			asprintf(&ret[cur_count].title_id, "%.9s", content);

			content += 10;
			asprintf(&ret[cur_count].name, "%s", content);

			LOG("%d - [%s] %s", cur_count, ret[cur_count].title_id, ret[cur_count].name);
			cur_count++;
		}

		if (ptr < end && *ptr == '\r')
		{
			ptr++;
		}
		if (ptr < end && *ptr == '\n')
		{
			ptr++;
		}
	}

	if (data) free(data);

	return ret;
}

/*
 * Function:		isGameActivated()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Checks if the game has been selected/activated by the user
 * Arguments:
 *	game:			Game to read
 * Return:			1 if selected, 0 if not selected
 */
int isGameActivated(save_entry_t game)
{
	int x = 0;
	for (x = 0; x < game.code_count; x++)
	{
		if (game.codes)
		{
			if (game.codes[x].activated)
				return 1;
		}
	}
	
	return 0;
}

/*
 * Function:		isCodeLineValid()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Checks if the line is a valid code line
 * Arguments:
 *	line:			Line to check
 * Return:			1 if valid, 0 if not valid
 */
int isCodeLineValid(char * line)
{
	int textSpaceCnt = 0;
	int lineLen = strlen(line);
	int spaceCnt = 0;
	int x = 0, z = 0;
	for (x = 0; x < lineLen; x++)
		if (line[x] == ' ')
			spaceCnt++;
	
	int isText = 0, isFloat = 0;
	if (line[0] == '1' && line[1] == ' ')
		isText = 1;
	else if (line[0] == '2' && line[1] == ' ')
		isFloat = 1;
	
	if (!isText && !isFloat)
	{
		//Check number of spaces
		if (spaceCnt != 2)
			return 0;
		
		//Check hex
		for (z = 0; z < lineLen; z++)
		{
			char c = line[z];
			if (c >= 0x61)
				c -= 0x20;
			if (c == ' ')
				textSpaceCnt++;
			
			
			if (!(c == 'A' || c == 'B' || c == 'C' || c == 'D' || c == 'E' || c == 'F' || c == ' ' || c == '\r' || c == '\n' || c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8' || c == '9'))
			{
				if (textSpaceCnt > 2)
					return 2;
				
				return 0;
			}
		}
	}
	else if (isText && !isFloat)
	{
		//Check number of spaces
		if (spaceCnt < 2)
			return 0;
		
		//Check hex
		for (z = 0; z < lineLen; z++)
		{
			char c = line[z];
			if (c == ' ')
				textSpaceCnt++;
			if (c >= 0x61)
				c -= 0x20;
			
			if (textSpaceCnt >= 2)
				break;
			
			if (!(c == 'A' || c == 'B' || c == 'C' || c == 'D' || c == 'E' || c == 'F' || c == ' ' || c == '\r' || c == '\n' || c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8' || c == '9'))
			{
				return 0;
			}
		}
	}
	else if (!isText && isFloat)
	{
		//Check number of spaces
		if (spaceCnt != 2)
			return 0;
		
		//Check hex
		for (z = 0; z < lineLen; z++)
		{
			char c = line[z];
			if (c >= 0x61)
				c -= 0x20;
			if (c == ' ')
				textSpaceCnt++;
			
			if (!(c == '.' || c == '-' || c == 'A' || c == 'B' || c == 'C' || c == 'D' || c == 'E' || c == 'F' || c == ' ' || c == '\r' || c == '\n' || c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8' || c == '9'))
			{
				if (textSpaceCnt > 2)
					return 2;
				
				return 0;
			}
		}
	}
	else
		return 0;
	
	return 1;
}

/*
 * Function:        replace_str_count()
 * File:            saves.c
 * Project:         Apollo PS3
 * Description:     Returns the number of instances of substring found within buffer (specifically for Options)
 * Arguments:
 *	buffer:			String
 *	substring:		Searched for within buffer
 * Return:          Number of times the instance of substring occurs
 */
int replace_str_count(char *buffer, char *substring)
{
	if (!buffer || !substring)
		return 0;
	
	int count = 0, inc = strlen(substring), len = strlen(buffer), off = 0;
	const char *tmp = buffer;
	while((off < len) && (tmp = strstr(tmp, substring)))
	{
		if (tmp[strlen(substring)] != 'Z')
		{
			count++;
			tmp += inc;
			off += inc;
		}
		else
		{
			while (off < len && tmp[0] == 'Z') { off++; tmp++; }
		}
	}
	
	return count;
}

/*
 * Function:        replace_str_id()
 * File:            saves.c
 * Project:         Apollo PS3
 * Description:     Replaces all instances of orig with rep in str and returns the pointer of the result
 * Arguments:
 *	str:			String
 *	orig:			To be replaced in str
 *	rep:			Replaces orig in str
 * Return:          Pointer (as char *) to the resulting string
 */
char *replace_str_id(char *str, char *orig, char *rep)
{
	if (!str || !orig || !rep)
		return str;
		
	LOG("replace_str_id()");
	
	char *p;
	int index = 0, len = strlen(str), pass = 0, count = replace_str_count(str, orig);
	if (count <= 0)
		return str;
	char * buffer = calloc(1, (len + 1) + ((strlen(rep) - strlen(orig)) * count));
	strcpy(buffer, str);
	free (str);
	
	while (index < strlen(buffer))
	{
		if(!(p = strstr(buffer + index, orig)))  // Is 'orig' even in 'str'?
			break;
		
		if (p[strlen(rep)] != 'Z') 
		{
			strncpy(buffer, buffer, p-buffer); // Copy characters from 'str' start to 'orig' st$
			buffer[p-buffer] = '\0';
			
			sprintf(buffer+(p-buffer), "%s%s", rep, p+strlen(orig));
		}
		
		index = (p - buffer) + strlen(rep);
		pass++;
	}
	
	return buffer;
}

/*
 * Function:		ParseOptionsID()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Replaces all tags with the count of id
 * Arguments:
 *	buffer:			String
 *	id:				Number of Z's in the tag
 *	replace:		What to replace the tag with
 * Return:			Returns pointer to newly allocated buffer containing the resulting string
 */
char * ParseOptionsID(char * buffer, int id, char * replace)
{
	char orig[id + 1];
	memset(orig, 0, id + 1);
	int o = 0;
	for (o = 0; o < id; o++)
		orig[o] = 'Z';
	
	buffer = replace_str_id(buffer, (char *)orig, replace);
   
	return buffer;
}

/*
 * Function:		ParseOptionsCode()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Parses the options from a given code into buffer
 * Arguments:
 *	code:			Code to process
 *	buffer:			Strings to replace tags with values
 * Return:			Returns pointer to converted buffer string (tags => values)
 */
char * ParseOptionsCode(code_entry_t code, char * buffer)
{
	if (!code.options || code.options_count <= 0)
		return buffer;

	int c = 0;
	for (c = 0; c < code.options_count; c++)
	{
		//printf ("code=%s, buffer=%s, sel=%d\n", code.name, buffer, code.options[c].sel);
		buffer = ParseOptionsID(buffer, code.options[c].id, code.options[c].value[code.options[c].sel]);
	}
	
	//printf ("---buffer = %s\n", buffer);
	return buffer;
}
