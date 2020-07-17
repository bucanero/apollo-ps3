/*
*
*	Naughty Dog PS3 Save Decrypter - (c) 2020 by Bucanero - www.bucanero.com.ar
*
* This tool is based (reversed) on the original tlou_save_data_decrypter by Red-EyeX32 and aerosoul94
*
* Information about the encryption method:
*	- https://github.com/RocketRobz/NTR_Launcher_3D/blob/master/twlnand-side/BootLoader/source/encryption.c
*	- http://www.ssugames.org/pluginfile.php/998/mod_resource/content/0/gbatek.htm#dsencryptionbygamecodeidcodekey1
*
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

int nd_decrypt_data(u32* data, u32 size)
{
	int i;
	size = data[size/4 -1];
	u32* key_table = malloc(KEYSIZE);

	if (!key_table)
		return 0;

	apply_keycode(key_table, KEY_DATA, SECRET_KEY);

	LOG("NaughtyDog Decrypted size 0x%X (%d bytes)", size, size);
	size = size/4;
	data += 2;

	for (i = 0; i < size; i+= 2)
		crypt_64bit_down(key_table, &data[i]);

	free(key_table);
	return 1;
}

int nd_encrypt_data(u32* data, u32 size)
{
	int i;
	size = data[size/4 -1];
	u32* key_table = malloc(KEYSIZE);

	if (!key_table)
		return 0;

	apply_keycode(key_table, KEY_DATA, SECRET_KEY);

	LOG("NaughtyDog Encrypted size 0x%X (%d bytes)", size, size);
	size = size/4;
	data += 2;

	for (i = 0; i < size; i+= 2)
		crypt_64bit_up(key_table, &data[i]);

	free(key_table);
	return 1;
}
