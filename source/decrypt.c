/*
*
*	Custom PS3 Save Decrypters - (c) 2020 by Bucanero - www.bucanero.com.ar
*
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <polarssl/aes.h>
#include <polarssl/des.h>
#include <polarssl/md5.h>
#include <polarssl/blowfish.h>
#include "keys.h"
#include "types.h"


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

void diablo_decrypt_data(u8* data, u32 size)
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

void diablo_encrypt_data(u8* data, u32 size)
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

void des_ecb_decrypt(u8* data, u32 len, u8* key, u32 key_len)
{
	des_context ctx;

	if (key_len != DES_KEY_SIZE)
		return;

	LOG("Decrypting DES ECB data (%d bytes)", len);

	des_init(&ctx);
	des_setkey_dec(&ctx, key);
	len = len / DES_BLOCK_SIZE;

	while (len--)
	{
		des_crypt_ecb(&ctx, data, data);
		data += DES_BLOCK_SIZE;
	}

	return;
}

void des_ecb_encrypt(u8* data, u32 len, u8* key, u32 key_len)
{
	des_context ctx;

	if (key_len != DES_KEY_SIZE)
		return;

	LOG("Encrypting DES ECB data (%d bytes)", len);

	des_init(&ctx);
	des_setkey_enc(&ctx, key);
	len = len / DES_BLOCK_SIZE;

	while (len--)
	{
		des_crypt_ecb(&ctx, data, data);
		data += DES_BLOCK_SIZE;
	}

	return;
}

void des3_cbc_decrypt(u8* data, u32 len, const u8* key, u32 key_len, u8* iv, u32 iv_len)
{
	des3_context ctx;

	if (key_len != DES_KEY_SIZE*3 || iv_len != DES_KEY_SIZE)
		return;

	LOG("Decrypting 3-DES CBC data (%d bytes)", len);

	des3_init(&ctx);
	des3_set3key_dec(&ctx, key);
	des3_crypt_cbc(&ctx, DES_DECRYPT, len, iv, data, data);

	return;
}

void des3_cbc_encrypt(u8* data, u32 len, const u8* key, u32 key_len, u8* iv, u32 iv_len)
{
	des3_context ctx;

	if (key_len != DES_KEY_SIZE*3 || iv_len != DES_KEY_SIZE)
		return;

	LOG("Encrypting 3-DES CBC data (%d bytes)", len);

	des3_init(&ctx);
	des3_set3key_enc(&ctx, key);
	des3_crypt_cbc(&ctx, DES_ENCRYPT, len, iv, data, data);

	return;
}

void xor_block(const u8* in, u8* out)
{
	for (int i = 0; i < XOR_BLOCK_SIZE; i++)
		out[i] ^= in[i];
}

void nfsu_decrypt_data(u8* data, u32 size)
{
	u8 xor_key[XOR_BLOCK_SIZE];
	u8 tmp[XOR_BLOCK_SIZE];

	LOG("[*] Total Decrypted Size Is 0x%X (%d bytes)", size, size);

	// init xor key
	memcpy(xor_key, NFS_XOR_KEY, XOR_BLOCK_SIZE);
	xor_block(data, xor_key);
	md5(xor_key, XOR_BLOCK_SIZE, xor_key);

	size /= XOR_BLOCK_SIZE;
	
	while (size--)
	{
		data += XOR_BLOCK_SIZE;

		md5(data, XOR_BLOCK_SIZE, tmp);
		xor_block(xor_key, data);
		memcpy(xor_key, tmp, XOR_BLOCK_SIZE);
	}

	return;
}

void nfsu_encrypt_data(u8* data, u32 size)
{
	u8 xor_key[XOR_BLOCK_SIZE];

	LOG("[*] Total Encrypted Size Is 0x%X (%d bytes)", size, size);

	// init xor key
	memcpy(xor_key, NFS_XOR_KEY, XOR_BLOCK_SIZE);
	xor_block(data, xor_key);
	md5(xor_key, XOR_BLOCK_SIZE, xor_key);

	size /= XOR_BLOCK_SIZE;

	while (size--)
	{
		data += XOR_BLOCK_SIZE;

		xor_block(xor_key, data);
		md5(data, XOR_BLOCK_SIZE, xor_key);
	}

	return;
}

void sh3_decrypt_data(u8* data, u32 size)
{
	u32 out;
	u64 input, key2 = SH3_KEY2;

	LOG("[*] Total Decrypted Size Is 0x%X (%d bytes)", size, size);

	size /= 4;

	while (size--)
	{
		input = *(u32*) data;
		out = (u32)((input ^ (u64)(key2 - SH3_KEY1)) & 0xFFFFFFFF);
		memcpy(data, &out, sizeof(u32));

		key2 = (input << 5 | input >> 27) + (u64)SH3_KEY2;
		data += 4;
	}

	return;
}

void sh3_encrypt_data(u8* data, u32 size)
{
	u32 out;
	u64 input, key2 = SH3_KEY2;

	LOG("[*] Total Encrypted Size Is 0x%X (%d bytes)", size, size);

	size /= 4;

	while (size--)
	{
		input = *(u32*) data;
		out = (u32)((input ^ (u64)(key2 - SH3_KEY1)) & 0xFFFFFFFF);
		memcpy(data, &out, sizeof(u32));

		key2 = (u64)(out << 5 | out >> 27) + (u64)SH3_KEY2;
		data += 4;
	}

	return;
}
