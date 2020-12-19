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

#define TOPBIT(W) (1 << (W - 1))
#define REFLECT_DATA(X)         ((uint8_t) reflect((X), 8))
#define REFLECT_REMAINDER16(X)  ((uint16_t) reflect((X), CRC_16_RESULT_WIDTH))
#define REFLECT_REMAINDER32(X)  ((uint32_t) reflect((X), CRC_32_RESULT_WIDTH))

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

/* Reverse the bytes in a 64-bit word. */
static inline uint64_t rev8(uint64_t a)
{
    uint64_t m;

    m = UINT64_C(0xff00ff00ff00ff);
    a = ((a >> 8) & m) | (a & m) << 8;
    m = UINT64_C(0xffff0000ffff);
    a = ((a >> 16) & m) | (a & m) << 16;
    return a >> 32 | a << 32;
}

void generate_crc64_table(uint64_t poly, uint64_t* crc_table)
{
    for(int i=0; i<256; i++)
    {
        uint64_t crc = i;

        for(int j=0; j<8; j++)
            crc = (crc & 1) ? (crc >> 1) ^ poly : (crc >> 1);

        crc_table[i] = rev8(crc);
    }
}

// "MC02" Electronic Arts hash table
// https://ideone.com/cy2rM7
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

uint32_t reflect(uint32_t data, uint8_t nBits)
{
    uint32_t  reflection = 0;
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
            reflection |= (1 << ((nBits - 1) - bit));

        data = (data >> 1);
    }

    return (reflection);

}   /* reflect() */

uint16_t crc16_hash(const uint8_t* data, uint32_t len, custom_crc_t* cfg)
{
    uint16_t crc = (uint16_t)cfg->initial_value;
    uint8_t bit;

    while (len--)
    {
        crc ^= ((cfg->reflection_input ? REFLECT_DATA(*data++) : *data++) << (CRC_16_RESULT_WIDTH - 8));

        for (bit = 8; bit > 0; --bit)
            crc = (crc & TOPBIT(CRC_16_RESULT_WIDTH)) ? (crc << 1) ^ (uint16_t)cfg->polynomial : (crc << 1);
    }

    if (cfg->reflection_output)
        crc = REFLECT_REMAINDER16(crc);

    return (crc ^ (uint16_t)cfg->output_xor);

}   /* crc16() */


uint32_t crc32_hash(const uint8_t* data, uint32_t len, custom_crc_t* cfg)
{
    uint32_t crc = cfg->initial_value;
    uint8_t bit;

    /*
     * Perform modulo-2 division, a byte at a time.
     */
    while (len--)
    {
        /*
         * Bring the next byte into the remainder.
         */
        crc ^= ((cfg->reflection_input ? REFLECT_DATA(*data++) : *data++) << (CRC_32_RESULT_WIDTH - 8));

        /*
         * Perform modulo-2 division, a bit at a time.
         */
        for (bit = 8; bit > 0; --bit)
        {
            /*
             * Try to divide the current data bit.
             */
            crc = (crc & TOPBIT(CRC_32_RESULT_WIDTH)) ? (crc << 1) ^ cfg->polynomial : (crc << 1);
        }
    }

    /*
     * The final remainder is the CRC result.
     */
    if (cfg->reflection_output)
        crc = REFLECT_REMAINDER32(crc);

    return (crc ^ cfg->output_xor);

}   /* crc32() */

uint64_t crc64_hash(uint64_t poly, const uint8_t *data, uint64_t len)
{
    uint64_t crc64_table[256];
    uint64_t crc = rev8(poly); // initial value

    generate_crc64_table(poly, crc64_table);

    while (len--)
        crc = crc64_table[(crc >> 56) ^ *data++] ^ (crc << 8);

    return rev8(crc);
}
