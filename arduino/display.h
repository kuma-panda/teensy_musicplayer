#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <SD.h>
#include "HX8357.h"
#include "algorithm.h"

// -----------------------------------------------------------------------------
// 色定義(RGB565)
#define   COLOR_WHITE                   0xFFFF
#define   COLOR_SNOW                    0xFFDE
#define   COLOR_GHOSTWHITE              0xF7BF
#define   COLOR_IVORY                   0xFFFD
#define   COLOR_MINTCREAM               0xF7FE
#define   COLOR_AZURE                   0xEFFF
#define   COLOR_FLORALWHITE             0xFFDD
#define   COLOR_ALICEBLUE               0xEFBF
#define   COLOR_LAVENDERBLUSH           0xFF7E
#define   COLOR_SEASHELL                0xFFBD
#define   COLOR_WHITESMOKE              0xF7BE
#define   COLOR_HONEYDEW                0xEFFD
#define   COLOR_LIGHTYELLOW             0xFFFB
#define   COLOR_LIGHTCYAN               0xDFFF
#define   COLOR_OLDLACE                 0xFFBC
#define   COLOR_CORNSILK                0xFFBB
#define   COLOR_LINEN                   0xF77C
#define   COLOR_LEMONCHIFFON            0xFFD9
#define   COLOR_LIGHTGOLDENRODYELLOW    0xF7D9
#define   COLOR_BEIGE                   0xF7BB
#define   COLOR_LAVENDER                0xE73E
#define   COLOR_MISTYROSE               0xFF1B
#define   COLOR_PAPAYAWHIP              0xFF7A
#define   COLOR_ANTIQUEWHITE            0xF75A
#define   COLOR_BLANCHEDALMOND          0xFF59
#define   COLOR_BISQUE                  0xFF18
#define   COLOR_MOCCASIN                0xFF16
#define   COLOR_GAINSBORO               0xDEDB
#define   COLOR_PEACHPUFF               0xFED6
#define   COLOR_PALETURQUOISE           0xAF7D
#define   COLOR_NAVAJOWHITE             0xFEF5
#define   COLOR_PINK                    0xFDF9
#define   COLOR_WHEAT                   0xF6F6
#define   COLOR_PALEGOLDENROD           0xEF34
#define   COLOR_LIGHTGREY               0xD69A
#define   COLOR_LIGHTPINK               0xFDB7
#define   COLOR_POWDERBLUE              0xAEFC
#define   COLOR_THISTLE                 0xD5FA
#define   COLOR_LIGHTBLUE               0xAEBC
#define   COLOR_KHAKI                   0xEF31
#define   COLOR_VIOLET                  0xEC1D
#define   COLOR_PLUM                    0xDCFB
#define   COLOR_LIGHTSTEELBLUE          0xAE1B
#define   COLOR_AQUAMARINE              0x7FFA
#define   COLOR_LIGHTSKYBLUE            0x867E
#define   COLOR_SILVER                  0xBDF7
#define   COLOR_SKYBLUE                 0x867D
#define   COLOR_PALEGREEN               0x97D2
#define   COLOR_ORCHID                  0xD37A
#define   COLOR_BURLYWOOD               0xDDB0
#define   COLOR_HOTPINK                 0xFB56
#define   COLOR_LIGHTSALMON             0xFCEE
#define   COLOR_TAN                     0xCD91
#define   COLOR_LIGHTGREEN              0x8F71
#define   COLOR_YELLOW                  0xFFE0
#define   COLOR_FUCHSIA                 0xF81F
#define   COLOR_MAGENTA                 0xF81F
#define   COLOR_AQUA                    0x07FF
#define   COLOR_CYAN                    0x07FF
#define   COLOR_DARKGRAY                0xA554
#define   COLOR_DARKSALMON              0xE4AE
#define   COLOR_SANDYBROWN              0xF50B
#define   COLOR_LIGHTCORAL              0xEBEF
#define   COLOR_TURQUOISE               0x3EF9
#define   COLOR_SALMON                  0xF3ED
#define   COLOR_CORNFLOWERBLUE          0x64BD
#define   COLOR_MEDIUMTURQUOISE         0x4699
#define   COLOR_MEDIUMORCHID            0xB2BA
#define   COLOR_DARKKHAKI               0xBDAD
#define   COLOR_PALEVIOLETRED           0xDB72
#define   COLOR_MEDIUMPURPLE            0x937B
#define   COLOR_MEDIUMAQUAMARINE        0x6674
#define   COLOR_GREENYELLOW             0xAFE5
#define   COLOR_ROSYBROWN               0xBC71
#define   COLOR_DARKSEAGREEN            0x8DD1
#define   COLOR_GOLD                    0xFEA0
#define   COLOR_MEDIUMSLATEBLUE         0x7B3D
#define   COLOR_CORAL                   0xFBE9
#define   COLOR_DEEPSKYBLUE             0x05FF
#define   COLOR_DODGERBLUE              0x1C7F
#define   COLOR_TOMATO                  0xFB08
#define   COLOR_DEEPPINK                0xF892
#define   COLOR_ORANGE                  0xFD20
#define   COLOR_GOLDENROD               0xD523
#define   COLOR_DARKTURQUOISE           0x0679
#define   COLOR_CADETBLUE               0x5CF3
#define   COLOR_YELLOWGREEN             0x9665
#define   COLOR_LIGHTSLATEGRAY          0x7432
#define   COLOR_DARKORCHID              0x9199
#define   COLOR_BLUEVIOLET              0x815B
#define   COLOR_MEDIUMSPRINGGREEN       0x07D2
#define   COLOR_PERU                    0xCC27
#define   COLOR_SLATEBLUE               0x62D9
#define   COLOR_DARKORANGE              0xFC40
#define   COLOR_ROYALBLUE               0x3B5B
#define   COLOR_INDIANRED               0xCACB
#define   COLOR_GRAY                    0x7BEF
#define   COLOR_SLATEGRAY               0x6BF1
#define   COLOR_CHARTREUSE              0x7FE0
#define   COLOR_SPRINGGREEN             0x07EF
#define   COLOR_STEELBLUE               0x4416
#define   COLOR_LIGHTSEAGREEN           0x1D94
#define   COLOR_LAWNGREEN               0x7FC0
#define   COLOR_DARKVIOLET              0x901A
#define   COLOR_MEDIUMVIOLETRED         0xC0B0
#define   COLOR_MEDIUMSEAGREEN          0x3D8D
#define   COLOR_CHOCOLATE               0xCB43
#define   COLOR_DARKGOLDENROD           0xB421
#define   COLOR_ORANGERED               0xFA20
#define   COLOR_DIMGRAY                 0x634C
#define   COLOR_LIMEGREEN               0x2E65
#define   COLOR_CRIMSON                 0xD887
#define   COLOR_SIENNA                  0x9A85
#define   COLOR_OLIVEDRAB               0x6C64
#define   COLOR_DARKMAGENTA             0x8811
#define   COLOR_DARKCYAN                0x0451
#define   COLOR_DARKSLATEBLUE           0x41F1
#define   COLOR_SEAGREEN                0x2C4A
#define   COLOR_OLIVE                   0x7BE0
#define   COLOR_PURPLE                  0x780F
#define   COLOR_TEAL                    0x03EF
#define   COLOR_RED                     0xF800
#define   COLOR_LIME                    0x07E0
#define   COLOR_BLUE                    0x001F
#define   COLOR_BROWN                   0xA144
#define   COLOR_FIREBRICK               0xA903
#define   COLOR_DARKOLIVEGREEN          0x5345
#define   COLOR_SADDLEBROWN             0x8A22
#define   COLOR_FORESTGREEN             0x1C43
#define   COLOR_INDIGO                  0x480F
#define   COLOR_DARKSLATEGRAY           0x2A69
#define   COLOR_MEDIUMBLUE              0x0019
#define   COLOR_MIDNIGHTBLUE            0x10CD
#define   COLOR_DARKRED                 0x8800
#define   COLOR_DARKBLUE                0x0011
#define   COLOR_MAROON                  0x7800
#define   COLOR_GREEN                   0x03E0
#define   COLOR_NAVY                    0x000F
#define   COLOR_DARKGREEN               0x0300
#define   COLOR_BLACK                   0x0000

