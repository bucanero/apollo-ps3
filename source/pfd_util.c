#include <polarssl/aes.h>
#include <apollo.h>

#include "saves.h"
#include "backend.h"
#include "config.h"
#include "pfd.h"
#include "util.h"
#include "pfd_internal.h"

#define PFD_AES_BLOCK_LEN	16

typedef struct {
	char *game_ids;
	u8 *disc_hash_key;
	list_t *secure_file_ids;
} game_keys_t;

static backend_t *backend = NULL;

static char *brute_file_path = NULL;
static list_t *file_names = NULL;
static u64 file_offset = 0;
static s64 advance_offset = 1;

list_t* games_keys = NULL;

const uint8_t xor_key[] = { 0xD4, 0xD1, 0x6B, 0x0C, 0x5D, 0xB0, 0x87, 0x91 };

static pfd_config_t config = {
	.authentication_id = {
		0xC4, 0xC1, 0x6B, 0x0C, 0x5C, 0xB0, 0x87, 0x92, },

	.syscon_manager_key = {
		0x00, 0xC2, 0xD3, 0x9A, 0x3E, 0x51, 0x79, 0x0E, 
		0xA1, 0xC5, 0x56, 0x37, 0xE9, 0xE6, 0xD5, 0xE5, },

	.fallback_disc_hash_key = {
		0x05, 0x10, 0x8A, 0x07, 0xC1, 0xE4, 0xF9, 0xF9, 
		0x4F, 0x51, 0x36, 0xC1, 0xCA, 0xA0, 0x49, 0x1C, },

	.keygen_key = {
		0xBF, 0xCB, 0xA5, 0xAE, 0x1B, 0x07, 0xC2, 0x6C, 
		0x5B, 0x42, 0x1D, 0x37, 0xCF, 0xB5, 0x13, 0x5C, 
		0x87, 0x99, 0x50, 0x8E, },

	.savegame_param_sfo_key = {
		0xD8, 0xD9, 0x6B, 0x02, 0x54, 0xB5, 0x83, 0x95, 
		0xD9, 0xD0, 0x64, 0x0C, 0x59, 0xB6, 0x85, 0x93, 
		0xDD, 0xD7, 0x66, 0x0F, },

	.trophy_param_sfo_key = {
		0x89, 0x8A, 0x0F, 0x75, 0x4A, 0xB2, 0xC9, 0x0A, 
		0x6C, 0x02, 0x5B, 0x44, 0x36, 0x29, 0xE9, 0xE8, 
		0x89, 0xAE, 0x28, 0x9E, },

	.tropsys_dat_key = {
		0x64, 0x51, 0xAF, 0x03, 0xAE, 0xE8, 0xE3, 0xA7, 
		0x5D, 0xF9, 0x7C, 0x3A, 0xFB, 0x0F, 0x92, 0x18, 
		0xF8, 0x2F, 0xCF, 0x3A, },

	.tropusr_dat_key = {
		0x53, 0xC0, 0x84, 0xF8, 0x5B, 0x21, 0xB8, 0x98, 
		0xE3, 0x20, 0x7E, 0xF6, 0xEF, 0x8D, 0x66, 0x38, 
		0x5D, 0xAB, 0x13, 0x96, },

	.troptrns_dat_key = {
		0x45, 0x3F, 0xEA, 0x59, 0x07, 0x7C, 0x9B, 0xDE, 
		0x61, 0x7B, 0x8E, 0x4A, 0x71, 0x4E, 0x9B, 0xF3, 
		0x70, 0x7E, 0x5D, 0xA9, },

	.tropconf_sfm_key = {
		0x36, 0x3C, 0x58, 0xCB, 0x41, 0xF4, 0xC9, 0x7A, 
		0x15, 0x33, 0x56, 0x6F, 0x07, 0x68, 0x6F, 0xBE, 
		0x9A, 0x1B, 0x25, 0x98, },
	};


void setup_key(u8* key, int len) {
	int i;

    for (i = 0; i < len; i++)
		key[i] ^= xor_key[i % 8];

	return;
}

