// -----------------------------------------------------------------------------
//  HX8357.cpp
//  HX8357D 8bitパラレルI/Fライブラリ(For Teensy4.1)
// -----------------------------------------------------------------------------
#include <arduino.h>
#include "HX8357.h"

/*
    ピンアサイン

    HX8357  Teensy4.1
    D0      33
    D1      34
    D2      35
    D3      36
    D4      37
    D5      38
    D6      39
    D7      40

    RST     28
    RD      29
    WR      30
    C/D     31
    CS      32
*/

// -----------------------------------------------------------------------------
#define CS_LOW      (CORE_PIN32_PORTCLEAR=CORE_PIN32_BITMASK)
#define CS_HIGH     (CORE_PIN32_PORTSET=CORE_PIN32_BITMASK)
#define CD_COMMAND  (CORE_PIN31_PORTCLEAR=CORE_PIN31_BITMASK)
#define CD_DATA     (CORE_PIN31_PORTSET=CORE_PIN31_BITMASK)
#define WR_HIGH     (CORE_PIN30_PORTSET=CORE_PIN30_BITMASK)
#define WR_LOW      (CORE_PIN30_PORTCLEAR=CORE_PIN30_BITMASK)
#define RD_HIGH     (CORE_PIN29_PORTSET=CORE_PIN29_BITMASK)
#define RESET_HIGH  (CORE_PIN28_PORTSET=CORE_PIN28_BITMASK)
#define RESET_LOW   (CORE_PIN28_PORTCLEAR=CORE_PIN28_BITMASK)

// -----------------------------------------------------------------------------
void HX8357::write8(uint8_t c)
{
    WR_LOW;
    if( c & 0x01 ){ CORE_PIN33_PORTSET = CORE_PIN33_BITMASK; } else { CORE_PIN33_PORTCLEAR = CORE_PIN33_BITMASK; }
    if( c & 0x02 ){ CORE_PIN34_PORTSET = CORE_PIN34_BITMASK; } else { CORE_PIN34_PORTCLEAR = CORE_PIN34_BITMASK; }
    if( c & 0x04 ){ CORE_PIN35_PORTSET = CORE_PIN35_BITMASK; } else { CORE_PIN35_PORTCLEAR = CORE_PIN35_BITMASK; }
    if( c & 0x08 ){ CORE_PIN36_PORTSET = CORE_PIN36_BITMASK; } else { CORE_PIN36_PORTCLEAR = CORE_PIN36_BITMASK; }
    if( c & 0x10 ){ CORE_PIN37_PORTSET = CORE_PIN37_BITMASK; } else { CORE_PIN37_PORTCLEAR = CORE_PIN37_BITMASK; }
    if( c & 0x20 ){ CORE_PIN38_PORTSET = CORE_PIN38_BITMASK; } else { CORE_PIN38_PORTCLEAR = CORE_PIN38_BITMASK; }
    if( c & 0x40 ){ CORE_PIN39_PORTSET = CORE_PIN39_BITMASK; } else { CORE_PIN39_PORTCLEAR = CORE_PIN39_BITMASK; }
    if( c & 0x80 ){ CORE_PIN40_PORTSET = CORE_PIN40_BITMASK; } else { CORE_PIN40_PORTCLEAR = CORE_PIN40_BITMASK; }
    WR_HIGH;
}

// -----------------------------------------------------------------------------
HX8357::HX8357() : m_width(HX8357_TFTWIDTH), m_height(HX8357_TFTHEIGHT)
{
}

// -----------------------------------------------------------------------------
void HX8357::begin(void)
{
    for( int i = 28 ; i <= 40 ; i++ )
    {
        pinMode(i, OUTPUT);
    }
    this->init();
}

