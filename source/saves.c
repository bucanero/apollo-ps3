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
#include <mini18n.h>

#include "saves.h"
#include "common.h"
#include "sfo.h"
#include "settings.h"
#include "util.h"
#include "pfd.h"
#include "trophy.h"
#include "ps1card.h"
#include "ps2mc.h"
#include "mcio.h"

#define UTF8_CHAR_STAR		"\xE2\x98\x85"

#define CHAR_ICON_NET		"\x09"
#define CHAR_ICON_ZIP		"\x0C"
#define CHAR_ICON_VMC		"\x0B"
#define CHAR_ICON_COPY		"\x0E"
#define CHAR_ICON_SIGN		"\x06"
#define CHAR_ICON_USER		"\x07"
#define CHAR_ICON_LOCK		"\x08"
#define CHAR_ICON_WARN		"\x0F"


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
static char* endsWith(const char * a, const char * b)
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
 * Description:		reads the contents of a file into a new buffer (null terminated)
 * Arguments:
 *	path:			Path to file
 * Return:			Pointer to the newly allocated buffer
 */
char * readTextFile(const char * path, long* size)
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

	string[fsize] = 0;
	if (size)
		*size = fsize;

	return string;
}

static code_entry_t* _createCmdCode(uint8_t type, const char* icon, const char* name, char code)
{
	code_entry_t* entry = (code_entry_t *)calloc(1, sizeof(code_entry_t));
	entry->type = type;
	asprintf(&entry->codes, "%c", code);
	if (name)
		asprintf(&entry->name, "%s%s", icon, name);

	return entry;
}

static option_entry_t* _initOptions(int count)
{
	option_entry_t* options = (option_entry_t*)calloc(count, sizeof(option_entry_t));

	for(int i = 0; i < count; i++)
	{
		options[i].sel = -1;
		options[i].opts = list_alloc();
	}

	return options;
}

static void _createOptions(code_entry_t* code, const char* name, char value)
{
	char path[32];
	option_value_t* optval;
	code->options_count = 1;
	code->options = _initOptions(1);

	for (int i = 0; i < MAX_USB_DEVICES; i++)
	{
		snprintf(path, sizeof(path), USB_PATH, i);
		if (i <= 1 || dir_exists(path) == SUCCESS)
		{
			optval = malloc(sizeof(option_value_t));
			asprintf(&optval->name, "%s %d", name, i);
			asprintf(&optval->value, "%c%c", value, i);
			list_append(code->options[0].opts, optval);
		}
	}

	return;
}

static save_entry_t* _createSaveEntry(uint16_t flag, const char* icon, const char* name)
{
	save_entry_t* entry = (save_entry_t *)calloc(1, sizeof(save_entry_t));
	entry->flags = flag;
	asprintf(&entry->name, "%s%s", icon, name);

	return entry;
}

static option_entry_t* _getFileOptions(const char* save_path, const char* mask, uint8_t is_cmd)
{
	DIR *d;
	struct dirent *dir;
	option_value_t* optval;
	option_entry_t* opt;

	d = opendir(save_path);
	if (!d)
	{
		opt = _initOptions(1);
		optval = malloc(sizeof(option_value_t));
		asprintf(&optval->name, CHAR_ICON_WARN " --- %s%s --- " CHAR_ICON_WARN, save_path, mask);
		asprintf(&optval->value, "%c", CMD_CODE_NULL);
		list_append(opt[0].opts, optval);
		return opt;
	}

	LOG("Loading filenames {%s} from '%s'...", mask, save_path);

	opt = _initOptions(1);

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

			optval = malloc(sizeof(option_value_t));
			asprintf(&optval->name, "%s", dir->d_name);
			if (is_cmd)
				asprintf(&optval->value, "%c", is_cmd);
			else
				asprintf(&optval->value, "%s", mask);

			list_append(opt[0].opts, optval);
		}
	}

	closedir(d);

	if (list_count(opt[0].opts) == 0)
	{
		optval = malloc(sizeof(option_value_t));
		asprintf(&optval->name, CHAR_ICON_WARN " --- %s%s --- " CHAR_ICON_WARN, save_path, mask);
		asprintf(&optval->value, "%c", CMD_CODE_NULL);
		list_append(opt[0].opts, optval);
		return opt;
	}

	return opt;
}

static void _addBackupCommands(save_entry_t* item)
{
	code_entry_t* cmd;
	option_value_t* optval;

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_SIGN " ", _("Apply Changes & Resign"), CMD_RESIGN_SAVE);
	list_append(item->codes, cmd);

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_USER " ", _("View Save Details"), CMD_VIEW_DETAILS);
	list_append(item->codes, cmd);

	cmd = _createCmdCode(PATCH_NULL, NULL, NULL, CMD_CODE_NULL);
	asprintf(&cmd->name, "----- " UTF8_CHAR_STAR " %s " UTF8_CHAR_STAR " -----", _("File Backup"));
	list_append(item->codes, cmd);

	if (item->flags & SAVE_FLAG_HDD)
	{
		cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " ", _("Copy save game to USB"), CMD_CODE_NULL);
		_createOptions(cmd, _("Copy Save to USB"), CMD_COPY_SAVE_USB);
		list_append(item->codes, cmd);
	
		if (apollo_config.ftp_url[0])
		{
			cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_NET " ", _("Upload save backup to FTP"), CMD_UPLOAD_SAVE);
			list_append(item->codes, cmd);
		}
	}
	else
	{
		cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " ", _("Copy save game to HDD"), CMD_COPY_SAVE_HDD);
		list_append(item->codes, cmd);

		cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_WARN " ", _("Delete save game"), CMD_DELETE_SAVE);
		list_append(item->codes, cmd);
	}

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_ZIP " ", _("Export save game to Zip"), CMD_CODE_NULL);
	_createOptions(cmd, _("Export Zip to USB"), CMD_EXPORT_ZIP_USB);
	optval = malloc(sizeof(option_value_t));
	asprintf(&optval->name, "%s", _("Export Zip to HDD"));
	asprintf(&optval->value, "%c%c", CMD_EXPORT_ZIP_HDD, STORAGE_HDD);
	list_append(cmd->options[0].opts, optval);
	list_append(item->codes, cmd);

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " ", _("Decrypt save game files"), CMD_CODE_NULL);
	cmd->options_count = 1;
	cmd->options = _getFileOptions(item->path, "*", CMD_DECRYPT_FILE);
	list_append(item->codes, cmd);

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " ", _("Import decrypted save files"), CMD_CODE_NULL);
	cmd->options_count = 1;
	cmd->options = _getFileOptions(item->path, "*", CMD_IMPORT_DATA_FILE);
	list_append(item->codes, cmd);

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_SIGN " ", _("Hex Edit save game files"), CMD_CODE_NULL);
	cmd->options_count = 1;
	cmd->options = _getFileOptions(item->path, "*", CMD_HEX_EDIT_FILE);
	list_append(item->codes, cmd);
}

static option_entry_t* _getSaveTitleIDs(const char* title_id)
{
	option_value_t* optval;
	option_entry_t* opt;
	char tmp[16];
	const char *ptr;
	const char *tid = get_game_title_ids(title_id);

	if (!tid)
		tid = title_id;

	LOG("Adding TitleIDs=%s", tid);
	opt = _initOptions(1);

	ptr = tid;
	while (*ptr++)
	{
		if ((*ptr == '/') || (*ptr == 0))
		{
			memset(tmp, 0, sizeof(tmp));
			strncpy(tmp, tid, ptr - tid);
			optval = malloc(sizeof(option_value_t));
			asprintf(&optval->name, "%s", tmp);
			asprintf(&optval->value, "%c", SFO_CHANGE_TITLE_ID);
			list_append(opt[0].opts, optval);
			tid = ptr+1;
		}
	}

	return opt;
}

static void _addSfoCommands(save_entry_t* save)
{
	code_entry_t* cmd;
	option_value_t* optval;

	cmd = _createCmdCode(PATCH_NULL, NULL, NULL, CMD_CODE_NULL);
	asprintf(&cmd->name, "----- " UTF8_CHAR_STAR " %s " UTF8_CHAR_STAR " -----", _("SFO Patches"));
	list_append(save->codes, cmd);

	cmd = _createCmdCode(PATCH_SFO, CHAR_ICON_USER " ", _("Change Account ID"), SFO_CHANGE_ACCOUNT_ID);
	cmd->options_count = 1;
	cmd->options = _initOptions(1);

	optval = malloc(sizeof(option_value_t));
	optval->name = strdup("Remove ID/Offline");
	optval->value = calloc(1, SFO_ACCOUNT_ID_SIZE);
	list_append(cmd->options[0].opts, optval);

	optval = malloc(sizeof(option_value_t));
	optval->name = strdup("Fake Owner/Rebug");
	optval->value = strdup("ffffffffffffffff");
	list_append(cmd->options[0].opts, optval);

	add_xml_owners(APOLLO_PATH OWNER_XML_FILE, cmd->options[0].opts);
	list_append(save->codes, cmd);

	cmd = _createCmdCode(PATCH_SFO, CHAR_ICON_USER " ", _("Remove Console ID"), SFO_REMOVE_PSID);
	list_append(save->codes, cmd);

	if (save->flags & SAVE_FLAG_LOCKED)
	{
		cmd = _createCmdCode(PATCH_SFO, CHAR_ICON_LOCK " ", _("Remove copy protection"), SFO_UNLOCK_COPY);
		list_append(save->codes, cmd);
	}

	cmd = _createCmdCode(PATCH_SFO, CHAR_ICON_USER " ", _("Change Region Title ID"), SFO_CHANGE_TITLE_ID);
	cmd->options_count = 1;
	cmd->options = _getSaveTitleIDs(save->title_id);
	list_append(save->codes, cmd);
}

