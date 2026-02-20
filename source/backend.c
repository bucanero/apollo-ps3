#include "backend.h"
#include "util.h"

extern int verbose_flag;

static int enumerate_callback(void *user, pfd_entry_info_t *entry_info) {
	backend_t *ctx;
	entry_info_t *new_entry;

	if (!user || !entry_info)
		return PFD_STOP;

	ctx = (backend_t *)user;

	new_entry = (entry_info_t *)malloc(sizeof(entry_info_t));
	if (!new_entry)
		return PFD_STOP;

	memset(new_entry, 0, sizeof(entry_info_t));
	new_entry->index = entry_info->index;
	new_entry->file_name = entry_info->file_name ? strdup(entry_info->file_name) : NULL;
	new_entry->file_size = entry_info->file_size;

	list_append(ctx->storage.list.entries, new_entry);

	return PFD_CONTINUE;
}

static int validate_callback(void *user, u32 type, pfd_validation_status_t *status) {
	backend_t *ctx;

	if (!user || !status)
		return PFD_STOP;

	ctx = (backend_t *)user;

	switch (type) {
		case PFD_VALIDATE_TYPE_TOP:
			ctx->storage.validate.structure.top = status->status;
			break;
		case PFD_VALIDATE_TYPE_BOTTOM:
			ctx->storage.validate.structure.bottom = status->status;
			break;
		case PFD_VALIDATE_TYPE_ENTRY:
			/* FIXME: did I forgot something? */
			if (status->entry.is_occupied)
				ctx->storage.validate.entries[status->entry.index].signature = status->status;
			else
				ctx->storage.validate.entries[status->entry.index].signature = status->status;
			break;
		case PFD_VALIDATE_TYPE_FILE:
		case PFD_VALIDATE_TYPE_FILE_CID:
		case PFD_VALIDATE_TYPE_FILE_DHK_CID2:
		case PFD_VALIDATE_TYPE_FILE_AID_UID:
			if (!ctx->storage.validate.files[status->file.entry_index].file_name)
				ctx->storage.validate.files[status->file.entry_index].file_name = status->file.file_name ? strdup(status->file.file_name) : NULL;
			if (type == PFD_VALIDATE_TYPE_FILE)
				ctx->storage.validate.files[status->file.entry_index].file = status->status;
			else if (type == PFD_VALIDATE_TYPE_FILE_CID)
				ctx->storage.validate.files[status->file.entry_index].cid = status->status;
			else if (type == PFD_VALIDATE_TYPE_FILE_DHK_CID2)
				ctx->storage.validate.files[status->file.entry_index].dhk_cid2 = status->status;
			else if (type == PFD_VALIDATE_TYPE_FILE_AID_UID)
				ctx->storage.validate.files[status->file.entry_index].aid_uid = status->status;
			break;
	}

	return PFD_CONTINUE;
}

static u8 * get_secure_file_id_callback(void *user, const char *file_name) {
	backend_t *ctx;
	list_node_t *node;
	u8 *secure_file_id;

	if (!user)
		return NULL;

	ctx = (backend_t *)user;

	if (!ctx->secure_file_ids)
		return NULL;

	secure_file_id = NULL;

	node = list_head(ctx->secure_file_ids);
	while (node) {
		secure_file_id = NULL;
		if (node->value) {
#if 0
			if (strncmp(file_name, ((pfd_secure_file_id_t *)node->value)->file_name, PFD_ENTRY_NAME_SIZE) == 0) {
				secure_file_id = ((pfd_secure_file_id_t *)node->value)->secure_file_id;
				break;
			}
#else /* use wildcard matching instead */
			if (wildcard_match(file_name, ((secure_file_id_t *)node->value)->file_name) != 0) {
				secure_file_id = ((secure_file_id_t *)node->value)->secure_file_id;
				break;
			}
#endif
		}
		node = node->next;
	}

	return secure_file_id;
}

backend_t * backend_initialize(pfd_config_t *config, list_t *secure_file_ids, const char *directory_path) {
	backend_t *ctx;

	ctx = (backend_t *)malloc(sizeof(backend_t));
	if (!ctx)
		return NULL;

	memset(ctx, 0, sizeof(backend_t));

	ctx->config = config;
	ctx->secure_file_ids = secure_file_ids;

	ctx->pfd = pfd_init(
		ctx->config,
		directory_path,
		enumerate_callback,
		validate_callback,
		get_secure_file_id_callback,
		ctx
	);

	return ctx;
}