// -----------------------------------------------------------------------------
#define TFTLCD_DELAY 0xFF
static const uint8_t HX8357D_regValues[] PROGMEM = {
    HX8357_SWRESET,
    0,
    HX8357D_SETC,
    3,
    0xFF,
    0x83,
    0x57,
    TFTLCD_DELAY,
    250,
    HX8357_SETRGB,
    4,
    0x00,
    0x00,
    0x06,
    0x06,
    HX8357D_SETCOM,
    1,
    0x25, // -1.52V
    HX8357_SETOSC,
    1,
    0x68, // Normal mode 70Hz, Idle mode 55 Hz
    HX8357_SETPANEL,
    1,
    0x05, // BGR, Gate direction swapped
    HX8357_SETPWR1,
    6,
    0x00,
    0x15,
    0x1C,
    0x1C,
    0x83,
    0xAA,
    HX8357D_SETSTBA,
    6,
    0x50,
    0x50,
    0x01,
    0x3C,
    0x1E,
    0x08,
    // MEME GAMMA HERE
    HX8357D_SETCYC,
    7,
    0x02,
    0x40,
    0x00,
    0x2A,
    0x2A,
    0x0D,
    0x78,
    HX8357_COLMOD,
    1,
    0x55,
    HX8357_MADCTL,
    1,
    0xC0,
    HX8357_TEON,
    1,
    0x00,
    HX8357_TEARLINE,
    2,
    0x00,
    0x02,
    HX8357_SLPOUT,
    0,
    TFTLCD_DELAY,
    150,
    HX8357_DISPON,
    0,
    TFTLCD_DELAY,
    50,
};

// -----------------------------------------------------------------------------
void HX8357::init() 
{
    reset();

    delay(200);

    CS_LOW;
    RD_HIGH;

    uint8_t i = 0;
    while( i < sizeof(HX8357D_regValues) ) 
    {
        uint8_t r = HX8357D_regValues[i++];
        uint8_t len = HX8357D_regValues[i++];
        if (r == TFTLCD_DELAY) 
        {
            delay(len);
        } 
        else 
        {
            CD_COMMAND;
            write8(r);
            CD_DATA;
            for (uint8_t d = 0; d < len; d++) 
            {
                uint8_t x = HX8357D_regValues[i++];
                write8(x);
            }
        }
    }
}

// -----------------------------------------------------------------------------
void HX8357::reset(void) 
{
    CS_HIGH;
    WR_HIGH;
    RD_HIGH;

    RESET_LOW;
    delay(2);
    RESET_HIGH;

    // Data transfer sync
    CS_LOW;
    CD_COMMAND;
    write8(0x00);
    for (uint8_t i = 0; i < 3; i++)
    {
        WR_LOW; WR_LOW; WR_LOW; WR_HIGH; WR_HIGH; WR_HIGH;
    }
}

// -----------------------------------------------------------------------------
void HX8357::setAddrWindow(int16_t x1, int16_t y1, int16_t x2, int16_t y2) 
{
    CD_COMMAND;
    write8(HX8357_CASET);
    CD_DATA;
    write8((uint8_t)(x1 >> 8));
    write8((uint8_t)(x1 & 0xFF));
    write8((uint8_t)(x2 >> 8));
    write8((uint8_t)(x2 & 0xFF));

    CD_COMMAND;
    write8(HX8357_PASET);
    CD_DATA;
    write8((uint8_t)(y1 >> 8));
    write8((uint8_t)(y1 & 0xFF));
    write8((uint8_t)(y2 >> 8));
    write8((uint8_t)(y2 & 0xFF));
}

