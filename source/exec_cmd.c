#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <net/netctl.h>
#include <sysutil/sysutil.h>
#include <polarssl/md5.h>

#include "saves.h"
#include "menu.h"
#include "common.h"
#include "util.h"
#include "pfd.h"
#include "sfo.h"
#include "ps1card.h"
#include "mcio.h"

static char host_buf[256];

static void _set_dest_path(char* path, int dest, const char* folder)
{
	switch (dest)
	{
	case STORAGE_USB0:
	case STORAGE_USB1:
	case STORAGE_USB2:
	case STORAGE_USB3:
	case STORAGE_USB4:
	case STORAGE_USB5:
	case STORAGE_USB6:
		sprintf(path, USB_PATH "%s", dest, folder);
		break;

	case STORAGE_HDD:
		sprintf(path, "%s%s", FAKE_USB_PATH, folder);
		break;

	default:
		path[0] = 0;
	}
}

static void downloadSave(const save_entry_t* entry, const char* file, int dst, const char* folder)
{
	char path[256];

	_set_dest_path(path, dst, folder);
	if (mkdirs(path) != SUCCESS)
	{
		show_message("Error! Export folder is not available:\n%s", path);
		return;
	}

	if (!http_download(entry->path, file, APOLLO_LOCAL_CACHE "tmpsave.zip", 1))
	{
		show_message("Error downloading save game from:\n%s%s", entry->path, file);
		return;
	}

	if (extract_zip(APOLLO_LOCAL_CACHE "tmpsave.zip", path))
		show_message("Save game successfully downloaded to:\n%s", path);
	else
		show_message("Error extracting save game!");

	unlink_secure(APOLLO_LOCAL_CACHE "tmpsave.zip");
}

static void _saveOwnerData(const char* path)
{
	char buff[SYSUTIL_SYSTEMPARAM_CURRENT_USERNAME_SIZE+1];

	sysUtilGetSystemParamString(SYSUTIL_SYSTEMPARAM_ID_CURRENT_USERNAME, buff, SYSUTIL_SYSTEMPARAM_CURRENT_USERNAME_SIZE);
	LOG("Saving User '%s'...", buff);
	save_xml_owner(path, buff);
	file_chmod(path);
}

static uint32_t get_filename_id(const char* dir, const char* title_id)
{
	char path[128];
	uint32_t tid = 0;

	do
	{
		tid++;
		snprintf(path, sizeof(path), "%s%s-%08d.zip", dir, title_id, tid);
	}
	while (file_exists(path) == SUCCESS);

	return tid;
}

static void zipSave(const save_entry_t* entry, int dest)
{
	char exp_path[256];
	char export_file[256];
	char* tmp;
	uint32_t fid;
	int ret;

	_set_dest_path(exp_path, dest, PS3_EXPORT_PATH);
	if (mkdirs(exp_path) != SUCCESS)
	{
		show_message("Error! Export folder is not available:\n%s", exp_path);
		return;
	}

	init_loading_screen("Exporting save game...");

	fid = get_filename_id(exp_path, entry->title_id);
	snprintf(export_file, sizeof(export_file), "%s%s-%08d.zip", exp_path, entry->title_id, fid);

	asprintf(&tmp, entry->path);
	*strrchr(tmp, '/') = 0;
	*strrchr(tmp, '/') = 0;

	ret = zip_directory(tmp, entry->path, export_file);
	free(tmp);

	if (ret)
	{
		snprintf(export_file, sizeof(export_file), "%s%08d.txt", exp_path, apollo_config.user_id);
		FILE* f = fopen(export_file, "a");
		if (f)
		{
			fprintf(f, "%s-%08d.zip=%s\n", entry->title_id, fid, entry->name);
			fclose(f);
			file_chmod(export_file);
		}

		snprintf(export_file, sizeof(export_file), "%s" OWNER_XML_FILE, exp_path);
		_saveOwnerData(export_file);
	}

	stop_loading_screen();
	if (!ret)
	{
		show_message("Error! Can't export save game to:\n%s", exp_path);
		return;
	}

	show_message("Zip file successfully saved to:\n%s%s-%08d.zip", exp_path, entry->title_id, fid);
}

static int _copy_save_usb(const save_entry_t* save, const char* exp_path)
{
	char copy_path[256];

	snprintf(copy_path, sizeof(copy_path), "%s%s/", exp_path, save->dir_name);
	LOG("Copying <%s> to %s...", save->path, copy_path);

	return (copy_directory(save->path, save->path, copy_path) == SUCCESS);
}

static void copySave(const save_entry_t* save, int dst, const char* path)
{
	char exp_path[256];

	_set_dest_path(exp_path, dst, path);
	if (strncmp(save->path, exp_path, strlen(exp_path)) == 0)
		return;

	if (mkdirs(exp_path) != SUCCESS)
	{
		show_message("Error! Export folder is not available:\n%s", exp_path);
		return;
	}

	init_loading_screen("Copying files...");
	_copy_save_usb(save, exp_path);
	stop_loading_screen();

	show_message("Files successfully copied to:\n%s%s", exp_path, save->dir_name);
}

static void copyAllSavesUSB(const save_entry_t* save, int dst, int all)
{
	char usb_path[256];
	int done = 0, err_count = 0;
	list_node_t *node;
	save_entry_t *item;
	uint64_t progress = 0;
	list_t *list = ((void**)save->dir_name)[0];

	_set_dest_path(usb_path, dst, PS3_SAVES_PATH_USB);
	if (mkdirs(usb_path) != SUCCESS)
	{
		show_message("Error! Export folder is not available:\n%s", usb_path);
		return;
	}

	init_progress_bar("Copying all saves...", save->path);
	LOG("Copying all saves from '%s' to USB '%s'...", save->path, usb_path);

	for (node = list_head(list); (item = list_get(node)); node = list_next(node))
	{
		progress++;
		update_progress_bar(progress, list_count(list), item->name);
		if (item->type != FILE_TYPE_MENU && (item->flags & SAVE_FLAG_PS3) && (all || item->flags & SAVE_FLAG_SELECTED))
			(_copy_save_usb(item, usb_path) ? done++ : err_count++);
	}
	end_progress_bar();

	show_message("%d/%d Saves copied to USB", done, done+err_count);
}

static int _copy_save_hdd(const save_entry_t *item)
{
	char copy_path[256];

	snprintf(copy_path, sizeof(copy_path), SAVES_PATH_HDD "%s/", apollo_config.user_id, item->dir_name);

	if (dir_exists(copy_path) == SUCCESS)
		LOG("Overwriting! Save-game folder already exists: %s", copy_path);

	if (!create_savegame_folder(item->dir_name))
	{
		LOG("Error! Can't create save folder: %s", item->dir_name);
		return 0;
	}

	LOG("Copying <%s> to %s...", item->path, copy_path);
	return (copy_directory(item->path, item->path, copy_path) == SUCCESS);
}

static void copySaveHDD(const save_entry_t* save)
{
	char hdd_path[256];

	snprintf(hdd_path, sizeof(hdd_path), SAVES_PATH_HDD "%s/", apollo_config.user_id, save->dir_name);
	if (dir_exists(hdd_path) == SUCCESS &&
		!show_dialog(DIALOG_TYPE_YESNO, "Save-game %s already exists! Overwrite?", save->dir_name))
	{
		show_message("Error! Save-game %s already exists", save->dir_name);
		return;
	}

	init_loading_screen("Copying save game...");
	int ret = _copy_save_hdd(save);
	stop_loading_screen();

	if (ret)
		show_message("Save-game %s copied to HDD", save->dir_name);
	else
		show_message("Error! Failed to copy Save-game %s", save->dir_name);
}

