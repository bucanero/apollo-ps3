//ps3-psvresigner by @dots_tb - Resigns non-console specific PS3 PSV savefiles. PSV files embed PS1 and PS2 save data. This does not inject!
//With help from the CBPS (https://discord.gg/2nDCbxJ) , especially:
// @AnalogMan151
// @teakhanirons
// Silica
// @notzecoxao
// @nyaaasen 

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <polarssl/aes.h>
#include <polarssl/sha1.h>

#include "types.h"

#define PSV_SEED_OFFSET 0x8
#define PSV_HASH_OFFSET 0x1C
#define PSV_TYPE_OFFSET 0x3C

#define PSV_MAGIC       "\x00\x56\x53\x50"

const uint8_t psv_ps2key[0x10] = {
	0xFA, 0x72, 0xCE, 0xEF, 0x59, 0xB4, 0xD2, 0x98, 0x9F, 0x11, 0x19, 0x13, 0x28, 0x7F, 0x51, 0xC7
}; 

const uint8_t psv_ps1key[0x10] = {
	0xAB, 0x5A, 0xBC, 0x9F, 0xC1, 0xF4, 0x9D, 0xE6, 0xA0, 0x51, 0xDB, 0xAE, 0xFA, 0x51, 0x88, 0x59
};

const uint8_t psv_iv[0x10] = {
	0xB3, 0x0F, 0xFE, 0xED, 0xB7, 0xDC, 0x5E, 0xB7, 0x13, 0x3D, 0xA6, 0x0D, 0x1B, 0x6B, 0x2C, 0xDC
};


void XorWithByte(uint8_t* buf, uint8_t byte, int length)
{
	for (int i = 0; i < length; ++i) {
    	buf[i] ^= byte;
	}
}

void XorWithIv(uint8_t* buf, const uint8_t* Iv)
{
  uint8_t i;
  for (i = 0; i < 16; ++i) // The block in AES is always 128bit no matter the key size
  {
    buf[i] ^= Iv[i];
  }
}
 
void generateHash(uint8_t *input, uint8_t *dest, size_t sz, uint8_t type)
{
	aes_context aes_ctx;
	sha1_context sha1_ctx;
	uint8_t iv[0x10];
	uint8_t salt[0x40];
	uint8_t work_buf[0x14];
	uint8_t *salt_seed = input + PSV_SEED_OFFSET;

	memset(salt , 0, sizeof(salt));
	memset(&aes_ctx, 0, sizeof(aes_context));

	LOG("Type detected: %d", type);
	if(type == 1)
	{	//PS1
		LOG("PS1 Save File");
		//idk why the normal cbc doesn't work.
		aes_setkey_dec(&aes_ctx, psv_ps1key, 128);
		memcpy(work_buf, salt_seed, 0x10);

		aes_crypt_ecb(&aes_ctx, AES_DECRYPT, work_buf, salt);

		memcpy(work_buf, salt_seed, 0x10);
		aes_setkey_enc(&aes_ctx, psv_ps1key, 128);
		aes_crypt_ecb(&aes_ctx, AES_ENCRYPT, work_buf, salt + 0x10);

		XorWithIv(salt, psv_iv);

		memset(work_buf, 0xFF, sizeof(work_buf));
		memcpy(work_buf, salt_seed + 0x10, 0x4);

		XorWithIv(salt + 0x10, work_buf);
	} 
	else if(type == 2)
	{	//PS2
		LOG("PS2 Save File");
		uint8_t laid_paid[16]  = {	
			0x10, 0x70, 0x00, 0x00, 0x02, 0x00, 0x00, 0x01, 0x10, 0x70, 0x00, 0x03, 0xFF, 0x00, 0x00, 0x01 };

		memcpy(salt, salt_seed, 0x14);
		memcpy(iv, psv_iv, sizeof(iv));
		XorWithIv(laid_paid, psv_ps2key);

		aes_setkey_dec(&aes_ctx, laid_paid, 128);
		aes_crypt_cbc(&aes_ctx, AES_DECRYPT, 0x40, iv, salt, salt);
	}
	
	memset(salt + 0x14, 0, sizeof(salt) - 0x14);
	memset(input + PSV_HASH_OFFSET, 0, 0x14);

	XorWithByte(salt, 0x36, 0x40);

	sha1_init(&sha1_ctx);
	sha1_starts(&sha1_ctx);
	sha1_update(&sha1_ctx, salt, 0x40);
	sha1_update(&sha1_ctx, input, sz);
	sha1_finish(&sha1_ctx, work_buf);
	sha1_free(&sha1_ctx);

	XorWithByte(salt, 0x6A, 0x40);

	sha1_init(&sha1_ctx);
	sha1_starts(&sha1_ctx);
	sha1_update(&sha1_ctx, salt, 0x40);
	sha1_update(&sha1_ctx, work_buf, 0x14);
	sha1_finish(&sha1_ctx, dest);
	sha1_free(&sha1_ctx);
}

int psv_resign(const char *src_psv, const char *dst_psv)
{
	LOG("\n=====ps3-psvresigner by @dots_tb=====");
//	LOG("\nWith CBPS help especially: @AnalogMan151, @teakhanirons, Silica, @nyaaasen, and @notzecoxao\n");
//	LOG("Resigns non-console specific PS3 PSV savefiles. PSV files embed PS1 and PS2 save data. This does not inject!\n\n");

	FILE *fin = 0, *fout = 0;
	fin = fopen(src_psv, "rb");
	if (!fin) {
		LOG("Failed to open input file");
		goto error;
	}

	fseek(fin, 0, SEEK_END);
	size_t sz = ftell(fin);
	LOG("File SZ: %lx\n", sz);
	fseek(fin, 0, SEEK_SET);
	
	uint8_t *input = (unsigned char*) calloc (1, sz);
	fread(input, sz,1,fin);
	
	if (memcmp(input, PSV_MAGIC, 4) != 0) {
		LOG("Not a PSV file");
		free(input);
		goto error;
	}
	
	LOG("Old signature: ");
	for(int i = 0; i < 0x14; i++ ) {
		LOG("%02X ", input[PSV_HASH_OFFSET + i]);
	}

	generateHash(input, input + PSV_HASH_OFFSET, sz, input[PSV_TYPE_OFFSET]);

	LOG("New signature: ");
	for(int i = 0; i < 0x14; i++ ) {
		LOG("%02X ", input[PSV_HASH_OFFSET + i]);
	}

	fout = fopen(dst_psv, "wb");
	if (!fout) {
		LOG("Failed to open output file");
		free(input);
		goto error;
	}
	fwrite(input,  1, sz, fout);
	free(input);
	LOG("PSV resigned successfully: %s\n", dst_psv);


error:
	if (fin)
		fclose(fin);
	if (fout)
		fclose(fout);	

	return 0;
}
