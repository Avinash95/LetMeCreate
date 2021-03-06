#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <letmecreate/click/oled.h>
#include <letmecreate/core/common.h>
#include <letmecreate/core/gpio.h>
#include <letmecreate/core/i2c.h>
#include <letmecreate/core/spi.h>

/* I2C address of SSD1306 */
#define SSD1306_ADDRESS             (0x3C)

#define DATA_ONLY                   (0xC0)

/* Commands of the OLED display controller */
#define SSD1306_DISPLAYOFF              (0xAE)
#define SSD1306_SETDISPLAYCLOCKDIV      (0xD5)
#define SSD1306_SETMULTIPLEX            (0xA8)
#define SSD1306_SETDISPLAYOFFSET        (0xD3)
#define SSD1306_SETSTARTLINE            (0x40)
#define SSD1306_CHARGEPUMP              (0x8D)
#define SSD1306_COMSCANDEC              (0xC8)
#define SSD1306_SETCOMPINS              (0xDA)
#define SSD1306_SETCONTRAST             (0x81)
#define SSD1306_SETPRECHARGE            (0xD9)
#define SSD1306_SETVCOMDETECT           (0xDB)
#define SSD1306_DISPLAYALLON_RESUME     (0xA4)
#define SSD1306_NORMALDISPLAY           (0xA6)
#define SSD1306_DISPLAYON               (0xAF)
#define SSD1306_SETSTARTPAGEADDR        (0xB0)
#define SSD1306_SETHIGHCOLSTARTADDR     (0x10)
#define SSD1306_SETDISPLAYSTARTLINE     (0x40)

#define SSD1306_LCDWIDTH            (96)    /* in pixels */
#define SSD1306_LCDHEIGHT           (39)    /* in pixels */
#define SSD1306_PAGE_COUNT          (5)     /* Each page represents 128x8 pixels, so page count is 5 */
#define CHARACTER_WIDTH             (11)
#define CHARACTER_HEIGHT            (16)
#define CHARACTER_COUNT_PER_LINE    (SSD1306_LCDWIDTH / CHARACTER_WIDTH)
#define OLED_CLICK_LINE_COUNT       (SSD1306_LCDHEIGHT / CHARACTER_HEIGHT)
#define OLED_CLICK_MAX_STR_LEN      (OLED_CLICK_LINE_COUNT * CHARACTER_COUNT_PER_LINE)
#define PAGE_PER_LINE               (2)


/* The default monospace font lookup table.
 *
 * Each character takes 11x16 pixels. Each byte represents 1x8 pixels. The
 * first 11 bytes stores the top half of the character and the consecutive
 * 11 bytes stores the bottom half.
 */