void pfd_util_end(void) {
	LOG("pfdtool clean up...");

	if (file_names)
		list_free(file_names);

/*
	if (secure_file_ids) {
		list_node_t *node;
		node = list_head(secure_file_ids);
		while (node) {
			if (node->value)
				free(node->value);
			node = node->next;
		}
		list_free(secure_file_ids);
	}
*/
	if (backend)
		backend_shutdown(backend);

	LOG("clean up complete");
}

game_keys_t* find_game_keys(const char* game_id) {
	list_node_t *node;
	game_keys_t *game;

	for (node = list_head(games_keys); (game = list_get(node)); node = list_next(node))
		if (strstr(game->game_ids, game_id) != NULL)
			return game;

	return NULL;
}

static int games_config_handler(void *user, const char *section, const char *name, const char *value) {

	game_keys_t* game_key = list_get(list_tail(games_keys));

	if (!game_key || strcmp(game_key->game_ids, section) != 0) {
		game_key = (game_keys_t *)malloc(sizeof(game_keys_t));
		game_key->game_ids = strdup(section);
		game_key->disc_hash_key = NULL;
		game_key->secure_file_ids = list_alloc();
		list_append(games_keys, game_key);
	}

	if (strcmp(name, "disc_hash_key") == 0) {
		if (strlen(value) != PFD_DISC_HASH_KEY_SIZE * 2) {
			LOG("[*] Error: Disc hash key needs to be 16 bytes.");
			return -1;
		}
		game_key->disc_hash_key = x_to_u8_buffer(value);

	} else if (strncmp(name, "secure_file_id:", 15) == 0) {
		secure_file_id_t *tmp;
		const char *file_name = name + 15;
		if (strlen(value) != PFD_SECURE_FILE_ID_SIZE * 2) {
			LOG("[*] Error: Secure file ID needs to be 16 bytes.");
			return -1;
		}
		u8 *secure_file_id = x_to_u8_buffer(value);
		if (file_name && secure_file_id) {
			tmp = (secure_file_id_t *)malloc(sizeof(secure_file_id_t));
			memset(tmp, 0, sizeof(secure_file_id_t));
			strncpy(tmp->file_name, file_name, PFD_ENTRY_NAME_SIZE);
			memcpy(tmp->secure_file_id, secure_file_id, PFD_SECURE_FILE_ID_SIZE);
			list_append(game_key->secure_file_ids, tmp);
		}
		if (secure_file_id)
			free(secure_file_id);
	}

	return 0;
}

int pfd_util_setup_keys() {
	int result = 0;

	setup_key(config.authentication_id, PFD_AUTHENTICATION_ID_SIZE);
	setup_key(config.syscon_manager_key, PFD_SYSCON_MANAGER_KEY_SIZE);
	setup_key(config.fallback_disc_hash_key, PFD_DISC_HASH_KEY_SIZE);
	setup_key(config.keygen_key, PFD_KEYGEN_KEY_SIZE);
	setup_key(config.trophy_param_sfo_key, PFD_PARAM_SFO_KEY_SIZE);
	setup_key(config.savegame_param_sfo_key, PFD_PARAM_SFO_KEY_SIZE);
	setup_key(config.tropsys_dat_key, PFD_TROPSYS_DAT_KEY_SIZE);
	setup_key(config.tropusr_dat_key, PFD_TROPUSR_DAT_KEY_SIZE);
	setup_key(config.troptrns_dat_key, PFD_TROPTRNS_DAT_KEY_SIZE);
	setup_key(config.tropconf_sfm_key, PFD_TROPCONF_SFM_KEY_SIZE);

	games_keys = list_alloc();

	if ((result = parse_config_file(APOLLO_DATA_PATH PFDTOOL_CONFIG_GAMES, &games_config_handler, NULL)) != 0) {
		if (result < 0)
			LOG("[*] Error: Unable to read a games config file.");
		else
			LOG("[*] Error: Could not parse a games config file (error at line: %d).", result);
	}
	LOG("Loaded %d games (" APOLLO_DATA_PATH PFDTOOL_CONFIG_GAMES ")", games_keys->count);

	return result;
}

