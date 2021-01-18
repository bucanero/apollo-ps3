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
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "saves.h"
#include "common.h"
#include "sfo.h"
#include "settings.h"
#include "util.h"
#include "pfd.h"

#define UTF8_CHAR_GROUP		"\xe2\x97\x86"
#define UTF8_CHAR_ITEM		"\xe2\x94\x97"
#define UTF8_CHAR_STAR		"\xE2\x98\x85"

#define CHAR_ICON_ZIP		"\x0C"
#define CHAR_ICON_COPY		"\x0B"
#define CHAR_ICON_SIGN		"\x06"
#define CHAR_ICON_USER		"\x07"
#define CHAR_ICON_LOCK		"\x08"

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
 * Function:		endsWith()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Checks to see if a ends with b
 * Arguments:
 *	a:				String
 *	b:				Potential end
 * Return:			pointer if true, NULL if false
 */
char* endsWith(const char * a, const char * b)
{
	int al = strlen(a), bl = strlen(b);
    
	if (al < bl)
		return NULL;

	a += (al - bl);
	while (*a)
		if (toupper(*a++) != toupper(*b++)) return NULL;

	return (char*) (a - bl);
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
 * Function:		rtrim()
 * File:			saves.c
 * Project:			Apollo PS3
 * Description:		Trims ending white spaces (' ') from a string
 * Arguments:
 *	buffer:			String
 * Return:			Amount of characters removed
 */
int rtrim(char * buffer)
{
	int i, max = strlen(buffer) - 1;
	for (i = max; (buffer[i] == ' ') && (i >= 0); i--)
		buffer[i] = 0;

	return (max - i);
}

code_entry_t* _createCmdCode(uint8_t type, const char* name, char code)
{
	code_entry_t* entry = (code_entry_t *)calloc(1, sizeof(code_entry_t));
	entry->type = type;
	asprintf(&entry->name, name);
	asprintf(&entry->codes, "%c", code);

	return entry;
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

option_entry_t* _createOptions(int count, const char* name, char value)
{
	option_entry_t* options = _initOptions(count);

	asprintf(&options->name[0], "%s %d", name, 0);
	asprintf(&options->value[0], "%c%c", value, 0);
	asprintf(&options->name[1], "%s %d", name, 1);
	asprintf(&options->value[1], "%c%c", value, 1);

	return options;
}

save_entry_t* _createSaveEntry(uint16_t flag, const char* name)
{
	save_entry_t* entry = (save_entry_t *)calloc(1, sizeof(save_entry_t));
	entry->flags = flag;
	asprintf(&entry->name, name);

	return entry;
}

void _remove_char(char * str, int len, char seek)
{
	int x;
	for (x = 0; x < len; x++)
		if (str[x] == seek)
			str[x] = '\n';
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
    	if ((wildcard_match(line, "[*]") ||
			wildcard_match(line, "; --- * ---") ||
			wildcard_match_icase(line, "GROUP:*")) && (i++ == code_id))
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
					if (!wildcard_match(line, "\?\?\?\?\?\?\?\? \?\?\?\?\?\?\?\?") || (
						(line[0] != '0') && (line[0] != '1') && (line[0] != '2') && (line[0] != '4') &&
						(line[0] != '5') && (line[0] != '6') && (line[0] != '7') && (line[0] != '8') &&
						(line[0] != '9') && (line[0] != 'A')))
						entry->type = PATCH_BSD;

					// set the correct file for the decompress command
					if (wildcard_match_icase(line, "DECOMPRESS *"))
					{
						line += strlen("DECOMPRESS ");
						if (entry->file)
							free(entry->file);

						entry->file = strdup(line);
					}

					// set the correct file for the compress command
					if (wildcard_match_icase(line, "COMPRESS *,*"))
					{
						line += strlen("COMPRESS ");
						if (entry->file)
							free(entry->file);

						char* tmp = strchr(line, ',');
						*tmp = 0;

						entry->file = strdup(line);
						*tmp = ',';
					}

			    }
		    	line = strtok(NULL, "\n");
		    }
    	}
    	line = strtok(NULL, "\n");
    }

//	LOG("Result (%s)", res);
	entry->codes = res;
}

option_entry_t* _getFileOptions(const char* save_path, const char* mask, uint8_t is_cmd)
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
			if (is_cmd)
				asprintf(&opt->value[i], "%c", is_cmd);
			else
				asprintf(&opt->value[i], "%s", mask);

			i++;
		}
	}

	closedir(d);

	return opt;
}

void _addBackupCommands(save_entry_t* item)
{
	code_entry_t* cmd;

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_SIGN " Apply Changes & Resign", CMD_RESIGN_SAVE);
	list_append(item->codes, cmd);

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_USER " View Save Details", CMD_VIEW_DETAILS);
	list_append(item->codes, cmd);

	cmd = _createCmdCode(PATCH_NULL, "----- " UTF8_CHAR_STAR " File Backup " UTF8_CHAR_STAR " -----", 0);
	list_append(item->codes, cmd);

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " Copy save game", 0);
	cmd->options_count = 1;
	cmd->options = _createOptions(3, "Copy Save to USB", CMD_COPY_SAVE_USB);
	asprintf(&cmd->options->name[2], "Copy Save to HDD");
	asprintf(&cmd->options->value[2], "%c", CMD_COPY_SAVE_HDD);
	list_append(item->codes, cmd);

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_ZIP " Export save game to Zip", 0);
	cmd->options_count = 1;
	cmd->options = _createOptions(3, "Export Zip to USB", CMD_EXPORT_ZIP_USB);
	asprintf(&cmd->options->name[2], "Export Zip to HDD");
	asprintf(&cmd->options->value[2], "%c", CMD_EXPORT_ZIP_HDD);
	list_append(item->codes, cmd);

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " Decrypt save game files", 0);
	cmd->options_count = 1;
	cmd->options = _getFileOptions(item->path, "*", CMD_DECRYPT_FILE);
	list_append(item->codes, cmd);
}