static int set_psp_codes(save_entry_t* item)
{
	code_entry_t* cmd;
	item->codes = list_alloc();

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_USER " ", _("View Save Details"), CMD_VIEW_DETAILS);
	list_append(item->codes, cmd);

	return list_count(item->codes);
}

static void add_vmp_commands(save_entry_t* save)
{
	code_entry_t* cmd;

	cmd = _createCmdCode(PATCH_NULL, NULL, NULL, CMD_CODE_NULL);
	asprintf(&cmd->name, "----- " UTF8_CHAR_STAR " %s " UTF8_CHAR_STAR " -----", _("Virtual Memory Card"));
	list_append(save->codes, cmd);

	if (endsWith(save->path, ".VMP"))
	{
		cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_SIGN " ", _("Resign Memory Card"), CMD_RESIGN_VMP);
		list_append(save->codes, cmd);
	}

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " ", _("Export Memory Card to .VM1"), CMD_EXP_VMP2MCR);
	list_append(save->codes, cmd);

	cmd = _createCmdCode(PATCH_COMMAND, NULL, NULL, CMD_CODE_NULL);
	asprintf(&cmd->name, CHAR_ICON_COPY " %s %s", _("Import .VM1 file to"), strrchr(save->path, '/')+1);
	cmd->options_count = 1;
	cmd->options = _getFileOptions(VMC_PS2_PATH_HDD, "*.VM1", CMD_IMP_MCR2VMP);
	list_append(save->codes, cmd);

	return;
}

static option_entry_t* get_file_entries(const char* path, const char* mask)
{
	return _getFileOptions(path, mask, CMD_CODE_NULL);
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
	list_node_t* node;
	code_entry_t * code;
	char filePath[256];
	char * buffer = NULL;

	if (save->flags & SAVE_FLAG_PSP)
		return set_psp_codes(save);

	save->codes = list_alloc();

	_addBackupCommands(save);
	_addSfoCommands(save);

	snprintf(filePath, sizeof(filePath), APOLLO_DATA_PATH "%s.savepatch", save->title_id);
	if ((buffer = readTextFile(filePath, NULL)) == NULL)
		return list_count(save->codes);

	code = _createCmdCode(PATCH_NULL, NULL, NULL, CMD_CODE_NULL);
	asprintf(&code->name, "----- " UTF8_CHAR_STAR " %s " UTF8_CHAR_STAR " -----", _("Cheats"));
	list_append(save->codes, code);

	code = _createCmdCode(PATCH_COMMAND, CHAR_ICON_USER " ", _("View Raw Patch File"), CMD_VIEW_RAW_PATCH);
	list_append(save->codes, code);

	node = list_tail(save->codes);
	LOG("Loading BSD codes '%s'...", filePath);
	load_patch_code_list(buffer, save->codes, &get_file_entries, save->path);
	free (buffer);

	// Check for any codes that are not valid for this game
	for (node = list_next(node); (code = list_get(node)); node = list_next(node))
		if (strchr(code->file, '\\') != NULL && code->file[1] != '~')
		{
			buffer = strdup(code->file);
			strchr(buffer, '\\')[0] = 0;
			if(!wildcard_match_icase(save->dir_name, buffer))
			{
				LOG("(%s) Disabled code '%s'", buffer, code->name);
				code->flags |= (APOLLO_CODE_FLAG_ALERT | APOLLO_CODE_FLAG_DISABLED);
			}
			free(buffer);
		}

	LOG("Loaded %d codes", list_count(save->codes));

	return list_count(save->codes);
}

static char* _get_xml_node_value(xmlNode * a_node, const xmlChar* node_name)
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

static int get_usb_trophies(save_entry_t* item)
{
	DIR *d;
	struct dirent *dir;
	code_entry_t * cmd;
	char filePath[256];
	long bufferLen;
	char * buffer = NULL;
	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL;
	char *name, *commid;

	d = opendir(item->path);
	if (!d)
		return 0;

	item->codes = list_alloc();
	while ((dir = readdir(d)) != NULL)
	{
		if (dir->d_type != DT_DIR || strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
			continue;

		snprintf(filePath, sizeof(filePath), "%s%s/TROPCONF.SFM", item->path, dir->d_name);
		if ((buffer = readTextFile(filePath, &bufferLen)) == NULL)
			continue;

		LOG("Reading %s...", filePath);
		/*parse the file and get the DOM */
		doc = xmlParseMemory(buffer + 0x40, bufferLen - 0x40);
		if (!doc)
		{
			LOG("XML: could not parse file %s", filePath);
			free(buffer);
			continue;
		}

		/*Get the root element node */
		root_element = xmlDocGetRootElement(doc);
		name = _get_xml_node_value(root_element->children, BAD_CAST "title-name");
		commid = _get_xml_node_value(root_element->children, BAD_CAST "npcommid");
		snprintf(filePath, sizeof(filePath), " (%s)", commid);

		cmd = _createCmdCode(PATCH_COMMAND, name, filePath, CMD_IMP_TROPHY_HDD);
		asprintf(&cmd->file, "%s%s/", item->path, dir->d_name);

		/*free the document */
		xmlFreeDoc(doc);
		xmlCleanupParser();
		free(buffer);

		LOG("[%s] F(%X) name '%s'", cmd->file, cmd->flags, cmd->name);
		list_append(item->codes, cmd);
	}

	closedir(d);

	return list_count(item->codes);
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
	u8* usr_data;
	tropTimeInfo_t* tti;

	// Import trophies from USB
	if (game->type == FILE_TYPE_MENU)
		return get_usb_trophies(game);

	snprintf(filePath, sizeof(filePath), "%s" "TROPCONF.SFM", game->path);
	if ((buffer = readTextFile(filePath, &bufferLen)) == NULL)
		return 0;

	/*parse the file and get the DOM */
	doc = xmlParseMemory(buffer + 0x40, bufferLen - 0x40);
	if (!doc)
	{
		LOG("XML: could not parse file %s", filePath);
		free(buffer);
		return 0;
	}

	snprintf(filePath, sizeof(filePath), "%s" "TROPUSR.DAT", game->path);
	if (read_buffer(filePath, &usr_data, NULL) != SUCCESS)
	{
		LOG("Cannot open %s.", filePath);
		free(buffer);
		return 0;
	}

	if ((tti = getTrophyTimeInfo(usr_data)) == NULL)
	{
		LOG("Cannot parse %s.", filePath);
		free(usr_data);
		free(buffer);
		return 0;
	}

	game->codes = list_alloc();

	trophy = _createCmdCode(PATCH_COMMAND, CHAR_ICON_SIGN " ", _("Apply Changes & Resign Trophy Set"), CMD_RESIGN_TROPHY);
	list_append(game->codes, trophy);

	trophy = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " ", _("Backup Trophy Set to USB"), CMD_CODE_NULL);
	trophy->file = strdup(game->path);
	_createOptions(trophy, _("Copy Trophy to USB"), CMD_EXP_TROPHY_USB);
	list_append(game->codes, trophy);

	trophy = _createCmdCode(PATCH_COMMAND, CHAR_ICON_ZIP " ", _("Export Trophy Set to Zip"), CMD_CODE_NULL);
	trophy->file = strdup(game->path);
	_createOptions(trophy, _("Save .Zip to USB"), CMD_EXPORT_ZIP_USB);
	list_append(game->codes, trophy);

	trophy = _createCmdCode(PATCH_NULL, NULL, NULL, CMD_CODE_NULL);
	asprintf(&trophy->name, "----- " UTF8_CHAR_STAR " %s " UTF8_CHAR_STAR " -----", _("Trophies"));
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
			trophy = _createCmdCode(PATCH_NULL, "   ", value, CMD_CODE_NULL);

			value = _get_xml_node_value(cur_node->children, BAD_CAST "detail");
			asprintf(&trophy->codes, "%s\n", value);

			value = (char*) xmlGetProp(cur_node, BAD_CAST "ttype");

			switch (value[0])
			{
			case 'B':
				trophy->name[0] = CHAR_TRP_BRONZE;
				break;

			case 'S':
				trophy->name[0] = CHAR_TRP_SILVER;
				break;

			case 'G':
				trophy->name[0] = CHAR_TRP_GOLD;
				break;

			case 'P':
				trophy->name[0] = CHAR_TRP_PLATINUM;
				break;

			default:
				break;
			}

			value = (char*) xmlGetProp(cur_node, BAD_CAST "id");
			if (value)
			{
				sscanf(value, "%d", &trop_count);
				trophy->file = malloc(sizeof(trop_count));
				memcpy(trophy->file, &trop_count, sizeof(trop_count));

				if (!tti[trop_count].unlocked)
					trophy->name[1] = CHAR_TAG_LOCKED;

				// if trophy has been synced, we can't allow changes
				if (tti[trop_count].syncState & TROP_STATE_SYNCED)
					trophy->name[1] = CHAR_TRP_SYNC;
				else
					trophy->type = (tti[trop_count].unlocked) ? PATCH_TROP_LOCK : PATCH_TROP_UNLOCK;
			}
			LOG("Trophy=%d [%d] '%s' (%s)", trop_count, trophy->type, trophy->name, trophy->codes);

			list_append(game->codes, trophy);
		}
	}

	/*free the document */
	xmlFreeDoc(doc);
	xmlCleanupParser();
	free(buffer);
	free(usr_data);

	return list_count(game->codes);
}

