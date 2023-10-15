#ifndef CD1619F3_D4E7_44BC_BADD_858010E4B1BB
#define CD1619F3_D4E7_44BC_BADD_858010E4B1BB
#include "lib_lcd.h"

extern const uint8_t IBM_8x16_bitmap[];
static struct bfont IBM_8x16 = {
    .char_width = 8,
    .char_height = 16,
    .ascii_offset = 0,
    .data = IBM_8x16_bitmap,
};

#endif /* CD1619F3_D4E7_44BC_BADD_858010E4B1BB */
