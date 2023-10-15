#ifndef DABF7699_FAAC_470A_98DA_6397E14B616A
#define DABF7699_FAAC_470A_98DA_6397E14B616A
#include "lib_lcd.h"

extern const uint8_t IBM_8x8_bitmap[];
static const struct bfont IBM_8x8 = {
    .char_width = 8,
    .char_height = 8,
    .ascii_offset = 0x00,
    .data = IBM_8x8_bitmap
};

#endif /* DABF7699_FAAC_470A_98DA_6397E14B616A */