u8* get_secure_file_id(const char* game_id, const char* filename)
{
	list_node_t *node;
	secure_file_id_t *secure_fid;
	game_keys_t *game_key = find_game_keys(game_id);

	if (!game_key)
		return NULL;

	for (node = list_head(game_key->secure_file_ids); (secure_fid = list_get(node)); node = list_next(node))
		if (secure_fid && (wildcard_match(filename, secure_fid->file_name) != 0))
			return (secure_fid->secure_file_id);

	return NULL;
}

char* get_game_title_ids(const char* game_id)
{
	game_keys_t *game_key = find_game_keys(game_id);

	if (!game_key)
		return NULL;

	return (game_key->game_ids);
}

int pfd_util_init(const u8* idps, u32 user_id, const char* game_id, const char* database_path) {
	u8 *disc_hash_key = NULL;
	list_t *secure_file_ids = NULL;
	game_keys_t *game_key = NULL;
	char uid[9];

	snprintf(uid, sizeof(uid), "%08d", user_id);
	memcpy(config.user_id, uid, PFD_USER_ID_SIZE);
	memcpy(config.console_id, idps, PFD_CONSOLE_ID_SIZE);

	LOG("pfdtool " PFDTOOL_VERSION " (c) 2012 by flatz");
	LOG("user_id [%.8s] console_id (%016lX %016lX)", config.user_id, ((uint64_t*)config.console_id)[0], ((uint64_t*)config.console_id)[1]);
	LOG("game_id [%s] data_path '%s'", game_id, database_path);

	game_key = find_game_keys(game_id);
	if (game_key) {
		disc_hash_key = game_key->disc_hash_key;
		secure_file_ids = game_key->secure_file_ids;
	} else
		LOG("[*] Warning: Game (%s) was not found in the key database.", game_id);

	if (disc_hash_key)
		memcpy(config.disc_hash_key, disc_hash_key, PFD_DISC_HASH_KEY_SIZE);
	else {
		memcpy(config.disc_hash_key, config.fallback_disc_hash_key, PFD_DISC_HASH_KEY_SIZE);
		LOG("[*] Warning: A disc hash key was not found. A fallback disc hash key will be used.");
	}

	backend = backend_initialize(&config, secure_file_ids, database_path);

	return (backend != NULL);
}

int pfd_util_process(pfd_cmd_t cmd, int partial_process) {
	int ret = 0;

	if (!backend) {
		return -1;
	}

	switch (cmd) {
		case PFD_CMD_LIST:
			ret = backend_cmd_list(backend);
			break;

		case PFD_CMD_CHECK:
			ret = backend_cmd_check(backend, (partial_process ? BACKEND_VALIDATE_FLAG_PARTIAL : BACKEND_VALIDATE_FLAG_NONE));
			break;

		case PFD_CMD_UPDATE:
			ret = backend_cmd_update(backend, (partial_process ? BACKEND_UPDATE_FLAG_PARTIAL : BACKEND_UPDATE_FLAG_NONE));
			break;

		case PFD_CMD_ENCRYPT:
			ret = backend_cmd_encrypt(backend, file_names);
			break;

		case PFD_CMD_DECRYPT:
			ret = backend_cmd_decrypt(backend, file_names);
			break;

		case PFD_CMD_BRUTE:
			ret = backend_cmd_brute(backend, brute_file_path, file_offset, advance_offset, file_names);
			break;
	}

	return ret;
}

pfd_entry_t* _find_pfd_entry(const char* data, const char* search)
{
	pfd_entry_t *pfd_entry = (pfd_entry_t*)(data + 0x240);

	for (int i = 0; i < 0x72; i++)
		if (strncmp(pfd_entry[i].file_name, search, PFD_ENTRY_NAME_SIZE) == 0)
			return &pfd_entry[i];

	return NULL;
}