// -----------------------------------------------------------------------------
// Point
class Point
{
    public:
        int16_t x;
        int16_t y;
        Point() : x(0), y(0){}
        Point(int16_t xx, int16_t yy) : x(xx), y(yy){}
        Point clone(){
            return Point(x, y);
        }
        void setPoint(int16_t xx, int16_t yy){
            x = xx;
            y = yy;
        }
        const Point& offset(int16_t dx, int16_t dy){
            x += dx;
            y += dy;
            return *this;
        }
};

//------------------------------------------------------------------------------
//  Rect
class Rect
{
    public:
        int16_t left;
        int16_t top;
        int16_t width;
        int16_t height;
        Rect() : left(0), top(0), width(0), height(0){}
        Rect(int16_t l, int16_t t, int16_t w, int16_t h) : left(l), top(t), width(w), height(h){}
        Rect clone(){
            return Rect(left, top, width, height);
        }
        void setRect(int16_t l, int16_t t, int16_t w, int16_t h){
            left = l;
            top = t;
            width = w;
            height = h;
        }
        Point topLeft(){ return Point(left, top); }
        Point bottomRight(){ return Point(left+width-1, top+height-1); }
        Point center(){ return Point(left+width/2, top+height/2); }
        bool include(int16_t x, int16_t y){
            return (left <= x) && (x < left+width) && (top <= y) && (y < top+height);
        }
        bool include(const Point& pt){
            return include(pt.x, pt.y);
        }
        Rect& move(int16_t x, int16_t y){
            left = x;
            top = y;
            return *this;
        }
        Rect& move(Point& pt){ return move(pt.x, pt.y); }
        Rect& offset(int16_t dx, int16_t dy){
            left += dx;
            top += dy;
            return *this;
        }
        Rect& inflate(int16_t dx, int16_t dy){
            left -= dx;
            top -= dy;
            width += (dx*2);
            height += (dy*2);
            return *this;
        }
        Rect& resize(int16_t w, int16_t h){
            width = w;
            height = h;
            return *this;
        }
        Rect& resizeWidth(int16_t w){
            width = w;
            return *this;
        }
        Rect& resizeHeight(int16_t h){
            height = h;
            return *this;
        }
        Rect& setCenter(int16_t x, int16_t y){
            left = x - width/2;
            top = y - height/2;
            return *this;
        }
        Rect& setCenter(Point& pt){ return setCenter(pt.x, pt.y); }
};

