/*******************************************************************************
 * Size: 14 px
 * Bpp: 1
 * Opts: --bpp 1 --size 14 --font E://assets/Antonio-VariableFont_wght.ttf -o E://assets\ui_font_uifontAntonio14.c --format lvgl -r 0x20-0x7f --no-compress --no-prefilter
 ******************************************************************************/

#include "ui.h"

#ifndef UI_FONT_UIFONTANTONIO14
#define UI_FONT_UIFONTANTONIO14 1
#endif

#if UI_FONT_UIFONTANTONIO14

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0xc8,

    /* U+0022 "\"" */
    0xbb, 0xaa, 0xa0,

    /* U+0023 "#" */
    0x28, 0xa2, 0x8a, 0xfc, 0xa2, 0x92, 0xfd, 0x45,
    0x14, 0x50,

    /* U+0024 "$" */
    0x22, 0x69, 0x99, 0xc6, 0x39, 0x99, 0x62, 0x20,

    /* U+0025 "%" */
    0x60, 0x89, 0x10, 0x91, 0x9, 0x10, 0x92, 0x9,
    0x26, 0x92, 0x99, 0x49, 0x64, 0x90, 0xc9, 0x8,
    0x90, 0x89, 0x10, 0x60,

    /* U+0026 "&" */
    0x31, 0x24, 0x92, 0x79, 0xc6, 0x1d, 0xd6, 0x78,
    0xa6, 0x74,

    /* U+0027 "'" */
    0xf0,

    /* U+0028 "(" */
    0x72, 0x49, 0x24, 0x92, 0x49, 0x18,

    /* U+0029 ")" */
    0xd5, 0x55, 0x55, 0x5c,

    /* U+002A "*" */
    0x23, 0x4c, 0xd2, 0x0,

    /* U+002B "+" */
    0x2f, 0x22,

    /* U+002C "," */
    0xd6,

    /* U+002D "-" */
    0xe0,

    /* U+002E "." */
    0x80,

    /* U+002F "/" */
    0x18, 0x84, 0x23, 0x10, 0x84, 0x62, 0x10, 0x8c,
    0x0,

    /* U+0030 "0" */
    0x69, 0x99, 0x99, 0x99, 0x99, 0x99, 0x60,

    /* U+0031 "1" */
    0x3e, 0x92, 0x49, 0x24, 0x92,

    /* U+0032 "2" */
    0x69, 0x99, 0x91, 0x22, 0x44, 0x88, 0xf0,

    /* U+0033 "3" */
    0x69, 0x99, 0x11, 0x61, 0x19, 0x99, 0x60,

    /* U+0034 "4" */
    0x11, 0x8c, 0x65, 0x29, 0x5a, 0x97, 0xc4, 0x21,
    0x0,

    /* U+0035 "5" */
    0xf8, 0x88, 0xe9, 0x11, 0x19, 0x99, 0x60,

    /* U+0036 "6" */
    0x69, 0x99, 0x8e, 0x99, 0x99, 0x99, 0x60,

    /* U+0037 "7" */
    0xf8, 0xc4, 0x23, 0x18, 0x84, 0x63, 0x18, 0xc4,
    0x0,

    /* U+0038 "8" */
    0x69, 0x99, 0x99, 0x69, 0x99, 0x99, 0x60,

    /* U+0039 "9" */
    0x69, 0x99, 0x99, 0x97, 0x19, 0x99, 0x60,

    /* U+003A ":" */
    0xc3,

    /* U+003B ";" */
    0xc7, 0xc0,

    /* U+003C "<" */
    0x13, 0xe8, 0xc3, 0x10,

    /* U+003D "=" */
    0xf0, 0xf0,

    /* U+003E ">" */
    0x8c, 0x71, 0x3c, 0x80,

    /* U+003F "?" */
    0x69, 0x99, 0x11, 0x36, 0x44, 0x4, 0x40,

    /* U+0040 "@" */
    0x1e, 0x30, 0x90, 0x53, 0xda, 0x2d, 0x16, 0x8b,
    0x45, 0xa6, 0xce, 0x90, 0xc, 0x23, 0xf0,

    /* U+0041 "A" */
    0x30, 0xc3, 0xc, 0x39, 0xa6, 0x92, 0x49, 0xe4,
    0x93, 0xcc,

    /* U+0042 "B" */
    0xe9, 0x99, 0x99, 0xe9, 0x99, 0x99, 0xe0,

    /* U+0043 "C" */
    0x69, 0x99, 0x98, 0x88, 0x99, 0x99, 0x60,

    /* U+0044 "D" */
    0xe9, 0x99, 0x99, 0x99, 0x99, 0x99, 0xe0,

    /* U+0045 "E" */
    0xf2, 0x49, 0x3c, 0x92, 0x4e,

    /* U+0046 "F" */
    0xf2, 0x49, 0x3c, 0x92, 0x48,

    /* U+0047 "G" */
    0x74, 0x63, 0x18, 0xc2, 0x71, 0x8c, 0x63, 0x37,
    0x80,

    /* U+0048 "H" */
    0x8c, 0x63, 0x18, 0xc7, 0xf1, 0x8c, 0x63, 0x18,
    0x80,

    /* U+0049 "I" */
    0xff, 0xf8,

    /* U+004A "J" */
    0x11, 0x11, 0x11, 0x11, 0x19, 0x99, 0x60,

    /* U+004B "K" */
    0x94, 0xad, 0x4a, 0x73, 0x1c, 0xa5, 0x2d, 0x29,
    0x80,

    /* U+004C "L" */
    0x92, 0x49, 0x24, 0x92, 0x4e,

    /* U+004D "M" */
    0xc7, 0x8f, 0x1e, 0x3c, 0x7d, 0xea, 0xd5, 0xab,
    0x56, 0xec, 0x99, 0x20,

    /* U+004E "N" */
    0x8c, 0x73, 0x9c, 0xd6, 0xb7, 0x9c, 0xe7, 0x18,
    0x80,

    /* U+004F "O" */
    0x74, 0x63, 0x18, 0xc6, 0x31, 0x8c, 0x63, 0x17,
    0x0,

    /* U+0050 "P" */
    0xe9, 0x99, 0x99, 0x9e, 0x88, 0x88, 0x80,

    /* U+0051 "Q" */
    0x74, 0x63, 0x18, 0xc6, 0x31, 0x8c, 0x63, 0x17,
    0x8, 0x0,

    /* U+0052 "R" */
    0xe9, 0x99, 0x99, 0xe9, 0x99, 0x99, 0x90,

    /* U+0053 "S" */
    0x69, 0x99, 0xc4, 0x63, 0x99, 0x99, 0x60,

    /* U+0054 "T" */
    0xf2, 0x22, 0x22, 0x22, 0x22, 0x22, 0x20,

    /* U+0055 "U" */
    0x8c, 0x63, 0x18, 0xc6, 0x31, 0x8c, 0x63, 0x17,
    0x0,

    /* U+0056 "V" */
    0xcd, 0x34, 0x92, 0x49, 0x26, 0x9e, 0x38, 0xc3,
    0xc, 0x30,

    /* U+0057 "W" */
    0xc9, 0xa4, 0xd3, 0x69, 0xa5, 0xd2, 0xa9, 0x54,
    0xaa, 0x75, 0x19, 0x8c, 0xc6, 0x62, 0x20,

    /* U+0058 "X" */
    0xca, 0xd4, 0xe3, 0x18, 0x86, 0x72, 0x94, 0xbc,
    0x80,

    /* U+0059 "Y" */
    0x9e, 0xd4, 0xa5, 0x38, 0x84, 0x21, 0x8, 0x42,
    0x0,

    /* U+005A "Z" */
    0xf3, 0x32, 0x26, 0x66, 0x44, 0xcc, 0xf0,

    /* U+005B "[" */
    0xea, 0xaa, 0xaa, 0xc0,

    /* U+005C "\\" */
    0x42, 0x10, 0x86, 0x10, 0x84, 0x30, 0x84, 0x21,
    0x0,

    /* U+005D "]" */
    0xd5, 0x55, 0x55, 0xc0,

    /* U+005E "^" */
    0x63, 0x1d, 0xad, 0x4a, 0x60,

    /* U+005F "_" */
    0xf0,

    /* U+0060 "`" */
    0x50,

    /* U+0061 "a" */
    0x69, 0x91, 0x3d, 0x99, 0x99, 0x70,

    /* U+0062 "b" */
    0x88, 0xe9, 0x99, 0x99, 0x99, 0x99, 0xe0,

    /* U+0063 "c" */
    0x69, 0x99, 0x88, 0x89, 0x99, 0x60,

    /* U+0064 "d" */
    0x11, 0x79, 0x99, 0x99, 0x99, 0x99, 0x70,

    /* U+0065 "e" */
    0x69, 0x99, 0xf8, 0x89, 0x99, 0x60,

    /* U+0066 "f" */
    0x69, 0x74, 0x92, 0x49, 0x24,

    /* U+0067 "g" */
    0x79, 0x99, 0x99, 0x99, 0x97, 0x19, 0xe0,

    /* U+0068 "h" */
    0x88, 0xf9, 0x99, 0x99, 0x99, 0x99, 0x90,

    /* U+0069 "i" */
    0xbf, 0xf8,

    /* U+006A "j" */
    0x45, 0x55, 0x55, 0x5c,

    /* U+006B "k" */
    0x84, 0x25, 0x6a, 0x53, 0x98, 0xe5, 0x2d, 0x29,
    0x0,

    /* U+006C "l" */
    0xff, 0xf8,

    /* U+006D "m" */
    0xfd, 0x26, 0x4c, 0x99, 0x32, 0x64, 0xc9, 0x93,
    0x26, 0x48,

    /* U+006E "n" */
    0xf9, 0x99, 0x99, 0x99, 0x99, 0x90,

    /* U+006F "o" */
    0x69, 0x99, 0x99, 0x99, 0x99, 0x60,

    /* U+0070 "p" */
    0xe9, 0x99, 0x99, 0x99, 0x99, 0xe8, 0x80,

    /* U+0071 "q" */
    0x79, 0x99, 0x99, 0x99, 0x99, 0x71, 0x10,

    /* U+0072 "r" */
    0xf2, 0x49, 0x24, 0x92, 0x0,

    /* U+0073 "s" */
    0x69, 0x99, 0xc6, 0x39, 0x99, 0x60,

    /* U+0074 "t" */
    0x4b, 0xa4, 0x92, 0x49, 0x26,

    /* U+0075 "u" */
    0x99, 0x99, 0x99, 0x99, 0x99, 0x70,

    /* U+0076 "v" */
    0xca, 0xd6, 0xb5, 0x29, 0x4e, 0x71, 0x8c,

    /* U+0077 "w" */
    0xdb, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x6a, 0x66,
    0x66, 0x66, 0x24,

    /* U+0078 "x" */
    0xda, 0x94, 0xe3, 0x10, 0xca, 0x52, 0xb6,

    /* U+0079 "y" */
    0x99, 0x99, 0xdf, 0x66, 0x66, 0x66, 0xc0,

    /* U+007A "z" */
    0xf3, 0x32, 0x66, 0x64, 0xcc, 0xf0,

    /* U+007B "{" */
    0x34, 0x44, 0x44, 0x48, 0x44, 0x44, 0x44, 0x70,

    /* U+007C "|" */
    0xff, 0xfc,

    /* U+007D "}" */
    0xc4, 0x44, 0x44, 0x43, 0x44, 0x44, 0x44, 0xc0,

    /* U+007E "~" */
    0xee, 0x60
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 54, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 58, .box_w = 1, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 81, .box_w = 4, .box_h = 5, .ofs_x = 1, .ofs_y = 8},
    {.bitmap_index = 6, .adv_w = 93, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 16, .adv_w = 88, .box_w = 4, .box_h = 15, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 24, .adv_w = 238, .box_w = 12, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 44, .adv_w = 105, .box_w = 6, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 54, .adv_w = 45, .box_w = 1, .box_h = 4, .ofs_x = 1, .ofs_y = 9},
    {.bitmap_index = 55, .adv_w = 61, .box_w = 3, .box_h = 15, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 61, .adv_w = 61, .box_w = 2, .box_h = 15, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 65, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 8},
    {.bitmap_index = 69, .adv_w = 74, .box_w = 4, .box_h = 4, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 71, .adv_w = 51, .box_w = 2, .box_h = 4, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 72, .adv_w = 76, .box_w = 3, .box_h = 1, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 73, .adv_w = 58, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 74, .adv_w = 80, .box_w = 5, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 83, .adv_w = 93, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 90, .adv_w = 93, .box_w = 3, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 95, .adv_w = 93, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 102, .adv_w = 93, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 109, .adv_w = 93, .box_w = 5, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 118, .adv_w = 93, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 125, .adv_w = 93, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 132, .adv_w = 93, .box_w = 5, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 141, .adv_w = 93, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 148, .adv_w = 93, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 155, .adv_w = 53, .box_w = 1, .box_h = 8, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 156, .adv_w = 61, .box_w = 1, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 158, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 162, .adv_w = 79, .box_w = 4, .box_h = 3, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 164, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 168, .adv_w = 93, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 175, .adv_w = 166, .box_w = 9, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 190, .adv_w = 99, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 200, .adv_w = 100, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 207, .adv_w = 100, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 214, .adv_w = 104, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 221, .adv_w = 83, .box_w = 3, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 226, .adv_w = 82, .box_w = 3, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 231, .adv_w = 103, .box_w = 5, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 240, .adv_w = 108, .box_w = 5, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 249, .adv_w = 54, .box_w = 1, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 251, .adv_w = 96, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 258, .adv_w = 99, .box_w = 5, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 267, .adv_w = 76, .box_w = 3, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 272, .adv_w = 142, .box_w = 7, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 284, .adv_w = 111, .box_w = 5, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 293, .adv_w = 103, .box_w = 5, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 302, .adv_w = 98, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 309, .adv_w = 103, .box_w = 5, .box_h = 15, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 319, .adv_w = 103, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 326, .adv_w = 91, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 333, .adv_w = 69, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 340, .adv_w = 104, .box_w = 5, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 349, .adv_w = 97, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 359, .adv_w = 147, .box_w = 9, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 374, .adv_w = 87, .box_w = 5, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 383, .adv_w = 90, .box_w = 5, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 392, .adv_w = 75, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 399, .adv_w = 69, .box_w = 2, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 403, .adv_w = 81, .box_w = 5, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 412, .adv_w = 69, .box_w = 2, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 416, .adv_w = 101, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 421, .adv_w = 82, .box_w = 4, .box_h = 1, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 422, .adv_w = 44, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 12},
    {.bitmap_index = 423, .adv_w = 99, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 429, .adv_w = 103, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 436, .adv_w = 96, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 442, .adv_w = 103, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 449, .adv_w = 96, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 455, .adv_w = 71, .box_w = 3, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 460, .adv_w = 102, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 467, .adv_w = 106, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 474, .adv_w = 55, .box_w = 1, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 476, .adv_w = 57, .box_w = 2, .box_h = 15, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 480, .adv_w = 98, .box_w = 5, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 489, .adv_w = 55, .box_w = 1, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 491, .adv_w = 156, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 501, .adv_w = 106, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 507, .adv_w = 100, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 513, .adv_w = 103, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 520, .adv_w = 102, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 527, .adv_w = 70, .box_w = 3, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 532, .adv_w = 88, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 538, .adv_w = 65, .box_w = 3, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 543, .adv_w = 105, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 549, .adv_w = 85, .box_w = 5, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 556, .adv_w = 128, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 567, .adv_w = 82, .box_w = 5, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 574, .adv_w = 86, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 581, .adv_w = 74, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 587, .adv_w = 72, .box_w = 4, .box_h = 15, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 595, .adv_w = 59, .box_w = 1, .box_h = 14, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 597, .adv_w = 72, .box_w = 4, .box_h = 15, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 605, .adv_w = 122, .box_w = 6, .box_h = 2, .ofs_x = 1, .ofs_y = 6}
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
    -10, -7, -21, -8, -24, -7, 12, -8,
    -8, -5, -2, -11, -11, -13, -4, -8,
    -8, -7, -5, 12, -8, -8, -5, -2,
    -11, -11, -13, -4, -12, -12, -7, -26,
    -26, -20, -4, -17, -5, -17, -9, -9,
    -8, -5, -5, -7, -3, -3, -3, -5,
    -2, -2, -20, -20, -4, -16, -3, -2,
    -2, -2, -2, -2, -2, -2, -2, -4,
    -1, -1, -1, -1, -1, -1, -1, -14,
    -11, -2, -2, -2, -5, -4, -4, -7,
    -2, -2, -2, -26, -26, -7, -14, -5,
    -5, -5, -3, -3, -3, -3, -3, -3,
    -3, -2, -2, -11, -11, -5, -7, -2,
    -2, -2, -2, -2, -2, -2, -11, -11,
    -5, -7, -2, -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2, -2, -2, -2,
    -2, -2, -13, -13, -7, -13, -3, -4,
    -4, -4, -4, -4, -4, -2, -2, -2,
    -3, -3, -2, -2, -2, -4, -3, -2,
    -2, -2, -4, -3, -2, -2, -2, -4,
    -9, -7, -2, -2, -2, -2, -2, -2,
    -2, -3, -2, -2, -2, -4, -3, -2,
    -2, -2, -4, -13, -13, -10, -11, -10,
    -11, -11, -13
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
const lv_font_t ui_font_uifontAntonio14 = {
#else
lv_font_t ui_font_uifontAntonio14 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 17,          /*The maximum line height required by the font*/
    .base_line = 3,             /*Baseline measured from the bottom of the line*/
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



#endif /*#if UI_FONT_UIFONTANTONIO14*/