static void add_vmc_import_saves(list_t* list, const char* path, const char* folder)
{
	code_entry_t * cmd;
	DIR *d;
	struct dirent *dir;
	char psvPath[256];

	snprintf(psvPath, sizeof(psvPath), "%s%s", path, folder);
	d = opendir(psvPath);

	if (!d)
		return;

	while ((dir = readdir(d)) != NULL)
	{
		if (!endsWith(dir->d_name, ".PSV") && !endsWith(dir->d_name, ".MCS") && !endsWith(dir->d_name, ".PSX") &&
			!endsWith(dir->d_name, ".PS1") && !endsWith(dir->d_name, ".MCB") && !endsWith(dir->d_name, ".PDA"))
			continue;

		// check for PS1 PSV saves
		if (endsWith(dir->d_name, ".PSV"))
		{
			snprintf(psvPath, sizeof(psvPath), "%s%s%s", path, folder, dir->d_name);
			if (read_file(psvPath, (uint8_t*) psvPath, 0x40) < 0 || psvPath[0x3C] != 0x01)
				continue;
		}

		snprintf(psvPath, sizeof(psvPath), CHAR_ICON_COPY "%c ", CHAR_TAG_PS1);
		cmd = _createCmdCode(PATCH_COMMAND, psvPath, dir->d_name, CMD_IMP_VMC1SAVE);
		asprintf(&cmd->file, "%s%s%s", path, folder, dir->d_name);
		cmd->codes[1] = FILE_TYPE_PS1;
		list_append(list, cmd);

		LOG("[%s] F(%X) name '%s'", cmd->file, cmd->flags, cmd->name+2);
	}

	closedir(d);
}

static void read_vmc1_files(const char* vmcPath, list_t* list)
{
	save_entry_t *item;
	DIR *d;
	struct dirent *dir;

	d = opendir(vmcPath);
	if (!d)
		return;

	while ((dir = readdir(d)) != NULL)
	{
		if (!endsWith(dir->d_name, ".VMP") && !endsWith(dir->d_name, ".MCR") && !endsWith(dir->d_name, ".GME") &&
			!endsWith(dir->d_name, ".VM1") && !endsWith(dir->d_name, ".MCD") && !endsWith(dir->d_name, ".VGS") &&
			!endsWith(dir->d_name, ".VMC") && !endsWith(dir->d_name, ".BIN") && !endsWith(dir->d_name, ".SRM"))
			continue;

		item = _createSaveEntry(SAVE_FLAG_PS1 | SAVE_FLAG_VMC, "", dir->d_name);
		item->type = FILE_TYPE_VMC;
		item->title_id = strdup("VMC");
		item->dir_name = strdup(VMC_PS1_PATH_USB);
		asprintf(&item->path, "%s%s", vmcPath, dir->d_name);
		list_append(list, item);

		LOG("[%s] F(%X) name '%s'", item->path, item->flags, item->name);
	}

	closedir(d);
}

int ReadVmc1Codes(save_entry_t * save)
{
	code_entry_t * cmd;
	option_value_t* optval;

	save->codes = list_alloc();

	if (save->type == FILE_TYPE_MENU)
	{
		add_vmc_import_saves(save->codes, save->path, PS1_IMP_PATH_USB);
		add_vmc_import_saves(save->codes, save->path, PSV_SAVES_PATH_USB);
		if (!list_count(save->codes))
		{
			list_free(save->codes);
			save->codes = NULL;
			return 0;
		}

		list_bubbleSort(save->codes, &sortCodeList_Compare);

		return list_count(save->codes);
	}

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_USER " ", _("View Save Details"), CMD_VIEW_DETAILS);
	list_append(save->codes, cmd);

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_WARN " ", _("Delete Save Game"), CMD_DELETE_SAVE);
	list_append(save->codes, cmd);

	cmd = _createCmdCode(PATCH_NULL, NULL, NULL, CMD_CODE_NULL);
	asprintf(&cmd->name, "----- " UTF8_CHAR_STAR " %s " UTF8_CHAR_STAR " -----", _("Save Game Backup"));
	list_append(save->codes, cmd);

	if (apollo_config.ftp_url[0])
	{
		cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_NET " ", _("Upload save backup to FTP"), CMD_UPLOAD_SAVE);
		list_append(save->codes, cmd);
	}

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " ", _("Export save game to .MCS format"), CMD_CODE_NULL);
	_createOptions(cmd, _("Copy .MCS Save to USB"), CMD_EXP_VMC1SAVE);
	optval = malloc(sizeof(option_value_t));
	asprintf(&optval->name, "%s", _("Copy .MCS Save to HDD"));
	asprintf(&optval->value, "%c%c", CMD_EXP_VMC1SAVE, STORAGE_HDD);
	list_append(cmd->options[0].opts, optval);
	cmd->options[0].id = PS1SAVE_MCS;
	list_append(save->codes, cmd);

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " ", _("Export save game to .PSV format"), CMD_CODE_NULL);
	_createOptions(cmd, _("Copy .PSV Save to USB"), CMD_EXP_VMC1SAVE);
	optval = malloc(sizeof(option_value_t));
	asprintf(&optval->name, "%s", _("Copy .PSV Save to HDD"));
	asprintf(&optval->value, "%c%c", CMD_EXP_VMC1SAVE, STORAGE_HDD);
	list_append(cmd->options[0].opts, optval);
	cmd->options[0].id = PS1SAVE_PSV;
	list_append(save->codes, cmd);

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " ", _("Export save game to .PSX format"), CMD_CODE_NULL);
	_createOptions(cmd, _("Copy .PSX Save to USB"), CMD_EXP_VMC1SAVE);
	optval = malloc(sizeof(option_value_t));
	asprintf(&optval->name, "%s", _("Copy .PSX Save to HDD"));
	asprintf(&optval->value, "%c%c", CMD_EXP_VMC1SAVE, STORAGE_HDD);
	list_append(cmd->options[0].opts, optval);
	cmd->options[0].id = PS1SAVE_AR;
	list_append(save->codes, cmd);

	LOG("Loaded %ld codes", list_count(save->codes));

	return list_count(save->codes);
}