static int webReqHandler(dWebRequest_t* req, dWebResponse_t* out, void* list)
{
	list_node_t *node;
	save_entry_t *item;

	// http://ps3-ip:8080/
	if (strcmp(req->resource, "/") == 0)
	{
		uint64_t hash[2];
		md5_context ctx;

		md5_starts(&ctx);
		for (node = list_head(list); (item = list_get(node)); node = list_next(node))
			md5_update(&ctx, (uint8_t*) item->name, strlen(item->name));

		md5_finish(&ctx, (uint8_t*) hash);
		asprintf(&out->data, APOLLO_LOCAL_CACHE "web%016lx%016lx.html", hash[0], hash[1]);

		if (file_exists(out->data) == SUCCESS)
			return 1;

		FILE* f = fopen(out->data, "w");
		if (!f)
			return 0;

		fprintf(f, "<html><head><meta charset=\"UTF-8\"><style>h1, h2 { font-family: arial; } img { display: none; } table { border-collapse: collapse; margin: 25px 0; font-size: 0.9em; font-family: sans-serif; min-width: 400px; box-shadow: 0 0 20px rgba(0, 0, 0, 0.15); } table thead tr { background-color: #009879; color: #ffffff; text-align: left; } table th, td { padding: 12px 15px; } table tbody tr { border-bottom: 1px solid #dddddd; } table tbody tr:nth-of-type(even) { background-color: #f3f3f3; } table tbody tr:last-of-type { border-bottom: 2px solid #009879; }</style>");
		fprintf(f, "<script language=\"javascript\">function show(sid,src){var im=document.getElementById('img'+sid);im.src=src;im.style.display='block';document.getElementById('btn'+sid).style.display='none';}</script>");
		fprintf(f, "<title>Apollo Save Tool</title></head><body><h1>.:: Apollo Save Tool</h1><h2>Index of %s</h2><table><thead><tr><th>Name</th><th>Icon</th><th>Title ID</th><th>Folder</th><th>Location</th></tr></thead><tbody>", selected_entry->path);

		int i = 0;
		for (node = list_head(list); (item = list_get(node)); node = list_next(node), i++)
		{
			if (item->type == FILE_TYPE_MENU || !(item->flags & SAVE_FLAG_PS3))
				continue;

			fprintf(f, "<tr><td><a href=\"/zip/%s.zip\">%s</a></td>", item->dir_name, item->name);
			fprintf(f, "<td><button type=\"button\" id=\"btn%d\" onclick=\"show(%d,'/icon/%s.png')\">Show Icon</button>", i, i, item->dir_name);
			fprintf(f, "<img id=\"img%d\" alt=\"%s\" width=\"320\" height=\"176\"></td>", i, item->name);
			fprintf(f, "<td>%s</td>", item->title_id);
			fprintf(f, "<td>%s</td>", item->dir_name);
			fprintf(f, "<td>%.3s</td></tr>", selected_entry->path + 5);
		}

		fprintf(f, "</tbody></table></body></html>");
		fclose(f);
		return 1;
	}

	// http://ps3-ip:8080/PS3/games.txt
	if (wildcard_match(req->resource, "/PS3/games.txt"))
	{
		asprintf(&out->data, "%s%s", APOLLO_LOCAL_CACHE, "web_games.txt");

		FILE* f = fopen(out->data, "w");
		if (!f)
			return 0;

		for (node = list_head(list); (item = list_get(node)); node = list_next(node))
		{
			if (item->type == FILE_TYPE_MENU || !(item->flags & SAVE_FLAG_PS3))
				continue;

			fprintf(f, "%s=%s\n", item->title_id, item->name);
		}

		fclose(f);
		return 1;
	}

	// http://ps3-ip:8080/PS3/BLUS12345/saves.txt
	if (wildcard_match(req->resource, "/PS3/\?\?\?\?\?\?\?\?\?/saves.txt"))
	{
		asprintf(&out->data, "%sweb%.9s_saves.txt", APOLLO_LOCAL_CACHE, req->resource + 5);

		FILE* f = fopen(out->data, "w");
		if (!f)
			return 0;

		int i = 0;
		for (node = list_head(list); (item = list_get(node)); node = list_next(node), i++)
		{
			if (item->type == FILE_TYPE_MENU || !(item->flags & SAVE_FLAG_PS3) || strncmp(item->title_id, req->resource + 5, 9))
				continue;

			fprintf(f, "%08d.zip=(%s) %s\n", i, item->dir_name, item->name);
		}

		fclose(f);
		return 1;
	}

	// http://ps3-ip:8080/PS3/BLUS12345/00000000.zip
	if (wildcard_match(req->resource, "/PS3/\?\?\?\?\?\?\?\?\?/*.zip"))
	{
		int id = 0;

		sscanf(req->resource + 15, "%08d", &id);
		item = list_get_item(list, id);

		asprintf(&out->data, "%s%s.zip", APOLLO_LOCAL_CACHE, item->dir_name);
		zip_savegame(item->dir_name, item->path, out->data);

		return (file_exists(out->data) == SUCCESS);
	}

	// http://ps3-ip:8080/zip/DIR-NAME.zip
	if (wildcard_match(req->resource, "/zip/*.zip"))
	{
		char *folder, *path;

		asprintf(&out->data, "%s%s", APOLLO_LOCAL_CACHE, req->resource + 5);
		folder = strdup(req->resource + 5);
		*strrchr(folder, '.') = 0;
		asprintf(&path, "%s%s/", selected_entry->path, folder);

		zip_savegame(folder, path, out->data);
		free(folder);
		free(path);

		return (file_exists(out->data) == SUCCESS);
	}

	// http://ps3-ip:8080/icon/DIR-NAME.png
	if (wildcard_match(req->resource, "/icon/*.png"))
	{
		asprintf(&out->data, "%s%s", selected_entry->path, req->resource + 6);
		*strrchr(out->data, '.') = 0;
		strcat(out->data, "/ICON0.PNG");

		return (file_exists(out->data) == SUCCESS);
	}

	// http://ps3-ip:8080/PS3/BLUS12345/ICON0.PNG
	if (wildcard_match(req->resource, "/PS3/\?\?\?\?\?\?\?\?\?/ICON0.PNG"))
	{
		for (node = list_head(list); (item = list_get(node)); node = list_next(node))
		{
			if (item->type == FILE_TYPE_MENU || !(item->flags & SAVE_FLAG_PS3) || strncmp(item->title_id, req->resource + 5, 9))
				continue;

			asprintf(&out->data, "%sICON0.PNG", item->path);
			return (file_exists(out->data) == SUCCESS);
		}

		return 0;
	}

	return 0;
}

static void enableWebServer(dWebReqHandler_t handler, void* data, int port)
{
	union net_ctl_info ip_info;

	memset(&ip_info, 0, sizeof(ip_info));
	netCtlGetInfo(NET_CTL_INFO_IP_ADDRESS, &ip_info);
	LOG("Starting local web server %s:%d ...", ip_info.ip_address, port);

	if (dbg_webserver_start(port, handler, data))
	{
		show_message("Web Server listening on http://%s:%d\nPress OK to stop the Server.", ip_info.ip_address, port);
		dbg_webserver_stop();
	}
	else show_message("Error starting Web Server!");
}

