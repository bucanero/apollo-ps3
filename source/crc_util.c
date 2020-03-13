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
        {
            reflection |= (1 << ((nBits - 1) - bit));
        }

        data = (data >> 1);
    }

    return (reflection);

}   /* reflect() */

uint16_t crc16_hash(const uint8_t* message, int nBytes, uint16_t Init, uint16_t Poly, uint8_t RefIn, uint8_t RefOut)
{
    uint16_t remainder = Init;
    int byte;
    uint8_t bit;

    /*
     * Perform modulo-2 division, a byte at a time.
     */
    for (byte = 0; byte < nBytes; ++byte)
    {
        /*
         * Bring the next byte into the remainder.
         */
        remainder ^= ((RefIn ? REFLECT_DATA(message[byte]) : message[byte]) << (CRC_16_RESULT_WIDTH - 8));

        /*
         * Perform modulo-2 division, a bit at a time.
         */
        for (bit = 8; bit > 0; --bit)
        {
            /*
             * Try to divide the current data bit.
             */
            if (remainder & TOPBIT(CRC_16_RESULT_WIDTH))
            {
                remainder = (remainder << 1) ^ Poly;
            }
            else
            {
                remainder = (remainder << 1);
            }
        }
    }

    /*
     * The final remainder is the CRC result.
     */
    return (RefOut ? REFLECT_REMAINDER16(remainder) : remainder);

}   /* crc16() */


uint32_t crc32_hash(const uint8_t* message, int nBytes, uint32_t Init, uint32_t Poly, uint8_t RefIn, uint8_t RefOut)
{
    uint32_t remainder = Init;
    int byte;
    uint8_t bit;

    /*
     * Perform modulo-2 division, a byte at a time.
     */
    for (byte = 0; byte < nBytes; ++byte)
    {
        /*
         * Bring the next byte into the remainder.
         */
        remainder ^= ((RefIn ? REFLECT_DATA(message[byte]) : message[byte]) << (CRC_32_RESULT_WIDTH - 8));

        /*
         * Perform modulo-2 division, a bit at a time.
         */
        for (bit = 8; bit > 0; --bit)
        {
            /*
             * Try to divide the current data bit.
             */
            if (remainder & TOPBIT(CRC_32_RESULT_WIDTH))
            {
                remainder = (remainder << 1) ^ Poly;
            }
            else
            {
                remainder = (remainder << 1);
            }
        }
    }

    /*
     * The final remainder is the CRC result.
     */
    return (RefOut ? REFLECT_REMAINDER32(remainder) : remainder);

}   /* crc32() */