static const uint8_t char_table[][22] = {
    {0x0, 0x0, 0x0, 0x0, 0xfe, 0xfe, 0xfe, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x33, 0x33, 0x33, 0x0, 0x0, 0x0, 0x0},               /* ! */
    {0x0, 0x0, 0x0, 0x3e, 0x3e, 0x0, 0x0, 0x3e, 0x3e, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},                 /* " */
    {0x60, 0x6c, 0xfc, 0xf0, 0x60, 0x7c, 0xfc, 0xe0, 0x60, 0x60, 0x0, 0x0, 0x6, 0x6, 0xf, 0x3f, 0x36, 0x6, 0xf, 0x3f, 0x36, 0x6},       /* # */
    {0x0, 0x0, 0x0, 0x18, 0x8c, 0xff, 0xcc, 0xfc, 0xf8, 0x0, 0x0, 0x0, 0x0, 0x1e, 0x3f, 0x33, 0xff, 0x31, 0x31, 0x18, 0x0, 0x0},        /* $ */
    {0x0, 0x0, 0x40, 0x40, 0x80, 0xbc, 0xfe, 0x42, 0x42, 0x7e, 0x3c, 0x0, 0x1e, 0x3f, 0x21, 0x21, 0x3f, 0x1e, 0x1, 0x1, 0x1, 0x0},      /* % */
    {0x80, 0x80, 0x0, 0x4, 0xc6, 0xfe, 0xfe, 0xdc, 0x80, 0x0, 0x0, 0x23, 0x3f, 0x3c, 0x1f, 0x37, 0x31, 0x38, 0x3f, 0x1f, 0xf, 0x0},     /* & */
    {0x0, 0x0, 0x0, 0x0, 0x0, 0x3e, 0x3e, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},                   /* ' */
    {0x0, 0x0, 0x0, 0x1, 0xf, 0xfe, 0xf8, 0xe0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x80, 0xf0, 0x7f, 0x1f, 0x7, 0x0, 0x0, 0x0},              /* ( */
    {0x0, 0x0, 0x0, 0xe0, 0xf8, 0xfe, 0xf, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x1f, 0x7f, 0xf0, 0x80, 0x0, 0x0, 0x0},              /* ) */
    {0xcc, 0xcc, 0x78, 0x78, 0xfe, 0xfe, 0x78, 0x78, 0xcc, 0xcc, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0},           /* * */
    {0x0, 0x80, 0x80, 0x80, 0x80, 0xf8, 0xf8, 0x80, 0x80, 0x80, 0x80, 0x0, 0x1, 0x1, 0x1, 0x1, 0x1f, 0x1f, 0x1, 0x1, 0x1, 0x1},         /* + */
    {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x38, 0xf8, 0xf8, 0x80, 0x0, 0x0, 0x0},                 /* , */
    {0x0, 0x0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x0, 0x0, 0x0},               /* - */
    {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x38, 0x38, 0x38, 0x0, 0x0, 0x0, 0x0},                  /* . */
    {0x0, 0x2, 0xe, 0x3c, 0xf0, 0xc0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x7, 0x1f, 0x78, 0xe0, 0x80, 0x0},              /* / */
    {0x0, 0xf0, 0xfc, 0xfe, 0xe, 0xc6, 0xe, 0xfe, 0xfc, 0xf0, 0x0, 0x0, 0x7, 0x1f, 0x3f, 0x38, 0x30, 0x38, 0x3f, 0x1f, 0x7, 0x0},       /* 0 */
    {0x0, 0x0, 0x0, 0x0, 0xfe, 0xfe, 0xfe, 0x6, 0xc, 0xc, 0x0, 0x0, 0x30, 0x30, 0x30, 0x3f, 0x3f, 0x3f, 0x30, 0x30, 0x30, 0x0},         /* 1 */
    {0x0, 0x38, 0x7c, 0xfe, 0xce, 0x6, 0x6, 0x6, 0x6, 0xc, 0x0, 0x0, 0x30, 0x30, 0x30, 0x31, 0x33, 0x36, 0x3c, 0x38, 0x30, 0x0},        /* 2 */
    {0x0, 0x38, 0xbc, 0xfe, 0xc6, 0xc6, 0xc6, 0xc6, 0x6, 0xc, 0x0, 0x0, 0xf, 0x1f, 0x3f, 0x39, 0x30, 0x30, 0x30, 0x30, 0x18, 0x0},      /* 3 */
    {0x0, 0x0, 0xfe, 0xfe, 0xfe, 0x1c, 0x78, 0xe0, 0xc0, 0x0, 0x0, 0x0, 0x6, 0x3f, 0x3f, 0x3f, 0x6, 0x6, 0x6, 0x7, 0x7, 0x0},           /* 4 */
    {0x0, 0x80, 0xc6, 0xe6, 0xe6, 0x66, 0x66, 0x7e, 0x7e, 0xfe, 0x0, 0x0, 0xf, 0x1f, 0x1f, 0x38, 0x30, 0x30, 0x30, 0x30, 0x18, 0x0},    /* 5 */
    {0x0, 0x80, 0xcc, 0xe6, 0x66, 0x66, 0x4e, 0xfc, 0xfc, 0xf0, 0x0, 0x0, 0xf, 0x1f, 0x3f, 0x30, 0x30, 0x30, 0x3f, 0x1f, 0x7, 0x0},     /* 6 */
    {0x0, 0xe, 0x3e, 0xfe, 0xfe, 0xc6, 0x6, 0x6, 0x6, 0x6, 0x0, 0x0, 0x0, 0x0, 0x1, 0x7, 0x1f, 0x3f, 0x3c, 0x20, 0x0, 0x0},             /* 7 */
    {0x0, 0x38, 0x3c, 0xfe, 0xc6, 0xc6, 0xc6, 0xfe, 0x3c, 0x38, 0x0, 0x0, 0xf, 0x1f, 0x3f, 0x30, 0x30, 0x30, 0x3f, 0x1f, 0xf, 0x0},     /* 8 */
    {0x0, 0xf0, 0xfc, 0xfe, 0x6, 0x6, 0x6, 0xfe, 0xfc, 0xf8, 0x0, 0x0, 0x7, 0x1f, 0x1f, 0x39, 0x33, 0x33, 0x33, 0x19, 0x0, 0x0},        /* 9 */
    {0x0, 0x0, 0x0, 0x0, 0xe0, 0xe0, 0xe0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x38, 0x38, 0x38, 0x0, 0x0, 0x0, 0x0},               /* : */
    {0x0, 0x0, 0x0, 0x0, 0x38, 0x38, 0x38, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xe, 0x3e, 0x7e, 0x60, 0x0, 0x0, 0x0},               /* ; */
    {0x0, 0x18, 0x30, 0x30, 0x30, 0x60, 0x60, 0x40, 0xc0, 0xc0, 0x0, 0x0, 0xc, 0x6, 0x6, 0x6, 0x3, 0x3, 0x1, 0x1, 0x1, 0x0},            /* < */
    {0x0, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x0, 0x0, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x0},            /* = */
    {0x0, 0xc0, 0xc0, 0x40, 0x60, 0x60, 0x30, 0x30, 0x30, 0x18, 0x0, 0x0, 0x1, 0x1, 0x1, 0x3, 0x3, 0x6, 0x6, 0x6, 0xc, 0x0},            /* > */
    {0x0, 0x0, 0x1c, 0x3e, 0x7e, 0xc6, 0x86, 0x6, 0xc, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x37, 0x37, 0x37, 0x0, 0x0, 0x0, 0x0},             /* ? */
    {0x0, 0xf8, 0xfc, 0x6e, 0x66, 0xe6, 0xc6, 0xc, 0x3c, 0xf8, 0xe0, 0x0, 0x4f, 0xef, 0xcc, 0xcc, 0xcf, 0xc7, 0x60, 0x78, 0x3f, 0xf},   /* @ */
    {0x0, 0x0, 0x0, 0xf0, 0xfe, 0xe, 0xfe, 0xf0, 0x0, 0x0, 0x0, 0x0, 0x30, 0x3f, 0x3f, 0x7, 0x6, 0x7, 0x3f, 0x3f, 0x30, 0x0},           /* A */
    {0x0, 0x3c, 0xfc, 0xfe, 0xc6, 0xc6, 0xc6, 0xfe, 0xfe, 0xfe, 0x0, 0x0, 0x1f, 0x1f, 0x3f, 0x30, 0x30, 0x30, 0x3f, 0x3f, 0x3f, 0x0},   /* B */
    {0x0, 0xc, 0x6, 0x6, 0x6, 0x6, 0x1e, 0xfc, 0xf8, 0xf0, 0x0, 0x0, 0x18, 0x30, 0x30, 0x30, 0x30, 0x3c, 0x1f, 0xf, 0x7, 0x0},          /* C */
    {0x0, 0xf0, 0xfc, 0xfc, 0xe, 0x6, 0x6, 0xfe, 0xfe, 0xfe, 0x0, 0x0, 0x7, 0x1f, 0x1f, 0x38, 0x30, 0x30, 0x3f, 0x3f, 0x3f, 0x0},       /* D */
    {0x0, 0x6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xfe, 0xfe, 0xfe, 0x0, 0x0, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3f, 0x3f, 0x3f, 0x0},    /* E */
    {0x0, 0x6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xfe, 0xfe, 0xfe, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3f, 0x3f, 0x3f, 0x0},          /* F */
    {0x0, 0x8c, 0x86, 0x86, 0x86, 0x86, 0x1e, 0xfc, 0xf8, 0xf0, 0x0, 0x0, 0x1f, 0x3f, 0x3f, 0x31, 0x31, 0x38, 0x1f, 0xf, 0x7, 0x0},     /* G */
    {0x0, 0xfe, 0xfe, 0xfe, 0xc0, 0xc0, 0xc0, 0xfe, 0xfe, 0xfe, 0x0, 0x0, 0x3f, 0x3f, 0x3f, 0x0, 0x0, 0x0, 0x3f, 0x3f, 0x3f, 0x0},      /* H */
    {0x0, 0x6, 0x6, 0x6, 0xfe, 0xfe, 0xfe, 0x6, 0x6, 0x6, 0x0, 0x0, 0x30, 0x30, 0x30, 0x3f, 0x3f, 0x3f, 0x30, 0x30, 0x30, 0x0},         /* I */
    {0x0, 0xfe, 0xfe, 0xfe, 0x6, 0x6, 0x6, 0x0, 0x0, 0x0, 0x0, 0x0, 0xf, 0x1f, 0x3f, 0x38, 0x30, 0x30, 0x30, 0x38, 0x18, 0x0},          /* J */
    {0x2, 0x6, 0xe, 0x9c, 0xf8, 0xf0, 0xc0, 0xfe, 0xfe, 0xfe, 0x0, 0x20, 0x38, 0x3e, 0x1f, 0x7, 0x1, 0x1, 0x3f, 0x3f, 0x3f, 0x0},       /* K */
    {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xfe, 0xfe, 0xfe, 0x0, 0x0, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3f, 0x3f, 0x3f, 0x0},         /* L */
    {0x0, 0xfe, 0xfe, 0xfe, 0x70, 0x80, 0x70, 0xfe, 0xfe, 0xfe, 0x0, 0x0, 0x3f, 0x3f, 0x3f, 0x0, 0x1, 0x0, 0x3f, 0x3f, 0x3f, 0x0},      /* M */
    {0x0, 0xfe, 0xfe, 0xfe, 0x0, 0xe0, 0x7e, 0xfe, 0xfe, 0xfe, 0x0, 0x0, 0x3f, 0x3f, 0x3f, 0x1f, 0x3, 0x0, 0x3f, 0x3f, 0x3f, 0x0},      /* N */
    {0x0, 0xf0, 0xfc, 0xfe, 0xe, 0x6, 0xe, 0xfe, 0xfc, 0xf0, 0x0, 0x0, 0x7, 0x1f, 0x3f, 0x38, 0x30, 0x38, 0x3f, 0x1f, 0x7, 0x0},        /* O */
    {0x0, 0x78, 0xfc, 0xfe, 0x86, 0x86, 0x86, 0xfe, 0xfe, 0xfe, 0x0, 0x0, 0x0, 0x0, 0x1, 0x1, 0x1, 0x1, 0x3f, 0x3f, 0x3f, 0x0},         /* P */
    {0x0, 0xf0, 0xfc, 0xfe, 0xe, 0x6, 0xe, 0xfe, 0xfc, 0xf0, 0x0, 0x0, 0x7, 0x5f, 0xff, 0x78, 0x30, 0x38, 0x3f, 0x1f, 0x7, 0x0},        /* Q */
    {0x0, 0x78, 0xfc, 0xfe, 0x86, 0x86, 0x86, 0xfe, 0xfe, 0xfe, 0x0, 0x20, 0x38, 0x3e, 0x1f, 0x7, 0x1, 0x1, 0x3f, 0x3f, 0x3f, 0x0},     /* R */
    {0x0, 0x0, 0x8c, 0xce, 0xc6, 0xc6, 0xe6, 0xfe, 0x7c, 0x38, 0x0, 0x0, 0xf, 0x1f, 0x3f, 0x31, 0x30, 0x30, 0x30, 0x38, 0x1c, 0x0},     /* S */
    {0x0, 0x6, 0x6, 0x6, 0xfe, 0xfe, 0xfe, 0x6, 0x6, 0x6, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3f, 0x3f, 0x3f, 0x0, 0x0, 0x0, 0x0},               /* T */
    {0x0, 0xfe, 0xfe, 0xfe, 0x0, 0x0, 0x0, 0xfe, 0xfe, 0xfe, 0x0, 0x0, 0xf, 0x1f, 0x3f, 0x30, 0x30, 0x30, 0x3f, 0x1f, 0xf, 0x0},        /* U */
    {0x0, 0x6, 0xfe, 0xfe, 0xc0, 0x0, 0xc0, 0xfe, 0xfe, 0x6, 0x0, 0x0, 0x0, 0x0, 0xf, 0x3f, 0x38, 0x3f, 0xf, 0x0, 0x0, 0x0},            /* V */
    {0x1e, 0xfe, 0xf0, 0x0, 0xf0, 0x70, 0xf0, 0x0, 0xf8, 0xfe, 0x1e, 0x0, 0x3f, 0x3f, 0x3f, 0xf, 0x0, 0xf, 0x3e, 0x3f, 0x1f, 0x0},      /* W */
    {0x0, 0x2, 0xe, 0x3e, 0xf8, 0xe0, 0xf8, 0x3e, 0xe, 0x2, 0x0, 0x0, 0x20, 0x38, 0x3e, 0xf, 0x3, 0xf, 0x3e, 0x38, 0x20, 0x0},          /* X */
    {0x2, 0xe, 0x3e, 0xfc, 0xe0, 0x80, 0xe0, 0xfc, 0x3e, 0xe, 0x2, 0x0, 0x0, 0x0, 0x0, 0x3f, 0x3f, 0x3f, 0x0, 0x0, 0x0, 0x0},           /* Y */
    {0x0, 0xe, 0x1e, 0x7e, 0xf6, 0xe6, 0x86, 0x6, 0x6, 0x6, 0x0, 0x0, 0x30, 0x30, 0x30, 0x30, 0x33, 0x37, 0x3f, 0x3c, 0x38, 0x0},       /* Z */
    {0x0, 0x0, 0x3, 0x3, 0xff, 0xff, 0xff, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xc0, 0xc0, 0xff, 0xff, 0xff, 0x0, 0x0, 0x0, 0x0},             /* [ */
    {0x0, 0x0, 0x0, 0x0, 0x0, 0xc0, 0xf0, 0x3c, 0xe, 0x2, 0x0, 0x0, 0x80, 0xe0, 0x78, 0x1e, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0},              /* \ */
    {0x0, 0x0, 0x0, 0xff, 0xff, 0xff, 0x3, 0x3, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xff, 0xff, 0xff, 0xc0, 0xc0, 0x0, 0x0, 0x0},             /* ] */
    {0x20, 0x30, 0x38, 0x1c, 0xe, 0xe, 0x1c, 0x38, 0x30, 0x20, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},             /* ^ */
    {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0},          /* _ */
    {0x0, 0x0, 0x0, 0x0, 0x4, 0x6, 0x3, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},                     /* ` */
    {0x0, 0xc0, 0xe0, 0xf0, 0x30, 0x30, 0x30, 0x30, 0x60, 0x0, 0x0, 0x0, 0x3f, 0x3f, 0x3f, 0x13, 0x33, 0x33, 0x3f, 0x3e, 0x1e, 0x0},    /* a */
    {0x0, 0xc0, 0xe0, 0xf0, 0x70, 0x30, 0x60, 0xff, 0xff, 0xff, 0x0, 0x0, 0xf, 0x1f, 0x3f, 0x38, 0x30, 0x18, 0x3f, 0x3f, 0x3f, 0x0},    /* b */
    {0x0, 0x60, 0x30, 0x30, 0x30, 0x30, 0x70, 0xe0, 0xe0, 0x80, 0x0, 0x0, 0x18, 0x30, 0x30, 0x30, 0x30, 0x38, 0x1f, 0x1f, 0x7, 0x0},    /* c */
    {0x0, 0xff, 0xff, 0xff, 0x60, 0x30, 0x70, 0xf0, 0xe0, 0xc0, 0x0, 0x0, 0x3f, 0x3f, 0x3f, 0x18, 0x30, 0x38, 0x3f, 0x1f, 0xf, 0x0},    /* d */
    {0x0, 0xc0, 0xe0, 0xf0, 0x30, 0x30, 0x30, 0xf0, 0xe0, 0xc0, 0x0, 0x0, 0x1b, 0x33, 0x33, 0x33, 0x33, 0x3b, 0x3f, 0x1f, 0xf, 0x0},    /* e */
    {0x0, 0x33, 0x33, 0x33, 0xff, 0xff, 0xfe, 0x30, 0x30, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3f, 0x3f, 0x3f, 0x0, 0x0, 0x0, 0x0},          /* f */
    {0x0, 0xfc, 0xfc, 0xfc, 0x18, 0xc, 0x1c, 0xfc, 0xf8, 0xf0, 0x0, 0x0, 0x3f, 0x7f, 0xff, 0xe6, 0xcc, 0xce, 0xcf, 0x67, 0x3, 0x0},     /* g */
    {0x0, 0xe0, 0xf0, 0xf0, 0x30, 0x30, 0x60, 0xff, 0xff, 0xff, 0x0, 0x0, 0x3f, 0x3f, 0x3f, 0x0, 0x0, 0x0, 0x3f, 0x3f, 0x3f, 0x0},      /* h */
    {0x0, 0x0, 0x0, 0x0, 0xf3, 0xf3, 0xf3, 0x30, 0x30, 0x30, 0x0, 0x0, 0x30, 0x30, 0x30, 0x3f, 0x3f, 0x3f, 0x30, 0x30, 0x30, 0x0},      /* i */
    {0x0, 0x0, 0x0, 0x0, 0xf7, 0xf7, 0x30, 0x30, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7f, 0xff, 0xc0, 0xc0, 0xc0, 0x0, 0x0},            /* j */
    {0x0, 0x0, 0x10, 0x30, 0xf0, 0xc0, 0x80, 0xff, 0xff, 0xff, 0x0, 0x0, 0x20, 0x38, 0x3c, 0x1f, 0x7, 0x3, 0x3f, 0x3f, 0x3f, 0x0},      /* k */
    {0x0, 0x0, 0x0, 0x0, 0x0, 0xff, 0xff, 0xff, 0x3, 0x3, 0x3, 0x0, 0x30, 0x30, 0x30, 0x30, 0x3f, 0x1f, 0xf, 0x0, 0x0, 0x0},            /* l */
    {0x0, 0xe0, 0xf0, 0x30, 0x30, 0xe0, 0xf0, 0x30, 0x30, 0xf0, 0xf0, 0x0, 0x3f, 0x3f, 0x0, 0x0, 0x3f, 0x3f, 0x0, 0x0, 0x3f, 0x3f},     /* m */
    {0x0, 0xe0, 0xf0, 0xf0, 0x30, 0x30, 0x60, 0xf0, 0xf0, 0xf0, 0x0, 0x0, 0x3f, 0x3f, 0x3f, 0x0, 0x0, 0x0, 0x3f, 0x3f, 0x3f, 0x0},      /* n */
    {0x0, 0xc0, 0xe0, 0xf0, 0x70, 0x30, 0x70, 0xf0, 0xe0, 0xc0, 0x0, 0x0, 0xf, 0x1f, 0x3f, 0x38, 0x30, 0x38, 0x3f, 0x1f, 0xf, 0x0},     /* o */
    {0x0, 0xc0, 0xe0, 0xf0, 0x70, 0x30, 0x60, 0xf0, 0xf0, 0xf0, 0x0, 0x0, 0xf, 0x1f, 0x3f, 0x38, 0x30, 0x18, 0xff, 0xff, 0xff, 0x0},    /* p */
    {0x0, 0xf0, 0xf0, 0xf0, 0x60, 0x30, 0x70, 0xf0, 0xe0, 0xc0, 0x0, 0x0, 0xff, 0xff, 0xff, 0x18, 0x30, 0x38, 0x3f, 0x1f, 0xf, 0x0},    /* q */
    {0x60, 0x30, 0x30, 0x30, 0x60, 0xf0, 0xf0, 0xf0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3f, 0x3f, 0x3f, 0x0, 0x0, 0x0},          /* r */
    {0x0, 0x0, 0x60, 0x30, 0x30, 0x30, 0xb0, 0xf0, 0xe0, 0xe0, 0x0, 0x0, 0x1e, 0x1e, 0x3f, 0x33, 0x33, 0x33, 0x33, 0x31, 0x18, 0x0},    /* s */
    {0x0, 0x0, 0x30, 0x30, 0x30, 0xfe, 0xfe, 0xfe, 0x30, 0x30, 0x30, 0x0, 0x0, 0x30, 0x30, 0x30, 0x3f, 0x3f, 0x1f, 0x0, 0x0, 0x0},      /* t */
    {0x0, 0xf0, 0xf0, 0xf0, 0x0, 0x0, 0x0, 0xf0, 0xf0, 0xf0, 0x0, 0x0, 0x3f, 0x3f, 0x3f, 0x18, 0x30, 0x30, 0x3f, 0x3f, 0x1f, 0x0},      /* u */
    {0x0, 0x30, 0xf0, 0xf0, 0x0, 0x0, 0x0, 0xf0, 0xf0, 0x30, 0x0, 0x0, 0x0, 0x1, 0xf, 0x3f, 0x30, 0x3f, 0xf, 0x1, 0x0, 0x0},            /* v */
    {0x70, 0xf0, 0x80, 0x0, 0x80, 0x80, 0x80, 0x0, 0x80, 0xf0, 0x70, 0x0, 0x7, 0x3f, 0x38, 0x1f, 0x1, 0x1f, 0x38, 0x3f, 0x7, 0x0},      /* w */
    {0x0, 0x10, 0x30, 0xf0, 0xe0, 0x80, 0xe0, 0xf0, 0x30, 0x10, 0x0, 0x0, 0x20, 0x30, 0x3c, 0x1f, 0x7, 0x1f, 0x3c, 0x30, 0x20, 0x0},    /* x */
    {0x0, 0x30, 0xf0, 0xf0, 0x80, 0x0, 0xc0, 0xf0, 0xf0, 0x10, 0x0, 0x0, 0x0, 0x1, 0xf, 0x7f, 0xfe, 0xdf, 0x7, 0x0, 0x0, 0x0},          /* y */
    {0x0, 0x70, 0xf0, 0xf0, 0xb0, 0x30, 0x30, 0x30, 0x30, 0x30, 0x0, 0x0, 0x30, 0x30, 0x30, 0x31, 0x33, 0x36, 0x3c, 0x3c, 0x38, 0x0},   /* z */
    {0x0, 0x2, 0x2, 0x2, 0x7e, 0xfe, 0xfe, 0x80, 0x80, 0x80, 0x0, 0x0, 0x80, 0x80, 0x80, 0xfe, 0xff, 0xff, 0x3, 0x1, 0x1, 0x0},         /* { */
    {0x0, 0x0, 0x0, 0x0, 0x0, 0xff, 0xff, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xff, 0xff, 0x0, 0x0, 0x0, 0x0},                 /* | */
    {0x0, 0x80, 0x80, 0x80, 0xfe, 0xfe, 0x7e, 0x2, 0x2, 0x2, 0x0, 0x0, 0x1, 0x1, 0x3, 0xff, 0xff, 0xfe, 0x80, 0x80, 0x80, 0x0},         /* } */
    {0x0, 0x80, 0x0, 0x0, 0x0, 0x80, 0x80, 0x80, 0x80, 0x0, 0x0, 0x0, 0x1, 0x3, 0x3, 0x3, 0x1, 0x1, 0x1, 0x1, 0x3, 0x0}                 /* ~ */
};

