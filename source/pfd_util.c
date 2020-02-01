#include "saves.h"
#include "backend.h"
#include "config.h"
#include "list.h"
#include "pfd.h"
#include "util.h"

#define LOG dbglogger_log

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


/*
static void show_usage(void) {
	show_version();

	LOG("USAGE: pfdtool [options] command");
	LOG("");
	LOG("COMMANDS       Parameters              Explanation");
	LOG(" -h, --help                            Print this help");
	LOG(" -l, --list    dir                     List entries");
	LOG(" -c, --check   dir                     Verify a database");
	LOG(" -u, --update  dir                     Update an existing database");
	LOG(" -e, --encrypt dir files...            Encrypt specified files and update a database");
	LOG(" -d, --decrypt dir files...            Decrypt specified files and update a database");
	LOG(" -b, --brute   dir elf offset files... Bruteforce a secure file IDs for specified files");
	LOG("");
	LOG("OPTIONS        Parameters              Explanation");
	LOG(" -g, --game    product-code            Use a specified game setting set");
	LOG(" -p, --partial                         Don't update/verify all hashes");
	LOG(" -a, --advance                         The offset to advance each time while bruteforcing");

//	exit(1);
}

static void parse_args(int argc, char *argv[]) {
	int option_index;
	int c;
	pfd_cmd cmd;

	static const char* short_options = "hl:c:u:e:d:b:g:a:p";
	static struct option long_options[] = {
		{ "help", ARG_NONE, ARG_NULL, 'h' },

		{ "list", ARG_REQ, ARG_NULL, 'l' },
		{ "check", ARG_REQ, ARG_NULL, 'c' },
		{ "update", ARG_REQ, ARG_NULL, 'u' },
		{ "encrypt", ARG_REQ, ARG_NULL, 'e' },
		{ "decrypt", ARG_REQ, ARG_NULL, 'd' },
		{ "brute", ARG_REQ, ARG_NULL, 'b' },

		{ "game", ARG_REQ, ARG_NULL, 'g' },
		{ "advance", ARG_NONE, ARG_NULL, 'a' },
		{ "partial", ARG_REQ, ARG_NULL, 'p' },

		{ ARG_NULL, ARG_NULL, ARG_NULL, ARG_NULL }
	};

	while ((c = option_index = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
		switch (c) {
			case 'e':
//				cmd_encrypt = 1;
				database_path = optarg;
				goto get_args;
			case 'd':
//				cmd_decrypt = 1;
				database_path = optarg;
				goto get_args;
			case 'b':
//				cmd_brute = 1;
				database_path = optarg;
				goto get_args;

			case 'g':
				game = optarg;
				break;
			case 'p':
				partial_process = 1;
				break;
			case 'a':
				advance_offset = strtol(optarg, NULL, 10);
				break;

			default:
				abort();
		}
	}

get_args:;
	if (cmd == PFD_CMD_encrypt) {
		if (argc - optind < 1) {
			LOG("[*] Error: Encrypt command needs input files!");
			show_usage();
		} else {
			if (optind < argc) {
				while (optind < argc) {
					list_append(file_names, argv[optind++]);
				}
			} else {
				LOG("[*] Error: Encrypt command needs file names!");
				show_usage();
			}
		}
		return;
	}

	if (cmd == PFD_CMD_decrypt) {
		if (argc - optind < 1) {
			LOG("[*] Error: Decrypt command needs input files!");
			show_usage();
		} else {
			if (optind < argc) {
				while (optind < argc) {
					list_append(file_names, argv[optind++]);
				}
			} else {
				LOG("[*] Error: Decrypt command needs file names!");
				show_usage();
			}
		}
		return;
	}

	if (cmd == PFD_CMD_brute) {
		if (argc - optind < 3) {
			LOG("[*] Error: Brute command needs additional parameters!");
			show_usage();
		} else {
			brute_file_path = argv[optind++];
			if (sscanf(argv[optind++], "%llu", &file_offset) < 1)
				file_offset = 0;
			if (optind < argc) {
				while (optind < argc) {
					list_append(file_names, argv[optind++]);
				}
			} else {
				LOG("[*] Error: Brute command needs file names!");
				show_usage();
			}
		}
		return;
	}
}
*/

void setup_key(u8* key, int len) {
	int i;

    for (i = 0; i < len; i++)
		key[i] ^= xor_key[i % 8];

	return;
}

void pfd_util_end(void) {
	LOG("clean up...");

	if (file_names)
		list_free(file_names);

	LOG("clean up 3");
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
	list_node_t *node = games_keys->head;
	game_keys_t *game;

	while (node) {
		game = list_get(node);
		if (strstr(game->game_ids, game_id) != NULL)
			return game;

		node = node->next;
	}

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

int pfd_util_setup_keys(const u8* console_id, u32 user_id) {
	int result = 0;
	char uid[9];

	sprintf(uid, "%08d", user_id);
	memcpy(config.user_id, uid, PFD_USER_ID_SIZE);
	memcpy(config.console_id, console_id, PFD_CONSOLE_ID_SIZE);

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

int pfd_util_init(const char* game_id, const char* database_path) {
	u8 *disc_hash_key = NULL;
	list_t *secure_file_ids = NULL;
	game_keys_t *game_key = NULL;

//	file_names = list_alloc();

	uint64_t* tmp = (uint64_t*)config.console_id;
	LOG("pfdtool " PFDTOOL_VERSION " (c) 2012 by flatz");
	LOG("user [%.8s] PSID (%016lX %016lX)", config.user_id, tmp[0], tmp[1]);
	LOG("game [%s] db '%s'", game_id, database_path);

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
