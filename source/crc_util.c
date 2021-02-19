/**********************************************************************
 *
 * Filename:    crc.c
 * 
 * Description: Slow and fast implementations of the CRC standards.
 *
 * Notes:       The parameters for each supported CRC standard are
 *				defined in the header file crc.h.  The implementations
 *				here should stand up to further additions to that list.
 *
 * 
 * Copyright (c) 2000 by Michael Barr.  This software is placed into
 * the public domain and may be used for any purpose.  However, this
 * notice must not be changed or removed and no warranty is either
 * expressed or implied by its publication or distribution.
 **********************************************************************/

#include <stdio.h>
#include "crc_util.h"

#define CREATE_CRC_FUNCTION(UINT, CRC_WIDTH) \
    UINT crc##CRC_WIDTH##_hash (const uint8_t* data, uint32_t len, custom_crc_t* cfg) \
    { \
        UINT crc = (UINT)cfg->initial_value; \
        /* Perform modulo-2 division, a byte at a time. */ \
        while (len--) { \
            /* Bring the next byte into the remainder. */ \
            crc ^= ((UINT)(cfg->reflection_input ? (uint8_t)reflect(*data++, 8) : *data++) << (CRC_WIDTH - 8)); \
            /* Perform modulo-2 division, a bit at a time. */ \
            for (uint8_t bit = 8; bit > 0; --bit) \
                /* Try to divide the current data bit. */ \
                crc = (crc & ((UINT)1 << (CRC_WIDTH - 1))) ? (crc << 1) ^ (UINT)cfg->polynomial : (crc << 1); \
        } \
        /* The final remainder is the CRC result. */ \
        if (cfg->reflection_output) \
            crc = (UINT)reflect(crc, CRC_WIDTH); \
        return (crc ^ (UINT)cfg->output_xor); \
    }

void generate_crc32_table(uint32_t poly, uint32_t* crc_table)
{
    for (int i = 0; i < 256; ++i)
    {
        uint32_t r = i << 24;
        
        for (int j = 0; j < 8; ++j)
            r = (r & 0x80000000) ? (r << 1) ^ poly : (r << 1);
        
        crc_table[i] = r;
    }
}

// "MC02" Electronic Arts hash table
// https://gist.github.com/Experiment5X/5025310 / https://ideone.com/cy2rM7
// I have no clue how this works and understand absolutely none of the math behind it.
// I just reversed it from Dead Space 3.
uint32_t MC02_hash(const uint8_t *pb, uint32_t cb)
{
	uint32_t MC02_table[0x100];
	generate_crc32_table(CRC_32_POLYNOMIAL, MC02_table);

	if (cb < 4)
		return 0;

	uint32_t rotatedThird = (pb[2] << 8) | (pb[2] >> 24);
	uint32_t ORedFirstPair = ((pb[0] << 24) | (pb[0] >> 8)) | ((pb[1] << 16) | (pb[1] >> 16));

	uint32_t seedValue = ~((rotatedThird | ORedFirstPair) | pb[3]);
	pb += 4;
	cb -= 4;

	for (uint32_t i = 0; i < cb; i++)
	{
		uint32_t lookedUpValue = MC02_table[((seedValue >> 22) & 0x3FC) >> 2];
		uint32_t insertedNum = pb[i] | (seedValue << 8);
		seedValue = lookedUpValue ^ insertedNum;
	}

	return ~seedValue;
}

// http://www.cse.yorku.ca/~oz/hash.html#sdbm
uint32_t sdbm_hash(const uint8_t* data, uint32_t len, uint32_t init)
{
    uint32_t crc = init;
    
    while (len--)
        crc = (crc * 0x1003f) + *data++;

    return (crc);
}

uint16_t adler16(unsigned char *data, size_t len)
/* 
    where data is the location of the data in physical memory and 
    len is the length of the data in bytes 
*/
{
    uint32_t a = 1, b = 0;

    // Process each byte of the data in order
    while (len--)
    {
        a = (a + *data++) % MOD_ADLER_16;
        b = (b + a) % MOD_ADLER_16;
    }

    return ((b << 8) | a);
}

uint64_t reflect(uint64_t data, uint8_t nBits)
{
    uint64_t reflection = 0;
    uint8_t bit;
    /*
     * Reflect the data about the center bit.
     */
    for (bit = 0; bit < nBits; ++bit)
    {
        /*
         * If the LSB bit is set, set the reflection of it.
         */
        if (data & 0x01)
            reflection |= ((uint64_t)1 << ((nBits - 1) - bit));

        data = (data >> 1);
    }

    return (reflection);

}   /* reflect() */

/* crc16_hash() */
CREATE_CRC_FUNCTION(uint16_t, 16)

/* crc32_hash() */
CREATE_CRC_FUNCTION(uint32_t, 32)

/* crc64_hash() */
CREATE_CRC_FUNCTION(uint64_t, 64)
