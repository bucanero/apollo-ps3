#ifndef _LV2_UTILS_H_
#define _LV2_UTILS_H_
#include <ppu-lv2.h>
#include "lv2_utils.h"


//----------------------------------------
//LV2 UTILS
//----------------------------------------

u64 lv2peek(u64 addr);
u64 lv2poke(u64 addr, u64 value);

static void lv2_poke32(u64 addr, uint32_t val)
{
    if(addr==0) return;
    uint32_t next = lv2peek(addr) & 0xffffffff;
    lv2poke(addr, (((u64) val) << 32) | next);
}

static inline void _poke(u64 addr, u64 val)
{
    if(addr==0) return;
    lv2poke(0x8000000000000000ULL + addr, val);
}

static inline void _poke32(u64 addr, uint32_t val)
{
    if(addr==0) return;
    lv2_poke32(0x8000000000000000ULL + addr, val);
}

#endif