static void add_vmc2_import_saves(list_t* list, const char* path, const char* folder)
{
	code_entry_t * cmd;
	DIR *d;
	struct dirent *dir;
	char psvPath[256];
	char data[64];
	int type, toff;

	snprintf(psvPath, sizeof(psvPath), "%s%s", path, folder);
	d = opendir(psvPath);

	if (!d)
		return;

	while ((dir = readdir(d)) != NULL)
	{
		if (dir->d_type != DT_REG)
			continue;

		// check for PS2 PSV saves
		if (endsWith(dir->d_name, ".PSV"))
		{
			snprintf(psvPath, sizeof(psvPath), "%s%s%s", path, folder, dir->d_name);
			if (read_file(psvPath, (uint8_t*) psvPath, 0x40) < 0 || psvPath[0x3C] != 0x02)
				continue;

			toff = 0x80;
			type = FILE_TYPE_PSV;
		}
		else if (endsWith(dir->d_name, ".PSU"))
		{
			toff = 0x40;
			type = FILE_TYPE_PSU;
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
		else if (endsWith(dir->d_name, ".XPS") || endsWith(dir->d_name, ".SPS"))
		{
			toff = 0x15;
			type = FILE_TYPE_XPS;
		}
		else continue;

		snprintf(psvPath, sizeof(psvPath), "%s%s%s", path, folder, dir->d_name);
		LOG("Reading %s...", psvPath);

		FILE *fp = fopen(psvPath, "rb");
		if (!fp) {
			LOG("Unable to open '%s'", psvPath);
			continue;
		}
		fseek(fp, toff, SEEK_SET);
		if (type == FILE_TYPE_XPS)
		{
			// Skip the variable size header
			fread(&toff, 1, sizeof(int), fp);
			fseek(fp, ES32(toff), SEEK_CUR);
			fread(&toff, 1, sizeof(int), fp);
			fseek(fp, ES32(toff) + 10, SEEK_CUR);
		}
		fread(data, 1, sizeof(data), fp);
		fclose(fp);

		cmd = _createCmdCode(PATCH_COMMAND, NULL, NULL, CMD_IMP_VMC2SAVE);
		cmd->file = strdup(psvPath);
		cmd->codes[1] = type;
		asprintf(&cmd->name, CHAR_ICON_COPY "%c (%.10s) %s", CHAR_TAG_PS2, data + 2, dir->d_name);
		list_append(list, cmd);

		LOG("[%s] F(%X) name '%s'", cmd->file, cmd->flags, cmd->name+2);
	}

	closedir(d);
}

int ReadVmc2Codes(save_entry_t * save)
{
	code_entry_t * cmd;
	option_value_t* optval;

	save->codes = list_alloc();

	if (save->type == FILE_TYPE_MENU)
	{
		add_vmc2_import_saves(save->codes, save->path, PS2_IMP_PATH_USB);
		add_vmc2_import_saves(save->codes, save->path, PSV_SAVES_PATH_USB);
		if (!list_count(save->codes))
		{
			list_free(save->codes);
			save->codes = NULL;
			return 0;
		}

		list_bubbleSort(save->codes, &sortCodeList_Compare);

		return list_count(save->codes);
	}

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_USER " ", _("View Save Details"), CMD_VIEW_DETAILS);
	list_append(save->codes, cmd);

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_WARN " ", _("Delete Save Game"), CMD_DELETE_SAVE);
	list_append(save->codes, cmd);

	cmd = _createCmdCode(PATCH_NULL, NULL, NULL, CMD_CODE_NULL);
	asprintf(&cmd->name, "----- " UTF8_CHAR_STAR " %s " UTF8_CHAR_STAR " -----", _("Save Game Backup"));
	list_append(save->codes, cmd);

	if (apollo_config.ftp_url[0])
	{
		cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_NET " ", _("Upload save backup to FTP"), CMD_UPLOAD_SAVE);
		list_append(save->codes, cmd);
	}

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " ", _("Export save game to .PSU format"), CMD_CODE_NULL);
	_createOptions(cmd, _("Export .PSU save to USB"), CMD_EXP_VMC2SAVE);
	optval = malloc(sizeof(option_value_t));
	asprintf(&optval->name, "%s", _("Export .PSU save to HDD"));
	asprintf(&optval->value, "%c%c", CMD_EXP_VMC2SAVE, STORAGE_HDD);
	list_append(cmd->options[0].opts, optval);
	cmd->options[0].id = FILE_TYPE_PSU;
	list_append(save->codes, cmd);

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " ", _("Export save game to .PSV format"), CMD_CODE_NULL);
	_createOptions(cmd, _("Export .PSV save to USB"), CMD_EXP_VMC2SAVE);
	optval = malloc(sizeof(option_value_t));
	asprintf(&optval->name, "%s", _("Export .PSV save to HDD"));
	asprintf(&optval->value, "%c%c", CMD_EXP_VMC2SAVE, STORAGE_HDD);
	list_append(cmd->options[0].opts, optval);
	cmd->options[0].id = FILE_TYPE_PSV;
	list_append(save->codes, cmd);

	LOG("Loaded %ld codes", list_count(save->codes));

	return list_count(save->codes);
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
	option_value_t* optval;
	char path[256];
	snprintf(path, sizeof(path), APOLLO_LOCAL_CACHE "%s.txt", game->title_id);

	if (apollo_config.online_opt == 0 && file_exists(path) == SUCCESS && strcmp(apollo_config.save_db, ONLINE_URL) == 0)
	{
		struct stat stats;
		stat(path, &stats);
		// re-download if file is +1 day old
		if ((stats.st_mtime + ONLINE_CACHE_TIMEOUT) < time(NULL))
			http_download(game->path, "saves.txt", path, 1);
	}
	else
	{
		if (!http_download(game->path, "saves.txt", path, 1))
			return 0;
	}

	long fsize;
	char *data = readTextFile(path, &fsize);
	if (!data)
		return 0;

	char *ptr = data;
	char *end = data + fsize;

	game->codes = list_alloc();

	while (ptr < end && *ptr)
	{
		char *tmp, *content = ptr;

		while (ptr < end && *ptr != '\n' && *ptr != '\r')
		{
			ptr++;
		}
		*ptr++ = 0;

		if ((tmp = strchr(content, '=')) != NULL)
		{
			*tmp++ = 0;
			item = _createCmdCode(PATCH_COMMAND, CHAR_ICON_ZIP " ", tmp, CMD_CODE_NULL);
			item->file = strdup(content);

			_createOptions(item, _("Download to USB"), CMD_DOWNLOAD_USB);
			optval = malloc(sizeof(option_value_t));
			asprintf(&optval->name, "%s", _("Download to HDD"));
			asprintf(&optval->value, "%c%c", CMD_DOWNLOAD_HDD, STORAGE_HDD);
			list_append(item->options[0].opts, optval);
			list_append(game->codes, item);

			LOG("[%s%s] %s", game->path, item->file, item->name + 1);
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

	free(data);

	return (list_count(game->codes));
}

list_t * ReadBackupList(const char* userPath)
{
	char tmp[128];
	save_entry_t * item;
	code_entry_t * cmd;
	list_t *list = list_alloc();

	item = _createSaveEntry(SAVE_FLAG_PS3, CHAR_ICON_COPY " ", _("Export Licenses"));
	asprintf(&item->path, EXDATA_PATH_HDD, apollo_config.user_id);
	item->title_id = strdup("HDD");
	item->type = FILE_TYPE_RIF;
	list_append(list, item);

	item = _createSaveEntry(SAVE_FLAG_PS3, CHAR_ICON_USER " ", _("Activate PS3 Account"));
	asprintf(&item->path, EXDATA_PATH_HDD, apollo_config.user_id);
	item->type = FILE_TYPE_ACT;
	list_append(list, item);

	for (int i = 0; i < MAX_USB_DEVICES; i++)
	{
		snprintf(tmp, sizeof(tmp), USB_PATH, i);
		if (i && dir_exists(tmp) != SUCCESS)
			continue;

		item = _createSaveEntry(SAVE_FLAG_PS3, CHAR_ICON_COPY " ", _("Import Licenses"));
		asprintf(&item->path, IMPORT_RAP_PATH_USB, i);
		asprintf(&item->title_id, "USB %d", i);
		item->type = FILE_TYPE_RAP;
		list_append(list, item);

		item = _createSaveEntry(SAVE_FLAG_PS2, CHAR_ICON_COPY " ", _("PS2 Classics: Import & Encrypt ISOs"));
		asprintf(&item->path, PS2ISO_PATH_USB, i);
		asprintf(&item->title_id, "USB %d", i);
		item->type = FILE_TYPE_ISO;
		list_append(list, item);

		item = _createSaveEntry(SAVE_FLAG_PS2, CHAR_ICON_COPY " ", _("Import PS2 raw memory cards"));
		asprintf(&item->path, IMP_PS2VMC_PATH_USB, i);
		asprintf(&item->title_id, "USB %d", i);
		item->type = FILE_TYPE_PS2RAW;
		list_append(list, item);

		item = _createSaveEntry(SAVE_FLAG_PS1, CHAR_ICON_COPY " ", _("Convert PS1 saves to .PSV format"));
		asprintf(&item->path, USB_PATH, i);
		asprintf(&item->title_id, "USB %d", i);
		item->type = FILE_TYPE_PS1;
		list_append(list, item);

		item = _createSaveEntry(SAVE_FLAG_PS2, CHAR_ICON_COPY " ", _("Convert PS2 saves to .PSV format"));
		asprintf(&item->path, USB_PATH, i);
		asprintf(&item->title_id, "USB %d", i);
		item->type = FILE_TYPE_PS2;
		list_append(list, item);

		item = _createSaveEntry(SAVE_FLAG_PS2, CHAR_ICON_COPY " ", _("Convert/Resign .PSV saves"));
		asprintf(&item->path, USB_PATH, i);
		asprintf(&item->title_id, "USB %d", i);
		item->type = FILE_TYPE_PSV;
		list_append(list, item);
	}

	item = _createSaveEntry(SAVE_FLAG_PS2, CHAR_ICON_COPY " ", _("PS2 Classics: Import & Encrypt ISOs"));
	item->path = strdup(PS2ISO_PATH_HDD);
	item->title_id = strdup("HDD");
	item->type = FILE_TYPE_ISO;
	list_append(list, item);

	item = _createSaveEntry(SAVE_FLAG_PS2, CHAR_ICON_COPY " ", _("PS2 Classics: Export & Decrypt BIN.ENC images"));
	item->path = strdup(PS2ISO_PATH_HDD);
	item->title_id = strdup("HDD");
	item->type = FILE_TYPE_BINENC;
	list_append(list, item);

	item = _createSaveEntry(SAVE_FLAG_PS3, CHAR_ICON_COPY " ", _("Export /dev_flash2"));
	item->path = strdup("/dev_flash2/");
	item->codes = list_alloc();
	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_ZIP " ", _("Backup /dev_flash2 to USB"), CMD_CODE_NULL);
	_createOptions(cmd, _("Save dev_flash2.zip to USB"), CMD_EXP_FLASH2_USB);
	list_append(item->codes, cmd);
	list_append(list, item);

	item = _createSaveEntry(0, CHAR_ICON_ZIP " ", _("Extract Archives (RAR, Zip, 7z)"));
	item->path = strdup(PS3_TMP_PATH);
	item->title_id = strdup("HDD");
	item->type = FILE_TYPE_ZIP;
	list_append(list, item);

	item = _createSaveEntry(0, CHAR_ICON_NET " ", _("Network Tools (Downloader, Web Server)"));
	item->path = strdup(PS3_TMP_PATH);
	item->type = FILE_TYPE_NET;
	list_append(list, item);

	return list;
}