int _get_aes_details_pfd(const char* path, const char* filename, const u8* secure_key, u64* file_size, u64* aligned_file_size, u8* entry_key)
{
	char *pfd_data;
	char file_path[256];
	pfd_entry_t *entry;
	size_t dsize;
	u8 iv_hash_key[PFD_KEY_SIZE];
	aes_context aes;
	int i, j;

	snprintf(file_path, sizeof(file_path), "%s" "PARAM.PFD", path);
	if (read_buffer(file_path, (uint8_t**) &pfd_data, &dsize) != 0)
		return 0;

	entry = _find_pfd_entry(pfd_data, filename);
	
	if (!entry)
	{
		LOG("Error: can't find '%s' in PARAM.PFD", filename);
		free(pfd_data);
		return 0;
	}

	memcpy(entry_key, entry->key, PFD_ENTRY_KEY_SIZE);
	*file_size = entry->file_size;
	*aligned_file_size = align_to_pow2(entry->file_size, PFD_FILE_SIZE_ALIGNMENT);

	LOG("(%s) fsize = %ld / aligned fs = %ld", entry->file_name, entry->file_size, *aligned_file_size);

	free(pfd_data);

	// decode keys
	memset(iv_hash_key, 0, PFD_KEY_SIZE);

	for (i = 0, j = 0; i < PFD_KEY_SIZE; ++i)
	{
		switch (i) {
			case 1: iv_hash_key[i] = 11; break;
			case 2: iv_hash_key[i] = 15; break;
			case 5: iv_hash_key[i] = 14; break;
			case 8: iv_hash_key[i] = 10; break;
			default:
				iv_hash_key[i] = secure_key[j++];
				break;
		}
	}

	// Override iv for TROPTRNS.DAT
	if (memcmp(secure_key, config.troptrns_dat_key, PFD_KEY_SIZE) == 0)
		memcpy(iv_hash_key, secure_key, PFD_KEY_SIZE);

	memset(&aes, 0, sizeof(aes_context));

	aes_setkey_dec(&aes, config.syscon_manager_key, 128);
	aes_crypt_cbc(&aes, AES_DECRYPT, PFD_ENTRY_KEY_SIZE, iv_hash_key, entry_key, entry_key);

	return 1;
}

int _update_details_pfd(const char* path, const char* filename)
{
	char *pfd_data;
	char file_path[256];
	pfd_entry_t *entry;
	size_t dsize;
	u64 file_size;

	snprintf(file_path, sizeof(file_path), "%s" "PARAM.PFD", path);
	if (read_buffer(file_path, (uint8_t**) &pfd_data, &dsize) != 0)
		return 0;

	entry = _find_pfd_entry(pfd_data, filename);
	
	if (!entry)
	{
		LOG("Error: can't find '%s' in PARAM.PFD", filename);
		free(pfd_data);
		return 0;
	}

	snprintf(file_path, sizeof(file_path), "%s%s", path, filename);
	get_file_size(file_path, &file_size);

	LOG("Check (%s) fsize = %ld / PFD fsize = %ld", entry->file_name, file_size, entry->file_size);

	if (file_size != entry->file_size)
	{
		LOG("Updating PARAM.PFD...");

		entry->file_size = file_size;
		snprintf(file_path, sizeof(file_path), "%s" "PARAM.PFD", path);
		write_buffer(file_path, (uint8_t*) pfd_data, dsize);
	}

	free(pfd_data);

	return 1;
}