static void* ps3_host_callback(int id, int* size)
{
	union net_ctl_info net_info;

	memset(host_buf, 0, sizeof(host_buf));

	switch (id)
	{
	case APOLLO_HOST_TEMP_PATH:
		return APOLLO_LOCAL_CACHE;

	case APOLLO_HOST_SYS_NAME:
		if (sysUtilGetSystemParamString(SYSUTIL_SYSTEMPARAM_ID_NICKNAME, host_buf, SYSUTIL_SYSTEMPARAM_NICKNAME_SIZE) < 0)
			LOG("Error getting System nickname");

		if (size) *size = strlen(host_buf);
		return host_buf;

	case APOLLO_HOST_PSID:
		memcpy(host_buf, apollo_config.psid, 16);
		if (size) *size = 16;
		return host_buf;

	case APOLLO_HOST_ACCOUNT_ID:
		memcpy(host_buf, &apollo_config.account_id, 8);
		if (size) *size = 8;
		return host_buf;

	case APOLLO_HOST_USERNAME:
		if (sysUtilGetSystemParamString(SYSUTIL_SYSTEMPARAM_ID_CURRENT_USERNAME, host_buf, SYSUTIL_SYSTEMPARAM_CURRENT_USERNAME_SIZE) < 0)
			LOG("Error getting Username");

		if (size) *size = strlen(host_buf);
		return host_buf;

	case APOLLO_HOST_LAN_ADDR:
	case APOLLO_HOST_WLAN_ADDR:
		memset(&net_info, 0, sizeof(net_info));
		if (netCtlGetInfo(NET_CTL_INFO_ETHER_ADDR, &net_info) < 0)
			LOG("Error getting Wlan Ethernet Address");

		memcpy(host_buf, net_info.ether_addr.data, NET_CTL_ETHER_ADDR_LEN);
		if (size) *size = NET_CTL_ETHER_ADDR_LEN;
		return host_buf;
	}

	if (size) *size = 1;
	return host_buf;
}

static void copyAllSavesHDD(const save_entry_t* save, int all)
{
	int done = 0, err_count = 0;
	list_node_t *node;
	save_entry_t *item;
	uint64_t progress = 0;
	list_t *list = ((void**)save->dir_name)[0];

	init_progress_bar("Copying all saves...", save->path);

	LOG("Copying all saves from '%s' to HDD...", save->path);
	for (node = list_head(list); (item = list_get(node)); node = list_next(node))
	{
		progress++;
		update_progress_bar(progress, list_count(list), item->name);
		if (item->type != FILE_TYPE_MENU && (item->flags & SAVE_FLAG_PS3) && (all || item->flags & SAVE_FLAG_SELECTED))
			(_copy_save_hdd(item) ? done++ : err_count++);
	}

	end_progress_bar();

	show_message("%d/%d Saves copied to HDD", done, done+err_count);
}

static void extractArchive(const char* file_path)
{
	int ret = 0;
	char exp_path[256];

	strncpy(exp_path, file_path, sizeof(exp_path));
	*strrchr(exp_path, '.') = 0;

	switch (strrchr(file_path, '.')[1])
	{
	case 'z':
	case 'Z':
		/* ZIP */
		strcat(exp_path, "/");
		ret = extract_zip(file_path, exp_path);
		break;

	case 'r':
	case 'R':
		/* RAR */
		ret = extract_rar(file_path, exp_path);
		break;

	case '7':
		/* 7-Zip */
		ret = extract_7zip(file_path, exp_path);
		break;

	default:
		break;
	}

	if (ret)
		show_message("All files extracted to:\n%s", exp_path);
	else
		show_message("Error: %s couldn't be extracted", file_path);
}

static void exportLicensesZip(int dst)
{
	char* export_file;
	char* lic_path;
	char* tmp;
	char exp_path[256];

	_set_dest_path(exp_path, dst, PS3_EXPORT_PATH);
	if (mkdirs(exp_path) != SUCCESS)
	{
		show_message("Error! Export folder is not available:\n%s", exp_path);
		return;
	}

	init_loading_screen("Exporting user licenses...");

	asprintf(&export_file, "%s" "licenses_%08d.zip", exp_path, apollo_config.user_id);
	asprintf(&lic_path, EXDATA_PATH_HDD, apollo_config.user_id);

	tmp = strdup(lic_path);
	*strrchr(tmp, '/') = 0;

	zip_directory(tmp, lic_path, export_file);

	sprintf(export_file, "%s" OWNER_XML_FILE, exp_path);
	_saveOwnerData(export_file);

	sprintf(export_file, "%s" "idps.hex", exp_path);
	write_buffer(export_file, (u8*) apollo_config.idps, 16);
	file_chmod(export_file);

	free(export_file);
	free(lic_path);
	free(tmp);

	stop_loading_screen();
	show_message("Licenses successfully saved to:\n%slicenses_%08d.zip", exp_path, apollo_config.user_id);
}

static void exportFlashZip(int dst)
{
	char* export_file;
	char exp_path[256];

	_set_dest_path(exp_path, dst, PS3_EXPORT_PATH);
	if (mkdirs(exp_path) != SUCCESS)
	{
		show_message("Error! Export folder is not available:\n%s", exp_path);
		return;
	}

	init_loading_screen("Exporting /dev_flash2.zip ...");

	asprintf(&export_file, "%s" "dev_flash2.zip", exp_path);
	zip_directory("/dev_flash2", "/dev_flash2/", export_file);

	sprintf(export_file, "%s" OWNER_XML_FILE, exp_path);
	_saveOwnerData(export_file);

	sprintf(export_file, "%s" "idps.hex", exp_path);
	write_buffer(export_file, (u8*) apollo_config.idps, 16);
	file_chmod(export_file);

	free(export_file);

	stop_loading_screen();
	show_message("Files successfully saved to:\n%sdev_flash2.zip", exp_path);
}

static void exportTrophiesZip(int dst)
{
	int ret;
	char* export_file;
	char* trp_path;
	char* tmp;
	char exp_path[256];

	_set_dest_path(exp_path, dst, PS3_EXPORT_PATH);
	if (mkdirs(exp_path) != SUCCESS)
	{
		show_message("Error! Export folder is not available:\n%s", exp_path);
		return;
	}

	init_loading_screen("Exporting Trophies ...");

	asprintf(&export_file, "%s" "trophies_%08d.zip", exp_path, apollo_config.user_id);
	asprintf(&trp_path, TROPHY_PATH_HDD, apollo_config.user_id);

	tmp = strdup(trp_path);
	*strrchr(tmp, '/') = 0;

	ret = zip_directory(tmp, trp_path, export_file);

	sprintf(export_file, "%s" OWNER_XML_FILE, exp_path);
	_saveOwnerData(export_file);

	free(export_file);
	free(trp_path);
	free(tmp);

	stop_loading_screen();
	if (!ret)
	{
		show_message("Error! Failed to export Trophies to Zip");
		return;
	}

	show_message("Trophies successfully saved to:\n%strophies_%08d.zip", exp_path, apollo_config.user_id);
}

static void importTrophy(const char* src_path)
{
	char *tmp;
	char dst_path[256];

	snprintf(dst_path, sizeof(dst_path), TROPHY_PATH_HDD, apollo_config.user_id);
	if (mkdirs(dst_path) != SUCCESS)
	{
		show_message("Error! Trophy folder is not available:\n%s", dst_path);
		return;
	}

	tmp = strdup(src_path);
	*strrchr(tmp, '/') = 0;
	*strrchr(tmp, '/') = 0;
	LOG("Copying <%s> to %s...", src_path, dst_path);

	init_loading_screen("Importing trophy...");
	copy_directory(tmp, src_path, dst_path);
	stop_loading_screen();

	show_message("Trophy successfully copied to:\n%s", dst_path);
	free(tmp);
}

static void exportAllSavesVMC(const save_entry_t* save, int dev, int all)
{
	char outPath[256];
	int done = 0, err_count = 0;
	list_node_t *node;
	save_entry_t *item;
	uint64_t progress = 0;
	list_t *list = ((void**)save->dir_name)[0];

	init_progress_bar("Exporting all VMC saves...", save->path);
	_set_dest_path(outPath, dev, PS1_IMP_PATH_USB);
	mkdirs(outPath);

	LOG("Exporting all saves from '%s' to %s...", save->path, outPath);
	for (node = list_head(list); (item = list_get(node)); node = list_next(node))
	{
		update_progress_bar(progress++, list_count(list), item->name);
		if (!all && !(item->flags & SAVE_FLAG_SELECTED))
			continue;

		if (item->type == FILE_TYPE_PS1)
			(saveSingleSave(outPath, save->path[strlen(save->path)+1], PS1SAVE_PSV) ? done++ : err_count++);

		if (item->type == FILE_TYPE_PS2)
			(vmc_export_psv(item->dir_name, outPath) ? done++ : err_count++);
	}

	end_progress_bar();

	show_message("%d/%d Saves exported to\n%s", done, done+err_count, outPath);
}