static int get_iso_files(save_entry_t * item)
{
	code_entry_t* cmd;
	DIR *d;
	struct dirent *dir;
	item->codes = list_alloc();

	d = opendir(item->path);

	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			if (dir->d_type == DT_REG && (endsWith(dir->d_name, ".BIN") || endsWith(dir->d_name, ".ISO") || endsWith(dir->d_name, ".CONFIG")))
			{
				cmd = _createCmdCode(PATCH_COMMAND, _("Encrypt: "), dir->d_name, endsWith(dir->d_name, "CONFIG") ? CMD_IMP_PS2_CONFIG : CMD_IMP_PS2_ISO);
				cmd->file = strdup(dir->d_name);
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

static int get_binenc_files(save_entry_t * item)
{
	code_entry_t* cmd;
	option_value_t* optval;
	DIR *d;
	struct dirent *dir;
	item->codes = list_alloc();

	d = opendir(item->path);

	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			if (dir->d_type == DT_REG && endsWith(dir->d_name, ".BIN.ENC"))
			{
				cmd = _createCmdCode(PATCH_COMMAND, _("Decrypt: "), dir->d_name, CMD_CODE_NULL);
				cmd->file = strdup(dir->d_name);
				_createOptions(cmd, _("Save .ISO to USB"), CMD_EXP_PS2_BINENC);
				optval = malloc(sizeof(option_value_t));
				asprintf(&optval->name, "%s", _("Save .ISO to HDD"));
				asprintf(&optval->value, "%c%c", CMD_EXP_PS2_BINENC, STORAGE_HDD);
				list_append(cmd->options[0].opts, optval);
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

static int get_ps2_raw_files(save_entry_t * item)
{
	code_entry_t* cmd;
	DIR *d;
	struct dirent *dir;
	item->codes = list_alloc();

	d = opendir(item->path);

	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			if (dir->d_type == DT_REG && endsWith(dir->d_name, ".VMC"))
			{
				cmd = _createCmdCode(PATCH_COMMAND, NULL, NULL, CMD_IMP_PS2VMC_USB);
				asprintf(&cmd->name, _("Import '%s' to .VM2 (HDD)"), dir->d_name);
				cmd->file = strdup(dir->d_name);
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

static int get_archives(save_entry_t * item)
{
	code_entry_t* cmd;
	DIR *d;
	struct dirent *dir;

	item->codes = list_alloc();
	LOG("Loading files from '%s'...", item->path);

	d = opendir(item->path);
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			if (!(dir->d_type != DT_DIR) ||
				(!endsWith(dir->d_name, ".RAR") && !endsWith(dir->d_name, ".ZIP") && !endsWith(dir->d_name, ".7Z")))
				continue;

			cmd = _createCmdCode(PATCH_COMMAND, NULL, NULL, CMD_EXTRACT_ARCHIVE);
			asprintf(&cmd->name, CHAR_ICON_ZIP " %s %s", _("Extract:"), dir->d_name);
			asprintf(&cmd->file, "%s%s", item->path, dir->d_name);

			LOG("[%s] name '%s'", cmd->file, cmd->name+2);
			list_append(item->codes, cmd);
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

static int get_license_files(save_entry_t* bup, const char* fext)
{
	DIR *d;
	struct dirent *dir;
	code_entry_t * cmd;
	option_value_t* optval;

	bup->codes = list_alloc();

	LOG("Loading %s files from '%s'...", fext, bup->path);

	if (bup->type == FILE_TYPE_RIF)
	{
		cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_ZIP " ", _("Backup All Licenses to .Zip"), CMD_CODE_NULL);
		_createOptions(cmd, _("Save .Zip to USB"), CMD_EXP_EXDATA_USB);
		list_append(bup->codes, cmd);

		cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " ", _("Export All Licenses as .RAPs"), CMD_CODE_NULL);
		_createOptions(cmd, _("Save .RAPs to USB"), CMD_EXP_LICS_RAPS);
		optval = malloc(sizeof(option_value_t));
		asprintf(&optval->name, "%s", _("Save .RAPs to HDD"));
		asprintf(&optval->value, "%c%c", CMD_EXP_LICS_RAPS, STORAGE_HDD);
		list_append(cmd->options[0].opts, optval);
		list_append(bup->codes, cmd);
	}

	if (bup->type == FILE_TYPE_RAP)
	{
		cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " ", _("Import All .RAPs as .RIF Licenses"), CMD_IMP_EXDATA_USB);
		list_append(bup->codes, cmd);
	}

	d = opendir(bup->path);

	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0  &&
				endsWith(dir->d_name, fext))
			{
				cmd = _createCmdCode(PATCH_COMMAND, "", dir->d_name, CMD_CODE_NULL);
				*strrchr(cmd->name, '.') = 0;

				if (bup->type == FILE_TYPE_RIF)
				{
					_createOptions(cmd, _("Save .RAP to USB"), CMD_EXP_LICS_RAPS);
					optval = malloc(sizeof(option_value_t));
					asprintf(&optval->name, "%s", _("Save .RAP to HDD"));
					asprintf(&optval->value, "%c%c", CMD_EXP_LICS_RAPS, STORAGE_HDD);
					list_append(cmd->options[0].opts, optval);
				}
				else if (bup->type == FILE_TYPE_RAP)
				{
					sprintf(cmd->codes, "%c", CMD_IMP_EXDATA_USB);
				}

				cmd->file = strdup(dir->d_name);
				list_append(bup->codes, cmd);

				LOG("Added File '%s'", cmd->file);
			}
		}
		closedir(d);
	}

	LOG("%d items loaded", list_count(bup->codes));

	return list_count(bup->codes);
}

static int read_psv_savegames(save_entry_t *item, const char* folder)
{
	DIR *d;
	struct dirent *dir;
	code_entry_t *cmd;
	option_value_t* optval;
	char psvPath[256];
	char data[0x100];

	snprintf(psvPath, sizeof(psvPath), "%s%s", item->path, folder);
	LOG("Loading PSV files from '%s'...", psvPath);

	d = opendir(psvPath);
	if (!d)
		return 0;

	item->codes = list_alloc();

	while ((dir = readdir(d)) != NULL)
	{
		if (dir->d_type != DT_REG || !endsWith(dir->d_name, ".PSV"))
			continue;

		snprintf(psvPath, sizeof(psvPath), "%s%s%s", item->path, folder, dir->d_name);
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

		cmd = _createCmdCode(PATCH_COMMAND, NULL, NULL, CMD_CODE_NULL);
		cmd->file = strdup(psvPath);
		if (type == 1)
		{
			_createOptions(cmd, _("Export Save to .MCS format - USB"), CMD_EXP_SAVE_PSV);
			cmd->options[0].id = FILE_TYPE_MCS;
		}
		else
		{
			_createOptions(cmd, _("Export Save to .PSU format - USB"), CMD_EXP_SAVE_PSV);
			cmd->options[0].id = FILE_TYPE_PSU;
		}

		optval = malloc(sizeof(option_value_t));
		asprintf(&optval->name, "%s", _("Resign .PSV Save"));
		asprintf(&optval->value, "%c", CMD_RESIGN_PSV);
		list_append(cmd->options[0].opts, optval);

		//PS2 Title offset 0xC0
		//PS1 Title offset 0x04
		char* item_name = sjis2utf8(data + (type == 1 ? 0x04 : 0xC0));
		asprintf(&cmd->name, CHAR_ICON_COPY "%c (%.10s) %s", (type == 1 ? CHAR_TAG_PS1 : CHAR_TAG_PS2), dir->d_name + 2, item_name);
		free(item_name);

		LOG("[%s] F(%X) name '%s'", dir->d_name, type, cmd->name);
		list_append(item->codes, cmd);
	}
	closedir(d);

	if (!list_count(item->codes))
	{
		list_free(item->codes);
		item->codes = NULL;
		return 0;
	}

	return list_count(item->codes);
}

static int read_psx_savegames(save_entry_t *item, const char* folder)
{
	DIR *d;
	struct dirent *dir;
	code_entry_t *cmd;
	char psvPath[256];
	char data[64];
	int type, toff;

	snprintf(psvPath, sizeof(psvPath), "%s%s", item->path, folder);
	LOG("Loading saves from '%s'...", psvPath);

	d = opendir(psvPath);
	if (!d)
		return 0;

	item->codes = list_alloc();

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
			toff = 0x15;
			type = FILE_TYPE_XPS;
		}
		else
			continue;

		snprintf(psvPath, sizeof(psvPath), "%s%s%s", item->path, folder, dir->d_name);
		LOG("Reading %s...", psvPath);

		FILE *fp = fopen(psvPath, "rb");
		if (!fp) {
			LOG("Unable to open '%s'", psvPath);
			continue;
		}

		fseek(fp, toff, SEEK_SET);
		if (type == FILE_TYPE_XPS)
		{
			// Skip the variable size header
			fread(&toff, 1, sizeof(int), fp);
			fseek(fp, ES32(toff), SEEK_CUR);
			fread(&toff, 1, sizeof(int), fp);
			fseek(fp, ES32(toff) + 10, SEEK_CUR);
		}
		fread(data, 1, sizeof(data), fp);
		fclose(fp);

		cmd = _createCmdCode(PATCH_COMMAND, NULL, NULL, CMD_CONVERT_TO_PSV);
		cmd->codes[1] = type;
		cmd->file = strdup(psvPath);
		asprintf(&cmd->name, CHAR_ICON_COPY "%c (%.10s) %s", (type == FILE_TYPE_PSX || type == FILE_TYPE_MCS) ? CHAR_TAG_PS1 : CHAR_TAG_PS2, data + 2, dir->d_name);

		LOG("[%s] F(%X) name '%s'", dir->d_name, cmd->type, cmd->name);
		list_append(item->codes, cmd);
	}
	closedir(d);

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

	switch(bup->type)
	{
	case FILE_TYPE_ISO:
		return get_iso_files(bup);

	case FILE_TYPE_BINENC:
		return get_binenc_files(bup);

	case FILE_TYPE_PS2RAW:
		return get_ps2_raw_files(bup);

	case FILE_TYPE_ZIP:
		return get_archives(bup);

	case FILE_TYPE_RIF:
		return get_license_files(bup, ".rif");

	case FILE_TYPE_RAP:
		return get_license_files(bup, ".rap");

	case FILE_TYPE_PSV:
		return read_psv_savegames(bup, PSV_SAVES_PATH_USB);

	case FILE_TYPE_PS1:
		return read_psx_savegames(bup, PS1_IMP_PATH_USB);

	case FILE_TYPE_PS2:
		return read_psx_savegames(bup, PS2_IMP_PATH_USB);

	case FILE_TYPE_ACT:
		bup->codes = list_alloc();
		cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_USER " ", _("Activate PS3 Account (act.dat)"), CMD_CREATE_ACT_DAT);
		list_append(bup->codes, cmd);
		return list_count(bup->codes);

	case FILE_TYPE_NET:
		bup->codes = list_alloc();
		cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_NET " ", _("URL link Downloader (http, https, ftp, ftps)"), CMD_URL_DOWNLOAD);
		list_append(bup->codes, cmd);
		cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_NET " ", _("Local Web Server (full system access)"), CMD_NET_WEBSERVER);
		list_append(bup->codes, cmd);
		return list_count(bup->codes);

	default:
		return 0;
	}

	return 0;
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
	list_node_t *node, *nc, *no;
	save_entry_t *item;
	code_entry_t *code;
	option_value_t* optval;

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
		
		if (item->dir_name)
		{
			free(item->dir_name);
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
						for (no = list_head(code->options[z].opts); (optval = list_get(no)); no = list_next(no))
						{
							if (optval->name)
								free(optval->name);
							if (optval->value)
								free(optval->value);

							free(optval);
						}
						list_free(code->options[z].opts);

						if (code->options[z].line)
							free(code->options[z].line);
					}
					
					free (code->options);
				}

				free(code);
			}
			
			list_free(item->codes);
			item->codes = NULL;
		}

		free(item);
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