int decrypt_save_file(const char* path, const char* fname, const char* outpath, u8* secure_file_key)
{
	u8 entry_key[PFD_ENTRY_KEY_SIZE];
	u64 file_size, aligned_file_size;
	u64 i, j;

	u8 counter_key[PFD_KEY_SIZE];
	aes_context aes1, aes2;
	u64 num_blocks;
	u8 *block_data;

	char file_path[256];
	u8 *file_data;

	if (!secure_file_key)
	{
		LOG("Skipping decryption: no Secure file key");
		return 0;
	}

	if (!_get_aes_details_pfd(path, fname, secure_file_key, &file_size, &aligned_file_size, entry_key))
	{
		// The requested file is not listed in PARAM.PFD, so we assume is not encrypted
		LOG("Error getting AES keys");
		return 1;
	}

	// read & decrypt file
	snprintf(file_path, sizeof(file_path), "%s%s", path, fname);

	file_data = (u8 *)malloc(aligned_file_size);
	if (!file_data)
		return 0;

	memset(file_data, 0, aligned_file_size);

	if (read_file(file_path, file_data, aligned_file_size) < 0) {
		free(file_data);
		return 0;
	}

	num_blocks = aligned_file_size / PFD_AES_BLOCK_LEN;

	aes_setkey_enc(&aes1, entry_key, 128);
	aes_setkey_dec(&aes2, entry_key, 128);

	for (i = 0; i < num_blocks; ++i)
	{
		block_data = file_data + i * PFD_AES_BLOCK_LEN;

		*(u64 *)(counter_key + 0) = i;
		*(u64 *)(counter_key + 8) = 0;

		aes_crypt_ecb(&aes1, AES_ENCRYPT, counter_key, counter_key);
		aes_crypt_ecb(&aes2, AES_DECRYPT, block_data, block_data);

		for (j = 0; j < PFD_KEY_SIZE; ++j)
			block_data[j] ^= counter_key[j];
	}

	if (outpath)
		snprintf(file_path, sizeof(file_path), "%s%s", outpath, fname);

	// save decrypted data
	if (write_file(file_path, file_data, file_size) < 0) {
		free(file_data);
		return 0;
	}

	free(file_data);

	return 1;
}

int encrypt_save_file(const char* path, const char* fname, u8* secure_file_key)
{
	u8 entry_key[PFD_ENTRY_KEY_SIZE];
	u64 file_size, aligned_file_size;
	u64 i, j;

	u8 counter_key[PFD_KEY_SIZE];
	aes_context aes1, aes2;
	u64 num_blocks;
	u8 *block_data;

	char file_path[256];
	u8 *file_data;

	if (!secure_file_key)
	{
		LOG("Skipping encryption: no Secure file key");
		return 0;
	}

	if (!_update_details_pfd(path, fname))
	{
		// The requested file is not listed in PARAM.PFD, so we assume is not encrypted
		LOG("Error updating PARAM.PFD details");
		return 1;
	}

	if (!_get_aes_details_pfd(path, fname, secure_file_key, &file_size, &aligned_file_size, entry_key))
	{
		LOG("Error getting AES keys");
		return 0;
	}

	// read & encrypt file
	snprintf(file_path, sizeof(file_path), "%s%s", path, fname);

	file_data = (u8 *)malloc(aligned_file_size);
	if (!file_data)
		return 0;

	memset(file_data, 0, aligned_file_size);

	if (read_file(file_path, file_data, file_size) < 0) {
		free(file_data);
		return 0;
	}

	num_blocks = aligned_file_size / PFD_AES_BLOCK_LEN;

	aes_setkey_enc(&aes1, entry_key, 128);
	aes_setkey_enc(&aes2, entry_key, 128);

	for (i = 0; i < num_blocks; ++i)
	{
		block_data = file_data + i * PFD_AES_BLOCK_LEN;

		*(u64 *)(counter_key + 0) = i;
		*(u64 *)(counter_key + 8) = 0;

		aes_crypt_ecb(&aes1, AES_ENCRYPT, counter_key, counter_key);

		for (j = 0; j < PFD_KEY_SIZE; ++j)
			block_data[j] ^= counter_key[j];

		aes_crypt_ecb(&aes2, AES_ENCRYPT, block_data, block_data);
	}

	// save encrypted data
	if (write_file(file_path, file_data, aligned_file_size) < 0) {
		free(file_data);
		return 0;
	}

	free(file_data);

	return 1;
}

int decrypt_trophy_trns(const char* path)
{
	return decrypt_save_file(path, "TROPTRNS.DAT", NULL, config.troptrns_dat_key);
}

int encrypt_trophy_trns(const char* path)
{
	return encrypt_save_file(path, "TROPTRNS.DAT", config.troptrns_dat_key);
}