static void exportVmcSave(const save_entry_t* save, int type, int dst_id)
{
	char outPath[256];
	struct tm t;

	_set_dest_path(outPath, dst_id, PS1_IMP_PATH_USB);
	mkdirs(outPath);
	if (type != PS1SAVE_PSV)
	{
		// build file path
		gmtime_r(&(time_t){time(NULL)}, &t);
		sprintf(strrchr(outPath, '/'), "/%s_%d-%02d-%02d_%02d%02d%02d.%s", save->title_id,
			t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec,
			(type == PS1SAVE_MCS) ? "mcs" : "psx");
	}

	if (saveSingleSave(outPath, save->path[strlen(save->path)+1], type))
		show_message("Save successfully exported to:\n%s", outPath);
	else
		show_message("Error exporting save:\n%s", save->path);
}

static void resignPSVfile(const char* psv_path)
{
	init_loading_screen("Resigning PSV file...");
	psv_resign(psv_path);
	stop_loading_screen();

	show_message("File successfully resigned!");
}

static void activateAccount(const char* ex_path)
{
	int ret;
	char path[256];
	char account_id[SFO_ACCOUNT_ID_SIZE+1];

	snprintf(path, sizeof(path), "%s" "act.dat", ex_path);
	if (file_exists(path) == SUCCESS)
	{
		show_message("Error! The account already has an act.dat");
		return;
	}

	if (mkdirs(ex_path) != SUCCESS)
	{
		show_message("Error! Folder is not available:\n%s", ex_path);
		return;
	}

	snprintf(account_id, sizeof(account_id), "%016lx", 0x6F6C6C6F70610000 + (~apollo_config.user_id & 0xFFFF));
	if ((apollo_config.account_id = get_account_id(apollo_config.user_id)) == 0 && (
		!osk_dialog_get_text("Enter the Account ID", account_id, sizeof(account_id)) ||
		!sscanf(account_id, "%lx", &apollo_config.account_id)))
	{
		show_message("Error! Account ID is not valid");
		return;
	};

	init_loading_screen("Activating PS3...");
	if (!create_fake_account(apollo_config.user_id, apollo_config.account_id))
	{
		stop_loading_screen();
		show_message("Error! Fake Account could not be assigned to xRegistry.sys");
		return;
	}

	ret = create_actdat(ex_path, apollo_config.account_id);
	stop_loading_screen();

	if (!ret)
	{
		show_message("Error! Account could not be activated!");
		return;
	}

	save_app_settings(&apollo_config);

	// wMM workaround to restore offline act.dat on HEN start
	if (is_ps3hen())
	{
		FILE* f = fopen("/dev_hdd0/boot_init.txt", "a");
		if (f)
		{
			fprintf(f, "\ncopy %s" "act.bak=%s" "act.dat\n", ex_path, ex_path);
			fclose(f);
		}
	}

	show_message("Account successfully activated!\nA system reboot might be required");
}

static void copyDummyPSV(const char* psv_file, int dst)
{
	char *in, *out;
	char out_path[256];

	_set_dest_path(out_path, dst, PSV_SAVES_PATH_USB);
	if (mkdirs(out_path) != SUCCESS)
	{
		show_message("Error! Export folder is not available:\n%s", out_path);
		return;
	}

	asprintf(&in, APOLLO_DATA_PATH "%s", psv_file);
	asprintf(&out, "%s%s", out_path, psv_file);

	init_loading_screen("Copying PSV file...");
	copy_file(in, out);
	stop_loading_screen();

	free(in);
	free(out);

	show_message("File successfully saved to:\n%s%s", out_path, psv_file);
}

static void exportPSVfile(const char* in_file, int dst, const char* path)
{
	char out_path[256];

	_set_dest_path(out_path, dst, path);
	if (mkdirs(out_path) != SUCCESS)
	{
		show_message("Error! Export folder is not available:\n%s", out_path);
		return;
	}

	init_loading_screen("Exporting PSV file...");

	if (selected_entry->flags & SAVE_FLAG_PS1)
		ps1_psv2mcs(in_file, out_path);
	else
		ps2_psv2psu(in_file, out_path);

	stop_loading_screen();
	show_message("File successfully saved to:\n%s", out_path);
}

static void convertSavePSV(const save_entry_t* save, int dst)
{
	char out_path[256];

	_set_dest_path(out_path, dst, PSV_SAVES_PATH_USB);
	if (mkdirs(out_path) != SUCCESS)
	{
		show_message("Error! Export folder is not available:\n%s", out_path);
		return;
	}

	init_loading_screen("Converting Save to PSV file...");

	switch (save->type)
	{
	case FILE_TYPE_MCS:
		ps1_mcs2psv(save->path, out_path);
		break;

	case FILE_TYPE_PSX:
		ps1_psx2psv(save->path, out_path);
		break;

	case FILE_TYPE_MAX:
		ps2_max2psv(save->path, out_path);
		break;

	case FILE_TYPE_PSU:
		ps2_psu2psv(save->path, out_path);
		break;

	case FILE_TYPE_CBS:
		ps2_cbs2psv(save->path, out_path);
		break;

	case FILE_TYPE_XPS:
		ps2_xps2psv(save->path, out_path);
		break;

	default:
		break;
	}

	stop_loading_screen();
	show_message("File successfully saved to:\n%s", out_path);
}

static void importVM2file(const char* vme_file, const char* src_name)
{
	int ret;
	char srcfile[256];

	snprintf(srcfile, sizeof(srcfile), "%s%s", VMC_PS2_PATH_HDD, src_name);

	init_loading_screen("Importing VM2 card...");
	ret = mcio_vmcImportImage(srcfile);
	stop_loading_screen();

	if (ret == sceMcResSucceed)
		show_message("File successfully imported to:\n%s", vme_file);
	else
		show_message("Error! Failed to import PS2 memory card");
}

static void importPS2VMC(const char* vmc_path, const char* vmc_file)
{
	char vm2file[256];
	char srcfile[256];

	snprintf(srcfile, sizeof(srcfile), "%s%s", vmc_path, vmc_file);
	snprintf(vm2file, sizeof(vm2file), "%s%s", VMC_PS2_PATH_HDD, vmc_file);
	strcpy(strrchr(vm2file, '.'), ".VM2");

	init_loading_screen("Importing PS2 memory card...");
	ps2_add_vmc_ecc(srcfile, vm2file);
	stop_loading_screen();

	show_message("File successfully saved to:\n%s", vm2file);
}

static void exportVM2raw(const char* vm2_file, int dst, int ecc)
{
	int ret;
	char dstfile[256];
	char dst_path[256];

	_set_dest_path(dst_path, dst, VMC_PS2_PATH_USB);
	if (mkdirs(dst_path) != SUCCESS)
	{
		show_message("Error! Export folder is not available:\n%s", dst_path);
		return;
	}

	snprintf(dstfile, sizeof(dstfile), "%s%s.%s", dst_path, vm2_file, ecc ? "VM2" : "vmc");

	init_loading_screen("Exporting PS2 memory card...");
	ret = mcio_vmcExportImage(dstfile, ecc);
	file_chmod(dstfile);
	stop_loading_screen();

	if (ret == sceMcResSucceed)
		show_message("File successfully saved to:\n%s", dstfile);
	else
		show_message("Error! Failed to export PS2 memory card");
}

