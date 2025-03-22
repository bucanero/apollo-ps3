#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <soundlib/audioplayer.h>

#include "menu.h"
#include "saves.h"
#include "common.h"

static char * db_opt[] = {"Online DB", "FTP Server", NULL};
static char * sort_opt[] = {"Disabled", "by Name", "by Title ID", "by Type", NULL};

static void log_callback(int sel);
static void sort_callback(int sel);
static void ani_callback(int sel);
static void server_callback(int sel);
static void db_url_callback(int sel);
static void ftp_url_callback(int sel);
static void redetect_callback(int sel);
static void clearcache_callback(int sel);
static void upd_appdata_callback(int sel);

menu_option_t menu_options[] = {
	{ .name = "Background Music", 
		.options = NULL, 
		.type = APP_OPTION_BOOL, 
		.value = &apollo_config.music, 
		.callback = music_callback 
	},
	{ .name = "Menu Animations", 
		.options = NULL, 
		.type = APP_OPTION_BOOL, 
		.value = &apollo_config.doAni, 
		.callback = ani_callback 
	},
	{ .name = "Sort Saves", 
		.options = sort_opt,
		.type = APP_OPTION_LIST,
		.value = &apollo_config.doSort,
		.callback = sort_callback
	},
	{ .name = "Set User FTP Server URL",
		.options = NULL,
		.type = APP_OPTION_CALL,
		.value = NULL,
		.callback = ftp_url_callback
	},
	{ .name = "\nVersion Update Check", 
		.options = NULL, 
		.type = APP_OPTION_BOOL, 
		.value = &apollo_config.update, 
		.callback = update_callback 
	},
	{ .name = "Clear Local Cache", 
		.options = NULL, 
		.type = APP_OPTION_CALL, 
		.value = NULL, 
		.callback = clearcache_callback 
	},
	{ .name = "Update Application Data", 
		.options = NULL, 
		.type = APP_OPTION_CALL, 
		.value = NULL, 
		.callback = upd_appdata_callback 
	},
	{ .name = "Change Online Database URL",
		.options = NULL,
		.type = APP_OPTION_CALL,
		.value = NULL,
		.callback = db_url_callback 
	},
	{ .name = "\nSaves Server",
		.options = db_opt,
		.type = APP_OPTION_LIST,
		.value = &apollo_config.db_opt,
		.callback = server_callback
	},
	{ .name = "Update Account & Console IDs",
		.options = NULL,
		.type = APP_OPTION_CALL,
		.value = NULL,
		.callback = redetect_callback 
	},
	{ .name = "Enable Debug Log",
		.options = NULL,
		.type = APP_OPTION_BOOL,
		.value = &apollo_config.dbglog,
		.callback = log_callback 
	},
	{ .name = NULL }
};


void music_callback(int sel)
{
	apollo_config.music = !sel;
	SND_Pause(sel);
}

static void sort_callback(int sel)
{
	apollo_config.doSort = sel;
}

static void ani_callback(int sel)
{
	apollo_config.doAni = !sel;
}

static void db_url_callback(int sel)
{
	if (!osk_dialog_get_text("Enter the URL of the online database", apollo_config.save_db, sizeof(apollo_config.save_db)))
		return;
	
	if (apollo_config.save_db[strlen(apollo_config.save_db)-1] != '/')
		strcat(apollo_config.save_db, "/");

	show_message("Online database URL changed to:\n%s", apollo_config.save_db);
}

static void ftp_url_callback(int sel)
{
	int ret;
	char tmp[512];

	strncpy(tmp, apollo_config.ftp_server[0] ? apollo_config.ftp_server : "ftp://user:pass@192.168.0.10:21/folder/", sizeof(tmp));
	if (!osk_dialog_get_text("Enter the URL of the FTP server", tmp, sizeof(tmp)))
		return;

	strncpy(apollo_config.ftp_server, tmp, sizeof(apollo_config.ftp_server));
	
	if (apollo_config.ftp_server[strlen(apollo_config.ftp_server)-1] != '/')
		strcat(apollo_config.ftp_server, "/");

	// test the connection
	init_loading_screen("Testing connection...");
	ret = http_download(apollo_config.ftp_server, "apollo.txt", APOLLO_TMP_PATH "users.ftp", 0);
	char *data = readTextFile(APOLLO_TMP_PATH "users.ftp", NULL);
	if (!data)
		data = strdup("; Apollo Save Tool (PS3) v" APOLLO_VERSION "\r\n");

	snprintf(tmp, sizeof(tmp), "%016lX", apollo_config.account_id);
	if (strstr(data, tmp) == NULL)
	{
		LOG("Updating users index...");
		FILE* fp = fopen(APOLLO_TMP_PATH "users.ftp", "w");
		if (fp)
		{
			fwrite(data, 1, strlen(data), fp);
			fprintf(fp, "%s\r\n", tmp);
			fclose(fp);
		}

		ret = ftp_upload(APOLLO_TMP_PATH "users.ftp", apollo_config.ftp_server, "apollo.txt", 0);
	}
	free(data);
	stop_loading_screen();

	if (ret)
		show_message("FTP server URL changed to:\n%s", apollo_config.ftp_server);
	else
		show_message("Error! Couldn't connect to FTP server\n%s\n\nCheck debug logs for more information", apollo_config.ftp_server);
}

