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
}

static uint32_t get_filename_id(const char* dir)
{
	char path[128];
	uint32_t tid = 0;

	do
	{
		tid++;
		snprintf(path, sizeof(path), "%s%08d.zip", dir, tid);
	}
	while (file_exists(path) == SUCCESS);

	return tid;
}

static void zipSave(const save_entry_t* entry, int dest)
{
	char exp_path[256];
	char* export_file;
	char* tmp;
	uint32_t fid;

	_set_dest_path(exp_path, dest, PS3_EXPORT_PATH);
	if (mkdirs(exp_path) != SUCCESS)
	{
		show_message("Error! Export folder is not available:\n%s", exp_path);
		return;
	}

	init_loading_screen("Exporting save game...");

	fid = get_filename_id(exp_path);
	asprintf(&export_file, "%s%08d.zip", exp_path, fid);

	asprintf(&tmp, entry->path);
	*strrchr(tmp, '/') = 0;
	*strrchr(tmp, '/') = 0;

	zip_directory(tmp, entry->path, export_file);

	sprintf(export_file, "%s%08d.txt", exp_path, fid);
	FILE* f = fopen(export_file, "a");
	if (f)
	{
		fprintf(f, "%08d.zip=[%s] %s\n", fid, entry->title_id, entry->name);
		fclose(f);
	}

	sprintf(export_file, "%s" OWNER_XML_FILE, exp_path);
	_saveOwnerData(export_file);

	free(export_file);
	free(tmp);

	stop_loading_screen();
	show_message("Zip file successfully saved to:\n%s%08d.zip", exp_path, fid);
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

	show_message("Files successfully copied to:\n%s", exp_path);
}

static void copyAllSavesUSB(const save_entry_t* save, int dst, int all)
{
	char usb_path[256];
	int err_count = 0;
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
			err_count += ! _copy_save_usb(item, usb_path);
	}
	end_progress_bar();

	if (err_count)
		show_message("Error: %d Saves couldn't be copied to USB", err_count);
	else
		show_message("All Saves copied to USB");
}

static int _copy_save_hdd(const save_entry_t *item)
{
	char copy_path[256];

	snprintf(copy_path, sizeof(copy_path), SAVES_PATH_HDD "%s/", apollo_config.user_id, item->dir_name);

	if (dir_exists(copy_path) == SUCCESS)
	{
		LOG("Error! Save-game folder already exists: %s", copy_path);
		return 0;
	}

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
	init_loading_screen("Copying save game...");
	int ret = _copy_save_hdd(save);
	stop_loading_screen();

	if (ret)
		show_message("Save-game copied to HDD");
	else
		show_message("Error! Save-game folder already exists:\n%s", save->dir_name);
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

		fprintf(f, "<html><head><meta charset=\"UTF-8\"><style>h1, h2 { font-family: arial; } table { border-collapse: collapse; margin: 25px 0; font-size: 0.9em; font-family: sans-serif; min-width: 400px; box-shadow: 0 0 20px rgba(0, 0, 0, 0.15); } table thead tr { background-color: #009879; color: #ffffff; text-align: left; } table th, td { padding: 12px 15px; } table tbody tr { border-bottom: 1px solid #dddddd; } table tbody tr:nth-of-type(even) { background-color: #f3f3f3; } table tbody tr:last-of-type { border-bottom: 2px solid #009879; }</style>");
		fprintf(f, "<script language=\"javascript\">document.addEventListener(\"DOMContentLoaded\",function(){var e;if(\"IntersectionObserver\"in window){e=document.querySelectorAll(\".lazy\");var n=new IntersectionObserver(function(e,t){e.forEach(function(e){if(e.isIntersecting){var t=e.target;t.src=t.dataset.src,t.classList.remove(\"lazy\"),n.unobserve(t)}})});e.forEach(function(e){n.observe(e)})}else{var t;function r(){t&&clearTimeout(t),t=setTimeout(function(){var n=window.pageYOffset;e.forEach(function(e){e.offsetTop<window.innerHeight+n&&(e.src=e.dataset.src,e.classList.remove(\"lazy\"))}),0==e.length&&(document.removeEventListener(\"scroll\",r),window.removeEventListener(\"resize\",r),window.removeEventListener(\"orientationChange\",r))},20)}e=document.querySelectorAll(\".lazy\"),document.addEventListener(\"scroll\",r),window.addEventListener(\"resize\",r),window.addEventListener(\"orientationChange\",r)}});</script>");
		fprintf(f, "<title>Apollo Save Tool</title></head><body><h1>.:: Apollo Save Tool</h1><h2>Index of %s</h2><table><thead><tr><th>Name</th><th>Icon</th><th>Title ID</th><th>Folder</th><th>Location</th></tr></thead><tbody>", selected_entry->path);

		for (node = list_head(list); (item = list_get(node)); node = list_next(node))
		{
			if (item->type == FILE_TYPE_MENU || !(item->flags & SAVE_FLAG_PS3))
				continue;

			fprintf(f, "<tr><td><a href=\"/zip/%s.zip\">%s</a></td>", item->dir_name, item->name);
			fprintf(f, "<td><img class=\"lazy\" data-src=\"/icon/%s.png\" alt=\"%s\" width=\"320\" height=\"176\"></td>", item->dir_name, item->name);
			fprintf(f, "<td>%s</td>", item->title_id);
			fprintf(f, "<td>%s</td>", item->dir_name);
			fprintf(f, "<td>%.3s</td></tr>", selected_entry->path + 5);
		}

		fprintf(f, "</tbody></table></body></html>");
		fclose(f);
		return 1;
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

static void copyAllSavesHDD(const save_entry_t* save, int all)
{
	int err_count = 0;
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
			err_count += ! _copy_save_hdd(item);
	}

	end_progress_bar();

	if (err_count)
		show_message("Error: %d Saves couldn't be copied to HDD", err_count);
	else
		show_message("All Saves copied to HDD");
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

	free(export_file);

	stop_loading_screen();
	show_message("Files successfully saved to:\n%sdev_flash2.zip", exp_path);
}