static void importPS2classicsCfg(const char* cfg_path, const char* cfg_file)
{
	char ps2file[256];
	char outfile[256];

	snprintf(ps2file, sizeof(ps2file), "%s%s", cfg_path, cfg_file);
	snprintf(outfile, sizeof(outfile), PS2ISO_PATH_HDD "%s", cfg_file);
	*strrchr(outfile, '.') = 0;
	strcat(outfile, ".ENC");

	init_progress_bar("Encrypting PS2 CONFIG...", cfg_file);
	ps2_encrypt_image(1, ps2file, outfile);
	end_progress_bar();

	show_message("File successfully saved to:\n%s", outfile);
}

static void importPS2classics(const char* iso_path, const char* iso_file)
{
	char ps2file[256];
	char outfile[256];

	snprintf(ps2file, sizeof(ps2file), "%s%s", iso_path, iso_file);
	snprintf(outfile, sizeof(outfile), PS2ISO_PATH_HDD "%s", iso_file);
	*strrchr(outfile, '.') = 0;
	strcat(outfile, ".BIN.ENC");

	init_progress_bar("Encrypting PS2 ISO...", iso_file);
	ps2_encrypt_image(0, ps2file, outfile);
	end_progress_bar();

	show_message("File successfully saved to:\n%s", outfile);
}

static void exportPS2classics(const char* enc_path, const char* enc_file, uint8_t dst)
{
	char path[256];
	char ps2file[256];
	char outfile[256];

	if (dst != STORAGE_HDD)
		_set_dest_path(path, dst, PS2ISO_PATH);
	else
		snprintf(path, sizeof(path), PS2ISO_PATH_HDD);

	snprintf(ps2file, sizeof(ps2file), "%s%s", enc_path, enc_file);
	snprintf(outfile, sizeof(outfile), "%s%s", path, enc_file);
	*strrchr(outfile, '.') = 0;
	strcat(outfile, ".ps2.iso");

	if (mkdirs(outfile) != SUCCESS)
	{
		show_message("Error! Export folder is not available:\n%s", outfile);
		return;
	}

	init_progress_bar("Decrypting PS2 BIN.ENC...", enc_file);
	ps2_decrypt_image(0, ps2file, outfile);
	file_chmod(outfile);
	end_progress_bar();

	show_message("File successfully saved to:\n%s", outfile);
}

static void copyAllTrophies(const save_entry_t* save, int dst, int all)
{
	char exp_path[256];
	int done = 0, err_count = 0;
	list_node_t *node;
	save_entry_t *item;
	uint64_t progress = 0;
	list_t *list = ((void**)save->dir_name)[0];

	_set_dest_path(exp_path, dst, TROPHIES_PATH_USB);
	if (mkdirs(exp_path) != SUCCESS)
	{
		show_message("Error! Export folder is not available:\n%s", exp_path);
		return;
	}

	init_progress_bar("Copying trophies...", exp_path);
	LOG("Copying all trophies from '%s'...", save->path);

	for (node = list_head(list); (item = list_get(node)); node = list_next(node))
	{
		update_progress_bar(progress++, list_count(list), item->name);
		if (item->type != FILE_TYPE_TRP || !(all || item->flags & SAVE_FLAG_SELECTED))
			continue;

		(_copy_save_usb(item, exp_path) ? done++ : err_count++);
	}
	end_progress_bar();

	show_message("%d/%d Trophy Sets copied to:\n%s", done, done+err_count, exp_path);
}

static void exportLicensesRap(const char* fname, uint8_t dest)
{
	DIR *d;
	struct dirent *dir;
	char lic_path[256];
	char exp_path[256];
	char msg[128] = "Exporting user licenses...";

	if (dest != STORAGE_HDD)
		_set_dest_path(exp_path, dest, PS3_LICENSE_PATH);
	else
		snprintf(exp_path, sizeof(exp_path), EXPORT_RAP_PATH_HDD);

	if (mkdirs(exp_path) != SUCCESS)
	{
		show_message("Error! Export folder is not available:\n%s", exp_path);
		return;
	}

	snprintf(lic_path, sizeof(lic_path), EXDATA_PATH_HDD, apollo_config.user_id);
	d = opendir(lic_path);
	if (!d)
		return;

    init_loading_screen(msg);

	LOG("Exporting RAPs from folder '%s'...", lic_path);
	while ((dir = readdir(d)) != NULL)
	{
		if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0 &&
			(!fname || (strcmp(dir->d_name, fname) == 0)) &&
			strcmp(strrchr(dir->d_name, '.'), ".rif") == 0)
		{
			LOG("Exporting %s", dir->d_name);
			snprintf(msg, sizeof(msg), "Exporting %.36s...", dir->d_name);
			rif2rap((u8*) apollo_config.idps, lic_path, dir->d_name, exp_path);
		}
	}
	closedir(d);

    stop_loading_screen();
	show_message("Files successfully copied to:\n%s", exp_path);
}

static void importLicenses(const char* fname, const char* exdata_path)
{
	DIR *d;
	struct dirent *dir;
	char lic_path[256];

	if (dir_exists(exdata_path) != SUCCESS)
	{
		show_message("Error! Import folder is not available:\n%s", exdata_path);
		return;
	}

	snprintf(lic_path, sizeof(lic_path), EXDATA_PATH_HDD, apollo_config.user_id);
	d = opendir(exdata_path);
	if (!d)
		return;

    init_loading_screen("Importing user licenses...");

	LOG("Importing RAPs from folder '%s'...", exdata_path);
	while ((dir = readdir(d)) != NULL)
	{
		if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0 &&
			(!fname || (strcmp(dir->d_name, fname)) == 0) &&
			strcasecmp(strrchr(dir->d_name, '.'), ".rap") == 0)
		{
			LOG("Importing %s", dir->d_name);
			rap2rif((u8*) apollo_config.idps, exdata_path, dir->d_name, lic_path);
		}
	}
	closedir(d);

    stop_loading_screen();
	show_message("Files successfully copied to:\n%s", lic_path);
}

static int apply_sfo_patches(save_entry_t* entry, sfo_patch_t* patch)
{
    code_entry_t* code;
    char in_file_path[256];
    char tmp_dir[SFO_DIRECTORY_SIZE];
    u8 tmp_psid[SFO_PSID_SIZE];
    list_node_t* node;

    for (node = list_head(entry->codes); (code = list_get(node)); node = list_next(node))
    {
        if (!code->activated || code->type != PATCH_SFO)
            continue;

        LOG("Active: [%s]", code->name);

        switch (code->codes[0])
        {
        case SFO_UNLOCK_COPY:
            if (entry->flags & SAVE_FLAG_LOCKED)
                entry->flags ^= SAVE_FLAG_LOCKED;

            patch->flags = SFO_PATCH_FLAG_REMOVE_COPY_PROTECTION;
            break;

        case SFO_CHANGE_ACCOUNT_ID:
            if (entry->flags & SAVE_FLAG_OWNER)
                entry->flags ^= SAVE_FLAG_OWNER;

            memcpy(patch->account_id, code->options->value[code->options->sel], SFO_ACCOUNT_ID_SIZE);
            break;

        case SFO_REMOVE_PSID:
            bzero(tmp_psid, SFO_PSID_SIZE);
            patch->psid = tmp_psid;
            break;

        case SFO_CHANGE_TITLE_ID:
            patch->directory = strstr(entry->path, entry->title_id);
            snprintf(in_file_path, sizeof(in_file_path), "%s", entry->path);
            strncpy(tmp_dir, patch->directory, SFO_DIRECTORY_SIZE);

            strncpy(entry->title_id, code->options[0].name[code->options[0].sel], 9);
            strncpy(patch->directory, entry->title_id, 9);
            strncpy(tmp_dir, entry->title_id, 9);
            *strrchr(tmp_dir, '/') = 0;
            patch->directory = tmp_dir;

            LOG("Moving (%s) -> (%s)", in_file_path, entry->path);
            rename(in_file_path, entry->path);
            break;

        default:
            break;
        }

        code->activated = 0;
    }

    snprintf(in_file_path, sizeof(in_file_path), "%s" "PARAM.SFO", entry->path);
    LOG("Applying SFO patches '%s'...", in_file_path);

    return (patch_sfo(in_file_path, patch) == SUCCESS);
}