static void clearcache_callback(int sel)
{
	LOG("Cleaning folder '%s'...", APOLLO_LOCAL_CACHE);
	clean_directory(APOLLO_LOCAL_CACHE);

	show_message("Local cache folder cleaned:\n" APOLLO_LOCAL_CACHE);
}

void unzip_app_data(const char* zip_file)
{
	if (extract_zip(zip_file, APOLLO_DATA_PATH))
		show_message("Successfully installed local application data");

	unlink_secure(zip_file);
}

static void upd_appdata_callback(int sel)
{
	if (http_download(ONLINE_PATCH_URL, "apollo-ps3-update.zip", APOLLO_LOCAL_CACHE "appdata.zip", 1))
		unzip_app_data(APOLLO_LOCAL_CACHE "appdata.zip");
}

void update_callback(int sel)
{
    apollo_config.update = !sel;

    if (!apollo_config.update)
        return;

	LOG("checking latest Apollo version at %s", APOLLO_UPDATE_URL);

	if (!http_download(APOLLO_UPDATE_URL, NULL, APOLLO_LOCAL_CACHE "ver.check", 0))
	{
		LOG("http request to %s failed", APOLLO_UPDATE_URL);
		return;
	}

	char *buffer;
	long size = 0;

	buffer = readTextFile(APOLLO_LOCAL_CACHE "ver.check", &size);

	if (!buffer)
		return;

	LOG("received %u bytes", size);

	static const char find[] = "\"name\":\"Apollo Save Tool v";
	const char* start = strstr(buffer, find);
	if (!start)
	{
		LOG("no name found");
		goto end_update;
	}

	LOG("found name");
	start += sizeof(find) - 1;

	char* end = strchr(start, '"');
	if (!end)
	{
		LOG("no end of name found");
		goto end_update;
	}
	*end = 0;
	LOG("latest version is %s", start);

	if (strcasecmp(APOLLO_VERSION, start) == 0)
	{
		LOG("no need to update");
		goto end_update;
	}

	start = strstr(end+1, "\"browser_download_url\":\"");
	if (!start)
		goto end_update;

	start += 24;
	end = strchr(start, '"');
	if (!end)
	{
		LOG("no download URL found");
		goto end_update;
	}

	*end = 0;
	LOG("download URL is %s", start);

	if (show_dialog(DIALOG_TYPE_YESNO, "New version available! Download update?"))
	{
		if (http_download(start, NULL, "/dev_hdd0/packages/apollo-ps3.pkg", 1))
			show_message("Update downloaded to /dev_hdd0/packages/");
		else
			show_message("Download error!");
	}

end_update:
	free(buffer);
	return;
}

static void server_callback(int sel)
{
	apollo_config.db_opt = sel;
}

static void log_callback(int sel)
{
	apollo_config.dbglog = !sel;

	if (!apollo_config.dbglog)
	{
		dbglogger_stop();
		show_message("Debug Logging Disabled");
		return;
	}

	dbglogger_init_mode(FILE_LOGGER, "/dev_hdd0/tmp/apollo.log", 1);
	show_message("Debug Logging Enabled!\n\n%s", "/dev_hdd0/tmp/apollo.log");
}

static void redetect_callback(int sel)
{
	init_loading_screen("Updating Account & Console IDs...");
	reset_app_settings(&apollo_config);
	stop_loading_screen();

	show_message("IDs Updated! Account ID: %016lx\nPSID: %016lX %016lX\nIDPS: %016lX %016lX", 
		apollo_config.account_id, apollo_config.psid[0], apollo_config.psid[1], apollo_config.idps[0], apollo_config.idps[1]);
}