static bool use_spi = false;
static uint8_t dc_pin = 0;          /* Only used for SPI to send data or commands */

static int oled_click_cmd(uint8_t cmd)
{
    if (use_spi) /* Do not set D/C pin low, we assume this is the default state */
        return spi_transfer(&cmd, NULL, sizeof(cmd));
    else
        return i2c_write_register(SSD1306_ADDRESS, 0, cmd);
}

static void sleep_50ms(void)
{
    struct timespec rem, req = {
        .tv_nsec = 50000000,
        .tv_sec = 0
    };

    while (nanosleep(&req, &rem))
        req = rem;
}

static int oled_click_set_page_addr(uint8_t pageno)
{
    if (pageno >= SSD1306_PAGE_COUNT) {
        fprintf(stderr, "oled: Invalid page number.");
        return -1;
    }

    return oled_click_cmd(SSD1306_SETSTARTPAGEADDR | pageno);
}

void oled_click_use_spi(void)
{
    use_spi = true;
}

void oled_click_use_i2c(void)
{
    use_spi = false;
}

int oled_click_enable(uint8_t mikrobus_index)
{
    uint8_t reset_pin = 0 , sa0_pin = 0;

    switch (mikrobus_index) {
        case MIKROBUS_1:
            reset_pin = MIKROBUS_1_RST;
            if (use_spi)
                dc_pin = MIKROBUS_1_PWM;
            else
                sa0_pin = MIKROBUS_1_PWM;
            break;
        case MIKROBUS_2:
            reset_pin = MIKROBUS_2_RST;
            if (use_spi)
                dc_pin = MIKROBUS_2_PWM;
            else
                sa0_pin = MIKROBUS_2_PWM;
            break;
        default:
            fprintf(stderr, "oled: Invalid mikrobus index.\n");
            return -1;
    }

    if (use_spi) {
        if (gpio_init(dc_pin) < 0
        ||  gpio_set_direction(dc_pin, GPIO_OUTPUT) < 0
        ||  gpio_set_value(dc_pin, 0) < 0) {
            fprintf(stderr, "oled: Failed to set D/C pin to 0.\n");
            return -1;
        }
    } else {
        /* Set SA0 (the least significant bit of the slave address) to 0.
         * It ensures that the address of the device is 0x3C.
         */
        if (gpio_init(sa0_pin) < 0
        ||  gpio_set_direction(sa0_pin, GPIO_OUTPUT) < 0
        ||  gpio_set_value(sa0_pin, 0) < 0) {
            fprintf(stderr, "oled: Failed to set SA0 to 0.\n");
            return -1;
        }
    }

    /* Reset device */
    if (gpio_init(reset_pin) < 0
    ||  gpio_set_direction(reset_pin, GPIO_OUTPUT) < 0
    ||  gpio_set_value(reset_pin, 0) < 0) {
        fprintf(stderr, "oled: Failed to reset device.\n");
        return -1;
    }

    sleep_50ms();

    if (gpio_set_value(reset_pin, 1) < 0) {
        fprintf(stderr, "oled: Failed to reset device.\n");
        return -1;
    }

    /* Initialize the oled display. */
    if (oled_click_cmd(SSD1306_DISPLAYOFF) < 0                /* 0xAE Set OLED Display Off */
    ||  oled_click_cmd(SSD1306_SETDISPLAYCLOCKDIV) < 0        /* 0xD5 Set Display Clock Divide Ratio/Oscillator Frequency */
    ||  oled_click_cmd(0x80) < 0
    ||  oled_click_cmd(SSD1306_SETMULTIPLEX) < 0              /* 0xA8 Set Multiplex Ratio */
    ||  oled_click_cmd(0x27) < 0
    ||  oled_click_cmd(SSD1306_SETDISPLAYOFFSET) < 0          /* 0xD3 Set Display Offset */
    ||  oled_click_cmd(0x00) < 0
    ||  oled_click_cmd(SSD1306_SETSTARTLINE) < 0              /* 0x40 Set Display Start Line */
    ||  oled_click_cmd(SSD1306_CHARGEPUMP) < 0                /* 0x8D Set Charge Pump */
    ||  oled_click_cmd(0x14) < 0                              /* 0x14 Enable Charge Pump */
    ||  oled_click_cmd(SSD1306_COMSCANDEC) < 0                /* 0xC8 Set COM Output Scan Direction */
    ||  oled_click_cmd(SSD1306_SETCOMPINS) < 0                /* 0xDA Set COM Pins Hardware Configuration */
    ||  oled_click_cmd(0x12) < 0
    ||  oled_click_cmd(SSD1306_SETCONTRAST) < 0               /* 0x81 Set Contrast Control */
    ||  oled_click_cmd(0xAF) < 0
    ||  oled_click_cmd(SSD1306_SETPRECHARGE) < 0              /* 0xD9 Set Pre-Charge Period */
    ||  oled_click_cmd(0x25) < 0
    ||  oled_click_cmd(SSD1306_SETVCOMDETECT) < 0             /* 0xDB Set VCOMH Deselect Level */
    ||  oled_click_cmd(0x20) < 0
    ||  oled_click_cmd(SSD1306_DISPLAYALLON_RESUME) < 0       /* 0xA4 Set Entire Display On/Off */
    ||  oled_click_cmd(SSD1306_NORMALDISPLAY) < 0             /* 0xA6 Set Normal/Inverse Display */
    ||  oled_click_cmd(SSD1306_DISPLAYON) < 0)                /* 0xAF Set OLED Display On */
        return -1;

    return 0;
}