option_entry_t* _getSaveTitleIDs(const char* title_id)
{
	int count = 1;
	option_entry_t* opt;
	char tmp[16];
	const char *ptr;
	const char *tid = get_game_title_ids(title_id);

	if (!tid)
		tid = title_id;

	ptr = tid;
	while (*ptr)
		if (*ptr++ == '/') count++;

	LOG("Adding (%d) TitleIDs=%s", count, tid);

	opt = _initOptions(count);
	int i = 0;

	ptr = tid;
	while (*ptr++)
	{
		if ((*ptr == '/') || (*ptr == 0))
		{
			memset(tmp, 0, sizeof(tmp));
			strncpy(tmp, tid, ptr - tid);
			asprintf(&opt->name[i], "%s", tmp);
			asprintf(&opt->value[i], "%c", SFO_CHANGE_TITLE_ID);
			tid = ptr+1;
			i++;
		}
	}

	return opt;
}

void _addSfoCommands(save_entry_t* save)
{
	code_entry_t* cmd;

	cmd = _createCmdCode(PATCH_NULL, "----- " UTF8_CHAR_STAR " SFO Patches " UTF8_CHAR_STAR " -----", 0);
	list_append(save->codes, cmd);

	cmd = _createCmdCode(PATCH_SFO, CHAR_ICON_USER " Change Account ID", SFO_CHANGE_ACCOUNT_ID);
	cmd->options_count = 1;
	cmd->options = _initOptions(2);
	cmd->options->name[0] = strdup("Remove ID/Blank");
	cmd->options->value[0] = calloc(1, SFO_ACCOUNT_ID_SIZE);
	cmd->options->name[1] = strdup("Fake Owner/Rebug");
	cmd->options->value[1] = strdup("FFFFFFFFFFFFFFFF");
	list_append(save->codes, cmd);

	cmd = _createCmdCode(PATCH_SFO, CHAR_ICON_USER " Remove Console ID", SFO_REMOVE_PSID);
	list_append(save->codes, cmd);

	if (save->flags & SAVE_FLAG_LOCKED)
	{
		cmd = _createCmdCode(PATCH_SFO, CHAR_ICON_LOCK " Remove copy protection", SFO_UNLOCK_COPY);
		list_append(save->codes, cmd);
	}

	cmd = _createCmdCode(PATCH_SFO, CHAR_ICON_USER " Change Region Title ID", SFO_CHANGE_TITLE_ID);
	cmd->options_count = 1;
	cmd->options = _getSaveTitleIDs(save->title_id);
	list_append(save->codes, cmd);
}

int set_psx_import_codes(save_entry_t* item)
{
	code_entry_t* cmd;
	item->codes = list_alloc();

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_USER " View Save Details", CMD_VIEW_DETAILS);
	list_append(item->codes, cmd);

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " Convert to .PSV", 0);
	cmd->options_count = 1;
	cmd->options = _createOptions(2, "Save .PSV file to USB", CMD_CONVERT_TO_PSV);
	list_append(item->codes, cmd);

	return list_count(item->codes);
}

int set_psp_codes(save_entry_t* item)
{
	code_entry_t* cmd;
	item->codes = list_alloc();

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_USER " View Save Details", CMD_VIEW_DETAILS);
	list_append(item->codes, cmd);

	return list_count(item->codes);
}

int set_psv_codes(save_entry_t* item)
{
	code_entry_t* cmd;
	item->codes = list_alloc();

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_USER " View Save Details", CMD_VIEW_DETAILS);
	list_append(item->codes, cmd);

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_SIGN " Resign .PSV file", CMD_RESIGN_PSV);
	list_append(item->codes, cmd);

	if (item->flags & SAVE_FLAG_PS1)
	{
		cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " Export PS1 save to .MCS", 0);
		cmd->options_count = 1;
		cmd->options = _createOptions(2, "Save .MCS file to USB", CMD_EXP_PSV_MCS);
	}
	else
	{
		cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " Export PS2 save to .PSU", 0);
		cmd->options_count = 1;
		cmd->options = _createOptions(2, "Save .PSU file to USB", CMD_EXP_PSV_PSU);
	}
	list_append(item->codes, cmd);

	return list_count(item->codes);
}

