// (c) flatz
// http://web.archive.org/web/20141118220924/http://pastie.org/private/9hjpnaewxg5twytosnx4w
// http://web.archive.org/web/20141118183317/http://pastie.org/private/pmnmsnqg6zbfnk9xactbw
// http://web.archive.org/web/20141117072342/http://pastie.org/private/yltlfwubsz8w5pyhmojyfg
//
// Dasanko (C#)
// https://playstationhax.xyz/forums/topic/1687-c-rap2rif-rap2rifkey-rif2rap-rifkey2rap/
//
// PS3Xploit-Resign (PS3XploitTeam)
// https://github.com/PS3Xploit/PS3xploit-resigner

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <polarssl/aes.h>
#include <polarssl/sha1.h>

#include "util.h"
#include "ecdsa.h"

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

u8 ec_k_nm[21] = {
	0x00, 0xbf, 0x21, 0x22, 0x4b, 0x04, 0x1f, 0x29, 0x54, 0x9d, 
	0xb2, 0x5e, 0x9a, 0xad, 0xe1, 0x9e, 0x72, 0x0a, 0x1f, 0xe0, 0xf1
};

u8 ec_Q_nm[40] = {
	0x94, 0x8D, 0xA1, 0x3E, 0x8C, 0xAF, 0xD5, 0xBA, 0x0E, 0x90,
	0xCE, 0x43, 0x44, 0x61, 0xBB, 0x32, 0x7F, 0xE7, 0xE0, 0x80,
	0x47, 0x5E, 0xAA, 0x0A, 0xD3, 0xAD, 0x4F, 0x5B, 0x62, 0x47,
	0xA7, 0xFD, 0xA8, 0x6D, 0xF6, 0x97, 0x90, 0x19, 0x67, 0x73
};


struct rif
{
    u32 version;
    u32 licenseType;
    u64 accountid;
    char titleid[0x30]; //Content ID
    u8 padding[0xC]; //Padding for randomness
    u32 actDatIndex; //Key index on act.dat between 0x00 and 0x7F
    u8 key[0x10]; //encrypted klicensee
    u64 timestamp; //timestamp??
    u64 expiration; //Always 0
    u8 r[0x14];
    u8 s[0x14];
} __attribute__ ((packed));

struct actdat
{
    u32 version;
    u32 licenseType;
    u64 accountId;
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

int rap2rif(const u8* idps_key, const char* exdata_path, const char* rap_file, const char *rif_path)
{
	struct actdat *actdat = NULL;
	struct rif rif;

	uint8_t rap_key[0x10];
	uint8_t idps_const[0x10];
	uint8_t act_dat_key[0x10];
	uint8_t sha1_digest[20];
	uint8_t R[0x15];
	uint8_t S[0x15];
	char path[256];

	const char *p1;
	const char *p2;

	actdat = actdat_get(rif_path);
	if (actdat == NULL) {
		LOG("Error: unable to load act.dat");
		goto fail;
	}

	snprintf(path, sizeof(path), "%s%s", exdata_path, rap_file);

	LOG("Loading RAP '%s'...", path);
	if (read_file(path, rap_key, sizeof(rap_key)) < 0) {
		LOG("Error: unable to load rap file");
		goto fail;
	}

	memset(&rif, 0, sizeof(struct rif));
	rif.version = 1;
	rif.licenseType = 0x00010002;
	rif.timestamp = 0x0000012F415C0000;
	rif.expiration = 0;
	rif.accountid = actdat->accountId;

	p1 = strrchr(rap_file, '/');
	if (p1 == NULL)
		p1 = rap_file;
	else
		++p1;
	p2 = strrchr(rap_file, '.');
	if (p1 == NULL || p2 == NULL || *(p1 + 1) == '\0' || p2 < p1) {
		LOG("Error: unable to get content ID");
		goto fail;
	}
	strncpy(rif.titleid, p1, p2 - p1);

	//convert rap to rifkey(klicensee)
	rap_to_klicensee(rap_key, rif.key);
	aesecb128_encrypt(idps_key, npdrm_const_key, idps_const);
	aesecb128_decrypt(idps_const, actdat->keyTable, act_dat_key);

	//encrypt rif with act.dat first key primary key table
	aesecb128_encrypt(act_dat_key, rif.key, rif.key);
	aesecb128_encrypt(npdrm_rif_key, rif.padding, rif.padding);

	sha1((uint8_t*) &rif, 0x70, sha1_digest);
	ecdsa_set_curve(0);
	ecdsa_set_pub(ec_Q_nm);
	ecdsa_set_priv(ec_k_nm);
	ecdsa_sign(sha1_digest, R, S);

	memcpy(rif.r, R+1, sizeof(rif.r));
	memcpy(rif.s, S+1, sizeof(rif.s));

    snprintf(path, sizeof(path), "%s%s", rif_path, p1);
    strcpy(strrchr(path, '.'), ".rif");

	LOG("Saving rif to '%s'...", path);
	if (write_file(path, (uint8_t*) &rif, sizeof(struct rif)) < 0) {
		LOG("Error: unable to create rif file");
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

int rif2klicensee(const u8* idps_key, const char* exdata_path, const char* rif_file, u8* rifKey)
{
	struct rif rif;
	struct actdat *actdat = NULL;

    uint8_t encryptedConst[0x10];
    uint8_t decryptedConst[0x10];
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

    free(actdat);
	return 1;

fail:
	if (actdat != NULL) {
		free(actdat);
	} 

	return 0;
}

int rif2rap(const u8* idps_key, const char* exdata_path, const char* rif_file, const char* rap_path)
{
    uint8_t rifKey[0x10];
    uint8_t rap[0x10];
    char path[256];

    if (!rif2klicensee(idps_key, exdata_path, rif_file, rifKey))
        return 0;

    klicensee_to_rap(rifKey, rap);

    snprintf(path, sizeof(path), "%s%s", rap_path, rif_file);
    strcpy(strrchr(path, '.'), ".rap");

	LOG("Saving RAP to '%s'...", path);
	if (write_file(path, rap, sizeof(rap)) < 0) {
		LOG("Error: unable to create rap file");
		return 0;
	}

	return 1;
}