int oled_click_set_contrast(uint8_t contrast)
{
    if (oled_click_cmd(SSD1306_SETCONTRAST) < 0
    ||  oled_click_cmd(contrast) < 0)
        return -1;

    return 0;
}

int oled_click_raw_write(uint8_t *data)
{
    uint8_t i = 0;

    if (data == NULL) {
        fprintf(stderr, "oled: Cannot write data using null pointer.\n");
        return -1;
    }

    for (; i < SSD1306_PAGE_COUNT; ++i) {
        uint8_t buffer[SSD1306_LCDWIDTH + 1];
        oled_click_set_page_addr(i);
        oled_click_cmd(SSD1306_SETHIGHCOLSTARTADDR);
        oled_click_cmd(SSD1306_SETDISPLAYSTARTLINE);

        memcpy(&buffer[1], &data[i * SSD1306_LCDWIDTH], SSD1306_LCDWIDTH);
        if (use_spi) {
            int ret = 0;
            if (gpio_set_value(dc_pin, 1) < 0)
                return -1;
            ret = spi_transfer(&buffer[1], NULL, sizeof(buffer) - 1);

            /* Set the D/C pin low even if the spi transfer failed */
            if (gpio_set_value(dc_pin, 0) < 0)
                return -1;
            if (ret < 0)
                return ret;
        } else {
            buffer[0] = DATA_ONLY;
            if (i2c_write(SSD1306_ADDRESS, buffer, sizeof(buffer)) < 0)
                return -1;
        }
    }

    return 0;
}

