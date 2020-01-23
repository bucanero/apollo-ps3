#include "saves.h"
#include "common.h"
#include "sfo.h"

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
	char fullPath[256];
	int count = 0;
	
	if (d)
	{
		
		while ((dir = readdir(d)) != NULL)
		{
			if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0)
			{
				snprintf(fullPath, sizeof(fullPath)-1, "%s%s", path, dir->d_name);
				snprintf(sfoPath, sizeof(sfoPath)-1, "%s/PARAM.SFO", fullPath);
				if ((dir_exists(fullPath) == SUCCESS) && (file_exists(sfoPath) == SUCCESS))
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
char * readFile(const char * path)
{
	FILE *f = fopen(path, "rb");
	if (f == NULL)
		return NULL;
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);
	if (fsize <= 0)
		return NULL;

	char * string = malloc(fsize + 1);
	fread(string, fsize, 1, f);
	fclose(f);

	//string[fsize] = 0;
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

/*
 * Function:		ReadCodes()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Reads an entire NCL file into an array of code_entry
 * Arguments:
 *	path:			Path to ncl
 *	_count_count:	Pointer to int (set to the number of codes within the ncl)
 * Return:			Returns an array of code_entry, null if failed to load
 */
code_entry_t * ReadCodes(const char * title_id, int * _code_count)
{
    int name_length, code_len = 0, code_count = 2, cur_count = 0;
//	int cur_count = 0, code_count = 2;
	int lockd = 1;

	code_entry_t * ret;
/*
	*_code_count = code_count;
	code_entry_t * ret = (code_entry_t *)calloc(1, sizeof(code_entry_t) * (code_count));

	ret[cur_count].activated = 0;
//	ret[cur_count].options = NULL;
	ret[cur_count].name = (char *)malloc(12);
	ret[cur_count].codes = (char *)malloc(strlen(CODE_RESIGN_SAVE)+1);
	strcpy(ret[cur_count].name, "Resign save");
	strcpy(ret[cur_count].codes, CODE_RESIGN_SAVE);

	cur_count++;
	ret[cur_count].activated = 0;
//	ret[cur_count].options = NULL;
	ret[cur_count].name = (char *)malloc(23);
	ret[cur_count].codes = (char *)malloc(strlen(CODE_UNLOCK_COPY)+1);
	strcpy(ret[cur_count].name, "Remove copy protection");
	strcpy(ret[cur_count].codes, CODE_UNLOCK_COPY);

*/

	char filePath[256];
	snprintf(filePath, sizeof(filePath)-1, APOLLO_DATA_PATH "%s.ps3savepatch", title_id);
	int fileSize = getFileSize(filePath);

	if (fileSize > 0)
	{
		LOG("Loading codes '%s'...", filePath);

		char savefile[128];
		char * buffer = (char *)malloc(fileSize + 1);
		readFileBuffered(filePath, buffer);
		buffer[fileSize]=0;
		//LOG("buffer=%s\n", buffer);

		int x = 0, bufferLen = strlen(buffer);

	    for (x = 0; x < bufferLen; x++)
	    {
	        if (buffer[x] == '[')
	            code_count++;
	    }

		ret = (code_entry_t *)calloc(1, sizeof(code_entry_t) * (code_count));
	
	    if (code_count > (1 + lockd))
		{
			for (x = 0; x < bufferLen; x++)
		    {
		        if (cur_count >= code_count)
		            break;
		        
		        if (buffer[x] == ';')
		        {
		        	x = findNextLine(buffer, x);
		        }
		
		        else if ((buffer[x] == ':') && (x++ < bufferLen))
		        {
					name_length = findNextLine(buffer, x) - x;
		            memcpy(savefile, &buffer[x], name_length);
		            savefile[name_length] = 0;
		                        
		            x += name_length - 1;
		        }
		
		        else if ((buffer[x] == '[') && (x++ < bufferLen))
		        {		
		            ret[cur_count].activated = 0;
		
					name_length = strchr(&buffer[x], ']') - &buffer[x];
		            ret[cur_count].name = (char *)malloc(name_length + 1);
		            memcpy(ret[cur_count].name, &buffer[x], name_length);
		            ret[cur_count].name[name_length] = 0;
		
		            ret[cur_count].file = (char *)malloc(strlen(savefile) + 1);
		            strcpy(ret[cur_count].file, savefile);
		
		            LOG ("readNCL() :: code[%d].name = '%s'", cur_count, ret[cur_count].name);
					LOG("file '%s'", ret[cur_count].file);
		                        
		            x += name_length + 1;
					code_len = findNextBreak(buffer, x) - x;

		            ret[cur_count].codes = (char *)malloc(code_len + 1);
		            memcpy(ret[cur_count].codes, &buffer[x], code_len);
		            ret[cur_count].codes[code_len] = 0;

		            LOG("cur_count=%d, line=%d, code_len=%d", cur_count, 0, code_len);
					LOG("code[%d] %s", cur_count, ret[cur_count].codes);
		                        
		            x += code_len - 1;
		        	cur_count++;
		        }
		    }
		}

		free (buffer);
	}
	else
	{
		ret = (code_entry_t *)calloc(1, sizeof(code_entry_t) * (code_count));
	}

	*_code_count = code_count;

	ret[cur_count].activated = 0;
//	ret[cur_count].options = NULL;
	ret[cur_count].name = (char *)malloc(12);
	ret[cur_count].codes = (char *)malloc(strlen(CODE_RESIGN_SAVE)+1);
	strcpy(ret[cur_count].name, "Resign save");
	strcpy(ret[cur_count].codes, CODE_RESIGN_SAVE);

	cur_count++;
	ret[cur_count].activated = 0;
//	ret[cur_count].options = NULL;
	ret[cur_count].name = (char *)malloc(23);
	ret[cur_count].codes = (char *)malloc(strlen(CODE_UNLOCK_COPY)+1);
	strcpy(ret[cur_count].name, "Remove copy protection");
	strcpy(ret[cur_count].codes, CODE_UNLOCK_COPY);

	LOG("cur_count=%d,code_count=%d", cur_count, code_count);

	return ret;
}

/*
 * Function:		ReadOnlineNCL()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Downloads an entire NCL file into an array of code_entry
 * Arguments:
 *	filename:		File name ncl
 *	_count_count:	Pointer to int (set to the number of codes within the ncl)
 * Return:			Returns an array of code_entry, null if failed to load
 */
code_entry_t * ReadOnlineNCL(const char * filename, int * _code_count)
{ 
	char path[256];
	snprintf(path, sizeof(path)-1, ONLINE_LOCAL_CACHE "%s", filename);

	if (isExist(path))
	{
		struct stat stats;
		stat(path, &stats);
		// re-download if file is +1 day old
		if ((stats.st_mtime + ONLINE_CACHE_TIMEOUT) < time(NULL))
			http_download(ONLINE_URL "codes/", filename, path, 0);
	}
	else
	{
		if (!http_download(ONLINE_URL "codes/", filename, path, 0))
			return NULL;
	}

	return ReadCodes(path, _code_count);
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
			
			if (list[x].account_id)
			{
				free(list[x].account_id);
				list[x].account_id = NULL;
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

/*
 * Function:		BubbleSortCodeList_Compare()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Compares two code_entry's names for BubbleSort
 * Arguments:
 *	a:				First code
 *	b:				Second code
 * Return:			1 if greater, 0 if less or equal
 */
int BubbleSortCodeList_Compare(code_entry_t a, code_entry_t b)
{
	if (!a.name || !b.name)
		return 0;
	
	//Set up vars
	int al = strlen(a.name), bl = strlen(b.name);
	int x = 0;
	
	//Do comparison
	int smallmax = (al <= bl) ? al : bl;
	for (x = 0; x < smallmax; x++)
	{
		char cA = a.name[x], cB = b.name[x];
		if (cA >= 'A' && cA <= 'Z')
			cA += 0x20;
		if (cB >= 'A' && cB <= 'Z')
			cB += 0x20;
		
		if (cA > cB)
			return 1;
		else if (cA < cB)
			return 0;
	}
	
	if (al > bl)
		return 1;
	
	return 0;
}

/*
 * Function:		BubbleSortCodeList()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Sorts the game_entry's codes by name (least to greatest)
 * Arguments:
 *	game:			Game's code list to sort
 * Return:			Returns sorted game list
 */
save_entry_t BubbleSortCodeList(save_entry_t game)
{
	if (game.code_sorted)
		return game;

	code_entry_t * swap = (code_entry_t *)malloc(sizeof(code_entry_t) * 1);
	int c = 0, d = 0, count = game.code_count;
	
	for (c = 0; c < (count-1); c++)
	{
		int dMax = (count - c - 1);
		for (d = 0 ; d < dMax; d++)
		{
			if (BubbleSortCodeList_Compare(game.codes[d], game.codes[d+1]))
			{
				swap[0] = game.codes[d];
				game.codes[d]   = game.codes[d+1];
				game.codes[d+1] = swap[0];
			}
		}
	}
	
	free (swap);
	
	game.code_sorted = 1;
	return game;
}

/*
 * Function:		BubbleSortGameList_Compare()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Compares two game_entry for BubbleSort
 * Arguments:
 *	a:				Code to process
 *	b:				Pointer to int (set to the number of tags within the code)
 * Return:			Returns an array of option_entry and the count at *count
 */
int BubbleSortGameList_Compare(save_entry_t a, save_entry_t b)
{
	//Set up vars
	int al = strlen(a.name), bl = strlen(b.name);
	int x = 0;
	
	//Do comparison
	int smallmax = (al <= bl) ? al : bl;
	for (x = 0; x < smallmax; x++)
	{
		char cA = a.name[x], cB = b.name[x];
		if (cA >= 'A' && cA <= 'Z')
			cA += 0x20;
		if (cB >= 'A' && cB <= 'Z')
			cB += 0x20;
		
		if (cA > cB)
			return 1;
		else if (cA < cB)
			return 0;
	}
	
	if (al > bl)
		return 1;
	
	return 0;
}

/*
 * Function:		BubbleSortGameList()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Sorts array of game_entry by name (least to greatest)
 * Arguments:
 *	games:			Pointer to array of games to be sorted
 *	count:			Number of games in games
 * Return:			void
 */
void BubbleSortGameList(save_entry_t * games, int count)
{
	//Allocate so we don't use the stack
	save_entry_t * swap = (save_entry_t *)malloc(sizeof(save_entry_t) * 1);
	int c = 0, d = 0;
	
	//Bubble sort
	for (c = 0; c < (count-1); c++)
	{
		for (d = 0 ; d < (count - c - 1); d++)
		{
			if (BubbleSortGameList_Compare(games[d], games[d+1]))
			{
				swap[0]       = games[d];
				games[d]   = games[d+1];
				games[d+1] = swap[0];
			}
		}
	}
	
	free (swap);
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
save_entry_t * ReadUserList(int * gmc)
{
	char * userPath;
	if (dir_exists(SAVES_PATH_USB0) == SUCCESS)
		userPath = (char*)SAVES_PATH_USB0;
	else if (dir_exists(SAVES_PATH_USB1) == SUCCESS)
		userPath = (char*)SAVES_PATH_USB1;
	else
		userPath = (char*)SAVES_PATH_HDD;
	
	int save_count = getDirListSize(userPath);
	if (!save_count)
		return NULL;

	*gmc = save_count;
	
	DIR *d;
	struct dirent *dir;
	d = opendir(userPath);
	
	if (!d)
		return NULL;

	save_entry_t * ret = (save_entry_t *)malloc(sizeof(save_entry_t) * save_count);

	char sfoPath[256];
	char fullPath[256];
	int cur_count = 0;

	while ((dir = readdir(d)) != NULL)
	{
		if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
			continue;

		snprintf(fullPath, sizeof(fullPath)-1, "%s%s/", userPath, dir->d_name);
		snprintf(sfoPath, sizeof(sfoPath)-1, "%s" "PARAM.SFO", fullPath);
		if ((dir_exists(fullPath) == SUCCESS) && (file_exists(sfoPath) == SUCCESS))
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
			ret[cur_count].code_sorted = 0;

			ret[cur_count].path = malloc(strlen(fullPath) + 1);
			strcpy(ret[cur_count].path, fullPath);

			ret[cur_count].title_id = malloc(10); // strlen("BLUS00000") + 1
			sprintf(ret[cur_count].title_id, "%.9s", dir->d_name);

			char *sfo_data = (char*) sfo_get_param_value(sfo, "TITLE");
			ret[cur_count].name = malloc(strlen(sfo_data) + 1);
			strcpy(ret[cur_count].name, sfo_data);

			sfo_data = (char*) sfo_get_param_value(sfo, "ACCOUNT_ID");
			ret[cur_count].account_id = malloc(17); // max len 16
			sprintf(ret[cur_count].account_id, "%.16s", sfo_data);

			sfo_data = (char*) sfo_get_param_value(sfo, "ATTRIBUTE");
			ret[cur_count].locked =	sfo_data[0];

			sfo_free(sfo);
				
			LOG("name '%s' [%s]", ret[cur_count].name, ret[cur_count].title_id);
			LOG("accid '%s' lockd (%d)", ret[cur_count].account_id, ret[cur_count].locked);
				
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
save_entry_t * ReadOnlineList(int * gmc)
{
	const char* path = ONLINE_LOCAL_CACHE "games.txt";

	if (isExist(path))
	{
		struct stat stats;
		stat(path, &stats);
		// re-download if file is +1 day old
		if ((stats.st_mtime + ONLINE_CACHE_TIMEOUT) < time(NULL))
			http_download(ONLINE_URL, "games.txt", path, 0);
	}
	else
	{
		if (!http_download(ONLINE_URL, "games.txt", path, 0))
			return NULL;
	}
	
	long fsize = getFileSize(path);
	char *data = readFile(path);
	
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

		int ccnt[1] = {0};

//        printf("ReadUserList() :: Reading %s...", content);

		ret[cur_count].codes = NULL;
//			ret[cur_count].path = content;
		ret[cur_count].path = malloc(strlen(content) + 1);
		strcpy(ret[cur_count].path, content);
		ret[cur_count].code_count = *ccnt;
		ret[cur_count].name = stripExt(content);
		ret[cur_count].code_sorted = 0;
		ret[cur_count].account_id = NULL;
		ret[cur_count].title_id = NULL;
//		parseVTID(&ret[cur_count]);

		cur_count++;

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

/*
 * Function:		StripMultilineComments()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Removes multiline comments and its contents from a string
 * Arguments:
 *	buffer:			String
 * Return:			void
 */
void StripMultilineComments(char * buffer)
{
	if (!buffer)
		return;
	
	//find /* in buffer
	int len = strlen(buffer);
	char * p = strstr(buffer, "/*");
	
	while (p)
	{
		int start = p - buffer;
		int end = len;
		
		p = strstr(buffer, "*/");
		if (p)
		{
			end = (p - buffer) + 2;
		}
			
		if (end < len)
			memcpy(&buffer[start], &buffer[end], len - end);
		
		buffer[start + (len - end)] = 0;
		len = start + (len - end);
		
		p = strstr(buffer, "/*");
	}
	
	
}

/*
 * Function:		ShrinkCode()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Removes comments, invalid code lines, and extra lines ("\n\n")
 * Arguments:
 *	buffer:			String
 * Return:			void
 */
void ShrinkCode(char * buffer)
{
	//Strip multiline comments
	StripMultilineComments(buffer);
	
	char * line = malloc(1000);
	
	//printf("----buffer = %s\n", buffer);
	
	int x = 0, lineStart = 0, max = strlen(buffer);
	while (x < max)
	{
		if (buffer[x] == '\n')
		{
			if ((x - lineStart) > 2)
			{
				memcpy(line, &buffer[lineStart], x - lineStart);
				line[x - lineStart] = 0;
				int isValid = isCodeLineValid(line);
				
				if (isValid == 0) //Remove line entirely
				{
					//strncpy(buffer, buffer, p-buffer); // Copy characters from 'str' start to 'orig' st$
					//buffer[p-buffer] = '\0';
					
					//sprintf(buffer+(p-buffer), "%s%s", rep, p+strlen(orig));
					
					memcpy(&buffer[lineStart], &buffer[x+1], max - x - 1);
					max -= (x - lineStart) + 1;
					x = lineStart - 1;
				}
				else if (isValid == 2) //Remove end part of code (c aaaaaaaa vvvvvvvv anything past here is extra crap we don't need)
				{
					int start = lineStart, spaceCnt = 0;
					while (start < max && spaceCnt < 3 && start < x)
					{
						if (buffer[start] == ' ')
							spaceCnt++;
						start++;
					}
					
					if (spaceCnt > 2)
					{
						memcpy(&buffer[start], &buffer[x], max - x);
						max -= (x - start);
					}
				}
			}
			
			lineStart = x + 1;
		}
		
		x++;
	}
	buffer[max] = 0;
	
	free(line);
}

/*
 * Function:		AppendCode()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Appends a condensed version of the code as a NCL formatted code
 * Arguments:
 *	buffer:			String
 *	code:			Code to convert to NCL code format
 * Return:			void
 */
void AppendCode(char * buffer, code_entry_t code)
{
	int len = strlen(buffer);
	
	//Name
	memcpy(&buffer[len], code.name, strlen(code.name));
	len += strlen(code.name);
	
	memcpy(&buffer[len], "\n", 1);
	len += 1;
	
	//Constant write
	memcpy(&buffer[len], code.cwrite ? "1\n" : "0\n", 2);
	len += 2;
	
	//Codes
	char * tCodeBuf = (char *)malloc(strlen(code.codes) + 1);
	memcpy(tCodeBuf, code.codes, strlen(code.codes));
	tCodeBuf[strlen(code.codes)] = 0;
	tCodeBuf = ParseOptionsCode(code, tCodeBuf);
	ShrinkCode(tCodeBuf);
	memcpy(&buffer[len], tCodeBuf, strlen(tCodeBuf));
	len += strlen(tCodeBuf);
	//free (tCodeBuf);
	
	//End of code
	if (buffer[len-1] == '\n')
	{
		memcpy(&buffer[len], "#\n", 2);
		len += 2;
	}
	else
	{
		memcpy(&buffer[len], "\n#\n", 3);
		len += 3;
	}
	buffer[len] = 0;
}

/*
 * Function:		ParseActivatedGameList()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Parses the selected codes into a ncl file for Artemis PRX to read
 * Arguments:
 *	list:			Array of game_entry to process
 *	count:			Number of games in list
 * Return:			Returns a buffer containing the processed codes in NCL format
 */
char * ParseActivatedGameList(save_entry_t * list, int count)
{
	char * ret = (char *)calloc(1, 5000); //Max
	
	if (list)
	{
		int x = 0, y = 0;
		for (x = 0; x < count; x++)
		{
			if (list[x].codes)
			{
				for (y = 0; y < list[x].code_count; y++)
				{
					if (list[x].codes[y].activated)
					{
						AppendCode(ret, list[x].codes[y]);
					}
				}
			}
		}
	}
	
	return ret;
}

