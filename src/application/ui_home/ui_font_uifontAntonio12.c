/*******************************************************************************
 * Size: 12 px
 * Bpp: 1
 * Opts: --bpp 1 --size 12 --font E://assets/Antonio-VariableFont_wght.ttf -o E://assets\ui_font_uifontAntonio12.c --format lvgl -r 0x20-0x7f --no-compress --no-prefilter
 ******************************************************************************/

#include "ui.h"

#ifndef UI_FONT_UIFONTANTONIO12
#define UI_FONT_UIFONTANTONIO12 1
#endif

#if UI_FONT_UIFONTANTONIO12

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0x40,

    /* U+0022 "\"" */
    0xb6, 0xd0,

    /* U+0023 "#" */
    0x29, 0x95, 0xf5, 0x2b, 0xea, 0x53, 0x0,

    /* U+0024 "$" */
    0x46, 0x99, 0xc6, 0x39, 0x96, 0x40,

    /* U+0025 "%" */
    0xf1, 0x12, 0x42, 0x48, 0x49, 0xf9, 0x53, 0x2a,
    0x7e, 0x48, 0x49, 0x9, 0x22, 0x3c,

    /* U+0026 "&" */
    0x72, 0x94, 0xa2, 0x32, 0xb2, 0x97, 0x40,

    /* U+0027 "'" */
    0xf0,

    /* U+0028 "(" */
    0xea, 0xaa, 0xac,

    /* U+0029 ")" */
    0xd5, 0x55, 0x5c,

    /* U+002A "*" */
    0x23, 0x9c, 0x40,

    /* U+002B "+" */
    0x4b, 0xa0,

    /* U+002C "," */
    0xe0,

    /* U+002D "-" */
    0xc0,

    /* U+002E "." */
    0x80,

    /* U+002F "/" */
    0x13, 0x22, 0x26, 0x44, 0x4c,

    /* U+0030 "0" */
    0xf6, 0xdb, 0x6d, 0xb7, 0x80,

    /* U+0031 "1" */
    0x3c, 0x92, 0x49, 0x24,

    /* U+0032 "2" */
    0x69, 0x99, 0x12, 0x24, 0xc8, 0xf0,

    /* U+0033 "3" */
    0x69, 0x91, 0x16, 0x11, 0x99, 0x60,

    /* U+0034 "4" */
    0x11, 0x8c, 0x65, 0x29, 0x5f, 0x10, 0x80,

    /* U+0035 "5" */
    0xf2, 0x7a, 0x4d, 0xbc,

    /* U+0036 "6" */
    0x69, 0x98, 0xe9, 0x99, 0x99, 0x60,

    /* U+0037 "7" */
    0xf1, 0x22, 0x26, 0x44, 0x44,

    /* U+0038 "8" */
    0x69, 0x99, 0x96, 0x99, 0x99, 0x60,

    /* U+0039 "9" */
    0x69, 0x99, 0x99, 0x71, 0x99, 0x60,

    /* U+003A ":" */
    0x84,

    /* U+003B ";" */
    0x87, 0x80,

    /* U+003C "<" */
    0x36, 0x84, 0x30,

    /* U+003D "=" */
    0xf0, 0xf0,

    /* U+003E ">" */
    0x86, 0x16, 0x80,

    /* U+003F "?" */
    0xf6, 0x92, 0x90, 0x48,

    /* U+0040 "@" */
    0x3c, 0x62, 0xc1, 0x9d, 0xa5, 0xa5, 0xa5, 0xa5,
    0xda, 0x40, 0x3e,

    /* U+0041 "A" */
    0x21, 0x9c, 0xa5, 0x29, 0x4f, 0x4e, 0x40,

    /* U+0042 "B" */
    0xf9, 0x99, 0xe9, 0x99, 0x9e,

    /* U+0043 "C" */
    0x69, 0x99, 0x88, 0x89, 0x99, 0x60,

    /* U+0044 "D" */
    0xe9, 0x99, 0x99, 0x99, 0x9e,

    /* U+0045 "E" */
    0xf2, 0x4f, 0x24, 0x9c,

    /* U+0046 "F" */
    0xf2, 0x4f, 0x24, 0x90,

    /* U+0047 "G" */
    0x69, 0x99, 0x8b, 0x99, 0x99, 0x70,

    /* U+0048 "H" */
    0x99, 0x99, 0xf9, 0x99, 0x99,

    /* U+0049 "I" */
    0xff, 0xc0,

    /* U+004A "J" */
    0x11, 0x11, 0x11, 0x99, 0x96,

    /* U+004B "K" */
    0x9a, 0xae, 0xcc, 0xaa, 0xb9,

    /* U+004C "L" */
    0x92, 0x49, 0x24, 0x9c,

    /* U+004D "M" */
    0xcf, 0x3c, 0xf3, 0xce, 0xdb, 0x6d, 0xb6, 0xd0,

    /* U+004E "N" */
    0x99, 0xdd, 0xdb, 0xbb, 0x99,

    /* U+004F "O" */
    0x69, 0x99, 0x99, 0x99, 0x96,

    /* U+0050 "P" */
    0xe9, 0x99, 0x9e, 0x88, 0x88,

    /* U+0051 "Q" */
    0x69, 0x99, 0x99, 0x99, 0x96, 0x20,

    /* U+0052 "R" */
    0xf9, 0x99, 0xe9, 0x99, 0x99,

    /* U+0053 "S" */
    0x69, 0x9c, 0x62, 0x99, 0x96,

    /* U+0054 "T" */
    0xe9, 0x24, 0x92, 0x48,

    /* U+0055 "U" */
    0x99, 0x99, 0x99, 0x99, 0x96,

    /* U+0056 "V" */
    0x8e, 0x56, 0xa5, 0x29, 0x46, 0x31, 0x0,

    /* U+0057 "W" */
    0xda, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x6a, 0x66,
    0x66, 0x24,

    /* U+0058 "X" */
    0xd5, 0x56, 0x26, 0x65, 0x5d,

    /* U+0059 "Y" */
    0xda, 0x94, 0xa2, 0x10, 0x84, 0x21, 0x0,

    /* U+005A "Z" */
    0xe4, 0xa4, 0x94, 0x9c,

    /* U+005B "[" */
    0xea, 0xaa, 0xac,

    /* U+005C "\\" */
    0x44, 0x44, 0x62, 0x22, 0x31,

    /* U+005D "]" */
    0xd5, 0x55, 0x5c,

    /* U+005E "^" */
    0x46, 0xaa, 0xa9,

    /* U+005F "_" */
    0xf0,

    /* U+0060 "`" */
    0x50,

    /* U+0061 "a" */
    0x69, 0x93, 0xd9, 0x99, 0x70,

    /* U+0062 "b" */
    0x88, 0xe9, 0x99, 0x99, 0x99, 0xe0,

    /* U+0063 "c" */
    0x69, 0x98, 0x88, 0x99, 0x60,

    /* U+0064 "d" */
    0x11, 0x79, 0x99, 0x99, 0x99, 0x70,

    /* U+0065 "e" */
    0x69, 0x99, 0xf8, 0x99, 0x70,

    /* U+0066 "f" */
    0x6b, 0xa4, 0x92, 0x49, 0x0,

    /* U+0067 "g" */
    0x79, 0x99, 0x99, 0x97, 0x1e,

    /* U+0068 "h" */
    0x88, 0xf9, 0x99, 0x99, 0x99, 0x90,

    /* U+0069 "i" */
    0xbf, 0xe0,

    /* U+006A "j" */
    0x45, 0x55, 0x57,

    /* U+006B "k" */
    0x88, 0x9a, 0xae, 0xce, 0xaa, 0x90,

    /* U+006C "l" */
    0xff, 0xe0,

    /* U+006D "m" */
    0xff, 0x26, 0x4c, 0x99, 0x32, 0x64, 0xc9, 0x92,

    /* U+006E "n" */
    0xf9, 0x99, 0x99, 0x99, 0x90,

    /* U+006F "o" */
    0x69, 0x99, 0x99, 0x99, 0x60,

    /* U+0070 "p" */
    0xe9, 0x99, 0x99, 0x99, 0xe8,

    /* U+0071 "q" */
    0x79, 0x99, 0x99, 0x99, 0x71,

    /* U+0072 "r" */
    0xf2, 0x49, 0x24, 0x80,

    /* U+0073 "s" */
    0x69, 0x9c, 0x63, 0x99, 0x60,

    /* U+0074 "t" */
    0x5d, 0x24, 0x92, 0x4c,

    /* U+0075 "u" */
    0x99, 0x99, 0x99, 0x99, 0xf0,

    /* U+0076 "v" */
    0x9d, 0x55, 0x56, 0x66, 0x60,

    /* U+0077 "w" */
    0x95, 0x29, 0xd3, 0xa6, 0xcd, 0x9b, 0x36, 0x6c,

    /* U+0078 "x" */
    0xd5, 0x66, 0x26, 0x65, 0x90,

    /* U+0079 "y" */
    0xd5, 0x55, 0x55, 0x62, 0x26,

    /* U+007A "z" */
    0xe5, 0xa4, 0xb4, 0xe0,

    /* U+007B "{" */
    0x69, 0x25, 0x12, 0x49, 0x80,

    /* U+007C "|" */
    0xff, 0xf0,

    /* U+007D "}" */
    0xc9, 0x24, 0x52, 0x4b, 0x0,

    /* U+007E "~" */
    0xcd, 0x80
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 46, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 50, .box_w = 1, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 70, .box_w = 3, .box_h = 4, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 5, .adv_w = 80, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 12, .adv_w = 75, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 18, .adv_w = 204, .box_w = 11, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 32, .adv_w = 90, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 39, .adv_w = 39, .box_w = 1, .box_h = 4, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 40, .adv_w = 53, .box_w = 2, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 43, .adv_w = 53, .box_w = 2, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 46, .adv_w = 82, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 49, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 51, .adv_w = 43, .box_w = 1, .box_h = 3, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 52, .adv_w = 65, .box_w = 2, .box_h = 1, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 53, .adv_w = 50, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 54, .adv_w = 68, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 59, .adv_w = 80, .box_w = 3, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 64, .adv_w = 80, .box_w = 3, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 68, .adv_w = 80, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 74, .adv_w = 80, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 80, .adv_w = 80, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 87, .adv_w = 80, .box_w = 3, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 91, .adv_w = 80, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 97, .adv_w = 80, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 102, .adv_w = 80, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 108, .adv_w = 80, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 114, .adv_w = 46, .box_w = 1, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 115, .adv_w = 53, .box_w = 1, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 117, .adv_w = 68, .box_w = 4, .box_h = 5, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 120, .adv_w = 68, .box_w = 4, .box_h = 3, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 122, .adv_w = 68, .box_w = 4, .box_h = 5, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 125, .adv_w = 80, .box_w = 3, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 129, .adv_w = 143, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 140, .adv_w = 85, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 147, .adv_w = 86, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 152, .adv_w = 86, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 158, .adv_w = 89, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 163, .adv_w = 71, .box_w = 3, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 167, .adv_w = 70, .box_w = 3, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 171, .adv_w = 88, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 177, .adv_w = 92, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 182, .adv_w = 46, .box_w = 1, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 184, .adv_w = 82, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 189, .adv_w = 85, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 194, .adv_w = 65, .box_w = 3, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 198, .adv_w = 122, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 206, .adv_w = 95, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 211, .adv_w = 89, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 216, .adv_w = 84, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 221, .adv_w = 89, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 227, .adv_w = 88, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 232, .adv_w = 78, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 237, .adv_w = 59, .box_w = 3, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 241, .adv_w = 89, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 246, .adv_w = 83, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 253, .adv_w = 126, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 263, .adv_w = 75, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 268, .adv_w = 78, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 275, .adv_w = 65, .box_w = 3, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 279, .adv_w = 59, .box_w = 2, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 282, .adv_w = 69, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 287, .adv_w = 59, .box_w = 2, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 290, .adv_w = 86, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 293, .adv_w = 70, .box_w = 4, .box_h = 1, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 294, .adv_w = 38, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 10},
    {.bitmap_index = 295, .adv_w = 85, .box_w = 4, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 300, .adv_w = 88, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 306, .adv_w = 82, .box_w = 4, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 311, .adv_w = 88, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 317, .adv_w = 83, .box_w = 4, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 322, .adv_w = 61, .box_w = 3, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 327, .adv_w = 87, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 332, .adv_w = 91, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 338, .adv_w = 47, .box_w = 1, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 340, .adv_w = 49, .box_w = 2, .box_h = 12, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 343, .adv_w = 84, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 349, .adv_w = 47, .box_w = 1, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 351, .adv_w = 134, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 359, .adv_w = 91, .box_w = 4, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 364, .adv_w = 86, .box_w = 4, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 369, .adv_w = 88, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 374, .adv_w = 88, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 379, .adv_w = 60, .box_w = 3, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 383, .adv_w = 75, .box_w = 4, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 388, .adv_w = 56, .box_w = 3, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 392, .adv_w = 90, .box_w = 4, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 397, .adv_w = 73, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 402, .adv_w = 109, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 410, .adv_w = 70, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 415, .adv_w = 74, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 420, .adv_w = 63, .box_w = 3, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 424, .adv_w = 62, .box_w = 3, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 429, .adv_w = 50, .box_w = 1, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 431, .adv_w = 62, .box_w = 3, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 436, .adv_w = 105, .box_w = 5, .box_h = 2, .ofs_x = 1, .ofs_y = 5}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};