int oled_click_write_text(char *str)
{
    uint8_t data[SSD1306_PAGE_COUNT * SSD1306_LCDWIDTH];
    uint32_t str_length = 0;
    uint8_t i = 0;  /* OLED_CLICK_MAX_STR_LEN fits in a byte */

    if (str == NULL) {
        fprintf(stderr, "oled: Cannot write text using null pointer.\n");
        return -1;
    }

    str_length = (uint32_t)strlen(str);
    if (str_length > OLED_CLICK_MAX_STR_LEN) {
        fprintf(stderr, "oled: Text is too long ! Maximum text length is %u.\n", OLED_CLICK_MAX_STR_LEN);
        return -1;
    }

    memset(data, 0, sizeof(data));

    for (i = 0; i < str_length; ++i) {
        const uint8_t *bitmap = NULL;
        uint8_t j = 0;
        uint32_t offset = 0;

        /* Since data is already filled with 0, we can skip spaces */
        if (str[i] == ' ')
            continue;

        if (oled_click_get_char(str[i], &bitmap) < 0)
            return -1;

        /* Each character takes 11x16 pixels, so it requires writing on
         * two pages.
         *
         * page no
         *    0   |first line |
         *    1   |  of text  |
         *  -------------------
         *    2   |second line|
         *    3   |  of text  |
         *
         * The origin is at the top right corner.
         */
        offset = (i / CHARACTER_COUNT_PER_LINE) * PAGE_PER_LINE * SSD1306_LCDWIDTH;
        offset += SSD1306_LCDWIDTH - ((i % CHARACTER_COUNT_PER_LINE) + 1) * CHARACTER_WIDTH;

        /* Writing top half of the character */
        for (j = 0; j < CHARACTER_WIDTH; ++j)
            data[offset + j] = bitmap[j];

        /* Moving offset to next page */
        offset += SSD1306_LCDWIDTH;

        /* Writing bottom half of the character */
        for (j = 0; j < CHARACTER_WIDTH; ++j)
            data[offset + j] = bitmap[CHARACTER_WIDTH + j];
    }

    return oled_click_raw_write(data);
}

int oled_click_get_char(char c, const uint8_t **out)
{
    if (out == NULL)
        return -1;
    if (c < '!' || c > '~') {
        fprintf(stderr, "oled: Cannot convert character %c.", c);
        return -1;
    }

    *out = char_table[c - '!'];

    return 0;
}

int oled_click_disable(void)
{
    return oled_click_cmd(SSD1306_DISPLAYOFF);
}
