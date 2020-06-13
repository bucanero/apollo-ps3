// (c) flatz
//
// Dasanko (C#)
// https://playstationhax.xyz/forums/topic/1687-c-rap2rif-rap2rifkey-rif2rap-rifkey2rap/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <polarssl/aes.h>

#include "util.h"

const u8 rap_initial_key[16] = {
    0x86, 0x9F, 0x77, 0x45, 0xC1, 0x3F, 0xD8, 0x90, 
    0xCC, 0xF2, 0x91, 0x88, 0xE3, 0xCC, 0x3E, 0xDF
};
const u8 pbox[16] = {
    0x0C, 0x03, 0x06, 0x04, 0x01, 0x0B, 0x0F, 0x08, 
    0x02, 0x07, 0x00, 0x05, 0x0A, 0x0E, 0x0D, 0x09
};
const u8 e1[16] = {
    0xA9, 0x3E, 0x1F, 0xD6, 0x7C, 0x55, 0xA3, 0x29, 
    0xB7, 0x5F, 0xDD, 0xA6, 0x2A, 0x95, 0xC7, 0xA5
};
const u8 e2[16] = {
    0x67, 0xD4, 0x5D, 0xA3, 0x29, 0x6D, 0x00, 0x6A, 
    0x4E, 0x7C, 0x53, 0x7B, 0xF5, 0x53, 0x8C, 0x74
};

const u8 rif_header[16] = {
	0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x02, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02
};

