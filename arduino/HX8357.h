// -----------------------------------------------------------------------------
//  HX8357.h
//  HX8357D 8bitパラレルI/Fライブラリ(For Teensy4.1)
// -----------------------------------------------------------------------------

#ifndef HX8357_H
#define HX8357_H

#include <Arduino.h>

#define HX8357_TFTWIDTH  320
#define HX8357_TFTHEIGHT 480

// #define HX8357_NOP     0x00
#define HX8357_SWRESET 0x01
// #define HX8357_RDDID   0x04
// #define HX8357_RDDST   0x09

// #define HX8357_SLPIN   0x10
#define HX8357_SLPOUT  0x11
// #define HX8357B_PTLON   0x12
// #define HX8357B_NORON   0x13

// #define HX8357_RDMODE  0x0A
// #define HX8357_RDMADCTL  0x0B
// #define HX8357_RDPIXFMT  0x0C
// #define HX8357_RDIMGFMT  0x0D
// #define HX8357_RDSELFDIAG  0x0F

// #define HX8357_INVOFF  0x20
// #define HX8357_INVON   0x21
// #define HX8357_GAMMASET 0x26
// #define HX8357_DISPOFF 0x28
#define HX8357_DISPON  0x29

#define HX8357_CASET   0x2A
#define HX8357_PASET   0x2B
#define HX8357_RAMWR   0x2C
// #define HX8357_RAMRD   0x2E

// #define HX8357B_PTLAR    0x30
#define HX8357_TEON  0x35
#define HX8357_TEARLINE  0x44
#define HX8357_MADCTL   0x36
// #define HX8357_VSCRSADD 0x37
#define HX8357_COLMOD  0x3A

#define HX8357_SETOSC 0xB0
#define HX8357_SETPWR1 0xB1
// #define HX8357B_SETDISPLAY 0xB2
#define HX8357_SETRGB 0xB3
#define HX8357D_SETCOM  0xB6

// #define HX8357B_SETDISPMODE  0xB4
#define HX8357D_SETCYC  0xB4
// #define HX8357B_SETOTP 0xB7
#define HX8357D_SETC 0xB9

// #define HX8357B_SET_PANEL_DRIVING 0xC0
#define HX8357D_SETSTBA 0xC0
// #define HX8357B_SETDGC  0xC1
// #define HX8357B_SETID  0xC3
// #define HX8357B_SETDDB  0xC4
// #define HX8357B_SETDISPLAYFRAME 0xC5
// #define HX8357B_GAMMASET 0xC8
// #define HX8357B_SETCABC  0xC9
#define HX8357_SETPANEL  0xCC

// #define HX8357B_SETPOWER 0xD0
// #define HX8357B_SETVCOM 0xD1
// #define HX8357B_SETPWRNORMAL 0xD2

// #define HX8357B_RDID1   0xDA
// #define HX8357B_RDID2   0xDB
// #define HX8357B_RDID3   0xDC
// #define HX8357B_RDID4   0xDD

// #define HX8357D_SETGAMMA 0xE0
// #define HX8357B_SETGAMMA 0xC8
// #define HX8357B_SETPANELRELATED  0xE9

#define HX8357_MADCTL_MY  0x80 ///< Bottom to top
#define HX8357_MADCTL_MX  0x40 ///< Right to left
#define HX8357_MADCTL_MV  0x20 ///< Reverse Mode
#define HX8357_MADCTL_ML  0x10 ///< LCD refresh Bottom to top
#define HX8357_MADCTL_RGB 0x00 ///< Red-Green-Blue pixel order
#define HX8357_MADCTL_BGR 0x08 ///< Blue-Green-Red pixel order
#define HX8357_MADCTL_MH  0x04 ///< LCD refresh right to left

class HX8357
{
    private:
        int16_t m_width;
        int16_t m_height;
        void init();
        void reset();
        void setAddrWindow(int16_t x1, int16_t y1, int16_t x2, int16_t y2); 
        void flood(uint16_t color, uint32_t len); 
        void setAddr(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
        void pushColors(const uint16_t *data, uint32_t len);
        static void write8(uint8_t c);

    public:
        HX8357();
        void begin();
        void setRotation(uint8_t m);
        void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
        void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
        void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
        void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
        void fillScreen(uint16_t color);
        void drawPixel(int16_t x, int16_t y, uint16_t color);
        void drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *bitmap);
        void drawGlyph(int16_t x, int16_t y, int16_t w, int16_t h, const uint32_t *glyph, uint16_t fgcol, uint16_t bkcol);

        int16_t getWidth(){ return this->m_width; }
        int16_t getHeight(){ return this->m_height; }
};

#endif