int set_ps2_codes(save_entry_t* item)
{
	code_entry_t* cmd;
	item->codes = list_alloc();

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_USER " View Save Details", CMD_VIEW_DETAILS);
	list_append(item->codes, cmd);

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " Decrypt SCEVMC0.VME", 0);
	asprintf(&cmd->file, "SCEVMC0.VME");
	cmd->options_count = 1;
	cmd->options = _createOptions(3, "Decrypt SCEVMC0.VME to USB", CMD_DECRYPT_PS2_VME);
	asprintf(&cmd->options->name[2], "Decrypt SCEVMC0.VME to HDD");
	asprintf(&cmd->options->value[2], "%c%c", CMD_DECRYPT_PS2_VME, 2);
	list_append(item->codes, cmd);

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " Decrypt SCEVMC1.VME", 0);
	asprintf(&cmd->file, "SCEVMC1.VME");
	cmd->options_count = 1;
	cmd->options = _createOptions(3, "Decrypt SCEVMC1.VME to USB", CMD_DECRYPT_PS2_VME);
	asprintf(&cmd->options->name[2], "Decrypt SCEVMC1.VME to HDD");
	asprintf(&cmd->options->value[2], "%c%c", CMD_DECRYPT_PS2_VME, 2);
	list_append(item->codes, cmd);

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " Import a .VM2 to SCEVMC0.VME", 0);
	asprintf(&cmd->file, "SCEVMC0.VME");
	cmd->options_count = 1;
	cmd->options = _getFileOptions(EXP_PS2_PATH_HDD, "*.VM2", CMD_ENCRYPT_PS2_VMC);
	list_append(item->codes, cmd);

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " Import a .VM2 to SCEVMC1.VME", 0);
	asprintf(&cmd->file, "SCEVMC1.VME");
	cmd->options_count = 1;
	cmd->options = _getFileOptions(EXP_PS2_PATH_HDD, "*.VM2", CMD_ENCRYPT_PS2_VMC);
	list_append(item->codes, cmd);

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " Copy dummy .PSV Save", 0);
	asprintf(&cmd->file, "APOLLO-99PS2.PSV");
	cmd->options_count = 1;
	cmd->options = _createOptions(2, "Copy APOLLO-99PS2.PSV to USB", CMD_COPY_DUMMY_PSV);
	list_append(item->codes, cmd);

	return list_count(item->codes);
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
	int code_count = 0;
	code_entry_t * code;
	option_entry_t * file_opt = NULL;
	char filePath[256] = "";
	char group = 0;
	long bufferLen;
	char * buffer = NULL;

	if (save->flags & SAVE_FLAG_PSV)
		return set_psv_codes(save);

	if (save->flags & SAVE_FLAG_PS2)
		return set_ps2_codes(save);

	if (save->flags & SAVE_FLAG_PSP)
		return set_psp_codes(save);

	save->codes = list_alloc();

	_addBackupCommands(save);
	_addSfoCommands(save);

	snprintf(filePath, sizeof(filePath), APOLLO_DATA_PATH "%s.ps3savepatch", save->title_id);
	if (file_exists(filePath) != SUCCESS)
		return list_count(save->codes);

	LOG("Loading BSD codes '%s'...", filePath);
	buffer = readFile(filePath, &bufferLen);
	buffer[bufferLen]=0;

	_remove_char(buffer, bufferLen, '\r');

	code = _createCmdCode(PATCH_NULL, "----- " UTF8_CHAR_STAR " Cheats " UTF8_CHAR_STAR " -----", 0);	
	list_append(save->codes, code);

	list_node_t* node = list_tail(save->codes);
	char *line = strtok(buffer, "\n");
		
	while (line)
	{
		if (wildcard_match(line, ":*"))
		{
			char* tmp_mask;

			strcpy(filePath, line+1);
			LOG("FILE: %s\n", filePath);

			if (strrchr(filePath, '\\'))
				tmp_mask = strrchr(filePath, '\\')+1;
			else
				tmp_mask = filePath;

			if (strchr(tmp_mask, '*'))
				file_opt = _getFileOptions(save->path, tmp_mask, 0);
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
		else if (wildcard_match(line, "[*]") ||
				wildcard_match(line, "; --- * ---") ||
				wildcard_match_icase(line, "GROUP:*"))
		{
			if (wildcard_match_icase(line, "[DEFAULT:*"))
			{
				line += 6;
				line[1] = CHAR_TAG_WARNING;
				line[2] = ' ';
			}
			else if (wildcard_match_icase(line, "[INFO:*"))
			{
				line += 3;
				line[1] = CHAR_TAG_WARNING;
				line[2] = ' ';
			}
			else if (wildcard_match_icase(line, "[GROUP:*"))
			{
				line += 6;
				group = 1;
				LOG("GROUP: %s\n", line);
			}
			else if (wildcard_match(line, "; --- * ---") || wildcard_match_icase(line, "GROUP:*"))
			{
				line += 5;
				group = 1;
				LOG("GROUP: %s\n", line);
			}
			line++;

			code = calloc(1, sizeof(code_entry_t));
			code->type = PATCH_GAMEGENIE;
			code->options = file_opt;
			code->options_count = (file_opt ? 1 : 0);
			asprintf(&code->file, "%s", filePath);
			list_append(save->codes, code);

			switch (group)
			{
				case 1:
					asprintf(&code->name, UTF8_CHAR_GROUP " %s", line);
					group = 2;
					break;
				case 2:
					asprintf(&code->name, " " UTF8_CHAR_ITEM " %s", line);
					break;
				
				default:
					asprintf(&code->name, "%s", line);
					break;
			}

			char* end = strrchr(code->name, ']');
			if (end) *end = 0;

			end = endsWith(code->name, " ---");
			if (end) *end = 0;
		}

		line = strtok(NULL, "\n");
	}

	while ((node = list_next(node)) != NULL)
	{
		code = list_get(node);
		// remove 0x00 from previous strtok(...)
		_remove_char(buffer, bufferLen, '\0');
		get_patch_code(buffer, code_count++, code);

		LOG("[%d] Name: %s\nFile: %s\nCode (%d): %s\n", code_count, code->name, code->file, code->type, code->codes);
	}

	free (buffer);

	LOG("Loaded %d codes", list_count(save->codes));

	return list_count(save->codes);
}

char* _get_xml_node_value(xmlNode * a_node, const xmlChar* node_name)
{
	xmlNode *cur_node = NULL;
	char *value = NULL;

	for (cur_node = a_node; cur_node && !value; cur_node = cur_node->next)
	{
		if (cur_node->type != XML_ELEMENT_NODE)
			continue;

		if (xmlStrcasecmp(cur_node->name, node_name) == 0)
		{
			value = (char*) xmlNodeGetContent(cur_node);
//			LOG("xml value=%s", value);
		}
	}

	return value;
}