static int parseTypeFlags(int flags)
{
	if (flags & SAVE_FLAG_PS3)
		return 1;
	else if (flags & SAVE_FLAG_PS1)
		return 2;
	else if (flags & SAVE_FLAG_PS2)
		return 3;
	else if (flags & SAVE_FLAG_VMC)
		return 4;

	return 0;
}

int sortSaveList_Compare_Type(const void* a, const void* b)
{
	int ta = parseTypeFlags(((save_entry_t*) a)->flags);
	int tb = parseTypeFlags(((save_entry_t*) b)->flags);

	if (ta == tb)
		return sortSaveList_Compare(a, b);
	else if (ta < tb)
		return -1;
	else
		return 1;
}

int sortSaveList_Compare_TitleID(const void* a, const void* b)
{
	char* ta = ((save_entry_t*) a)->title_id;
	char* tb = ((save_entry_t*) b)->title_id;

	if (!ta)
		return (-1);

	if (!tb)
		return (1);

	int ret = strcasecmp(ta, tb);

	return (ret ? ret : sortSaveList_Compare(a, b));
}

static void read_savegames(const char* userPath, const char* folder, list_t *list, uint32_t flag)
{
	DIR *d;
	struct dirent *dir;
	save_entry_t *item;
	char sfoPath[256];

	snprintf(sfoPath, sizeof(sfoPath), "%s%s", userPath, folder);
	d = opendir(sfoPath);

	if (!d)
		return;

	while ((dir = readdir(d)) != NULL)
	{
		if (dir->d_type != DT_DIR || strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
			continue;

		snprintf(sfoPath, sizeof(sfoPath), "%s%s%s/PARAM.SFO", userPath, folder, dir->d_name);
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

			if (flag & SAVE_FLAG_PS2)
			{
				snprintf(sfoPath, sizeof(sfoPath), "%s%s%s/SCEVMC0.VME", userPath, folder, dir->d_name);
				if (file_exists(sfoPath) != SUCCESS)
				{
					sfo_free(sfo);
					continue;
				}

				snprintf(sfoPath, sizeof(sfoPath), "%s (MemCard)", sfo_data);
				item = _createSaveEntry(flag | SAVE_FLAG_VMC | SAVE_FLAG_LOCKED, "", sfoPath);
				item->type = FILE_TYPE_VMC;
				asprintf(&item->path, "%s%s%s/SCEVMC0.VME", userPath, folder, dir->d_name);
				item->title_id = strdup("VME 0");
				item->dir_name = strdup(userPath);
				list_append(list, item);

				item = _createSaveEntry(flag | SAVE_FLAG_VMC | SAVE_FLAG_LOCKED, "", sfoPath);
				item->type = FILE_TYPE_VMC;
				asprintf(&item->path, "%s%s%s/SCEVMC1.VME", userPath, folder, dir->d_name);
				item->title_id = strdup("VME 1");
				item->dir_name = strdup(userPath);
				list_append(list, item);

				sfo_free(sfo);
				LOG("[%s] F(%X) name '%s'", item->title_id, item->flags, item->name);
				continue;
			}

			item = _createSaveEntry(flag, "", sfo_data);
			asprintf(&item->path, "%s%s%s/", userPath, folder, dir->d_name);
			asprintf(&item->title_id, "%.9s", dir->d_name);
			item->dir_name = strdup(dir->d_name);

			if (flag & SAVE_FLAG_PS3)
			{
				sfo_data = (char*) sfo_get_param_value(sfo, "ATTRIBUTE");
				item->flags |=	(sfo_data[0] ? SAVE_FLAG_LOCKED : 0);
				item->type = FILE_TYPE_PS3;

				snprintf(sfoPath, sizeof(sfoPath), "%*lx", SFO_ACCOUNT_ID_SIZE, apollo_config.account_id);
				sfo_data = (char*) sfo_get_param_value(sfo, "ACCOUNT_ID");
				if (strncmp(sfo_data, sfoPath, SFO_ACCOUNT_ID_SIZE) == 0)
					item->flags |=	SAVE_FLAG_OWNER;
			}

			sfo_free(sfo);
				
			LOG("[%s] F(%X) name '%s'", item->title_id, item->flags, item->name);
			list_append(list, item);
		}
	}

	closedir(d);
}

static void read_vmc2_files(const char* userPath, list_t *list)
{
	DIR *d;
	struct dirent *dir;
	save_entry_t *item;
	char psvPath[256];
	uint64_t size;

	d = opendir(userPath);
	if (!d)
		return;

	while ((dir = readdir(d)) != NULL)
	{
		if (dir->d_type != DT_REG || !(endsWith(dir->d_name, ".VMC") || endsWith(dir->d_name, ".VM2") || 
			endsWith(dir->d_name, ".BIN") || endsWith(dir->d_name, ".PS2")|| endsWith(dir->d_name, ".VME")))
			continue;

		snprintf(psvPath, sizeof(psvPath), "%s%s", userPath, dir->d_name);
		get_file_size(psvPath, &size);

		LOG("Adding %s...", psvPath);
		if (size % 0x840000 != 0 && size % 0x800000 != 0)
			continue;

		item = _createSaveEntry(SAVE_FLAG_PS2 | SAVE_FLAG_VMC, "", dir->d_name);
		item->type = FILE_TYPE_VMC;
		item->path = strdup(psvPath);
		item->title_id = strdup("VMC");
		item->dir_name = strdup(userPath);

		LOG("[%s] F(%X) name '%s'", item->title_id, item->flags, item->name);
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

	if (dir_exists(userPath) != SUCCESS)
		return NULL;

	list = list_alloc();

	item = _createSaveEntry(SAVE_FLAG_PS3 | SAVE_FLAG_HDD, CHAR_ICON_COPY " ", _("Bulk Save Management"));
	item->type = FILE_TYPE_MENU;
	item->codes = list_alloc();
	asprintf(&item->path, "%s%s", userPath, PS3_SAVES_PATH_HDD);
	//bulk management hack
	item->dir_name = malloc(sizeof(void**));
	((void**)item->dir_name)[0] = list;

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_SIGN " ", _("Resign & Unlock all Saves"), CMD_RESIGN_ALL_SAVES);
	list_append(item->codes, cmd);

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_SIGN " ", _("Resign & Unlock selected Saves"), CMD_RESIGN_SAVES);
	list_append(item->codes, cmd);

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " ", _("Copy all Saves to USB"), CMD_CODE_NULL);
	_createOptions(cmd, _("Copy Saves to USB"), CMD_COPY_ALL_SAVES_USB);
	list_append(item->codes, cmd);

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " ", _("Copy selected Saves to USB"), CMD_CODE_NULL);
	_createOptions(cmd, _("Copy Saves to USB"), CMD_COPY_SAVES_USB);
	list_append(item->codes, cmd);

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_NET " ", _("Start local Web Server"), CMD_SAVE_WEB_SERVER);
	list_append(item->codes, cmd);
	list_append(list, item);

	read_savegames(userPath, PS3_SAVES_PATH_HDD, list, SAVE_FLAG_PS3 | SAVE_FLAG_HDD);
	read_savegames(userPath, PS2_SAVES_PATH_HDD, list, SAVE_FLAG_PS2 | SAVE_FLAG_HDD);
	read_savegames(userPath, PSP_SAVES_PATH_HDD, list, SAVE_FLAG_PSP | SAVE_FLAG_HDD);

	read_vmc1_files(VMC_PS2_PATH_HDD, list);
	read_vmc2_files(VMC_PS2_PATH_HDD, list);

	return list;
}