int backend_shutdown(backend_t *ctx) {
	if (!ctx)
		return -1;

	if (ctx->pfd)
		pfd_destroy(ctx->pfd);

	free(ctx);

	return 0;
}

int backend_cmd_list(backend_t *ctx) {
	pfd_info_t info;
	list_node_t *node;
	entry_info_t *entry_info;

	if (!ctx)
		return -1;

	if (pfd_import(ctx->pfd) < 0)
		return -1;

	if (pfd_get_info(ctx->pfd, &info) < 0)
		return -1;

	memset(&ctx->storage.list, 0, sizeof(ctx->storage.list));

	ctx->storage.list.entries = list_alloc();
	if (!ctx->storage.list.entries)
		return -1;

	if (pfd_enumerate(ctx->pfd) < 0)
		return -1;

	LOG("[*] Version: %llu", info.version);
	LOG("[*] Type: %s", info.is_trophy ? "trophy" : "savegame");
	LOG("[*]");
	LOG("[*] Entries:");
	node = list_head(ctx->storage.list.entries);
	while (node) {
		entry_info = (entry_info_t *)list_get(node);
		if (entry_info) {
			LOG("[*]   #%03lld:", entry_info->index);
			if (entry_info->file_name)
				LOG("[*]     File Name: %s", entry_info->file_name);
			LOG("[*]     File Size: %lld", entry_info->file_size);
		}
		node = list_next(node);
	}
	node = list_head(ctx->storage.list.entries);
	while (node) {
		entry_info = (entry_info_t *)list_get(node);
		if (entry_info) {
			if (entry_info->file_name)
				free(entry_info->file_name);
			free(entry_info);
		}
		node = list_next(node);
	}

	list_free(ctx->storage.list.entries);

	return 0;
}

int backend_cmd_check(backend_t *ctx, u32 options) {
	pfd_info_t info;
	u32 flags;
	u64 i, j;

	if (!ctx)
		return -1;

	if (pfd_import(ctx->pfd) < 0)
		return -1;

	if (pfd_get_info(ctx->pfd, &info) < 0)
		return -1;

	memset(&ctx->storage.validate, 0, sizeof(ctx->storage.validate));
	
	ctx->storage.validate.entries = (entry_status_t *)malloc(info.num_reserved_entries * sizeof(entry_status_t));
	if (!ctx->storage.validate.entries)
		return -1;
	memset(ctx->storage.validate.entries, 0, info.num_reserved_entries * sizeof(entry_status_t));

	ctx->storage.validate.files = (file_status_t *)malloc(info.num_used_entries * sizeof(file_status_t));
	if (!ctx->storage.validate.files)
		return -1;
	memset(ctx->storage.validate.files, 0, info.num_used_entries * sizeof(file_status_t));

	flags = PFD_VALIDATE_TYPE_ALL;
	if ((options & BACKEND_VALIDATE_FLAG_PARTIAL) != 0) {
		flags &= ~PFD_VALIDATE_TYPE_FILE;
		flags &= ~PFD_VALIDATE_TYPE_FILE_DHK_CID2;
	}
	if (pfd_validate(ctx->pfd, flags) < 0)
		return -1;

	LOG("[*] Version: %llu", info.version);
	LOG("[*] Type: %s", info.is_trophy ? "trophy" : "savegame");
	LOG("[*]");
	LOG("[*] Statuses:");
	LOG("[*]   Structure:");
	LOG("[*]     Top Hash:    %s", ctx->storage.validate.structure.top == PFD_VALIDATE_SUCCESS ? "OK" : "FAIL");
	LOG("[*]     Bottom Hash: %s", ctx->storage.validate.structure.bottom == PFD_VALIDATE_SUCCESS ? "OK" : "FAIL");
	LOG("[*]");
	LOG("[*]   Entries:");
	for (i = 0; i < info.num_reserved_entries; ++i) {
		LOG("[*]     #%03lld:", i);
		LOG("[*]       Signature Hash: %s", ctx->storage.validate.entries[i].signature == PFD_VALIDATE_SUCCESS ? "OK" : "FAIL");
	}
	LOG("[*]");
	LOG("[*]   Files:");
	for (i = 0, j = 0; i < info.num_used_entries; ++i) {
		if (!ctx->storage.validate.files[i].file_name)
			continue;
		if (strncmp(ctx->storage.validate.files[i].file_name, "PARAM.SFO", PFD_ENTRY_NAME_SIZE) == 0) {
			LOG("[*]     #%03lld (%s):", j, ctx->storage.validate.files[i].file_name);
			LOG("[*]       Entry Index:             %lld", i);
			LOG("[*]       PARAM.SFO Hash Key Hash: %s", ctx->storage.validate.files[i].file == PFD_VALIDATE_SUCCESS ? "OK" : "FAIL");
			if (!info.is_trophy) {
				LOG("[*]       Console ID Hash:         %s", ctx->storage.validate.files[i].cid == PFD_VALIDATE_SUCCESS ? "OK" : "FAIL");
				LOG("[*]       Disc Hash Key Hash:      %s", ctx->storage.validate.files[i].dhk_cid2 == PFD_VALIDATE_SUCCESS ? "OK" : "FAIL");
				LOG("[*]       Authentication ID Hash:  %s", ctx->storage.validate.files[i].aid_uid == PFD_VALIDATE_SUCCESS ? "OK" : "FAIL");
			} else {
				LOG("[*]       Console ID Hash:         %s", ctx->storage.validate.files[i].dhk_cid2 == PFD_VALIDATE_SUCCESS ? "OK" : "FAIL");
				LOG("[*]       User ID Hash:            %s", ctx->storage.validate.files[i].aid_uid == PFD_VALIDATE_SUCCESS ? "OK" : "FAIL");
			}
		} else {
			LOG("[*]     #%03lld (%s):", j, ctx->storage.validate.files[i].file_name);
			LOG("[*]       Entry Index:             %lld", i);
			LOG("[*]       File Key Hash:           %s", ctx->storage.validate.files[i].file == PFD_VALIDATE_SUCCESS ? "OK" : "FAIL");
		}
		++j;
	}

	free(ctx->storage.validate.entries);
	free(ctx->storage.validate.files);

	return 0;
}