int ReadTrophies(save_entry_t * game)
{
	int trop_count = 0;
	code_entry_t * trophy;
	char filePath[256];
	long bufferLen;
	char * buffer = NULL;
	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL;
	xmlNode *cur_node = NULL;
	char *value;

	snprintf(filePath, sizeof(filePath), "%s" "TROPCONF.SFM", game->path);
	if (file_exists(filePath) != SUCCESS)
		return 0;

	buffer = readFile(filePath, &bufferLen);
	buffer[bufferLen]=0;

	/*parse the file and get the DOM */
	doc = xmlReadMemory(buffer + 0x40, bufferLen - 0x40, NULL, NULL, XML_PARSE_NONET);

	if (!doc)
	{
		LOG("XML: could not parse file %s", filePath);
		free(buffer);
		return 0;
	}

	game->codes = list_alloc();

	trophy = _createCmdCode(PATCH_COMMAND, CHAR_ICON_SIGN "Resign Trophy Set", CMD_RESIGN_TROPHY);
	list_append(game->codes, trophy);

	trophy = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " Backup Trophy Set to USB", 0);
	trophy->file = strdup(game->path);
	trophy->options_count = 1;
	trophy->options = _createOptions(2, "Copy Trophy to USB", CMD_EXP_TROPHY_USB);
	list_append(game->codes, trophy);

	trophy = _createCmdCode(PATCH_NULL, "----- " UTF8_CHAR_STAR " Trophies " UTF8_CHAR_STAR " -----", 0);
	list_append(game->codes, trophy);

	/*Get the root element node */
	root_element = xmlDocGetRootElement(doc);

	for (cur_node = root_element->children; cur_node; cur_node = cur_node->next)
	{
		if (cur_node->type != XML_ELEMENT_NODE)
			continue;

		if (xmlStrcasecmp(cur_node->name, BAD_CAST "trophy") == 0)
		{
			value = _get_xml_node_value(cur_node->children, BAD_CAST "name");
			trophy = _createCmdCode(PATCH_NULL, value, CMD_CODE_NULL);

			value = _get_xml_node_value(cur_node->children, BAD_CAST "detail");
			trophy->codes = strdup(value);

			value = (char*) xmlGetProp(cur_node, BAD_CAST "ttype");
			trophy->type = value[0];

			value = (char*) xmlGetProp(cur_node, BAD_CAST "id");
			if (value)
			{
				sscanf(value, "%d", &trop_count);
				trophy->file = malloc(sizeof(trop_count));
				memcpy(trophy->file, &trop_count, sizeof(trop_count));
			}
			LOG("Trophy=%d [%c] '%s' (%s)", trop_count, trophy->type, trophy->name, trophy->codes);

			list_append(game->codes, trophy);
		}
	}

	/*free the document */
	xmlFreeDoc(doc);
	xmlCleanupParser();
	free(buffer);

	return list_count(game->codes);
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
	code_entry_t* item;
	char path[256], url[256];
	snprintf(path, sizeof(path), APOLLO_LOCAL_CACHE "%s.txt", game->title_id);
	snprintf(url, sizeof(url), ONLINE_URL "PS3/%s/", game->title_id);

	if (file_exists(path) == SUCCESS)
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

	game->codes = list_alloc();

	int cur_count = 0;
	ptr = data;
	
	while (ptr < end && *ptr && cur_count < game_count)
	{
		const char* content = ptr;

		while (ptr < end && *ptr != '\n' && *ptr != '\r')
		{
			ptr++;
		}
		*ptr++ = 0;

        LOG("ReadOnlineSaves() :: Reading %s...", content);
		if (content[12] == '=')
		{
			item = calloc(1, sizeof(code_entry_t));
			item->type = PATCH_COMMAND;
			asprintf(&item->codes, "%s/%.12s", game->title_id, content);

			content += 13;
			asprintf(&item->name, CHAR_ICON_ZIP " %s", content);

			item->options_count = 1;
			item->options = _createOptions(2, "Download to USB", CMD_DOWNLOAD_USB);
			list_append(game->codes, item);

			LOG("%d - [%s] %s", cur_count, item->codes, item->name);
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

	return cur_count;
}

list_t * ReadBackupList(const char* userPath)
{
	save_entry_t * item;
	code_entry_t * cmd;
	list_t *list = list_alloc();

	item = _createSaveEntry(SAVE_FLAG_PS3, CHAR_ICON_COPY " Export Licenses");
	asprintf(&item->path, EXDATA_PATH_HDD, apollo_config.user_id);
	item->type = FILE_TYPE_RIF;
	list_append(list, item);

	item = _createSaveEntry(SAVE_FLAG_PS3, CHAR_ICON_COPY " Import Licenses (USB 0)");
	asprintf(&item->path, IMPORT_RAP_PATH_USB0);
	item->type = FILE_TYPE_RAP;
	list_append(list, item);

	item = _createSaveEntry(SAVE_FLAG_PS3, CHAR_ICON_COPY " Import Licenses (USB 1)");
	asprintf(&item->path, IMPORT_RAP_PATH_USB1);
	item->type = FILE_TYPE_RAP;
	list_append(list, item);

	item = _createSaveEntry(SAVE_FLAG_PS2, CHAR_ICON_COPY " PS2 Classics: Import & Encrypt ISOs (USB 0)");
	asprintf(&item->path, IMPORT_PS2_PATH_USB0);
	item->type = FILE_TYPE_ISO;
	list_append(list, item);

	item = _createSaveEntry(SAVE_FLAG_PS2, CHAR_ICON_COPY " PS2 Classics: Import & Encrypt ISOs (USB 1)");
	asprintf(&item->path, IMPORT_PS2_PATH_USB1);
	item->type = FILE_TYPE_ISO;
	list_append(list, item);

	item = _createSaveEntry(SAVE_FLAG_PS2, CHAR_ICON_COPY " PS2 Classics: Import & Encrypt ISOs (HDD)");
	asprintf(&item->path, IMPORT_PS2_PATH_HDD);
	item->type = FILE_TYPE_ISO;
	list_append(list, item);

	item = _createSaveEntry(SAVE_FLAG_PS2, CHAR_ICON_COPY " PS2 Classics: Export & Decrypt BIN.ENC images");
	asprintf(&item->path, IMPORT_PS2_PATH_HDD);
	item->type = FILE_TYPE_BINENC;
	list_append(list, item);

	item = _createSaveEntry(SAVE_FLAG_PS3, CHAR_ICON_COPY " Export /dev_flash2");
	asprintf(&item->path, "/dev_flash2/");
	item->codes = list_alloc();
	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_ZIP " Zip /dev_flash2 to USB", 0);
	cmd->options_count = 1;
	cmd->options = _createOptions(2, "Save dev_flash2.zip to USB", CMD_EXP_FLASH2_USB);
	list_append(item->codes, cmd);
	list_append(list, item);

	item = _createSaveEntry(SAVE_FLAG_PS2, CHAR_ICON_COPY " Export PS2 .VM2 memory cards to USB");
	asprintf(&item->path, EXP_PS2_PATH_HDD);
	item->type = FILE_TYPE_VM2;
	list_append(list, item);

	item = _createSaveEntry(SAVE_FLAG_PS2, CHAR_ICON_COPY " Import PS2 raw memory cards (USB 0)");
	asprintf(&item->path, EXP_PS2_PATH_USB0);
	item->type = FILE_TYPE_PS2RAW;
	list_append(list, item);

	item = _createSaveEntry(SAVE_FLAG_PS2, CHAR_ICON_COPY " Import PS2 raw memory cards (USB 1)");
	asprintf(&item->path, EXP_PS2_PATH_USB1);
	item->type = FILE_TYPE_PS2RAW;
	list_append(list, item);

	return list;
}