list_t * ReadUsbList(const char* userPath)
{
	save_entry_t *item;
	code_entry_t *cmd;
	list_t *list;
	char savePath[256];

	if (dir_exists(userPath) != SUCCESS)
		return NULL;

	list = list_alloc();

	item = _createSaveEntry(SAVE_FLAG_PS3, CHAR_ICON_COPY " ", _("Bulk Save Management"));
	item->type = FILE_TYPE_MENU;
	item->codes = list_alloc();
	asprintf(&item->path, "%s%s", userPath, PS3_SAVES_PATH_USB);
	//bulk management hack
	item->dir_name = malloc(sizeof(void**));
	((void**)item->dir_name)[0] = list;

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_SIGN " ", _("Resign & Unlock all Saves"), CMD_RESIGN_ALL_SAVES);
	list_append(item->codes, cmd);

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_SIGN " ", _("Resign & Unlock selected Saves"), CMD_RESIGN_SAVES);
	list_append(item->codes, cmd);

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " ", _("Copy all Saves to HDD"), CMD_COPY_ALL_SAVES_HDD);
	list_append(item->codes, cmd);

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " ", _("Copy selected Saves to HDD"), CMD_COPY_SAVES_HDD);
	list_append(item->codes, cmd);

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_NET " ", _("Start local Web Server"), CMD_SAVE_WEB_SERVER);
	list_append(item->codes, cmd);
	list_append(list, item);

	read_savegames(userPath, PS3_SAVES_PATH_USB, list, SAVE_FLAG_PS3);
	read_savegames(userPath, PS2_SAVES_PATH_USB, list, SAVE_FLAG_PS2);
	read_savegames(userPath, PSP_SAVES_PATH_USB, list, SAVE_FLAG_PSP);

	snprintf(savePath, sizeof(savePath), "%s%s", userPath, VMC_PS1_PATH_USB);
	read_vmc1_files(savePath, list);

	snprintf(savePath, sizeof(savePath), "%s%s", userPath, VMC_PS2_PATH_USB);
	read_vmc2_files(savePath, list);

	snprintf(savePath, sizeof(savePath), "%s%s", userPath, "VMC/");
	read_vmc2_files(savePath, list);

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
static void _ReadOnlineListEx(const char* urlPath, uint16_t flag, list_t *list)
{
	save_entry_t *item;
	char path[256];

	snprintf(path, sizeof(path), APOLLO_LOCAL_CACHE "%04X_games.txt", flag);

	if (apollo_config.online_opt == 0 && file_exists(path) == SUCCESS && strcmp(apollo_config.save_db, ONLINE_URL) == 0)
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
			return;
	}
	
	long fsize;
	char *data = readTextFile(path, &fsize);
	if (!data)
		return;
	
	char *ptr = data;
	char *end = data + fsize;

	while (ptr < end && *ptr)
	{
		char *tmp, *content = ptr;

		while (ptr < end && *ptr != '\n' && *ptr != '\r')
		{
			ptr++;
		}
		*ptr++ = 0;

		if ((tmp = strchr(content, '=')) != NULL)
		{
			*tmp++ = 0;
			item = _createSaveEntry(flag | SAVE_FLAG_ONLINE, "", tmp);
			item->title_id = strdup(content);
			asprintf(&item->path, "%s%s/", urlPath, item->title_id);

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

	free(data);
}

list_t * ReadOnlineList(const char* urlPath)
{
	char url[256];
	list_t *list = list_alloc();

	// PS3 save-games (Zip folder)
	snprintf(url, sizeof(url), "%s" "PS3/", urlPath);
	_ReadOnlineListEx(url, SAVE_FLAG_PS3, list);

	// PS2 save-games (Zip PSV)
	snprintf(url, sizeof(url), "%s" "PS2/", urlPath);
	_ReadOnlineListEx(url, SAVE_FLAG_PS2, list);

	// PS1 save-games (Zip PSV)
	snprintf(url, sizeof(url), "%s" "PS1/", urlPath);
	_ReadOnlineListEx(url, SAVE_FLAG_PS1, list);

	if (!list_count(list))
	{
		list_free(list);
		return NULL;
	}

	return list;
}

list_t * ReadVmc1List(const char* userPath)
{
	char filePath[256];
	save_entry_t *item;
	code_entry_t *cmd;
	list_t *list;
	ps1mcData_t* mcdata;

	if (!openMemoryCard(userPath, 0))
	{
		LOG("Error: no PS1 Memory Card detected! (%s)", userPath);
		return NULL;
	}

	mcdata = getMemoryCardData();
	if (!mcdata)
		return NULL;

	list = list_alloc();

	item = _createSaveEntry(SAVE_FLAG_PS1, CHAR_ICON_VMC " ", _("Memory Card Management"));
	item->type = FILE_TYPE_MENU;
	item->path = strdup(userPath);
	item->title_id = strdup("VMC");
	item->codes = list_alloc();
	//bulk management hack
	item->dir_name = malloc(sizeof(void**));
	((void**)item->dir_name)[0] = list;

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " ", _("Export selected Saves to USB"), CMD_CODE_NULL);
	_createOptions(cmd, _("Copy selected Saves to USB"), CMD_EXP_SAVES_VMC);
	list_append(item->codes, cmd);
	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " ", _("Export all Saves to USB"), CMD_CODE_NULL);
	_createOptions(cmd, _("Copy all Saves to USB"), CMD_EXP_ALL_SAVES_VMC);
	list_append(item->codes, cmd);
	add_vmp_commands(item);
	list_append(list, item);

	item = _createSaveEntry(SAVE_FLAG_PS1, CHAR_ICON_COPY " ", _("Import Saves to Virtual Card"));
	item->path = strdup(FAKE_USB_PATH);
	item->title_id = strdup("HDD");
	item->dir_name = strdup(userPath);
	item->type = FILE_TYPE_MENU;
	list_append(list, item);

	for (int i = 0; i < MAX_USB_DEVICES; i++)
	{
		snprintf(filePath, sizeof(filePath), USB_PATH, i);
		if (i && dir_exists(filePath) != SUCCESS)
			continue;

		item = _createSaveEntry(SAVE_FLAG_PS1, CHAR_ICON_COPY " ", _("Import Saves to Virtual Card"));
		asprintf(&item->path, USB_PATH, i);
		asprintf(&item->title_id, "USB %d", i);
		item->dir_name = strdup(userPath);
		item->type = FILE_TYPE_MENU;
		list_append(list, item);
	}

	for (int i = 0; i < PS1CARD_MAX_SLOTS; i++)
	{
		if (mcdata[i].saveType != PS1BLOCK_INITIAL)
			continue;

		LOG("Reading '%s'...", mcdata[i].saveName);

		char* tmp = sjis2utf8(mcdata[i].saveTitle);
		item = _createSaveEntry(SAVE_FLAG_PS1 | SAVE_FLAG_VMC, "", tmp);
		item->type = FILE_TYPE_PS1;
		item->blocks = i;
		item->title_id = strdup(mcdata[i].saveProdCode);
		item->dir_name = strdup(mcdata[i].saveName);
		asprintf(&item->path, "%s\n%s", userPath, mcdata[i].saveName);
		free(tmp);

		if(strlen(item->title_id) == 10 && item->title_id[4] == '-')
			memmove(&item->title_id[4], &item->title_id[5], 6);

		LOG("[%s] F(%X) name '%s'", item->title_id, item->flags, item->name);
		list_append(list, item);
	}

	return list;
}

