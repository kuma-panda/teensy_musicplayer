#include <SD.h>
#include "display.h"

// -----------------------------------------------------------------------------
//  AlphaBrender
// -----------------------------------------------------------------------------
AlphaBrender& AlphaBrend()
{
    static AlphaBrender _brender;
    return _brender;
}

// -----------------------------------------------------------------------------
//  Font
// -----------------------------------------------------------------------------
Font::Font(uint8_t height, const uint32_t *data, const uint32_t *map)
    : m_height(height), m_data(data), m_dataAA(nullptr), m_map(map), m_antialiased(false)
{

}
Font::Font(uint8_t height, const uint8_t *data, const uint32_t *map)
    : m_height(height), m_data(nullptr), m_dataAA(data), m_map(map), m_antialiased(true)
{
    Serial.print("offset of '0' : ");
    Serial.println(this->m_map['0'], HEX);
}

// -----------------------------------------------------------------------------
int16_t Font::drawChar(HX8357 *display, int16_t x, int16_t y, uint16_t code, uint16_t fgcol, uint16_t bkcol)
{
    uint32_t offset = this->m_map[code];
    if( offset < 0xFFFFFFFF )
    {
        if( this->m_antialiased )
        {
            const uint8_t *p = this->m_dataAA + offset + 1;
            int16_t width = (int16_t)this->m_dataAA[offset];
            uint16_t *image = AlphaBrend().createImage(p, width * this->m_height, fgcol, bkcol);
            display->drawBitmap(x, y, width, this->m_height, image);
            x += width;
        }
        else
        {
            const uint32_t *p = this->m_data + offset + 1;
            int16_t width = (int16_t)(this->m_data[offset] & 0xFF);
            display->drawGlyph(x, y, width, this->m_height, p, fgcol, bkcol);
            x += width;
        }
    }
    return x;
}

// -----------------------------------------------------------------------------
char *Font::getCharCodeAt(char *p, uint16_t *code)
{
    if( *p == 0 )
    {
        *code = 0x0000;
        return p;
    }
    if( (*p & 0xF0) == 0xE0 )
    {
        *code = (((uint16_t)(*p & 0x0F))<<12) | (((uint16_t)(*(p+1) & 0x3F))<<6) | ((uint16_t)(*(p+2) & 0x3F));
        if( *code == 0xFF5E ){ *code = 0x301C; }     // '～' の誤変換対策
        return p+3;
    }
    if( (*p & 0xE0) == 0xC0 )
    {
        *code = (((uint16_t)(*p & 0x1F))<<6) | ((uint16_t)(*(p+1) & 0x3F));
        return p+2;
    }
    else if( 0x20 <= *p && *p <= 0x7E )
    {
        *code = (uint16_t)*p;
        return p+1;
    }
    *code = 0x0000;
    return p+1;
}

// -----------------------------------------------------------------------------
int16_t Font::drawString(HX8357 *display, int16_t x, int16_t y, const char *text, uint16_t fgcol, uint16_t bkcol)
{
    char *p = const_cast<char *>(text);
    uint16_t code;
    while( *p )
    {
        p = Font::getCharCodeAt(p, &code);
        if( code )
        {
            x = this->drawChar(display, x, y, code, fgcol, bkcol);
        }
    }
    return x;
}

// -----------------------------------------------------------------------------
int16_t Font::getTextWidth(const char *text)
{
    char *p = const_cast<char *>(text);
    uint16_t code;
    int16_t w = 0;
    while( *p )
    {
        p = Font::getCharCodeAt(p, &code);
        if( code )
        {
            uint32_t offset = this->m_map[code];
            if( offset < 0xFFFFFFFF )
            {
                if( this->m_antialiased )
                {
                    w += (int16_t)this->m_dataAA[offset];
                }
                else
                {
                    w += (int16_t)(this->m_data[offset] & 0xFF);
                }
            }
        }
    }
    return w;
}


// -----------------------------------------------------------------------------
//  Icon
// -----------------------------------------------------------------------------
// uint16_t *Icon::m_buffer = nullptr;

Icon::Icon(const uint8_t *addr) : m_data(addr+2)
{
    this->m_width  = addr[0];
    this->m_height = addr[1];
    // if( Icon::m_buffer == nullptr )
    // {
    //     Icon::m_buffer = new uint16_t[Icon::BUFFER_SIZE];
    // }
}

// -----------------------------------------------------------------------------
void Icon::draw(HX8357 *display, int16_t x, int16_t y, uint16_t fgcol, uint16_t bkcol)
{
    int size = this->getSize();
    uint16_t *image = AlphaBrend().createImage(this->m_data, size, fgcol, bkcol);
    display->drawBitmap(x, y, this->m_width, this->m_height, image);
    // for( int i = 0 ; i < size ; i++ )
    // {
    //     Icon::m_buffer[i] = Icon::alphaBlendRGB565(fgcol, bkcol, this->m_data[i]);
    // }
    // display->drawBitmap(x, y, this->m_width, this->m_height, Icon::m_buffer);
}


// -----------------------------------------------------------------------------
//  Bitmap
// -----------------------------------------------------------------------------
Bitmap::Bitmap(uint8_t w, uint8_t h) : m_width(w), m_height(h)
{
    uint32_t size = ((uint32_t)this->m_width)*((uint32_t)this->m_height);
    this->m_data = new uint16_t[size];
}

// -----------------------------------------------------------------------------
void Bitmap::load(File& f)
{
    uint32_t size = ((uint32_t)this->m_width)*((uint32_t)this->m_height);
    f.read((uint8_t *)(this->m_data), 2*size);
}

// -----------------------------------------------------------------------------
void Bitmap::draw(HX8357 *display, int16_t x, int16_t y)
{
    display->drawBitmap(x, y, this->m_width, this->m_height, this->m_data);
}   