static void exportTrophiesZip(int dst)
{
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

	zip_directory(tmp, trp_path, export_file);

	sprintf(export_file, "%s" OWNER_XML_FILE, exp_path);
	_saveOwnerData(export_file);

	free(export_file);
	free(trp_path);
	free(tmp);

	stop_loading_screen();
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

	if (!apollo_config.account_id && (apollo_config.account_id = create_fake_account(apollo_config.user_id)) == 0)
	{
		show_message("Error! Fake Account could not be assigned to xRegistry.sys");
		return;
	}

	init_loading_screen("Activating PS3...");
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

static void decryptVMEfile(const char* vme_path, const char* vme_file, uint8_t dst)
{
	char vmefile[256];
	char outfile[256];
	char path[256];

	_set_dest_path(path, dst, VMC_PS2_PATH_USB);
	if (dst == STORAGE_HDD)
		snprintf(path, sizeof(path), VMC_PS2_PATH_HDD);

	if (mkdirs(path) != SUCCESS)
	{
		show_message("Error! Export folder is not available:\n%s", path);
		return;
	}

	snprintf(vmefile, sizeof(vmefile), "%s%s", vme_path, vme_file);
	snprintf(outfile, sizeof(outfile), "%sAPOLLO%c.VM2", path, vme_file[6]);

	init_loading_screen("Decrypting VME card...");
	ps2_crypt_vmc(0, vmefile, outfile, 0);
	stop_loading_screen();

	show_message("File successfully saved to:\n%s", outfile);
}

static void encryptVM2file(const char* vme_path, const char* vme_file, const char* src_name)
{
	char vmefile[256];
	char srcfile[256];

	snprintf(vmefile, sizeof(vmefile), "%s%s", vme_path, vme_file);
	snprintf(srcfile, sizeof(srcfile), "%s%s", VMC_PS2_PATH_HDD, src_name);

	init_loading_screen("Encrypting VM2 card...");
	ps2_crypt_vmc(0, srcfile, vmefile, 1);
	stop_loading_screen();

	show_message("File successfully saved to:\n%s", vmefile);
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

static void exportVM2raw(const char* vm2_path, const char* vm2_file, int dst)
{
	char vm2file[256];
	char dstfile[256];
	char dst_path[256];

	_set_dest_path(dst_path, dst, VMC_PS2_PATH_USB);
	if (mkdirs(dst_path) != SUCCESS)
	{
		show_message("Error! Export folder is not available:\n%s", dst_path);
		return;
	}

	snprintf(vm2file, sizeof(vm2file), "%s%s", vm2_path, vm2_file);
	snprintf(dstfile, sizeof(dstfile), "%s%s.vmc", dst_path, vm2_file);

	init_loading_screen("Exporting PS2 .VM2 memory card...");
	ps2_remove_vmc_ecc(vm2file, dstfile);
	stop_loading_screen();

	show_message("File successfully saved to:\n%s", dstfile);
}

static void importPS2classicsCfg(const char* cfg_path, const char* cfg_file)
{
	char ps2file[256];
	char outfile[256];

	snprintf(ps2file, sizeof(ps2file), "%s%s", cfg_path, cfg_file);
	snprintf(outfile, sizeof(outfile), PS2ISO_PATH_HDD "%s", cfg_file);
	*strrchr(outfile, '.') = 0;
	strcat(outfile, ".ENC");

	init_loading_screen("Encrypting PS2 CONFIG...");
	ps2_encrypt_image(1, ps2file, outfile, NULL);
	stop_loading_screen();

	show_message("File successfully saved to:\n%s", outfile);
}

static void importPS2classics(const char* iso_path, const char* iso_file)
{
	char ps2file[256];
	char outfile[256];
	char msg[128] = "Encrypting PS2 ISO...";

	snprintf(ps2file, sizeof(ps2file), "%s%s", iso_path, iso_file);
	snprintf(outfile, sizeof(outfile), PS2ISO_PATH_HDD "%s", iso_file);
	*strrchr(outfile, '.') = 0;
	strcat(outfile, ".BIN.ENC");

	init_loading_screen(msg);
	ps2_encrypt_image(0, ps2file, outfile, msg);
	stop_loading_screen();

	show_message("File successfully saved to:\n%s", outfile);
}

static void exportPS2classics(const char* enc_path, const char* enc_file, uint8_t dst)
{
	char path[256];
	char ps2file[256];
	char outfile[256];
	char msg[128] = "Decrypting PS2 BIN.ENC...";

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

	init_loading_screen(msg);
	ps2_decrypt_image(0, ps2file, outfile, msg);
	stop_loading_screen();

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

		if (!apply_cheat_patch_code(tmpfile, entry->title_id, code, APOLLO_LOCAL_CACHE))
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

		case CMD_DECRYPT_PS2_VME:
			decryptVMEfile(selected_entry->path, code->file, codecmd[1]);
			code->activated = 0;
			break;

		case CMD_ENCRYPT_PS2_VMC:
			encryptVM2file(selected_entry->path, code->file, code->options[0].name[code->options[0].sel]);
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

		case CMD_EXP_VM2_RAW:
			exportVM2raw(selected_entry->path, code->file, codecmd[1]);
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

		default:
			break;
	}

}