list_t * ReadVmc2List(const char* userPath)
{
	char filePath[256];
	save_entry_t *item;
	code_entry_t *cmd;
	option_value_t* optval;
	list_t *list;
	ps2_IconSys_t iconsys;
	int r, dd, fd;

	r = mcio_vmcInit(userPath);
	if (r < 0)
	{
		LOG("Error: no PS2 Memory Card detected! (%d)", r);
		return NULL;
	}

	list = list_alloc();

	item = _createSaveEntry(SAVE_FLAG_PS2, CHAR_ICON_VMC " ", _("Memory Card Management"));
	item->type = FILE_TYPE_MENU;
	item->path = strdup(userPath);
	item->title_id = strdup("VMC");
	item->codes = list_alloc();
	//bulk management hack
	item->dir_name = malloc(sizeof(void**));
	((void**)item->dir_name)[0] = list;

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " ", _("Export selected Saves to USB"), CMD_CODE_NULL);
	_createOptions(cmd, _("Copy selected Saves to USB"), CMD_EXP_SAVES_VMC);
	list_append(item->codes, cmd);
	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " ", _("Export all Saves to USB"), CMD_CODE_NULL);
	_createOptions(cmd, _("Copy all Saves to USB"), CMD_EXP_ALL_SAVES_VMC);
	list_append(item->codes, cmd);
	list_append(list, item);

	cmd = _createCmdCode(PATCH_NULL, NULL, NULL, CMD_CODE_NULL);
	asprintf(&cmd->name, "----- " UTF8_CHAR_STAR " %s " UTF8_CHAR_STAR " -----", _("Virtual Memory Card"));
	list_append(item->codes, cmd);

	if (!endsWith(userPath, ".VM2"))
	{
		cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " ", _("Export Memory Card to .VM2 format"), CMD_CODE_NULL);
		cmd->file = strdup(strrchr(userPath, '/')+1);
		_createOptions(cmd, _("Save .VM2 Memory Card to USB"), CMD_EXP_PS2_VM2);
		optval = malloc(sizeof(option_value_t));
		asprintf(&optval->name, "%s", _("Save .VM2 Memory Card to HDD"));
		asprintf(&optval->value, "%c%c", CMD_EXP_PS2_VM2, STORAGE_HDD);
		list_append(cmd->options[0].opts, optval);
		list_append(item->codes, cmd);
	}

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " ", _("Export Memory Card to .VMC format (No ECC)"), CMD_CODE_NULL);
	cmd->file = strdup(strrchr(userPath, '/')+1);
	_createOptions(cmd, _("Save .VMC Memory Card to USB"), CMD_EXP_VM2_RAW);
	list_append(item->codes, cmd);

	if (!endsWith(userPath, ".VMC"))
	{
		cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " ", _("Import a .VM2 file to Memory Card"), CMD_CODE_NULL);
		cmd->file = strdup(strrchr(userPath, '/')+1);
		cmd->options_count = 1;
		cmd->options = _getFileOptions(VMC_PS2_PATH_HDD, "*.VM2", CMD_IMP_PS2_VM2);
		list_append(item->codes, cmd);
	}

	item = _createSaveEntry(SAVE_FLAG_PS2, CHAR_ICON_COPY " ", _("Import Saves to Virtual Card"));
	item->path = strdup(FAKE_USB_PATH);
	item->title_id = strdup("HDD");
	item->type = FILE_TYPE_MENU;
	list_append(list, item);

	for (int i = 0; i < MAX_USB_DEVICES; i++)
	{
		snprintf(filePath, sizeof(filePath), USB_PATH, i);
		if (i && dir_exists(filePath) != SUCCESS)
			continue;

		item = _createSaveEntry(SAVE_FLAG_PS2, CHAR_ICON_COPY " ", _("Import Saves to Virtual Card"));
		asprintf(&item->path, USB_PATH, i);
		asprintf(&item->title_id, "USB %d", i);
		item->type = FILE_TYPE_MENU;
		list_append(list, item);
	}

	dd = mcio_mcDopen("/");
	if (dd < 0)
	{
		LOG("mcio Dopen Error %d", dd);
		return list;
	}

	struct io_dirent dirent;

	do {
		r = mcio_mcDread(dd, &dirent);
		if ((r) && (strcmp(dirent.name, ".")) && (strcmp(dirent.name, "..")))
		{
			snprintf(filePath, sizeof(filePath), "%s/icon.sys", dirent.name);
			LOG("Reading %s...", filePath);

			fd = mcio_mcOpen(filePath, sceMcFileAttrReadable | sceMcFileAttrFile);
			if (fd < 0) {
				LOG("Unable to read from '%s'", filePath);
				continue;
			}

			r = mcio_mcRead(fd, &iconsys, sizeof(ps2_IconSys_t));
			mcio_mcClose(fd);

			if (r != sizeof(ps2_IconSys_t))
				continue;

			if (iconsys.secondLineOffset)
			{
				iconsys.secondLineOffset = ES16(iconsys.secondLineOffset);
				memmove(&iconsys.title[iconsys.secondLineOffset+2], &iconsys.title[iconsys.secondLineOffset], sizeof(iconsys.title) - iconsys.secondLineOffset);
				iconsys.title[iconsys.secondLineOffset] = 0x81;
				iconsys.title[iconsys.secondLineOffset+1] = 0x50;
			}

			char* title = sjis2utf8(iconsys.title);
			item = _createSaveEntry(SAVE_FLAG_PS2 | SAVE_FLAG_VMC, "", title);
			item->type = FILE_TYPE_PS2;
			item->dir_name = strdup(dirent.name);
			asprintf(&item->title_id, "%.10s", dirent.name+2);
			asprintf(&item->path, "%s\n%s/\n%s", userPath, dirent.name, iconsys.copyIconName);
			free(title);

			if(strlen(item->title_id) == 10 && item->title_id[4] == '-')
				memmove(&item->title_id[4], &item->title_id[5], 6);

			LOG("[%s] F(%X) name '%s'", item->title_id, item->flags, item->name);
			list_append(list, item);
		}
	} while (r);

	mcio_mcDclose(dd);

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

	item = _createSaveEntry(SAVE_FLAG_PS3, CHAR_ICON_COPY " ", _("Export Trophies"));
	item->type = FILE_TYPE_MENU;
	item->path = strdup(userPath);
	item->codes = list_alloc();
	//bulk management hack
	item->dir_name = malloc(sizeof(void**));
	((void**)item->dir_name)[0] = list;

	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " ", _("Backup selected Trophies to USB"), CMD_CODE_NULL);
	_createOptions(cmd, _("Save Trophies to USB"), CMD_COPY_TROPHIES_USB);
	list_append(item->codes, cmd);
	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_COPY " ", _("Backup All Trophies to USB"), CMD_CODE_NULL);
	_createOptions(cmd, _("Save Trophies to USB"), CMD_COPY_ALL_TROP_USB);
	list_append(item->codes, cmd);
	cmd = _createCmdCode(PATCH_COMMAND, CHAR_ICON_ZIP " ", _("Export All Trophies to .Zip"), CMD_CODE_NULL);
	_createOptions(cmd, _("Save .Zip to USB"), CMD_ZIP_TROPHY_USB);
	list_append(item->codes, cmd);
	list_append(list, item);

	for (int i = 0; i < MAX_USB_DEVICES; i++)
	{
		snprintf(filePath, sizeof(filePath), USB_PATH TROPHIES_PATH_USB, i);
		if (i && dir_exists(filePath) != SUCCESS)
			continue;

		item = _createSaveEntry(SAVE_FLAG_PS3 | SAVE_FLAG_TROPHY, CHAR_ICON_COPY " ", _("Import Trophies"));
		asprintf(&item->path, USB_PATH TROPHIES_PATH_USB, i);
		asprintf(&item->title_id, " USB %d", i);
		item->type = FILE_TYPE_MENU;
		list_append(list, item);
		break;
	}

	d = opendir(userPath);
	if (!d)
		return list;

	while ((dir = readdir(d)) != NULL)
	{
		if (dir->d_type != DT_DIR || strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0 || strncmp(dir->d_name, "_BU_", 4) == 0)
			continue;

		snprintf(filePath, sizeof(filePath), "%s%s/TROPCONF.SFM", userPath, dir->d_name);
		if ((buffer = readTextFile(filePath, &bufferLen)) != NULL)
		{
			LOG("Reading %s...", filePath);
			/*parse the file and get the DOM */
			doc = xmlParseMemory(buffer + 0x40, bufferLen - 0x40);
			if (!doc)
			{
				LOG("XML: could not parse file %s", filePath);
				free(buffer);
				continue;
			}

			/*Get the root element node */
			root_element = xmlDocGetRootElement(doc);
			value = _get_xml_node_value(root_element->children, BAD_CAST "title-name");

			item = _createSaveEntry(SAVE_FLAG_PS3 | SAVE_FLAG_TROPHY, "", value);
			asprintf(&item->path, "%s%s/", userPath, dir->d_name);
			item->dir_name = strdup(dir->d_name);

			value = _get_xml_node_value(root_element->children, BAD_CAST "npcommid");
			item->title_id = strdup(value);
			item->type = FILE_TYPE_TRP;

			/*free the document */
			xmlFreeDoc(doc);
			xmlCleanupParser();
			free(buffer);
				
			LOG("[%s] F(%X) name '%s'", item->title_id, item->flags, item->name);
			list_append(list, item);
		}
	}

	closedir(d);

	return list;
}