// -----------------------------------------------------------------------------
// Fast block fill operation for fillScreen, fillRect, H/V line, etc.
// Requires setAddrWindow() has previously been called to set the fill
// bounds.  'len' is inclusive, MUST be >= 1.
void HX8357::flood(uint16_t color, uint32_t len) 
{
    uint8_t hi = (uint8_t)(color >> 8);
    uint8_t lo = (uint8_t)(color & 0xFF);

    CD_COMMAND;
    write8(HX8357_RAMWR);
    CD_DATA;
    write8(hi);
    write8(lo);
    len--;

    uint16_t blocks = (uint16_t)(len / 64); // 64 pixels/block
    if( hi == lo ) 
    {
        // High and low bytes are identical.  Leave prior data
        // on the port(s) and just toggle the write strobe.
        while( blocks-- ) 
        {
            int i = 16; // 64 pixels/block / 4 pixels/pass
            do 
            {
                // ストローブパルスを８回出す。
                // LOW/HIGH の繰り返しではパルスサイクルが速すぎるので、以下のようにする
                WR_LOW; WR_LOW; WR_LOW; WR_HIGH; WR_HIGH; WR_HIGH;
                WR_LOW; WR_LOW; WR_LOW; WR_HIGH; WR_HIGH; WR_HIGH;
                WR_LOW; WR_LOW; WR_LOW; WR_HIGH; WR_HIGH; WR_HIGH;
                WR_LOW; WR_LOW; WR_LOW; WR_HIGH; WR_HIGH; WR_HIGH;
                WR_LOW; WR_LOW; WR_LOW; WR_HIGH; WR_HIGH; WR_HIGH;
                WR_LOW; WR_LOW; WR_LOW; WR_HIGH; WR_HIGH; WR_HIGH;
                WR_LOW; WR_LOW; WR_LOW; WR_HIGH; WR_HIGH; WR_HIGH;
                WR_LOW; WR_LOW; WR_LOW; WR_HIGH; WR_HIGH; WR_HIGH;
            }
            while( --i );
        }
        // Fill any remaining pixels (1 to 64)
        for( uint8_t i = (uint8_t)(len & 63) ; i-- ; ) 
        {
            WR_LOW; WR_LOW; WR_LOW; WR_HIGH; WR_HIGH; WR_HIGH;
            WR_LOW; WR_LOW; WR_LOW; WR_HIGH; WR_HIGH; WR_HIGH;
        }
    } 
    else 
    {
        while( blocks-- ) 
        {
            int i = 16; // 64 pixels/block / 4 pixels/pass
            do 
            {
                write8(hi);
                write8(lo);
                write8(hi);
                write8(lo);
                write8(hi);
                write8(lo);
                write8(hi);
                write8(lo);
            } 
            while( --i );
        }
        for( uint8_t i = (uint8_t)(len & 63) ; i-- ; ) 
        {
            write8(hi);
            write8(lo);
        }
    }
}

// -----------------------------------------------------------------------------
void HX8357::drawFastHLine(int16_t x, int16_t y, int16_t length, uint16_t color) 
{
    int16_t x2;

    // Initial off-screen clipping
    if ((length <= 0) || (y < 0) || (y >= m_height) || (x >= m_width) ||
        ((x2 = (x + length - 1)) < 0))
        return;

    if (x < 0) { // Clip left
        length += x;
        x = 0;
    }
    if (x2 >= m_width) { // Clip right
        x2 = m_width - 1;
        length = x2 - x + 1;
    }

    setAddrWindow(x, y, x2, y);
    flood(color, length);
}

// -----------------------------------------------------------------------------
void HX8357::drawFastVLine(int16_t x, int16_t y, int16_t length, uint16_t color) 
{
    int16_t y2;

    // Initial off-screen clipping
    if ((length <= 0) || (x < 0) || (x >= m_width) || (y >= m_height) ||
        ((y2 = (y + length - 1)) < 0))
        return;
    if (y < 0) { // Clip top
        length += y;
        y = 0;
    }
    if (y2 >= m_height) { // Clip bottom
        y2 = m_height - 1;
        length = y2 - y + 1;
    }

    setAddrWindow(x, y, x, y2);
    flood(color, length);
}

// -----------------------------------------------------------------------------
void HX8357::drawRect(int16_t x1, int16_t y1, int16_t w, int16_t h, uint16_t color)
{
    int16_t x2, y2;

    // Initial off-screen clipping
    if ((w <= 0) || (h <= 0) || (x1 >= m_width) || (y1 >= m_height) ||
        ((x2 = x1 + w - 1) < 0) || ((y2 = y1 + h - 1) < 0))
        return;
    if (x1 < 0) { // Clip left
        w += x1;
        x1 = 0;
    }
    if (y1 < 0) { // Clip top
        h += y1;
        y1 = 0;
    }
    if (x2 >= m_width) { // Clip right
        x2 = m_width - 1;
        w = x2 - x1 + 1;
    }
    if (y2 >= m_height) { // Clip bottom
        y2 = m_height - 1;
        h = y2 - y1 + 1;
    }

    drawFastHLine(x1, y1, w, color);
    drawFastHLine(x1, y2, w, color);
    drawFastVLine(x1, y1, h, color);
    drawFastVLine(x2, y1, h, color);
}