/*-----------------
 *    KERNING
 *----------------*/


/*Pair left and right glyphs for kerning*/
static const uint8_t kern_pair_glyph_ids[] =
{
    3, 21,
    3, 34,
    3, 43,
    11, 34,
    11, 43,
    12, 24,
    13, 16,
    13, 18,
    13, 24,
    13, 53,
    13, 54,
    13, 55,
    13, 56,
    13, 58,
    13, 85,
    13, 87,
    13, 88,
    13, 90,
    14, 24,
    15, 16,
    15, 18,
    15, 24,
    15, 53,
    15, 54,
    15, 55,
    15, 56,
    15, 58,
    15, 85,
    15, 87,
    15, 88,
    15, 90,
    16, 13,
    16, 15,
    16, 16,
    24, 12,
    24, 13,
    24, 14,
    24, 15,
    24, 16,
    34, 3,
    34, 11,
    34, 55,
    34, 56,
    34, 58,
    34, 71,
    34, 87,
    34, 88,
    34, 90,
    36, 57,
    37, 57,
    39, 13,
    39, 15,
    39, 34,
    39, 43,
    39, 66,
    39, 68,
    39, 69,
    39, 70,
    39, 72,
    39, 80,
    39, 82,
    43, 13,
    43, 15,
    43, 34,
    44, 66,
    44, 68,
    44, 69,
    44, 70,
    44, 72,
    44, 80,
    44, 82,
    45, 3,
    45, 11,
    45, 36,
    45, 40,
    45, 48,
    45, 53,
    45, 55,
    45, 56,
    45, 58,
    45, 87,
    45, 88,
    48, 57,
    49, 13,
    49, 15,
    49, 34,
    49, 43,
    53, 13,
    53, 15,
    53, 43,
    53, 66,
    53, 68,
    53, 69,
    53, 70,
    53, 72,
    53, 80,
    53, 82,
    54, 13,
    54, 15,
    55, 13,
    55, 15,
    55, 34,
    55, 43,
    55, 66,
    55, 68,
    55, 69,
    55, 70,
    55, 72,
    55, 80,
    55, 82,
    56, 13,
    56, 15,
    56, 34,
    56, 43,
    56, 66,
    56, 68,
    56, 69,
    56, 70,
    56, 72,
    56, 80,
    56, 82,
    57, 36,
    57, 40,
    57, 48,
    57, 68,
    57, 69,
    57, 70,
    57, 72,
    57, 80,
    57, 82,
    58, 13,
    58, 15,
    58, 34,
    58, 43,
    58, 66,
    58, 68,
    58, 69,
    58, 70,
    58, 72,
    58, 80,
    58, 82,
    66, 53,
    66, 55,
    66, 56,
    66, 58,
    67, 53,
    67, 55,
    67, 56,
    67, 57,
    67, 58,
    68, 53,
    68, 55,
    68, 56,
    68, 57,
    68, 58,
    70, 53,
    70, 55,
    70, 56,
    70, 57,
    70, 58,
    71, 13,
    71, 15,
    71, 66,
    71, 68,
    71, 69,
    71, 70,
    71, 72,
    71, 80,
    71, 82,
    80, 53,
    80, 55,
    80, 56,
    80, 57,
    80, 58,
    81, 53,
    81, 55,
    81, 56,
    81, 57,
    81, 58,
    83, 13,
    83, 15,
    87, 13,
    87, 15,
    88, 13,
    88, 15,
    90, 13,
    90, 15
};

