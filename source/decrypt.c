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

#define shift_bits(base, bits, value)			((u64)(value) << bits | (u64)(value) >> (base - bits))


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

void ff13_init_key_value(u8* byte_0, u64 ulong_1, u64* ulong_2, u64* ulong_4, u64 ulong_5)
{
	u64 ulong_3 = ((((ulong_1 + 0xD4) ^ ulong_5) & 0xFF) ^ 0x45);
	*byte_0 = (u8)ulong_3;
	*ulong_2 = byte_0[1] + ulong_3;
	*ulong_4 = (shift_bits(32, 2, ulong_3) & 0x3FC);
}

void ff13_init_key(u8* key_table, u32 ff_game, const u64* kdata)
{
	u64 init[4] = {0};
	u64 tmp;
	u64 ff_key = FFXIII_KEY;

	if (ff_game > 1)
	{
		ff_key = (ff_game == 2) ? FFXIII_2_KEY : FFXIII_3_KEY;
		ff_key ^= ES64(ES64(kdata[0]) ^ ES64(kdata[1])) | 1L;
	}

	init[2] = ES32((ff_key >> 32) & 0xFFFFFFFF);
	init[3] = ES32(ff_key & 0xFFFFFFFF);
	init[1] = ((init[2] & 0xFFFFFFFF) | (shift_bits(64, 32, init[3]) & FFXIII_MASK_2));
	init[2] = (u64)((shift_bits(64, 24, (init[1] << 8 & 0xFFFFFFFF)) & FFXIII_MASK_2) | (init[1] & 0xFF000000));
	init[3] = (u64)(shift_bits(64, 16, init[1]) & 0xFFFF);
	init[0] = (u64)((shift_bits(64, 8, init[2]) & 0xFFFFFFFFFFFFFF00) | init[3] | ((shift_bits(64, 32, init[1]) & 0xFFFFFFFF) << 16 & 0xFFFFFFFF));
	init[2] = (u64)((ES64(init[0] >> 32) >> 32 & 0xFFFFFFFF) | (shift_bits(64, 32, ES64(init[0] & 0xFFFFFFFF) >> 32) & FFXIII_MASK_2));

	memcpy(key_table, &init[2], sizeof(uint64_t));

	key_table[0] += 69;
	ff13_init_key_value(&key_table[1], key_table[0] + key_table[1], &init[3], &init[0], shift_bits(64, 2, (u64)key_table[0]));
	ff13_init_key_value(&key_table[2], init[3], &init[3], &init[0], init[0]);
	ff13_init_key_value(&key_table[3], init[3], &init[2], &tmp, init[0]);
	ff13_init_key_value(&key_table[4], init[2], &tmp, &init[1], tmp);
	ff13_init_key_value(&key_table[5], tmp, &tmp, &init[3], init[1]);
	ff13_init_key_value(&key_table[6], tmp, &init[2], &tmp, init[3]);
	key_table[7] = (u8)((((init[2] + 0xD4) ^ tmp) & 0xFF) ^ 0x45);

	for (int j = 0; j < 31; j++)
	{
		init[2] = ES64(*(u64*)(key_table + j*8));
		init[2] = init[2] + (u64)(shift_bits(64, 2, init[2]) & 0xFFFFFFFFFFFFFFFC);
		tmp = (u64)((ES64(init[2] >> 32) >> 32 & 0xFFFFFFFF) | (shift_bits(64, 32, ES64(init[2] & 0xFFFFFFFF) >> 32) & FFXIII_MASK_2));

		memcpy(key_table + (j+1)*8, &tmp, sizeof(uint64_t));
	}
}

u32 ff13_checksum(u8* bytes, u32 len)
{
	u32 ff_csum = 0;
	len /= 8;

	while (len--)
	{
		ff_csum += (u32)bytes[4] + bytes[0];
		bytes += 8;
	}

	return (ff_csum);
}