static int _is_decrypted(list_t* list, const char* fname)
{
	list_node_t *node;
	u8 *protected_file_id = get_secure_file_id(selected_entry->title_id, "UNPROTECTED");

	if (protected_file_id && (strncmp("UNPROTECTEDGAME", (char*)protected_file_id, 16) == 0))
		return 1;

	for (node = list_head(list); node; node = list_next(node))
		if (strcmp(list_get(node), fname) == 0)
			return 1;

	return 0;
}

static int apply_cheat_patches(const save_entry_t *entry)
{
	int ret = 1;
	char tmpfile[256];
	char* filename;
	code_entry_t* code;
	uint8_t* protected_file_id;
	list_t* decrypted_files = list_alloc();
	list_node_t* node;

	init_loading_screen("Applying changes...");

	for (node = list_head(entry->codes); (code = list_get(node)); node = list_next(node))
	{
		if (!code->activated || (code->type != PATCH_GAMEGENIE && code->type != PATCH_BSD))
			continue;

    	LOG("Active code: [%s]", code->name);

		if (strrchr(code->file, '\\'))
			filename = strrchr(code->file, '\\')+1;
		else
			filename = code->file;

		if (strchr(filename, '*'))
			filename = code->options[0].name[code->options[0].sel];

		if (strstr(code->file, "~extracted\\"))
			snprintf(tmpfile, sizeof(tmpfile), "%s[%s]%s", APOLLO_LOCAL_CACHE, entry->title_id, filename);
		else
		{
			snprintf(tmpfile, sizeof(tmpfile), "%s%s", entry->path, filename);

			if (!_is_decrypted(decrypted_files, filename))
			{
				LOG("Decrypting '%s'...", filename);
				protected_file_id = get_secure_file_id(entry->title_id, filename);

				if (decrypt_save_file(entry->path, filename, NULL, protected_file_id))
				{
					list_append(decrypted_files, strdup(filename));
				}
				else
				{
					LOG("Error: failed to decrypt (%s)", filename);
					ret = 0;
					continue;
				}
			}
		}

		if (!apply_cheat_patch_code(tmpfile, entry->title_id, code, &ps3_host_callback))
		{
			LOG("Error: failed to apply (%s)", code->name);
			ret = 0;
		}

		code->activated = 0;
	}

	for (node = list_head(decrypted_files); (filename = list_get(node)); node = list_next(node))
	{
		LOG("Encrypting '%s'...", filename);
		protected_file_id = get_secure_file_id(entry->title_id, filename);
		
		if (!encrypt_save_file(entry->path, filename, protected_file_id))
		{
			LOG("Error: failed to encrypt (%s)", filename);
			ret = 0;
		}

		free(filename);
	}

	list_free(decrypted_files);
	free_patch_var_list();
	stop_loading_screen();

	return ret;
}

static void resignSave(save_entry_t* entry)
{
    char acct_id[SFO_ACCOUNT_ID_SIZE+1] = {0};
    sfo_patch_t patch = {
        .flags = 0,
        .user_id = apollo_config.user_id,
        .psid = (u8*) apollo_config.psid,
        .account_id = acct_id,
        .directory = NULL,
    };

    if (apollo_config.account_id)
        snprintf(patch.account_id, sizeof(acct_id), "%*lx", SFO_ACCOUNT_ID_SIZE, apollo_config.account_id);

    if (!apply_sfo_patches(entry, &patch))
        show_message("Error! Account changes couldn't be applied");

    LOG("Applying cheats to '%s'...", entry->name);
    if (!apply_cheat_patches(entry))
        show_message("Error! Cheat codes couldn't be applied");

    LOG("Resigning save '%s'...", entry->name);
    if (!pfd_util_init((u8*) apollo_config.idps, apollo_config.user_id, entry->title_id, entry->path) ||
        (pfd_util_process(PFD_CMD_UPDATE, 0) != SUCCESS))
        show_message("Error! Save %s couldn't be resigned", entry->title_id);
    else
        show_message("Save %s successfully modified!", entry->title_id);

    pfd_util_end();
}

static void resignAllSaves(const save_entry_t* save, int all)
{
	char sfoPath[256];
	char acct_id[SFO_ACCOUNT_ID_SIZE+1] = {0};
	int err_count = 0;
	list_node_t *node;
	save_entry_t *item;
	uint64_t progress = 0;
	list_t *list = ((void**)save->dir_name)[0];
	sfo_patch_t patch = {
		.flags = SFO_PATCH_FLAG_REMOVE_COPY_PROTECTION,
		.user_id = apollo_config.user_id,
		.psid = (u8*) apollo_config.psid,
		.account_id = acct_id,
		.directory = NULL,
	};

	if (apollo_config.account_id)
		snprintf(patch.account_id, sizeof(acct_id), "%*lx", SFO_ACCOUNT_ID_SIZE, apollo_config.account_id);

	init_progress_bar("Resigning all saves...", save->path);

	LOG("Resigning all saves from '%s'...", save->path);
	for (node = list_head(list); (item = list_get(node)); node = list_next(node))
	{
		progress++;
		update_progress_bar(progress, list_count(list), item->name);
		if (item->type != FILE_TYPE_MENU && (item->flags & SAVE_FLAG_PS3) && (all || item->flags & SAVE_FLAG_SELECTED))
		{
			snprintf(sfoPath, sizeof(sfoPath), "%s" "PARAM.SFO", item->path);
			if (file_exists(sfoPath) != SUCCESS)
				continue;

			LOG("Patching SFO '%s'...", sfoPath);
			err_count += (patch_sfo(sfoPath, &patch) != SUCCESS);

			LOG("Resigning save '%s'...", item->path);
			if (!pfd_util_init((u8*) apollo_config.idps, apollo_config.user_id, item->title_id, item->path) ||
				(pfd_util_process(PFD_CMD_UPDATE, 0) != SUCCESS))
				LOG("Error! Save file couldn't be resigned");

			pfd_util_end();
		}
	}

	end_progress_bar();

	if (err_count)
		show_message("Error: %d Saves couldn't be resigned", err_count);
	else
		show_message("All saves successfully resigned!");
}

static int apply_trophy_account(const save_entry_t* entry)
{
	char sfoPath[256];
	char account_id[SFO_ACCOUNT_ID_SIZE+1];

	snprintf(account_id, sizeof(account_id), "%*lx", SFO_ACCOUNT_ID_SIZE, apollo_config.account_id);
	if (!apollo_config.account_id)
		memset(account_id, 0, SFO_ACCOUNT_ID_SIZE);

	snprintf(sfoPath, sizeof(sfoPath), "%s" "PARAM.SFO", entry->path);

	if (patch_sfo_trophy(sfoPath, account_id) != SUCCESS || !patch_trophy_account(entry->path, account_id))
		return 0;

	return 1;
}

static int apply_trophy_patches(const save_entry_t* entry)
{
	int ret = 1;
	uint32_t trophy_id;
	code_entry_t* code;
	list_node_t* node;

	init_loading_screen("Applying changes...");

	for (node = list_head(entry->codes); (code = list_get(node)); node = list_next(node))
	{
		if (!code->activated || (code->type != PATCH_TROP_UNLOCK && code->type != PATCH_TROP_LOCK))
			continue;

		trophy_id = *(uint32_t*)(code->file);
    	LOG("Active code: [%d] '%s'", trophy_id, code->name);

		if (!apply_trophy_patch(entry->path, trophy_id, (code->type == PATCH_TROP_UNLOCK)))
		{
			LOG("Error: failed to apply (%s)", code->name);
			ret = 0;
		}

		if (code->type == PATCH_TROP_UNLOCK)
		{
			code->type = PATCH_TROP_LOCK;
			code->name[1] = ' ';
		}
		else
		{
			code->type = PATCH_TROP_UNLOCK;
			code->name[1] = CHAR_TAG_LOCKED;
		}

		code->activated = 0;
	}

	stop_loading_screen();

	return ret;
}