// -----------------------------------------------------------------------------
class AlphaBrender
{
    friend AlphaBrender& AlphaBrend();
    private:
        enum{BUFFER_SIZE = 48*48};
        uint16_t m_buffer[BUFFER_SIZE];
        AlphaBrender(){}
    public:
        uint16_t *createImage(const uint8_t *source, int16_t size, uint16_t fgcol, uint16_t bkcol){
            for( int16_t i = 0 ; i < size ; i++ )
            {
                this->m_buffer[i] = alphaBlendRGB565(fgcol, bkcol, source[i]);
            }
            return this->m_buffer;
        }
        static uint16_t alphaBlendRGB565(uint32_t fg, uint32_t bg, uint8_t alpha) __attribute__((always_inline)) {
            alpha = ( alpha + 4 ) >> 3; // from 0-255 to 0-31
            bg = (bg | (bg << 16)) & 0b00000111111000001111100000011111;
            fg = (fg | (fg << 16)) & 0b00000111111000001111100000011111;
            uint32_t result = ((((fg - bg) * alpha) >> 5) + bg) & 0b00000111111000001111100000011111;
            return (uint16_t)((result >> 16) | result); // contract result
        }
};
AlphaBrender& AlphaBrend();

// -----------------------------------------------------------------------------
// Font
//  フォントを表すクラス
class Font
{
    private:
        uint8_t         m_height;       // 文字高さ(px)
        const uint32_t *m_data;         // グリフデータの配列(非アンチエイリアス)
        const uint8_t  *m_dataAA;       // 同(アンチエイリアス)
        const uint32_t *m_map;          // 文字コードからグリフデータ配列のオフセットを得るためのマップデータ
        bool            m_antialiased;  // アンチエイリアスフォントの場合は true
        static char *getCharCodeAt(char *p, uint16_t *code);
    public:
        Font(uint8_t height, const uint32_t *data, const uint32_t *map);
        Font(uint8_t height, const uint8_t *data, const uint32_t *map);
        uint8_t getHeight(){ return this->m_height; }
        int16_t drawChar(HX8357 *display, int16_t x, int16_t y, uint16_t code, uint16_t fgcol, uint16_t bkcol);
        int16_t drawString(HX8357 *display, int16_t x, int16_t y, const char *text, uint16_t fgcol, uint16_t bkcol);
        int16_t getTextWidth(const char *text);
};

// -----------------------------------------------------------------------------
// Icon
//  単一色で描画される画像を扱うクラス
//  各ピクセルは8bitのアルファ値を表す(0～255)
class Icon
{
    private:
        // enum{BUFFER_SIZE = 48*48};
        // static uint16_t *m_buffer;
        uint8_t m_width;
        uint8_t m_height;
        const uint8_t *m_data;
        int getSize(){ return (int)m_width * (int)m_height; }

    public:
        Icon(const uint8_t *addr);
        uint8_t getWidth(){ return this->m_width; }
        uint8_t getHeight(){ return this->m_height; }
        const uint8_t *getData(){ return this->m_data; }
        void draw(HX8357 *display, int16_t x, int16_t y, uint16_t fgcol, uint16_t bkcol);                
};

// -----------------------------------------------------------------------------
// Bitmap
//  16bit(RGB565)形式で保存されたピクセルデータを読み込んで使うためのクラス
class Bitmap
{
    private:
        uint8_t m_width;
        uint8_t m_height;
        uint16_t *m_data;
    public:
        Bitmap(uint8_t w, uint8_t h);
        // void load(const char *path);
        void load(File& f);
        uint8_t getWidth(){ return this->m_width; }
        uint8_t getHeight(){ return this->m_height; }
        uint16_t *getData(){ return this->m_data; }     
        void draw(HX8357 *display, int16_t x, int16_t y);   
};




#endif