void ff13_decrypt_data(u32 type, u8* data, u32 size, const u8* key, u32 key_len)
{
	u8 key_table[272];
	u8 sub_table[256];
	u64 block[3], tmp;
	u32 ff_pos = 0;
	u32 *csum, ff_csum;

	if (type != 1 && key_len != 16)
		return;

	LOG("[*] Total Decrypted Size Is 0x%X (%d bytes)", size, size);

	memset(key_table, 0, sizeof(key_table));
	ff13_init_key(key_table, type, (u64*) key);

	// init sub table
	for (int i = 0; i < 256; i++)
		sub_table[i] = (0x78 + i) & 0xFF;

	size /= 8;
	for (int i = 0; i < size; i++)
	{
		ff_pos = i << 3;
		block[0] = (u64)((shift_bits(32, 29, ff_pos) & 0xFF) ^ 0x45);

		for (int j = 0; j < 8; j++)
		{
			tmp = (block[0] & 0xFF);
			block[0] = data[ff_pos + j];
			block[2] = sub_table[(block[0] ^ tmp) & 0xFF] - key_table[(0xF8 & ff_pos)];
			block[1] = sub_table[block[2] & 0xFF] - key_table[(0xF8 & ff_pos) + 1];
			tmp = sub_table[block[1] & 0xFF] - key_table[(0xF8 & ff_pos) + 2];
			block[1] = sub_table[tmp & 0xFF] - key_table[(0xF8 & ff_pos) + 3];
			tmp = sub_table[block[1] & 0xFF] - key_table[(0xF8 & ff_pos) + 4];
			block[1] = sub_table[tmp & 0xFF] - key_table[(0xF8 & ff_pos) + 5];
			tmp = sub_table[block[1] & 0xFF] - key_table[(0xF8 & ff_pos) + 6];
			data[ff_pos + j] = sub_table[tmp & 0xFF] - key_table[(0xF8 & ff_pos) + 7];
		}

		block[0] = (u64)((shift_bits(64, 10, shift_bits(64, 10, ff_pos) | ff_pos) & FFXIII_MASK_1) | ff_pos);
		tmp = (u64)(((shift_bits(64, 10, block[0]) & FFXIII_MASK_1) | ff_pos) + FFXIII_CONST);

		block[2] = *(u64*)(&key_table[0xF8 & ff_pos]);
		block[0] = ES64(ES64(*(u64*)(data + ff_pos)) - ES64(block[2]));
		block[1] = ES32(tmp & 0xFFFFFFFF);
		tmp = ES32((tmp >> 32) & 0xFFFFFFFF);

		block[1] = (u64)(block[2] ^ (block[0] ^ ((tmp & 0xFFFFFFFF) | (shift_bits(64, 32, block[1]) & FFXIII_MASK_2))));
		
		tmp = ((block[1] >> 32 & 0xFFFFFFFF) | (shift_bits(64, 32, block[1]) & FFXIII_MASK_2));
		memcpy(data + ff_pos, &tmp, sizeof(uint64_t));
	}

	ff_csum = ES32(ff13_checksum(data, ff_pos));
	csum = (u32*)(data + ff_pos + 4);

	if (*csum == ff_csum)
		LOG("[*] Decrypted File Successfully!");
	else
		LOG("[!] Decrypted data did not pass file integrity check. (Expected: %08X Got: %08X)", *csum, ff_csum);

	return;
}

void ff13_encrypt_data(u32 type, u8* data, u32 size, const u8* key, u32 key_len)
{
	u8 key_table[272];
	u8 add_table[256];
	u64 block[3], tmp;
	u32 ff_pos = 0;

	if (type != 1 && key_len != 16)
		return;

	LOG("[*] Total Encrypted Size Is 0x%X (%d bytes)", size, size);

	memset(key_table, 0, sizeof(key_table));
	ff13_init_key(key_table, type, (u64*) key);

	// init add table
	for (int i = 0; i < 256; i++)
		add_table[i] = (0x88 + i) & 0xFF;

	size /= 8;
	for (int i = 0; i < size; i++)
	{
		ff_pos = (i << 3);

		tmp = *(u64*)(data + ff_pos);
		block[1] = (u64)((shift_bits(64, 10, shift_bits(64, 10, ff_pos) | ff_pos) & FFXIII_MASK_1) | ff_pos);
		block[2] = (u64)(FFXIII_CONST + ((shift_bits(64, 10, block[1]) & FFXIII_MASK_1) | ff_pos));
		block[0] = ES32((block[2] >> 32) & 0xFFFFFFFF);
		block[1] = ES32(block[2] & 0xFFFFFFFF);
		block[2] = *(u64*)(&key_table[0xF8 & ff_pos]);

		tmp = (((block[0] & 0xFFFFFFFF) | (shift_bits(64, 32, block[1]) & FFXIII_MASK_2)) ^ block[2] ^ ((shift_bits(64, 32, tmp) & 0xFFFFFFFF) | (shift_bits(64, 32, tmp) & FFXIII_MASK_2)));

		tmp = ES64(ES64(tmp) + ES64(block[2]));
		memcpy(data + ff_pos, &tmp, sizeof(uint64_t));

		block[0] = (u64)((shift_bits(32, 29, ff_pos) & 0xFF) ^ 0x45);

		for (int j = 0; j < 8; j++)
		{
			tmp = (block[0] & 0xFF);
			block[0] = (u64)((u64)key_table[(0xF8 & ff_pos)] + (u64)data[ff_pos + j]);
			for (int k = 1; k < 8; k++)
				block[0] = add_table[(block[0] & 0xFF)] + key_table[(0xF8 & ff_pos) + k];

			block[0] = (add_table[(block[0] & 0xFF)] ^ tmp);
			data[ff_pos + j] = (u8)block[0];
		}
	}

	LOG("[*] Encrypted File Successfully!");
	return;
}