/* Kerning between the respective left and right glyphs
 * 4.4 format which needs to scaled with `kern_scale`*/
static const int8_t kern_pair_values[] =
{
    -8, -6, -18, -7, -21, -6, 10, -7,
    -7, -5, -2, -9, -9, -11, -4, -7,
    -7, -6, -5, 10, -7, -7, -5, -2,
    -9, -9, -11, -4, -10, -10, -6, -22,
    -22, -17, -4, -15, -5, -15, -7, -7,
    -7, -5, -5, -6, -3, -3, -3, -5,
    -2, -2, -17, -17, -4, -14, -3, -2,
    -2, -2, -2, -2, -2, -2, -2, -4,
    -1, -1, -1, -1, -1, -1, -1, -12,
    -9, -2, -2, -2, -5, -4, -4, -6,
    -2, -2, -2, -22, -22, -6, -12, -5,
    -5, -5, -3, -3, -3, -3, -3, -3,
    -3, -2, -2, -9, -9, -5, -6, -2,
    -2, -2, -2, -2, -2, -2, -9, -9,
    -5, -6, -2, -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2, -2, -2, -2,
    -2, -2, -11, -11, -6, -11, -3, -4,
    -4, -4, -4, -4, -4, -2, -2, -2,
    -3, -3, -2, -2, -2, -4, -3, -2,
    -2, -2, -4, -3, -2, -2, -2, -4,
    -7, -6, -2, -2, -2, -2, -2, -2,
    -2, -3, -2, -2, -2, -4, -3, -2,
    -2, -2, -4, -11, -11, -8, -9, -8,
    -9, -9, -11
};

/*Collect the kern pair's data in one place*/
static const lv_font_fmt_txt_kern_pair_t kern_pairs =
{
    .glyph_ids = kern_pair_glyph_ids,
    .values = kern_pair_values,
    .pair_cnt = 187,
    .glyph_ids_size = 0
};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = &kern_pairs,
    .kern_scale = 16,
    .cmap_num = 1,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t ui_font_uifontAntonio12 = {
#else
lv_font_t ui_font_uifontAntonio12 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 14,          /*The maximum line height required by the font*/
    .base_line = 2,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = 0,
    .underline_thickness = 0,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if UI_FONT_UIFONTANTONIO12*/