// -----------------------------------------------------------------------------
void HX8357::fillRect(int16_t x1, int16_t y1, int16_t w, int16_t h, uint16_t fillcolor) 
{
    int16_t x2, y2;

    // Initial off-screen clipping
    if ((w <= 0) || (h <= 0) || (x1 >= m_width) || (y1 >= m_height) ||
        ((x2 = x1 + w - 1) < 0) || ((y2 = y1 + h - 1) < 0))
        return;
    if (x1 < 0) { // Clip left
        w += x1;
        x1 = 0;
    }
    if (y1 < 0) { // Clip top
        h += y1;
        y1 = 0;
    }
    if (x2 >= m_width) { // Clip right
        x2 = m_width - 1;
        w = x2 - x1 + 1;
    }
    if (y2 >= m_height) { // Clip bottom
        y2 = m_height - 1;
        h = y2 - y1 + 1;
    }

    setAddrWindow(x1, y1, x2, y2);
    flood(fillcolor, (uint32_t)w * (uint32_t)h);
}

// -----------------------------------------------------------------------------
void HX8357::fillScreen(uint16_t color) 
{
    setAddrWindow(0, 0, m_width - 1, m_height - 1);
    flood(color, (long)HX8357_TFTWIDTH * (long)HX8357_TFTHEIGHT);
}

// -----------------------------------------------------------------------------
void HX8357::drawPixel(int16_t x, int16_t y, uint16_t color) 
{
    // Clip
    if ((x < 0) || (y < 0) || (x >= m_width) || (y >= m_height))
        return;

    setAddrWindow(x, y, m_width - 1, m_height - 1);
    CD_COMMAND;
    write8(HX8357_RAMWR);
    CD_DATA;
    write8((uint8_t)(color >> 8));
    write8((uint8_t)(color & 0xFF));
}

// -----------------------------------------------------------------------------
void HX8357::pushColors(const uint16_t *data, uint32_t len) 
{
    CD_COMMAND;
    write8(HX8357_RAMWR);
    CD_DATA;
    for( uint32_t i = 0 ; i < len ; i++ )
    {
        write8((uint8_t)(data[i] >> 8));
        write8((uint8_t)(data[i] & 0xFF));
    }
}

// -----------------------------------------------------------------------------
void HX8357::setRotation(uint8_t x) 
{
    uint8_t rotation = x & 3;
    uint16_t t = 0;

    switch(rotation) 
    {
        case 2:
            t = HX8357_MADCTL_RGB;
            this->m_width  = HX8357_TFTWIDTH;
            this->m_height = HX8357_TFTHEIGHT;
            break;
        case 3:
            t = HX8357_MADCTL_MX | HX8357_MADCTL_MV | HX8357_MADCTL_RGB;
            this->m_width  = HX8357_TFTHEIGHT;
            this->m_height = HX8357_TFTWIDTH;
            break;
        case 0:
            t = HX8357_MADCTL_MX | HX8357_MADCTL_MY | HX8357_MADCTL_RGB;
            this->m_width  = HX8357_TFTWIDTH;
            this->m_height = HX8357_TFTHEIGHT;
            break;
        case 1:
            t = HX8357_MADCTL_MV | HX8357_MADCTL_BGR;
            // t = HX8357B_MADCTL_MY | HX8357B_MADCTL_MV | HX8357B_MADCTL_RGB;
            this->m_width  = HX8357_TFTHEIGHT;
            this->m_height = HX8357_TFTWIDTH;
            break;
    }
    CD_COMMAND;
    write8(HX8357_MADCTL);
    CD_DATA;
    write8(t);
    // For 8357, init default full-screen address window:
    setAddrWindow(0, 0, m_width - 1, m_height - 1);
}

// -----------------------------------------------------------------------------
void HX8357::drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *bitmap)
{
    uint32_t len = ((uint32_t)w) * ((uint32_t)h);
    setAddrWindow(x, y, x+w-1, y+h-1);
    pushColors(bitmap, len);
}

// -----------------------------------------------------------------------------
void HX8357::drawGlyph(int16_t x, int16_t y, int16_t w, int16_t h, const uint32_t *glyph, uint16_t fgcol, uint16_t bkcol)
{
    uint16_t buffer[32];

    for( int16_t i = 0 ; i < w ; i++ )
    {
        if( i+x < 0 ){ continue; }
        if( i+x >= this->m_width ){ break; }
        for( int16_t j = 0 ; j < h ; j++ )
        {
            buffer[j] = (glyph[i] & (1 << j))? fgcol : bkcol;
        }
        setAddrWindow(x+i, y, x+i, y+h-1);
        pushColors(buffer, h);
    }
}
