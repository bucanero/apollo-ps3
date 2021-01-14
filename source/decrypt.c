/*
*
*	Custom PS3 Save Decrypters - (c) 2020 by Bucanero - www.bucanero.com.ar
*
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <polarssl/aes.h>
#include <polarssl/blowfish.h>
#include "keys.h"
#include "types.h"


void crypt_64bit_up(const u32* keybuf, u32* ptr)
{
	u32 x = ptr[0];
	u32 y = ptr[1];
	u32 z;
	int i;

	for (i = 0; i < 0x10; i++) {
		z = keybuf[i] ^ x;
		x = keybuf[0x012 + ((z>>24)&0xff)];
		x = keybuf[0x112 + ((z>>16)&0xff)] + x;
		x = keybuf[0x212 + ((z>> 8)&0xff)] ^ x;
		x = keybuf[0x312 + ((z>> 0)&0xff)] + x;
		x = y ^ x;
		y = z;
	}

	ptr[1] = x ^ keybuf[0x10];
	ptr[0] = y ^ keybuf[0x11];
}

void crypt_64bit_down(const u32* keybuf, u32* ptr)
{
	u32 x = ptr[0];
	u32 y = ptr[1];
	u32 z;
	int i;

	for (i = 0x11; i > 0x01; i--) {
		z = keybuf[i] ^ x;
		x = keybuf[0x012 + ((z>>24)&0xff)];
		x = keybuf[0x112 + ((z>>16)&0xff)] + x;
		x = keybuf[0x212 + ((z>> 8)&0xff)] ^ x;
		x = keybuf[0x312 + ((z>> 0)&0xff)] + x;
		x = y ^ x;
		y = z;
	}

	ptr[1] = x ^ keybuf[0x01];
	ptr[0] = y ^ keybuf[0x00];
}

void apply_keycode(u32* keybuf, const u8* keydata, const char* keycode)
{
	int i, j;
	u32 scratch[2] = {0, 0};
	char tmp[4];
	int len = strlen(keycode);

	memcpy(keybuf + 0x12, keydata, 0x1000);

	for (i = 0; i < 0x12; i++)
	{
		for (j = 0; j < 4; j++)
			tmp[j]=keycode[(i*4 +j) % len];

		keybuf[i] = *(u32*)(keydata + 0x1000 + i*4) ^ *(u32*)tmp;
	}

	for (i = 0; i < 0x412; i += 2)
	{
		crypt_64bit_up(keybuf, scratch);
		keybuf[i] = scratch[0];
		keybuf[i+1] = scratch[1];
	}
}

void blowfish_ecb_decrypt(u8* data, u32 len, u8* key, u32 key_len)
{
	blowfish_context ctx;

	LOG("Decrypting Blowfish ECB data (%d bytes)", len);

	blowfish_init(&ctx);
	blowfish_setkey(&ctx, key, key_len * 8);
	len = len / BLOWFISH_BLOCKSIZE;

	while (len--)
	{
		blowfish_crypt_ecb(&ctx, BLOWFISH_DECRYPT, data, data);
		data += BLOWFISH_BLOCKSIZE;
	}

	return;
}

void blowfish_ecb_encrypt(u8* data, u32 len, u8* key, u32 key_len)
{
	blowfish_context ctx;

	LOG("Encrypting Blowfish ECB data (%d bytes)", len);

	blowfish_init(&ctx);
	blowfish_setkey(&ctx, key, key_len * 8);
	len = len / BLOWFISH_BLOCKSIZE;

	while (len--)
	{
		blowfish_crypt_ecb(&ctx, BLOWFISH_ENCRYPT, data, data);
		data += BLOWFISH_BLOCKSIZE;
	}

	return;
}

int nd_decrypt_data(u32* data, u32 size)
{
	int i;
	size = data[size/4 -1];

	LOG("NaughtyDog Decrypted size 0x%X (%d bytes)", size, size);
	data += 2;

	blowfish_ecb_decrypt((u8*) data, size, (u8*) SECRET_KEY, strlen(SECRET_KEY));

	return 1;
}

int nd_encrypt_data(u32* data, u32 size)
{
	size = data[size/4 -1];

	LOG("NaughtyDog Encrypted size 0x%X (%d bytes)", size, size);
	data += 2;

	blowfish_ecb_encrypt((u8*) data, size, (u8*) SECRET_KEY, strlen(SECRET_KEY));

	return 1;
}

void d3_decrypt_data(u8* data, u32 size)
{
	u32 xor_key1 = DIABLO3_KEY1;
	u32 xor_key2 = DIABLO3_KEY2;
	u32 tmp;

	LOG("[*] Total Decrypted Size 0x%X (%d bytes)", size, size);

	for (int i = 0; i < size; i++)
	{
		data[i] ^= (xor_key1 & 0xFF);
		tmp = data[i] ^ xor_key1;
		xor_key1 = xor_key1 >> 8 | xor_key2 << 0x18;
		xor_key2 = xor_key2 >> 8 | tmp << 0x18;
	}

	LOG("[*] Decrypted File Successfully!");
	return;
}

void d3_encrypt_data(u8* data, u32 size)
{
	u32 xor_key1 = DIABLO3_KEY1;
	u32 xor_key2 = DIABLO3_KEY2;
	u32 tmp;

	LOG("[*] Total Encrypted Size 0x%X (%d bytes)", size, size);

	for (int i = 0; i < size; i++)
	{
		tmp = data[i] ^ xor_key1;
		data[i] ^= (xor_key1 & 0xFF);
		xor_key1 = xor_key1 >> 8 | xor_key2 << 0x18;
		xor_key2 = xor_key2 >> 8 | tmp << 0x18;
	}

	LOG("[*] Encrypted File Successfully!");
	return;
}

void aes_ecb_decrypt(u8* data, u32 len, u8* key, u32 key_len)
{
	aes_context ctx;

	LOG("Decrypting AES ECB data (%d bytes)", len);

	aes_init(&ctx);
	aes_setkey_dec(&ctx, key, key_len * 8);
	len = len / AES_BLOCK_SIZE;

	while (len--)
	{
		aes_crypt_ecb(&ctx, AES_DECRYPT, data, data);
		data += AES_BLOCK_SIZE;
	}

	return;
}

void aes_ecb_encrypt(u8* data, u32 len, u8* key, u32 key_len)
{
	aes_context ctx;

	LOG("Encrypting AES ECB data (%d bytes)", len);

	aes_init(&ctx);
	aes_setkey_enc(&ctx, key, key_len * 8);
	len = len / AES_BLOCK_SIZE;

	while (len--)
	{
		aes_crypt_ecb(&ctx, AES_ENCRYPT, data, data);
		data += AES_BLOCK_SIZE;
	}

	return;
}