int backend_cmd_update(backend_t *ctx, u32 options) {
	u32 flags;

	if (!ctx)
		return -1;

	if (pfd_import(ctx->pfd) < 0)
		return -1;

	flags = PFD_UPDATE_TYPE_ALL;
	if ((options & BACKEND_UPDATE_FLAG_PARTIAL) != 0) {
		flags &= ~PFD_UPDATE_TYPE_FILE;
		flags &= ~PFD_UPDATE_TYPE_FILE_DHK_CID2;
	}

	if (pfd_update(ctx->pfd, flags) < 0)
		return -1;

	if (pfd_export(ctx->pfd) < 0)
		return -1;

	return 0;
}

int backend_cmd_encrypt(backend_t *ctx, list_t *file_names) {
	list_node_t *node;
	const char *file_name;

	if (!ctx)
		return -1;

	if (!file_names)
		return -1;

	if (pfd_import(ctx->pfd) < 0)
		return -1;

	node = list_head(file_names);
	while (node) {
		file_name = (const char *)list_get(node);
		if (file_name) {
			if (pfd_encrypt_file(ctx->pfd, file_name) < 0)
				return -1;
		}
		node = list_next(node);
	}

	if (pfd_update(ctx->pfd, PFD_UPDATE_TYPE_ALL) < 0)
		return -1;

	if (pfd_export(ctx->pfd) < 0)
		return -1;

	return 0;
}

int backend_cmd_decrypt(backend_t *ctx, list_t *file_names) {
	list_node_t *node;
	const char *file_name;

	if (!ctx)
		return -1;

	if (!file_names)
		return -1;

	if (pfd_import(ctx->pfd) < 0)
		return -1;

	node = list_head(file_names);
	while (node) {
		file_name = (const char *)list_get(node);
		if (file_name) {
			if (pfd_decrypt_file(ctx->pfd, file_name) < 0)
				return -1;
		}
		node = list_next(node);
	}

	if (pfd_update(ctx->pfd, PFD_UPDATE_TYPE_ALL) < 0)
		return -1;

	if (pfd_export(ctx->pfd) < 0)
		return -1;

	return 0;
}