int get_iso_files(save_entry_t * item)
{
	code_entry_t* cmd;
	DIR *d;
	struct dirent *dir;
	char name[256];
	item->codes = list_alloc();

	d = opendir(item->path);

	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			if (dir->d_type == DT_REG && (endsWith(dir->d_name, ".BIN") || endsWith(dir->d_name, ".ISO")))
			{
				snprintf(name, sizeof(name), "Encode %s", dir->d_name);

				cmd = _createCmdCode(PATCH_COMMAND, name, CMD_IMP_PS2ISO_USB);
				asprintf(&cmd->file, dir->d_name);
				list_append(item->codes, cmd);

				LOG("Adding File '%s'", cmd->file);
			}
		}
		closedir(d);
	}

	if (!list_count(item->codes))
	{
		list_free(item->codes);
		item->codes = NULL;
		return 0;
	}

	return list_count(item->codes);
}

int get_binenc_files(save_entry_t * item)
{
	code_entry_t* cmd;
	DIR *d;
	struct dirent *dir;
	char name[256];
	item->codes = list_alloc();

	d = opendir(item->path);

	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			if (dir->d_type == DT_REG && endsWith(dir->d_name, ".BIN.ENC"))
			{
				snprintf(name, sizeof(name), "Decode %s to .ISO", dir->d_name);

				cmd = _createCmdCode(PATCH_COMMAND, name, 0);
				asprintf(&cmd->file, dir->d_name);
				cmd->options_count = 1;
				cmd->options = _createOptions(3, "Save .ISO to USB", CMD_EXP_PS2_BINENC);
				asprintf(&cmd->options->name[2], "Save .ISO to HDD");
				asprintf(&cmd->options->value[2], "%c%c", CMD_EXP_PS2_BINENC, 2);
				list_append(item->codes, cmd);

				LOG("Adding File '%s'", cmd->file);
			}
		}
		closedir(d);
	}

	if (!list_count(item->codes))
	{
		list_free(item->codes);
		item->codes = NULL;
		return 0;
	}

	return list_count(item->codes);
}

int get_vm2_files(save_entry_t * item)
{
	code_entry_t* cmd;
	DIR *d;
	struct dirent *dir;
	char name[256];
	item->codes = list_alloc();

	d = opendir(item->path);

	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			if (dir->d_type == DT_REG && endsWith(dir->d_name, ".VM2"))
			{
				snprintf(name, sizeof(name), "Export %s to .VMC", dir->d_name);

				cmd = _createCmdCode(PATCH_COMMAND, name, 0);
				asprintf(&cmd->file, dir->d_name);
				cmd->options_count = 1;
				cmd->options = _createOptions(2, "Save .VMC to USB", CMD_EXP_VM2_RAW);
				list_append(item->codes, cmd);

				LOG("Adding File '%s'", cmd->file);
			}
		}
		closedir(d);
	}

	if (!list_count(item->codes))
	{
		list_free(item->codes);
		item->codes = NULL;
		return 0;
	}

	return list_count(item->codes);
}

