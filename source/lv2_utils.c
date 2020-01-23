#include <ppu-lv2.h>

#include "lv2_utils.h"

//----------------------------------------
//LV2 UTILS
//----------------------------------------

u64 lv2peek(u64 addr)
{ 
    lv2syscall1(6, (u64) addr >> 0ULL) ;
    return_to_user_prog(u64);
}

u64 lv2poke(u64 addr, u64 value)
{ 
    lv2syscall2(7, (u64) addr, (u64) value); 
    return_to_user_prog(u64);
}