static void resignTrophy(const save_entry_t* entry)
{
	LOG("Decrypting TROPTRNS.DAT ...");
	if (!decrypt_trophy_trns(entry->path))
	{
		LOG("Error: failed to decrypt TROPTRNS.DAT");
		return;
	}

    if (!apply_trophy_account(entry))
        show_message("Error! Account changes couldn't be applied");

    LOG("Applying trophy changes to '%s'...", entry->name);
    if (!apply_trophy_patches(entry))
        show_message("Error! Trophy changes couldn't be applied");

	LOG("Encrypting TROPTRNS.DAT ...");
	if (!encrypt_trophy_trns(entry->path))
	{
		LOG("Error: failed to encrypt TROPTRNS.DAT");
		return;
	}

    LOG("Resigning trophy '%s'...", entry->name);

    if (!pfd_util_init((u8*) apollo_config.idps, apollo_config.user_id, entry->title_id, entry->path) ||
        (pfd_util_process(PFD_CMD_UPDATE, 0) != SUCCESS))
        show_message("Error! Trophy %s couldn't be resigned", entry->title_id);
    else
        show_message("Trophy %s successfully modified!", entry->title_id);

    pfd_util_end();

	if ((file_exists("/dev_hdd0/mms/db.err") != SUCCESS) && show_dialog(DIALOG_TYPE_YESNO, "Schedule Database rebuild on next boot?"))
	{
		LOG("Creating db.err file for database rebuild...");
		write_buffer("/dev_hdd0/mms/db.err", (u8*) "\x00\x00\x03\xE9", 4);
	}
}

static int _copy_save_file(const char* src_path, const char* dst_path, const char* filename)
{
	char src[256], dst[256];

	snprintf(src, sizeof(src), "%s%s", src_path, filename);
	snprintf(dst, sizeof(dst), "%s%s", dst_path, filename);

	return (copy_file(src, dst) == SUCCESS);
}

static void decryptSaveFile(const save_entry_t* entry, const char* filename)
{
	char path[256];

	snprintf(path, sizeof(path), APOLLO_TMP_PATH "%s/", entry->dir_name);
	mkdirs(path);

	if (_is_decrypted(NULL, filename))
	{
		_copy_save_file(entry->path, path, filename);
		show_message("Save-game %s is not encrypted. File was not decrypted:\n%s%s", entry->title_id, path, filename);
		return;
	}

	u8* protected_file_id = get_secure_file_id(entry->title_id, filename);

	LOG("Decrypt '%s%s' to '%s'...", entry->path, filename, path);

	if (decrypt_save_file(entry->path, filename, path, protected_file_id))
		show_message("File successfully decrypted to:\n%s%s", path, filename);
	else
		show_message("Error! File %s couldn't be decrypted", filename);
}

static void encryptSaveFile(const save_entry_t* entry, const char* filename)
{
	char path[256];

	snprintf(path, sizeof(path), APOLLO_TMP_PATH "%s/%s", entry->dir_name, filename);

	if (file_exists(path) != SUCCESS)
	{
		show_message("Error! Can't find decrypted save-game file:\n%s", path);
		return;
	}
	*(strrchr(path, '/')+1) = 0;

	if (_is_decrypted(NULL, filename))
	{
		_copy_save_file(path, entry->path, filename);
		show_message("Save-game %s is not encrypted.\nFile %s was not encrypted", entry->title_id, filename);
		return;
	}

	u8* protected_file_id = get_secure_file_id(entry->title_id, filename);

	LOG("Encrypt '%s%s' to '%s'...", path, filename, entry->path);
	_copy_save_file(path, entry->path, filename);

	if (encrypt_save_file(entry->path, filename, protected_file_id))
		show_message("File successfully encrypted to:\n%s%s", entry->path, filename);
	else
		show_message("Error! File %s couldn't be encrypted", filename);

	LOG("Resigning save '%s'...", entry->path);
	if (!pfd_util_init((u8*) apollo_config.idps, apollo_config.user_id, entry->title_id, entry->path) ||
		(pfd_util_process(PFD_CMD_UPDATE, 0) != SUCCESS))
		LOG("Error! Save file couldn't be resigned");

	pfd_util_end();
}

static void downloadLink(const char* path)
{
	char url[256] = "http://";
	char out_path[256];

	if (!osk_dialog_get_text("Download URL", url, sizeof(url)))
		return;

	char *fname = strrchr(url, '/');
	snprintf(out_path, sizeof(out_path), "%s%s", path, fname ? ++fname : "download.bin");

	if (http_download(url, NULL, out_path, 1))
		show_message("File successfully downloaded to:\n%s", out_path);
	else
		show_message("Error! File couldn't be downloaded");
}

static void import_mcr2vmp(const save_entry_t* save, const char* src)
{
	char mcrPath[256];
	uint8_t *data = NULL;
	size_t size = 0;

	snprintf(mcrPath, sizeof(mcrPath), VMC_PS2_PATH_HDD "%s/%s", save->title_id, src);
	read_buffer(mcrPath, &data, &size);

	if (openMemoryCardStream(data, size, 0) && saveMemoryCard(save->path, 0, 0))
		show_message("Memory card successfully imported to:\n%s", save->path);
	else
		show_message("Error importing memory card:\n%s", mcrPath);
}

static void export_vmp2mcr(const save_entry_t* save)
{
	char mcrPath[256];

	snprintf(mcrPath, sizeof(mcrPath), VMC_PS2_PATH_HDD "%s/%s", save->title_id, strrchr(save->path, '/') + 1);
	strcpy(strrchr(mcrPath, '.'), ".MCR");
	mkdirs(mcrPath);

	if (saveMemoryCard(mcrPath, PS1CARD_RAW, 0))
		show_message("Memory card successfully exported to:\n%s", mcrPath);
	else
		show_message("Error exporting memory card:\n%s", save->path);
}