int get_ps2_raw_files(save_entry_t * item)
{
	code_entry_t* cmd;
	DIR *d;
	struct dirent *dir;
	char name[256];
	item->codes = list_alloc();

	d = opendir(item->path);

	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			if (dir->d_type == DT_REG && endsWith(dir->d_name, ".VMC"))
			{
				snprintf(name, sizeof(name), "Import %s to .VM2 (HDD)", dir->d_name);

				cmd = _createCmdCode(PATCH_COMMAND, name, CMD_IMP_PS2VMC_USB);
				asprintf(&cmd->file, dir->d_name);
				list_append(item->codes, cmd);

				LOG("Adding File '%s'", cmd->file);
			}
		}
		closedir(d);
	}

	if (!list_count(item->codes))
	{
		list_free(item->codes);
		item->codes = NULL;
		return 0;
	}

	return list_count(item->codes);
}

int ReadBackupCodes(save_entry_t * bup)
{
	code_entry_t * cmd;
	char fext[5] = "";

	switch(bup->type)
	{
	case FILE_TYPE_ISO:
		return get_iso_files(bup);

	case FILE_TYPE_BINENC:
		return get_binenc_files(bup);

	case FILE_TYPE_VM2:
		return get_vm2_files(bup);

	case FILE_TYPE_PS2RAW:
		return get_ps2_raw_files(bup);

	case FILE_TYPE_RIF:
		sprintf(fext, ".rif");
		break;

	case FILE_TYPE_RAP:
		sprintf(fext, ".rap");
		break;

	default:
		return 0;
	}

	bup->codes = list_alloc();

	LOG("Loading backup files from '%s'...", bup->path);

	if (bup->type == FILE_TYPE_RIF)
	{
		cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_ZIP " Backup All Licenses to .Zip", 0);
		cmd->options_count = 1;
		cmd->options = _createOptions(2, "Save .Zip to USB", CMD_EXP_EXDATA_USB);
		list_append(bup->codes, cmd);

		cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " Export All Licenses as .RAPs", 0);
		cmd->options_count = 1;
		cmd->options = _createOptions(3, "Save .RAPs to USB", CMD_EXP_LICS_RAPS);
		asprintf(&cmd->options->name[2], "Save .RAPs to HDD");
		asprintf(&cmd->options->value[2], "%c%c", CMD_EXP_LICS_RAPS, 2);
		list_append(bup->codes, cmd);
	}

	if (bup->type == FILE_TYPE_RAP)
	{
		cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " Import All .RAPs as .RIF Licenses", CMD_IMP_EXDATA_USB);
		list_append(bup->codes, cmd);
	}

	DIR *d;
	struct dirent *dir;
	d = opendir(bup->path);

	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0  &&
				endsWith(dir->d_name, fext))
			{
				cmd = _createCmdCode(PATCH_COMMAND, dir->d_name, 0);
				*strrchr(cmd->name, '.') = 0;

				if (bup->type == FILE_TYPE_RIF)
				{
					cmd->options_count = 1;
					cmd->options = _createOptions(3, "Save .RAP to USB", CMD_EXP_LICS_RAPS);
					asprintf(&cmd->options->name[2], "Save .RAP to HDD");
					asprintf(&cmd->options->value[2], "%c%c", CMD_EXP_LICS_RAPS, 2);
				}
				else if (bup->type == FILE_TYPE_RAP)
				{
					sprintf(cmd->codes, "%c", CMD_IMP_EXDATA_USB);
				}

				asprintf(&cmd->file, dir->d_name);
				list_append(bup->codes, cmd);

				LOG("Added File '%s'", cmd->file);
			}
		}
		closedir(d);
	}

	LOG("%d items loaded", list_count(bup->codes));

	return list_count(bup->codes);
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
void UnloadGameList(list_t * list)
{
	list_node_t *node, *nc;
	save_entry_t *item;
	code_entry_t *code;

	for (node = list_head(list); (item = list_get(node)); node = list_next(node))
	{
		if (item->name)
		{
			free(item->name);
			item->name = NULL;
		}

		if (item->path)
		{
			free(item->path);
			item->path = NULL;
		}

		if (item->title_id)
		{
			free(item->title_id);
			item->title_id = NULL;
		}
		
		if (item->codes)
		{
			for (nc = list_head(item->codes); (code = list_get(nc)); nc = list_next(nc))
			{
				if (code->codes)
				{
					free (code->codes);
					code->codes = NULL;
				}
				if (code->name)
				{
					free (code->name);
					code->name = NULL;
				}
				if (code->options && code->options_count > 0)
				{
					for (int z = 0; z < code->options_count; z++)
					{
						if (code->options[z].line)
							free(code->options[z].line);
						if (code->options[z].name)
							free(code->options[z].name);
						if (code->options[z].value)
							free(code->options[z].value);
					}
					
					free (code->options);
				}
			}
			
			list_free(item->codes);
			item->codes = NULL;
		}
	}

	list_free(list);
	
	LOG("UnloadGameList() :: Successfully unloaded game list");
}