const u8 rif_footer[16] = {
	0x00, 0x00, 0x01, 0x2F, 0x41, 0x5C, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const u8 rif_junk = 0x11;

// npdrm_const /klicenseeConst
const u8 npdrm_const_key[16] = {
    0x5E, 0x06, 0xE0, 0x4F, 0xD9, 0x4A, 0x71, 0xBF, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 
};

// NP_rif_key /actdatIndexDecKey
const u8 npdrm_rif_key[16] = {
    0xDA, 0x7D, 0x4B, 0x5E, 0x49, 0x9A, 0x4F, 0x53, 
    0xB1, 0xC1, 0xA1, 0x4A, 0x74, 0x84, 0x44, 0x3B 
};

struct rif
{
    u8 unk1[0x10]; //version, license type and user number
    u8 titleid[0x30]; //Content ID
    u8 padding[0xC]; //Padding for randomness
    u32 actDatIndex; //Key index on act.dat between 0x00 and 0x7F
    u8 key[0x10]; //encrypted klicensee
    u64 unk2; //timestamp??
    u64 unk3; //Always 0
    u8 rs[0x28];
} __attribute__ ((packed));

struct actdat
{
    u8 unk1[0x10]; //Version, User number
    u8 keyTable[0x800]; //Key Table
    u8 unk2[0x800];
    u8 signature[0x28];
} __attribute__ ((packed));


void aesecb128_encrypt(const u8 *key, const u8 *in, u8 *out)
{
	aes_context ctx;
	aes_setkey_enc(&ctx, key, 128);
	aes_crypt_ecb(&ctx, AES_ENCRYPT, in, out);
}

void aesecb128_decrypt(const u8 *key, const u8 *in, u8 *out)
{
	aes_context ctx;
	aes_setkey_dec(&ctx, key, 128);
	aes_crypt_ecb(&ctx, AES_DECRYPT, in, out);
}

int klicensee_to_rap(u8 *klicensee, u8 *rap_key)
{
	int round_num;
	int i;

	u8 key[16];
	memset(key, 0, sizeof(key));
	memcpy(key, klicensee, sizeof(key));

	for (round_num = 0; round_num < 5; ++round_num) {
		int o = 0;
		for (i = 0; i < 16; ++i) {
			int p = pbox[i];
			u8 ec2 = e2[p];
			u8 kc = key[p] + ec2;
			key[p] = kc + (u8)o;
			if (o != 1 || kc != 0xFF) {
				o = kc < ec2 ? 1 : 0;
			}
		}
		for (i = 1; i < 16; ++i) {
			int p = pbox[i];
			int pp = pbox[i - 1];
			key[p] ^= key[pp];
		}
		for (i = 0; i < 16; ++i) {
			key[i] ^= e1[i];
		}
	}
	aesecb128_encrypt(rap_initial_key, key, rap_key);
	return 0;
}

int rap_to_klicensee(u8 *rap_key, u8 *klicensee)
{
	int round_num;
	int i;

	u8 key[16];
	aesecb128_decrypt(rap_initial_key, rap_key, key);

	for (round_num = 0; round_num < 5; ++round_num) {
		for (i = 0; i < 16; ++i) {
			int p = pbox[i];
			key[p] ^= e1[p];
		}
		for (i = 15; i >= 1; --i) {
			int p = pbox[i];
			int pp = pbox[i - 1];
			key[p] ^= key[pp];
		}
		int o = 0;
		for (i = 0; i < 16; ++i) {
			int p = pbox[i];
			u8 kc = key[p] - o;
			u8 ec2 = e2[p];
			if (o != 1 || kc != 0xFF) {
				o = kc < ec2 ? 1 : 0;
				key[p] = kc - ec2;
			} else if (kc == 0xFF) {
				key[p] = kc - ec2;
			} else {
				key[p] = kc;
			}
		}
	}

	memcpy(klicensee, key, sizeof(key));
	return 0;
}

struct actdat *actdat_get(const char* base) {
	char path[256];
    struct actdat *actdat;

    actdat = malloc(sizeof(struct actdat));
	if (actdat == NULL)
		goto fail;

    snprintf(path, sizeof(path), "%s" "act.dat", base);

    LOG("Loading '%s'...", path);
    if (read_file(path, (u8*) actdat, sizeof(struct actdat)) < 0)
        goto fail;

    return actdat;

fail:
	if (actdat != NULL) {
		free(actdat);
	}

	return NULL; 
}

int rifkey2rap(const char *rifkey_file, const char *rap_file)
{
	FILE *fp = NULL;

	u8 rap_key[16];
	u8 klicensee[16];
	memset(klicensee, 0, sizeof(klicensee));
	memset(rap_key, 0, sizeof(rap_key));

	if (read_file(rifkey_file, klicensee, sizeof(klicensee)) < 0) {
		LOG("Error: unable to load rif key file.\n");
		goto fail;
	}

	klicensee_to_rap(klicensee, rap_key);

	fp = fopen(rap_file, "wb");
	if (fp == NULL) {
		LOG("Error: unable to create rap file.\n");
		goto fail;
	}
	fwrite(rap_key, sizeof(rap_key), 1, fp);
	fclose(fp);

	return 1;

fail:
	if (fp != NULL) {
		fclose(fp);
	}

	return 0;
}

int rap2rif(const u8* idps_key, const char* rap_file, const char *rif_file)
{
	struct actdat *actdat = NULL;
	FILE *fp = NULL;

	u8 rap_key[16];
	u8 klicensee[16];
	u8 content_id[48];
	u8 padding[16];
	u8 rif_key[16];
	u8 enc_const[16];
	u8 dec_actdat[16];
	u8 signature[40];
	u32 actdat_key_index;

	const char *p1;
	const char *p2;

    actdat = actdat_get("/dev_hdd0/tmp/");
	if (actdat == NULL) {
		LOG("Error: unable to load act.dat");
		goto fail;
	}

	LOG("Loading RAP '%s'...", rap_file);
	if (read_file(rap_file, rap_key, sizeof(rap_key)) < 0) {
		LOG("Error: unable to load rap file");
		goto fail;
	}

	memset(content_id, 0, sizeof(content_id));
	p1 = strrchr(rap_file, '/');
	if (p1 == NULL)
		p1 = strrchr(rap_file, '\\');
	else
		++p1;
	if (p1 == NULL)
		p1 = rap_file;
	else
		++p1;
	p2 = strrchr(rap_file, '.');
	if (p1 == NULL || p2 == NULL || *(p1 + 1) == '\0' || p2 < p1) {
		LOG("Error: unable to get content ID");
		goto fail;
	}
//	strncpy(content_id, p1, p2 - p1);
	memcpy(content_id, p1, p2 - p1);

	memset(klicensee, 0, sizeof(klicensee));
	rap_to_klicensee(rap_key, klicensee);

	memset(padding, 0, sizeof(padding));
	memset(rif_key, 0, sizeof(rif_key));

	actdat_key_index = 0;
//	memcpy(padding + sizeof(padding) - sizeof(actdat_key_index), &actdat_key_index, 4);
//	wbe32(padding + sizeof(padding) - sizeof(actdat_key_index), actdat_key_index);

	aesecb128_encrypt(idps_key, npdrm_const_key, enc_const);
	aesecb128_decrypt(enc_const, &actdat->keyTable[actdat_key_index * 16], dec_actdat);
	aesecb128_encrypt(npdrm_rif_key, padding, padding);
	aesecb128_encrypt(dec_actdat, klicensee, rif_key);

	memset(signature, rif_junk, sizeof(signature));

	fp = fopen(rif_file, "wb");
	if (fp == NULL) {
		LOG("Error: unable to create rif file");
		goto fail;
	}
	fwrite(rif_header, sizeof(rif_header), 1, fp);
	fwrite(content_id, sizeof(content_id), 1, fp);
	fwrite(padding, sizeof(padding), 1, fp);
	fwrite(rif_key, sizeof(rif_key), 1, fp);
	fwrite(rif_footer, sizeof(rif_footer), 1, fp);
	fwrite(signature, sizeof(signature), 1, fp);
	fclose(fp);

	return 1;

fail:
	if (fp != NULL) {
		fclose(fp);
	}

	if (actdat != NULL) {
		free(actdat);
	}

	return 0;
}

int rif2rap(const u8* idps_key, const char* exdata_path, const char* rif_file, const char* rap_path)
{
	struct rif rif;
	struct actdat *actdat = NULL;

    uint8_t rifKey[0x10];
    uint8_t encryptedConst[0x10];
    uint8_t decryptedConst[0x10];
    uint8_t rap[0x10];
	char path[256];

    snprintf(path, sizeof(path), "%s%s", exdata_path, rif_file);

    LOG("Loading RIF '%s'...", path);
    if (read_file(path, (u8*) &rif, sizeof(struct rif)) < 0) {
        LOG("Error: unable to load rif file '%s'", path);
        goto fail;
    }

    aesecb128_decrypt(npdrm_rif_key, rif.padding, rif.padding);

    actdat = actdat_get(exdata_path);
	if (actdat == NULL) {
		LOG("Error: unable to load act.dat");
		goto fail;
	}

    aesecb128_encrypt(idps_key, npdrm_const_key, encryptedConst);
    aesecb128_decrypt(encryptedConst, &actdat->keyTable[rif.actDatIndex * 0x10], decryptedConst);
    aesecb128_decrypt(decryptedConst, rif.key, rifKey);

    klicensee_to_rap(rifKey, rap);

    snprintf(path, sizeof(path), "%s%s", rap_path, rif_file);
    strcpy(strrchr(path, '.'), ".rap");

	LOG("Saving RAP to '%s'...", path);
	if (write_file(path, rap, sizeof(rap)) < 0) {
		LOG("Error: unable to create rap file");
		goto fail;
	}

    free(actdat);
	return 1;

fail:
	if (actdat != NULL) {
		free(actdat);
	} 

	return 0;
}