static void export_vmc2save(const save_entry_t* save, int type, int dst_id)
{
	int ret = 0;
	char outPath[256];
	struct tm t;

	_set_dest_path(outPath, dst_id, (type == FILE_TYPE_PSV) ? PSV_SAVES_PATH_USB : PS2_IMP_PATH_USB);
	mkdirs(outPath);
	if (type != FILE_TYPE_PSV)
	{
		// build file path
		gmtime_r(&(time_t){time(NULL)}, &t);
		sprintf(strrchr(outPath, '/'), "/%s_%d-%02d-%02d_%02d%02d%02d.psu", save->title_id,
			t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
	}

	switch (type)
	{
	case FILE_TYPE_PSV:
		ret = vmc_export_psv(save->dir_name, outPath);
		break;

	case FILE_TYPE_PSU:
		ret = vmc_export_psu(save->dir_name, outPath);
		break;

	default:
		break;
	}

	if (ret)
		show_message("Save successfully exported to:\n%s", outPath);
	else
		show_message("Error exporting save:\n%s", save->path);
}

static void import_save2vmc(const char* src, int type)
{
	int ret = 0;

	switch (type)
	{
	case FILE_TYPE_PSV:
		ret = vmc_import_psv(src);
		break;

	case FILE_TYPE_PSU:
		ret = vmc_import_psu(src);
		break;

	default:
		break;
	}

	if (ret)
		show_message("Successfully imported to VMC:\n%s", src);
	else
		show_message("Error importing save:\n%s", src);
}

void execCodeCommand(code_entry_t* code, const char* codecmd)
{
	switch (codecmd[0])
	{
		case CMD_DECRYPT_FILE:
			decryptSaveFile(selected_entry, code->options[0].name[code->options[0].sel]);
			code->activated = 0;
			break;

		case CMD_DOWNLOAD_USB:
		case CMD_DOWNLOAD_HDD:
			if (selected_entry->flags & SAVE_FLAG_PS3)
				downloadSave(selected_entry, code->file, codecmd[1], PS3_SAVES_PATH_USB);
			else
				downloadSave(selected_entry, code->file, codecmd[1], PSV_SAVES_PATH_USB);
			
			code->activated = 0;
			break;

		case CMD_EXPORT_ZIP_USB:
		case CMD_EXPORT_ZIP_HDD:
			zipSave(selected_entry, codecmd[1]);
			code->activated = 0;
			break;

		case CMD_COPY_SAVE_USB:
			copySave(selected_entry, codecmd[1], PS3_SAVES_PATH_USB);
			code->activated = 0;
			break;

		case CMD_COPY_SAVE_HDD:
			copySaveHDD(selected_entry);
			code->activated = 0;
			break;

		case CMD_EXP_EXDATA_USB:
			exportLicensesZip(codecmd[1]);
			code->activated = 0;
			break;

		case CMD_EXP_LICS_RAPS:
			exportLicensesRap(code->file, codecmd[1]);
			code->activated = 0;
			break;

		case CMD_CREATE_ACT_DAT:
			activateAccount(selected_entry->path);
			code->activated = 0;
			break;

		case CMD_EXP_TROPHY_USB:
			copySave(selected_entry, codecmd[1], TROPHIES_PATH_USB);
			code->activated = 0;
			break;

		case CMD_IMP_TROPHY_HDD:
			importTrophy(code->file);
			code->activated = 0;
			break;

		case CMD_ZIP_TROPHY_USB:
			exportTrophiesZip(codecmd[1]);
			code->activated = 0;
			break;

		case CMD_COPY_TROPHIES_USB:
		case CMD_COPY_ALL_TROP_USB:
			copyAllTrophies(selected_entry, codecmd[1], codecmd[0] == CMD_COPY_ALL_TROP_USB);
			code->activated = 0;
			break;

		case CMD_COPY_SAVES_USB:
		case CMD_COPY_ALL_SAVES_USB:
			copyAllSavesUSB(selected_entry, codecmd[1], codecmd[0] == CMD_COPY_ALL_SAVES_USB);
			code->activated = 0;
			break;

		case CMD_EXP_FLASH2_USB:
			exportFlashZip(codecmd[1]);
			code->activated = 0;
			break;

		case CMD_IMP_EXDATA_USB:
			importLicenses(code->file, selected_entry->path);
			code->activated = 0;
			break;

		case CMD_RESIGN_SAVE:
			resignSave(selected_entry);
			code->activated = 0;
			break;

		case CMD_RESIGN_SAVES:
		case CMD_RESIGN_ALL_SAVES:
			resignAllSaves(selected_entry, codecmd[0] == CMD_RESIGN_ALL_SAVES);
			code->activated = 0;
			break;

		case CMD_COPY_SAVES_HDD:
		case CMD_COPY_ALL_SAVES_HDD:
			copyAllSavesHDD(selected_entry, codecmd[0] == CMD_COPY_ALL_SAVES_HDD);
			code->activated = 0;
			break;

		case CMD_RESIGN_PSV:
			resignPSVfile(selected_entry->path);
			code->activated = 0;
			break;

		case CMD_IMP_PS2_VM2:
			importVM2file(selected_entry->path, code->options[0].name[code->options[0].sel]);
			code->activated = 0;
			break;

		case CMD_IMP_PS2_ISO:
			importPS2classics(selected_entry->path, code->file);
			code->activated = 0;
			break;

		case CMD_IMP_PS2_CONFIG:
			importPS2classicsCfg(selected_entry->path, code->file);
			code->activated = 0;
			break;

		case CMD_CONVERT_TO_PSV:
			convertSavePSV(selected_entry, codecmd[1]);
			code->activated = 0;
			break;

		case CMD_COPY_DUMMY_PSV:
			copyDummyPSV(code->file, codecmd[1]);
			code->activated = 0;
			break;

		case CMD_EXP_PS2_BINENC:
			exportPS2classics(selected_entry->path, code->file, codecmd[1]);
			code->activated = 0;
			break;

		case CMD_EXP_PSV_MCS:
			exportPSVfile(selected_entry->path, codecmd[1], PS1_IMP_PATH_USB);
			code->activated = 0;
			break;

		case CMD_EXP_PSV_PSU:
			exportPSVfile(selected_entry->path, codecmd[1], PS2_IMP_PATH_USB);
			code->activated = 0;
			break;

		case CMD_EXP_PS2_VM2:
		case CMD_EXP_VM2_RAW:
			exportVM2raw(code->file, codecmd[1], codecmd[0] == CMD_EXP_PS2_VM2);
			code->activated = 0;
			break;

		case CMD_RESIGN_VMP:
			if (vmp_resign(selected_entry->path))
				show_message("Memory card successfully resigned:\n%s", selected_entry->path);
			else
				show_message("Error resigning memory card:\n%s", selected_entry->path);
			code->activated = 0;
			break;

		case CMD_EXP_VMP2MCR:
			export_vmp2mcr(selected_entry);
			code->activated = 0;
			break;

		case CMD_EXP_SAVES_VMC1:
		case CMD_EXP_ALL_SAVES_VMC1:
			exportAllSavesVMC(selected_entry, codecmd[1], codecmd[0] == CMD_EXP_ALL_SAVES_VMC1);
			code->activated = 0;
			break;

		case CMD_EXP_VMC1SAVE:
			exportVmcSave(selected_entry, code->options[0].id, codecmd[1]);
			code->activated = 0;
			break;

		case CMD_IMP_VMC1SAVE:
			if (openSingleSave(code->file, (int*) host_buf))
			{
				saveMemoryCard(selected_entry->dir_name, 0, 0);
				show_message("Save successfully imported:\n%s", code->file);
			}
			else
				show_message("Error! Couldn't import save:\n%s", code->file);
			code->activated = 0;
			break;

		case CMD_EXP_SAVES_VMC2:
		case CMD_EXP_ALL_SAVES_VMC2:
			exportAllSavesVMC(selected_entry, codecmd[1], codecmd[0] == CMD_EXP_ALL_SAVES_VMC2);
			code->activated = 0;
			break;

		case CMD_EXP_VMC2SAVE:
			export_vmc2save(selected_entry, code->options[0].id, codecmd[1]);
			code->activated = 0;
			break;

		case CMD_IMP_VMC2SAVE:
			import_save2vmc(code->file, codecmd[1]);
			code->activated = 0;
			break;

		case CMD_IMP_MCR2VMP:
			import_mcr2vmp(selected_entry, code->options[0].name[code->options[0].sel]);
			code->activated = 0;
			break;

		case CMD_IMP_PS2VMC_USB:
			importPS2VMC(selected_entry->path, code->file);
			code->activated = 0;
			break;

		case CMD_IMPORT_DATA_FILE:
			encryptSaveFile(selected_entry, code->options[0].name[code->options[0].sel]);
			code->activated = 0;
			break;

		case CMD_RESIGN_TROPHY:
			resignTrophy(selected_entry);
			code->activated = 0;
			break;

		case CMD_SAVE_WEB_SERVER:
			enableWebServer(webReqHandler, ((void**)selected_entry->dir_name)[0], 8080);
			code->activated = 0;
			break;

		case CMD_EXTRACT_ARCHIVE:
			extractArchive(code->file);
			code->activated = 0;
			break;

		case CMD_URL_DOWNLOAD:
			downloadLink(selected_entry->path);
			code->activated = 0;
			break;

		case CMD_NET_WEBSERVER:
			enableWebServer(dbg_simpleWebServerHandler, NULL, 8080);
			code->activated = 0;
			break;

		default:
			break;
	}

}