int sortCodeList_Compare(const void* a, const void* b)
{
	return strcasecmp(((code_entry_t*) a)->name, ((code_entry_t*) b)->name);
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
int sortSaveList_Compare(const void* a, const void* b)
{
	return strcasecmp(((save_entry_t*) a)->name, ((save_entry_t*) b)->name);
}

void read_savegames(const char* userPath, list_t *list, uint32_t flag)
{
	DIR *d;
	struct dirent *dir;
	save_entry_t *item;
	char sfoPath[256];

	d = opendir(userPath);

	if (!d)
		return;

	while ((dir = readdir(d)) != NULL)
	{
		if (dir->d_type != DT_DIR || strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
			continue;

		snprintf(sfoPath, sizeof(sfoPath), "%s%s/PARAM.SFO", userPath, dir->d_name);
		if (file_exists(sfoPath) == SUCCESS)
		{
			LOG("Reading %s...", sfoPath);

			sfo_context_t* sfo = sfo_alloc();
			if (sfo_read(sfo, sfoPath) < 0) {
				LOG("Unable to read from '%s'", sfoPath);
				sfo_free(sfo);
				continue;
			}

			char *sfo_data = (char*) sfo_get_param_value(sfo, "TITLE");
			item = _createSaveEntry(flag, sfo_data);

			asprintf(&item->path, "%s%s/", userPath, dir->d_name);
			asprintf(&item->title_id, "%.9s", dir->d_name);

			if (flag & SAVE_FLAG_PS3)
			{
				sfo_data = (char*) sfo_get_param_value(sfo, "ATTRIBUTE");
				item->flags |=	(sfo_data[0] ? SAVE_FLAG_LOCKED : 0);

				sprintf(sfoPath, "%016lx", apollo_config.account_id);
				sfo_data = (char*) sfo_get_param_value(sfo, "ACCOUNT_ID");
				if (strncmp(sfo_data, sfoPath, 16) == 0)
					item->flags |=	SAVE_FLAG_OWNER;
			}

			sfo_free(sfo);
				
			LOG("[%s] F(%d) name '%s'", item->title_id, item->flags, item->name);
			list_append(list, item);
		}
	}

	closedir(d);
}

void read_psv_savegames(const char* userPath, list_t *list)
{
	DIR *d;
	struct dirent *dir;
	save_entry_t *item;
	char psvPath[256];
	char data[0x100];

	d = opendir(userPath);

	if (!d)
		return;

	while ((dir = readdir(d)) != NULL)
	{
		if (dir->d_type != DT_REG || !endsWith(dir->d_name, ".PSV"))
			continue;

		snprintf(psvPath, sizeof(psvPath), "%s%s", userPath, dir->d_name);
		LOG("Reading %s...", psvPath);

		FILE *psvf = fopen(psvPath, "rb");
		if (!psvf) {
			LOG("Unable to open '%s'", psvPath);
			continue;
		}

		fread(data, 1, sizeof(data), psvf);
		if (memcmp(data, "\x00VSP", 4) != 0)
		{
			LOG("Not a PSV file");
			fclose(psvf);
			continue;
		}

		uint8_t type = data[0x3C];
		uint32_t pos = ES32(*(uint32_t*)(data + 0x44));

		fseek(psvf, pos, SEEK_SET);
		fread(data, 1, sizeof(data), psvf);
		fclose(psvf);

		item = (save_entry_t *)malloc(sizeof(save_entry_t));
		item->codes = NULL;
		item->flags = SAVE_FLAG_PSV | (type == 1 ? SAVE_FLAG_PS1 : SAVE_FLAG_PS2);

		asprintf(&item->path, "%s%s", userPath, dir->d_name);
		asprintf(&item->title_id, "%.12s", dir->d_name);

		//PS2 Title offset 0xC0
		//PS1 Title offset 0x04
		item->name = sjis2utf8(data + (type == 1 ? 0x04 : 0xC0));
			
		LOG("[%s] F(%d) name '%s'", item->title_id, item->flags, item->name);
		list_append(list, item);
	}

	closedir(d);
}

void read_psx_savegames(const char* userPath, list_t *list)
{
	DIR *d;
	struct dirent *dir;
	save_entry_t *item;
	char psvPath[256];
	char data[64];
	int type, toff;

	d = opendir(userPath);

	if (!d)
		return;

	while ((dir = readdir(d)) != NULL)
	{
		if (dir->d_type != DT_REG)
			continue;

		if (endsWith(dir->d_name, ".PSX"))
		{
			toff = 0;
			type = FILE_TYPE_PSX;
		}
		else if (endsWith(dir->d_name, ".MCS"))
		{
			toff = 0x0A;
			type = FILE_TYPE_MCS;
		}
		else if (endsWith(dir->d_name, ".MAX"))
		{
			toff = 0x10;
			type = FILE_TYPE_MAX;
		}
		else if (endsWith(dir->d_name, ".CBS"))
		{
			toff = 0x14;
			type = FILE_TYPE_CBS;
		}
		else if (endsWith(dir->d_name, ".PSU"))
		{
			toff = 0x40;
			type = FILE_TYPE_PSU;
		}
		else if (endsWith(dir->d_name, ".XPS") || endsWith(dir->d_name, ".SPS"))
		{
			toff = 0x04;
			type = FILE_TYPE_XPS;
		}
		else
			continue;

		snprintf(psvPath, sizeof(psvPath), "%s%s", userPath, dir->d_name);
		LOG("Reading %s...", psvPath);

		FILE *fp = fopen(psvPath, "rb");
		if (!fp) {
			LOG("Unable to open '%s'", psvPath);
			continue;
		}

		fseek(fp, toff, SEEK_SET);
		fread(data, 1, sizeof(data), fp);
		fclose(fp);

		item = _createSaveEntry(SAVE_FLAG_PS2, dir->d_name);
		set_psx_import_codes(item);

		if (type == FILE_TYPE_PSX || type == FILE_TYPE_MCS)
			item->flags = SAVE_FLAG_PS1;

		item->type = type;
		asprintf(&item->path, "%s%s", userPath, dir->d_name);
		asprintf(&item->title_id, "%.12s", data);
			
		LOG("[%s] F(%d) name '%s'", item->title_id, item->flags, item->name);
		list_append(list, item);
	}

	closedir(d);
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
list_t * ReadUserList(const char* userPath)
{
	save_entry_t *item;
	code_entry_t *cmd;
	list_t *list;
	char savePath[256];
	char * save_paths[3];

	if (dir_exists(userPath) != SUCCESS)
		return NULL;

	list = list_alloc();

	item = _createSaveEntry(SAVE_FLAG_PS3, CHAR_ICON_COPY " Bulk Save Management");
	item->codes = list_alloc();

	if (strncmp(userPath, "/dev_hdd0/", 10) == 0)
	{
		asprintf(&item->path, SAVES_PATH_HDD, apollo_config.user_id);

		cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " Copy all Saves to USB", 0);
		cmd->options_count = 1;
		cmd->options = _createOptions(2, "Copy Saves to USB", CMD_COPY_SAVES_USB);
		list_append(item->codes, cmd);

		save_paths[0] = PS3_SAVES_PATH_HDD;
		save_paths[1] = PS2_SAVES_PATH_HDD;
		save_paths[2] = PSP_SAVES_PATH_HDD;
	}
	else
	{
		asprintf(&item->path, "%s", userPath);

		cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_SIGN " Resign & Unlock all Saves", CMD_RESIGN_SAVES_USB);
		list_append(item->codes, cmd);

		cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " Copy all Saves to HDD", CMD_COPY_SAVES_HDD);
		list_append(item->codes, cmd);

		save_paths[0] = PS3_SAVES_PATH_USB;
		save_paths[1] = PS2_SAVES_PATH_USB;
		save_paths[2] = PSP_SAVES_PATH_USB;
	}
	list_append(list, item);

	snprintf(savePath, sizeof(savePath), "%s%s", userPath, save_paths[0]);
	read_savegames(savePath, list, SAVE_FLAG_PS3);

	snprintf(savePath, sizeof(savePath), "%s%s", userPath, save_paths[1]);
	read_savegames(savePath, list, SAVE_FLAG_PS2);

	snprintf(savePath, sizeof(savePath), "%s%s", userPath, save_paths[2]);
	read_savegames(savePath, list, SAVE_FLAG_PSP);

	if (strncmp(userPath, "/dev_usb00", 10) == 0)
	{
		snprintf(savePath, sizeof(savePath), "%s%s", userPath, PSV_SAVES_PATH_USB);
		read_psv_savegames(savePath, list);

		snprintf(savePath, sizeof(savePath), "%s%s", userPath, PS2_IMP_PATH_USB);
		read_psx_savegames(savePath, list);

		snprintf(savePath, sizeof(savePath), "%s%s", userPath, PS1_IMP_PATH_USB);
		read_psx_savegames(savePath, list);
	}

	return list;
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
list_t * ReadOnlineList(const char* urlPath)
{
	list_t *list;
	save_entry_t *item;
	const char* path = APOLLO_LOCAL_CACHE "games.txt";

	if (file_exists(path) == SUCCESS)
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

	list = list_alloc();

	while (ptr < end && *ptr)
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
			item = _createSaveEntry(SAVE_FLAG_PS3, content + 10);
			asprintf(&item->title_id, "%.9s", content);

			LOG("+ [%s] %s", item->title_id, item->name);
			list_append(list, item);
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

	return list;
}

list_t * ReadTrophyList(const char* userPath)
{
	DIR *d;
	struct dirent *dir;
	save_entry_t *item;
	code_entry_t *cmd;
	list_t *list;
	char filePath[256];
	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL;
	char *value, *buffer;
	long bufferLen;

	if (dir_exists(userPath) != SUCCESS)
		return NULL;

	list = list_alloc();

	item = _createSaveEntry(SAVE_FLAG_PS3, CHAR_ICON_COPY " Export Trophies");
	asprintf(&item->path, userPath);
	item->codes = list_alloc();
	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " Backup Trophies to USB", 0);
	cmd->options_count = 1;
	cmd->options = _createOptions(2, "Save Trophies to USB", CMD_EXP_TROPHY_USB);
	list_append(item->codes, cmd);
	list_append(list, item);

	d = opendir(userPath);

	if (!d)
		return list;

	while ((dir = readdir(d)) != NULL)
	{
		if (dir->d_type != DT_DIR || strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
			continue;

		snprintf(filePath, sizeof(filePath), "%s%s/TROPCONF.SFM", userPath, dir->d_name);
		if (file_exists(filePath) == SUCCESS)
		{
			LOG("Reading %s...", filePath);

			buffer = readFile(filePath, &bufferLen);
			buffer[bufferLen]=0;

			/*parse the file and get the DOM */
			doc = xmlReadMemory(buffer + 0x40, bufferLen - 0x40, NULL, NULL, XML_PARSE_NONET);

			if (!doc)
			{
				LOG("XML: could not parse file %s", filePath);
				free(buffer);
				continue;
			}

			/*Get the root element node */
			root_element = xmlDocGetRootElement(doc);
			value = _get_xml_node_value(root_element->children, BAD_CAST "title-name");

			item = _createSaveEntry(SAVE_FLAG_PS3 | SAVE_FLAG_TROPHY, value);
			item->type = FILE_TYPE_TROPHY;
			asprintf(&item->path, "%s%s/", userPath, dir->d_name);

			value = _get_xml_node_value(root_element->children, BAD_CAST "npcommid");
			item->title_id = strdup(value);

			/*free the document */
			xmlFreeDoc(doc);
			xmlCleanupParser();
			free(buffer);
				
			LOG("[%s] F(%d) name '%s'", item->title_id, item->flags, item->name);
			list_append(list, item);
		}
	}

	closedir(d);

	return list;
}
