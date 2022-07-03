#include <Arduino.h>
#include <SD.h>
#include <W25Q64.h>
#include "view.h"
#include "display.h"
#include "font_17aa.h"
#include "font_20aa.h"
#include "digits.h"
#include "icon.h"
#include "music_player.h"

#ifndef MIN
#define MIN(a,b)    (((a)<(b))? (a):(b))
#endif


// =============================================================================
//   TouchManager
// =============================================================================
TouchManager::TouchManager(TouchScreen *touch) : m_touchScreen(touch), m_touched(false), m_waitUntil(0)
{

}

// -----------------------------------------------------------------------------
void TouchManager::execute(UIWidget *listener)
{
    if( millis() <= this->m_waitUntil )
    {
        return;
    }

    TSPoint p = this->m_touchScreen->getPoint();
    Point dp;
    if( p.z > this->m_touchScreen->pressureThreshhold ) 
    {
        if( this->convertPosition(p, &dp) )
        {
            if( !this->m_touched )
            {
                this->m_touched = true;
                Serial.print("touched (");
                Serial.print(dp.x, DEC);
                Serial.print(", ");
                Serial.print(dp.y, DEC);
                Serial.println(")");
                TouchEvent e(true, dp.x, dp.y);
                listener->handleTouchEvent(e);
                this->m_waitUntil = millis() + 200;
            }
        }
    }
    else
    {
        if( this->m_touched )
        {
            this->m_touched = false;
            Serial.println("released");
            TouchEvent e(false);
            listener->handleTouchEvent(e);
            this->m_waitUntil = millis() + 1000;
        }
    }
}


// =============================================================================
//  Graphics
// =============================================================================
Font *Graphics::m_font[2] = {nullptr, nullptr};

Graphics::Graphics(HX8357 *display, Rect rc) : m_display(display), m_clipRect(rc),
    m_fillColor(COLOR_BLACK), m_strokeColor(COLOR_WHITE), m_fontColor(COLOR_WHITE)
{
    Serial.println("Graphics +");
    if( Graphics::m_font[0] == nullptr )
    {
        Graphics::m_font[Graphics::SMALL_FONT] = new Font(17, FONT_17AA, FONTMAP_17AA);
        Graphics::m_font[Graphics::LARGE_FONT] = new Font(20, FONT_20AA, FONTMAP_20AA);
    }
    Serial.println("Graphics -");
}

void Graphics::beginPaint()
{
    // this->m_display->setClipRect(this->m_clipRect.left, this->m_clipRect.top, this->m_clipRect.width, this->m_clipRect.height);
}

void Graphics::endPaint()
{
    // this->m_display->setClipRect();
}

void Graphics::clear()
{
    this->fillRect(this->m_clipRect);
}

void Graphics::fillRect(int16_t left, int16_t top, int16_t width, int16_t height)
{
    this->fillRect(Rect(left, top, width, height));
}

void Graphics::fillRect(Rect rc)
{
    rc = this->toScreenCoord(rc);
    this->m_display->fillRect(rc.left, rc.top, rc.width, rc.height, this->m_fillColor);
}

void Graphics::drawRect(int16_t left, int16_t top, int16_t width, int16_t height)
{
    this->drawRect(Rect(left, top, width, height));
}

void Graphics::drawRect(Rect rc)
{
    rc = this->toScreenCoord(rc);
    this->m_display->drawRect(rc.left, rc.top, rc.width, rc.height, this->m_strokeColor);
}

void Graphics::drawHzLine(int16_t x, int16_t y, int16_t length)
{
    Point pt = this->toScreenCoord(Point(x, y));
    this->m_display->drawFastHLine(pt.x, pt.y, length, this->m_strokeColor);
}

void Graphics::drawVtLine(int16_t x, int16_t y, int16_t length)
{
    Point pt = this->toScreenCoord(Point(x, y));
    this->m_display->drawFastVLine(pt.x, pt.y, length, this->m_strokeColor);
}

// void Graphics::drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
// {
//     Point pt1 = this->toScreenCoord(Point(x1, y1));
//     Point pt2 = this->toScreenCoord(Point(x2, y2));
//     this->m_display->drawLine(pt1.x, pt1.y, pt2.x, pt2.y, this->m_strokeColor);
// }

void Graphics::drawText(int16_t x, int16_t y, const char *text, uint8_t alignment)
{
    Point pt = this->toScreenCoord(Point(x, y));
    Font *font = Graphics::m_font[this->m_fontIndex];
    int16_t len = font->getTextWidth(text);
    switch( alignment & Graphics::HZALIGN_MASK )
    {
        case Graphics::ALIGN_CENTER:
            pt.x = pt.x - len/2;
            break;
        case Graphics::ALIGN_RIGHT:
            pt.x = pt.x - len;
            break;
    }
    switch( alignment & Graphics::VTALIGN_MASK )
    {
        case Graphics::ALIGN_MIDDLE:
            pt.y = pt.y - font->getHeight()/2;
            break;
        case Graphics::ALIGN_BOTTOM:
            pt.y = pt.y - font->getHeight();
            break;
    }
    font->drawString(this->m_display, pt.x, pt.y, text, this->m_fontColor, this->m_fillColor);
}

void Graphics::drawText(Rect rc, const char *text, uint8_t alignment)
{
    rc = this->toScreenCoord(rc);
    Font *font = Graphics::m_font[this->m_fontIndex];
    int16_t len = font->getTextWidth(text);
    int16_t x, y;
    switch( alignment & Graphics::HZALIGN_MASK )
    {
        case Graphics::ALIGN_CENTER:
            x = rc.center().x - len/2;
            break;
        case Graphics::ALIGN_RIGHT:
            x = rc.bottomRight().x - len;
            break;
        default:
            x = rc.left;
            break;
    }
    switch( alignment & Graphics::VTALIGN_MASK )
    {
        case Graphics::ALIGN_MIDDLE:
            y = rc.center().y - font->getHeight()/2;
            break;
        case Graphics::ALIGN_BOTTOM:
            y = rc.bottomRight().y - font->getHeight();
            break;
        default:
            y = rc.top;
            break;
    }
    font->drawString(this->m_display, x, y, text, this->m_fontColor, this->m_fillColor);
}


void Graphics::drawIcon(int16_t x, int16_t y, Icon *icon, uint16_t fgcol, uint16_t bkcol)
{
    Point pt = this->toScreenCoord(Point(x, y));
    icon->draw(this->m_display, pt.x, pt.y, fgcol, bkcol);
}

void Graphics::drawBitmap(int16_t x, int16_t y, Bitmap *bitmap)
{
    Point pt = this->toScreenCoord(Point(x, y));
    bitmap->draw(this->m_display, pt.x, pt.y);
}

void Graphics::drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *image)
{
    Point pt = this->toScreenCoord(Point(x, y));
    this->m_display->drawBitmap(pt.x, pt.y, w, h, image);
}


// =============================================================================
//  UIWidget
//  すべてのウィジェットの基本クラス
// =============================================================================

// -----------------------------------------------------------------------------
//  コンストラクタ
// -----------------------------------------------------------------------------
UIWidget::UIWidget(uint16_t id, UIWidget *parent, HX8357 *display, int16_t left, int16_t top, uint16_t width, uint16_t height)
    : m_id(id), m_parent(parent), m_position(Point(left, top)), 
    m_width(width), m_height(height), m_visible(true), m_captured(false)
{
    Serial.println("UIWidget +");

    if( this->m_parent )
    {
        this->m_parent->m_children.add(this);
    }
    Rect rc = this->clientToScreen(Rect(0, 0, width, height));
    this->m_graphics = new Graphics(display, rc);
    Serial.println("UIWidget -");
}

// -----------------------------------------------------------------------------
//  クライアント領域を表す矩形を取得する
// -----------------------------------------------------------------------------
Rect UIWidget::getClientRect()
{
    return Rect(0, 0, this->m_width, this->m_height);
}

// -----------------------------------------------------------------------------
//  クライアント座標（自身の左上隅を(0, 0)とする座標）を画面座標へ変換する
// -----------------------------------------------------------------------------
Point UIWidget::clientToScreen(Point pt)
{
    pt.x += this->m_position.x;
    pt.y += this->m_position.y;
    if( this->m_parent == nullptr )
    {
        return pt;
    }
    return this->m_parent->clientToScreen(pt);
}

// -----------------------------------------------------------------------------
Rect UIWidget::clientToScreen(Rect rc)
{
    rc.offset(this->m_position.x, this->m_position.y);
    if( this->m_parent == nullptr )
    {
        return rc;
    }
    return this->m_parent->clientToScreen(rc);
}

// -----------------------------------------------------------------------------
//  画面座標をクライアント座標に変換する
// -----------------------------------------------------------------------------
Point UIWidget::screenToClient(Point pt)
{
    Point delta = this->clientToScreen(Point(0, 0));
    pt.x -= delta.x;
    pt.y -= delta.y;
    return pt;
}

// -----------------------------------------------------------------------------
//  (x, y)がクライアント領域内の点かどうか？
// -----------------------------------------------------------------------------
bool UIWidget::contains(int16_t x, int16_t y)
{
    if( !this->isVisible() )
    {
        return false;
    }
    Rect rc = this->clientToScreen(Rect(0, 0, this->m_width, this->m_height));
    return rc.include(x, y);
}

// ------------------------------------------------------------------------------
//  タッチイベントの処理
//  e: タッチイベント
//  戻り値 : このイベントを処理した場合は true，そうでなければ false
// ------------------------------------------------------------------------------
bool UIWidget::handleTouchEvent(TouchEvent e)
{
    // 最初に子ウィジェットに処理させてみる
    bool handled = this->m_children.forEach([](int n, void *value, void *param){
        UIWidget *child = (UIWidget *)value;
        TouchEvent *event = (TouchEvent *)param;
        return !child->handleTouchEvent(*event);
    }, &e);
    if( handled )
    {
        return true;
    }

    if( !this->isVisible() )
    {
        return false;
    }

    if( e.touched )
    {
        if( this->contains(e.pos.x, e.pos.y) )
        {
            Point pt = this->screenToClient(e.pos);
            this->m_captured = true;
            this->onTouched(pt.x, pt.y);
            return true;
        }
    }
    else
    {
        if( this->m_captured )
        {
            this->m_captured = false;
            this->onReleased();
            return true;
        }
    }
    return false;
}

// ------------------------------------------------------------------------------
//  タッチされた時の処理
//  (派生クラスでオーバーライド)
//  (x, y) : タッチ位置の座標（クライアント座標単位）
// ------------------------------------------------------------------------------
void UIWidget::onTouched(int16_t x, int16_t y)
{
}

// ------------------------------------------------------------------------------
//  離された時の処理
//  (派生クラスでオーバーライド)
// ------------------------------------------------------------------------------
void UIWidget::onReleased()
{
}

//------------------------------------------------------------------------------
//  可視状態にする
//  (派生クラスでオーバーライド)
//------------------------------------------------------------------------------
void UIWidget::show()
{
    this->m_visible = true;
}

//------------------------------------------------------------------------------
//  不可視にする
//  (派生クラスでオーバーライド)
//------------------------------------------------------------------------------
void UIWidget::hide()
{
    this->m_visible = false;
}

// -----------------------------------------------------------------------------
//  「可視」か？
// -----------------------------------------------------------------------------
bool UIWidget::isVisible()
{
    if( this->m_visible )
    {
        if( !this->m_parent || this->m_parent->isVisible() )
        {
            return true;
        }
    }
    return false;
}


// -----------------------------------------------------------------------------
//  このウィジェットのクライアント領域全体を再描画する
// -----------------------------------------------------------------------------
void UIWidget::refresh()
{
    if( !this->isVisible() )
    {
        return;
    }

    this->m_graphics->beginPaint();
    this->draw(this->m_graphics);
    this->m_graphics->endPaint();

    this->m_children.forEach([](int n, void *value, void *param){
        UIWidget *child = (UIWidget *)value;
        child->refresh();
        return true;
    }, nullptr);
}

//------------------------------------------------------------------------------
//  描画
//------------------------------------------------------------------------------
void UIWidget::draw(Graphics *g)
{
    g->clear();
}


// =============================================================================
//  Label
// =============================================================================
Label::Label(uint16_t id, UIWidget *parent, HX8357 *display, int16_t left, int16_t top, uint16_t width, uint16_t height, int size)
    : UIWidget(id, parent, display, left, top, width, height),
    m_padding(0), m_alignment(0)
{
    this->m_text = new char[size];
    this->m_text[0] = '\0';
}

// -----------------------------------------------------------------------------
Label::Label(uint16_t id, UIWidget *parent, HX8357 *display, int16_t left, int16_t top, uint16_t width, uint16_t height, const char *text)
    : UIWidget(id, parent, display, left, top, width, height),
    m_padding(0), m_alignment(0)
{
    this->m_text = new char[strlen(text)+1];
    strcpy(this->m_text, text);
}

// -----------------------------------------------------------------------------
void Label::setPadding(int16_t value)
{
    this->m_padding = value;
}

// -----------------------------------------------------------------------------
void Label::setTextAlign(uint8_t align)
{
    this->m_alignment = align;
}

// -----------------------------------------------------------------------------
void Label::setTextColor(uint16_t color)
{
    this->m_graphics->setFontColor(color);
}

// -----------------------------------------------------------------------------
void Label::setBackColor(uint16_t color)
{
    this->m_graphics->setFillColor(color);
}

// -----------------------------------------------------------------------------
void Label::setFont(int index)
{
    this->m_graphics->setFont(index);
}

// -----------------------------------------------------------------------------
void Label::setText(const char *text)
{
    if( text )
    {
        strcpy(this->m_text, text);
    }
    else
    {
        this->m_text[0] = '\0';
    }
}

// -----------------------------------------------------------------------------
void Label::draw(Graphics *g)
{
    // UIWidget::draw(g);
    Rect rc = this->getClientRect();
    g->setFillColor(COLOR_BLACK);
    g->fillRect(rc);
    rc.inflate(-this->m_padding, -this->m_padding);
    g->drawText(rc, this->m_text, this->m_alignment);
}


// =============================================================================
//  Button
// =============================================================================
Button::Button(uint16_t id, UIWidget *parent, HX8357 *display, int16_t left, int16_t top, uint16_t width, uint16_t height, Icon *icon)
    : UIWidget(id, parent, display, left, top, width, height), 
    m_icon(icon), m_callback(nullptr), m_param(nullptr)
{

}

// -----------------------------------------------------------------------------
void Button::onTouched(int16_t x, int16_t y)
{
    UIWidget::onTouched(x, y);
    this->refresh();
}

// -----------------------------------------------------------------------------
void Button::onReleased()
{
    UIWidget::onReleased();
    this->refresh();
    if( this->m_callback )
    {
        this->m_callback(this, this->m_param);
    }
}

// -----------------------------------------------------------------------------
void Button::draw(Graphics *g)
{
    uint16_t fgcol = this->m_captured? COLOR_WHITE : COLOR_DARKGRAY;
    uint16_t bkcol = this->m_captured? COLOR_DARKBLUE : COLOR_BLACK;
    g->setFillColor(bkcol);
    g->fillRect(this->getClientRect());
    Point pt = this->getClientRect().center();
    pt.offset(-this->m_icon->getWidth()/2, -this->m_icon->getHeight()/2);
    g->drawIcon(pt.x, pt.y, this->m_icon, fgcol, bkcol);
}


// =============================================================================
//  ListBox
// =============================================================================
uint16_t ListBox::m_images[ListBox::PAGE_SIZE][ListBox::ITEM_HEIGHT*ListBox::ITEM_HEIGHT];
int      ListBox::m_imageIndices[ListBox::PAGE_SIZE];

// -----------------------------------------------------------------------------
ListBox::ListBox(uint16_t id, UIWidget *parent, HX8357 *display, int16_t left, int16_t top, int16_t width, bool hasImage)
    : UIWidget(id, parent, display, left, top, width, ListBox::ITEM_HEIGHT*ListBox::PAGE_SIZE),
    m_itemCount(0), m_selectedIndex(-1), m_touchedIndex(-1), m_pageIndex(0), 
    m_drawItemProc(nullptr), m_drawItemParam(nullptr),
    m_selectItemProc(nullptr), m_selectItemParam(nullptr),
    m_hasImage(hasImage), m_getImageProc(nullptr), m_getImageParam(nullptr)
{
    this->m_pageSize = ListBox::PAGE_SIZE;
    this->m_itemHeight = ListBox::ITEM_HEIGHT;
}

// -----------------------------------------------------------------------------
void ListBox::setItems(int count, int selection)
{
    this->m_itemCount = count;
    this->m_selectedIndex = selection;
    this->m_pageIndex = (selection >= 0)? (selection / this->m_pageSize) : 0;
    this->m_touchedIndex = -1;
    for( int i = 0 ; i < ListBox::PAGE_SIZE ; i++ )
    {
        ListBox::m_imageIndices[i] = -1;
    }
}

// -----------------------------------------------------------------------------
void ListBox::setSelection(int index)
{
    if( index != this->m_selectedIndex )
    {
        int page = (index >= 0)? (index / this->m_pageSize) : 0;
        if( page != this->m_pageIndex )
        {
            this->m_pageIndex = page;
            this->m_selectedIndex = index;
            this->refresh();
            return;
        }
        if( this->m_drawItemProc )
        {
            this->m_graphics->beginPaint();
            if( this->isItemVisible(this->m_selectedIndex) )
            {
                this->drawItem(this->m_graphics, this->m_selectedIndex, false, false);
            }
            if( this->isItemVisible(index) )
            {
                this->drawItem(this->m_graphics, index, true, false);
            }
            this->m_graphics->endPaint();
        }
        this->m_selectedIndex = index;
    }
}

// -----------------------------------------------------------------------------
Rect ListBox::getItemRect(int index)
{
    if( index < 0 ){ index = 0; }
    int16_t x = this->m_hasImage? ListBox::IMAGE_WIDTH : 0;
    int16_t w = this->m_hasImage? (this->m_width - ListBox::IMAGE_WIDTH) : this->m_width;
    int16_t y = (index % this->m_pageSize)*this->m_itemHeight;
    return Rect(x, y, w, ListBox::ITEM_HEIGHT);
}

// -----------------------------------------------------------------------------
bool ListBox::isItemVisible(int index)
{
    if( !this->isVisible() )
    {
        return false;
    }
    if( index < 0 || this->m_itemCount <= index )
    {
        return false;
    }
    int topIndex = this->m_pageIndex * this->m_pageSize;
    int lastIndex = topIndex + this->m_pageSize;
    return (topIndex <= index) && (index < lastIndex);
}

// -----------------------------------------------------------------------------
void ListBox::draw(Graphics *g)
{
    if( this->m_drawItemProc )
    {
        int topIndex = this->m_pageIndex * this->m_pageSize;
        int lastIndex = topIndex + this->m_pageSize;
        for( int i = topIndex ; i < lastIndex ; i++ )
        {
            bool selected = i == this->m_selectedIndex;
            bool touched  = i == this->m_touchedIndex;
            this->drawItem(g, i, selected, touched);
        }
    }
}

// -----------------------------------------------------------------------------
void ListBox::drawItem(Graphics *g, int index, bool selected, bool touched)
{
    Rect rc = this->getItemRect(index);
    if( index >= this->m_itemCount )
    {
        g->setFillColor(COLOR_BLACK);
        g->fillRect(0, rc.top, this->m_width, rc.height);
        return;
    }

    if( this->m_hasImage && this->m_getImageProc )
    {
        if( ListBox::m_imageIndices[index % ListBox::PAGE_SIZE] != index )
        {
            GETIMAGESTRUCT gis;
            gis.index  = index;
            gis.buffer = ListBox::m_images[index % ListBox::PAGE_SIZE];
            gis.param  = this->m_getImageParam;
            this->m_getImageProc(this, &gis);
            ListBox::m_imageIndices[index % ListBox::PAGE_SIZE] = index;
        }
        g->drawBitmap(0, rc.top, ListBox::IMAGE_WIDTH, ListBox::ITEM_HEIGHT, ListBox::m_images[index % ListBox::PAGE_SIZE]);
    }
    DRAWITEMSTRUCT dis;
    dis.graphics = g;
    dis.index    = index;
    dis.rect     = rc;
    dis.selected = selected;
    dis.touched  = touched;
    dis.param    = this->m_selectItemParam;
    this->m_drawItemProc(this, &dis);
}

// -----------------------------------------------------------------------------
void ListBox::onTouched(int16_t x, int16_t y)
{
    int index = (int)(y / this->m_itemHeight) + this->m_pageIndex * this->m_pageSize;
    if( (index < this->m_itemCount) && (index != this->m_selectedIndex) )
    {
        this->m_touchedIndex = index;
        if( this->m_drawItemProc && this->isItemVisible(index) )
        {
            this->m_graphics->beginPaint();
            this->drawItem(this->m_graphics, index, false, true);
            this->m_graphics->endPaint();
        }
    }
}

// -----------------------------------------------------------------------------
void ListBox::onReleased()
{
    if( this->m_touchedIndex >= 0 )
    {
        if( this->m_drawItemProc )
        {
            this->m_graphics->beginPaint();
            if( this->isItemVisible(this->m_selectedIndex) )
            {
                this->drawItem(this->m_graphics, this->m_selectedIndex, false, false);
            }
            if( this->isItemVisible(this->m_touchedIndex) )
            {
                this->drawItem(this->m_graphics, this->m_touchedIndex, true, false);
            }
            this->m_graphics->endPaint();
        }
        this->m_selectedIndex = this->m_touchedIndex;
        this->m_touchedIndex = -1;
        if( this->m_selectItemProc )
        {
            SELECTITEMSTRUCT sis;
            sis.index = this->m_selectedIndex;
            sis.param = this->m_selectItemParam;
            this->m_selectItemProc(this, &sis);
        }        
    }
}

// -----------------------------------------------------------------------------
void ListBox::prevPage()
{
    if( this->canMovePrevPage() )
    {
        --(this->m_pageIndex);
        this->refresh();
    }
}

// -----------------------------------------------------------------------------
void ListBox::nextPage()
{
    if( this->canMoveNextPage() )
    {
        ++(this->m_pageIndex);
        Serial.print("next page index : ");
        Serial.println(this->m_pageIndex, DEC);
        this->refresh();
    }
    else
    {
        Serial.println("cannot move next");
        Serial.println(this->m_itemCount, DEC);
        Serial.println(this->m_pageIndex, DEC);
        Serial.println(this->m_pageSize, DEC);
        Serial.println(this->getLastPageIndex(), DEC);
    }
}

// -----------------------------------------------------------------------------
bool ListBox::canMovePrevPage()
{
    return this->m_pageIndex > 0;
}

// -----------------------------------------------------------------------------
bool ListBox::canMoveNextPage()
{
    return this->m_pageIndex < this->getLastPageIndex();
}

// -----------------------------------------------------------------------------
int ListBox::getLastPageIndex()
{
    if( this->m_itemCount == 0 )
    {
        return 0;
    }
    return (this->m_itemCount - 1)/ this->m_pageSize;
}


// =============================================================================
//  SevenSegLabel
// =============================================================================
Icon *SevenSegLabel::m_icon[11] = {nullptr};

// -----------------------------------------------------------------------------
SevenSegLabel::SevenSegLabel(uint16_t id, UIWidget *parent, HX8357 *display, int16_t left, int16_t top, int16_t length)
    : UIWidget(id, parent, display, left, top, length*(SevenSegLabel::DIGIT_WIDTH+SevenSegLabel::DIGIT_SPACE), SevenSegLabel::DIGIT_HEIGHT),
    m_length(length), m_formatProc(nullptr)
{
    this->m_value = new char[length+1];
    this->m_buffer = new char[length+1];
    this->m_dirty = new bool[length];
    for( int16_t i = 0 ; i < length ; i++ )
    {
        this->m_value[i] = '0';
        this->m_dirty[i] = false;
    }
    // this->m_formatProc = [](uint16_t value, char *buffer){
    //     char format[10];
    //     sprintf(format, "%%0%dd", length);
    //     sprintf(buffer, format, value);
    // };
    if( SevenSegLabel::m_icon[0] == nullptr )
    {
        for( uint16_t i = 0 ; i < 11 ; i++ )
        {
            SevenSegLabel::m_icon[i] = new Icon(SSEG_DIGITS+i*(2+SevenSegLabel::DIGIT_WIDTH*SevenSegLabel::DIGIT_HEIGHT));
        }
    }
}

// -----------------------------------------------------------------------------
void SevenSegLabel::setValue(uint16_t value)
{
    bool needDraw = false;
    if( this->m_formatProc )
    {
        this->m_formatProc(value, this->m_buffer);
    }
    else
    {
        char format[10];
        sprintf(format, "%%0%dd", this->m_length);
        sprintf(this->m_buffer, format, value);
    }

    for( int16_t i = 0 ; i < this->m_length ; i++ )
    {
        if( this->m_buffer[i] != this->m_value[i] )
        {
            if( (this->m_buffer[i] == ' ') || ('0' <= this->m_buffer[i] && this->m_buffer[i] <= ':') )
            {
                {
                    this->m_dirty[i] = true;
                    this->m_value[i] = this->m_buffer[i];
                    needDraw = true;
                }
            }
        }
    }
    if( needDraw && this->isVisible() )
    {
        this->m_graphics->beginPaint();
        this->draw(this->m_graphics);
        this->m_graphics->endPaint();
    }
}

// -----------------------------------------------------------------------------
void SevenSegLabel::draw(Graphics *g)
{
    uint16_t fgcol = COLOR_SILVER;
    uint16_t bkcol = COLOR_BLACK;
    g->setFillColor(COLOR_BLACK);
    Point pt(0, 0);
    for( int16_t i = 0 ; i < this->m_length ; i++ )
    {
        if( this->m_dirty[i] )
        {
            if( this->m_value[i] == ' ' )
            {
                g->fillRect(pt.x, pt.y, SevenSegLabel::DIGIT_WIDTH+SevenSegLabel::DIGIT_SPACE, SevenSegLabel::DIGIT_HEIGHT);
            }
            else
            {
                g->drawIcon(pt.x, pt.y, SevenSegLabel::m_icon[this->m_value[i]-'0'], fgcol, bkcol);
                g->fillRect(pt.x+SevenSegLabel::DIGIT_WIDTH, pt.y, SevenSegLabel::DIGIT_SPACE, SevenSegLabel::DIGIT_HEIGHT);
            }
            this->m_dirty[i] = false;
        }
        pt.offset(SevenSegLabel::DIGIT_WIDTH+SevenSegLabel::DIGIT_SPACE, 0);
    }
}

// -----------------------------------------------------------------------------
void SevenSegLabel::refresh()
{
    for( int16_t i = 0 ; i < this->m_length ; i++ )
    {
        this->m_dirty[i] = true;
    }
    UIWidget::refresh();
}


// =============================================================================
//  PaintBox
// =============================================================================
PaintBox::PaintBox(uint16_t id, UIWidget *parent, HX8357 *display, int16_t left, int16_t top, int16_t width, int16_t height)
    : UIWidget(id, parent, display, left, top, width, height),
    m_paintProc(nullptr), m_context(nullptr)
{

}

// -----------------------------------------------------------------------------
void PaintBox::draw(Graphics *g)
{
    g->setFillColor(COLOR_BLACK);
    UIWidget::draw(g);
    if( this->m_paintProc )
    {
        this->m_paintProc(this, g, this->m_context);
    }
}


// =============================================================================
//  SpectrumView
// =============================================================================
const float SpectrumView::SCALE = 100; 

// -----------------------------------------------------------------------------
SpectrumView::SpectrumView(uint16_t id, UIWidget *parent, HX8357 *display, int16_t left, int16_t top)
    : UIWidget(id, parent, display, left, top, 288, 50)
{
    for( int i = 0 ; i < SpectrumView::NUM_BANDS ; i++ )
    {
        this->m_spectrum[i] = 0;
    }
}

// -----------------------------------------------------------------------------
void SpectrumView::update(AudioAnalyzeFFT1024 *fft)
{
    float level[16];

    if( fft->available() )
    {
        level[0] =  fft->read(0);
        level[1] =  fft->read(1);
        level[2] =  fft->read(2, 3);
        level[3] =  fft->read(4, 6);
        level[4] =  fft->read(7, 10);
        level[5] =  fft->read(11, 15);
        level[6] =  fft->read(16, 22);
        level[7] =  fft->read(23, 32);
        level[8] =  fft->read(33, 46);
        level[9] =  fft->read(47, 66);
        level[10] = fft->read(67, 93);
        level[11] = fft->read(94, 131);
        level[12] = fft->read(132, 184);
        level[13] = fft->read(185, 257);
        level[14] = fft->read(258, 359);
        level[15] = fft->read(360, 511);

        for( int i = 0 ; i < 16 ; i++ )
        {
            int16_t val = (int16_t)(level[i] * SpectrumView::SCALE);
            if( val >= 10 )
            { 
                val = 10; 
            }
            if( val >= this->m_spectrum[i] )
            {
                this->m_spectrum[i] = val;
            }
            else
            {
                if( this->m_spectrum[i] > 0 )
                { 
                    --this->m_spectrum[i]; 
                }
            }
        }
        if( this->isVisible() )
        {
            this->m_graphics->beginPaint();
            this->internalDraw(this->m_graphics);
            this->m_graphics->endPaint();
        }
    }
}

// -----------------------------------------------------------------------------
void SpectrumView::clear()
{
    for( int i = 0 ; i < 16 ; i++ )
    {
        this->m_spectrum[i] = 0;
    }
    if( this->isVisible() )
    {
        this->m_graphics->beginPaint();
        this->internalDraw(this->m_graphics);
        this->m_graphics->endPaint();
    }
}

// -----------------------------------------------------------------------------
void SpectrumView::internalDraw(Graphics *g)
{
    int16_t x = 0;
    int16_t y;
    for( int i = 0 ; i < 16 ; i++ )
    {
        y = 50 - this->m_spectrum[i]*5;
        if( this->m_spectrum[i] < 10 )
        {
            g->setFillColor(Graphics::RGBToColor(0x1D, 0x19, 0x59));    //COLOR_MIDNIGHTBLUE);
            g->fillRect(x, 0, 14, y);
        }
        if( this->m_spectrum[i] > 0 )
        {
            g->setFillColor(Graphics::RGBToColor(0x82, 0x7F, 0xB2));    //COLOR_DODGERBLUE);
            g->fillRect(x, y, 14, 50-y);
        }
        x += 18;
    }
}

// -----------------------------------------------------------------------------
void SpectrumView::draw(Graphics *g)
{
    g->setFillColor(COLOR_BLACK);
    UIWidget::draw(g);
    this->internalDraw(g);
}



// ================================================================================
//  ProgressBar
// ================================================================================
ProgressBar::ProgressBar(uint16_t id, UIWidget *parent, HX8357 *display, int16_t left, int16_t top, int16_t width, int16_t height)
    : UIWidget(id, parent, display, left, top, width, height),
    m_maximum(100), m_value(0)
{

}

// --------------------------------------------------------------------------------
void ProgressBar::setMaximum(int value)
{
    if( (this->m_maximum != value) && (value > 0) )
    {
        this->m_maximum = value;
        if( this->m_value > this->m_maximum )
        {
            this->m_value = this->m_maximum;
        }
        this->refresh();
    }
}

// --------------------------------------------------------------------------------
void ProgressBar::setValue(int value)
{
    if( value < 0 ){ value = 0; }
    if( value > this->m_maximum ){ value = this->m_maximum; }
    if( this->m_value != value )
    {
        this->m_value = value;
        this->refresh();
    }
}

// --------------------------------------------------------------------------------
void ProgressBar::draw(Graphics *g)
{
    Rect rc = this->getClientRect();
    g->setStrokeColor(COLOR_SILVER);
    g->drawRect(rc);
    rc.inflate(-1, -1);
    int barsize = this->getBarSizeOfValue(this->m_value);
    if( barsize )
    {
        rc.resizeWidth(barsize);
        g->setFillColor(COLOR_ORANGERED);
        g->fillRect(rc);
        rc.offset(barsize, 0);
        barsize = this->m_width - 2 - barsize;
    }
    if( barsize )
    {
        rc.resizeWidth(barsize);
        g->setFillColor(COLOR_BLACK);
        g->fillRect(rc);
    }
}


// ================================================================================
//  Desktop
// ================================================================================
Desktop::Desktop(HX8357 *display)
    : UIWidget(9999, nullptr, display, 0, 0, Graphics::SCREEN_WIDTH, Graphics::SCREEN_HEIGHT)
{
    this->m_progressbar = new ProgressBar(0, this, display, 139, 230, 202, 20);
    this->m_progressbar->hide();
    this->m_label = new Label(1, this, display, 0, 260, Graphics::SCREEN_WIDTH, 20, 64);
    this->m_label->setTextColor(COLOR_SILVER);
    this->m_label->setBackColor(COLOR_BLACK);
    this->m_label->setTextAlign(Graphics::ALIGN_CENTER);
    this->m_label->setFont(Graphics::LARGE_FONT);
    this->m_label->hide();
}

// -----------------------------------------------------------------------------
void Desktop::showProgress(int maximum, const char *message)
{
    this->m_progressbar->setMaximum(maximum);
    this->m_label->setText(message);
    this->m_progressbar->show();
    this->m_label->show();
}

// -----------------------------------------------------------------------------
void Desktop::updateProgress(int value)
{
    this->m_progressbar->setValue(value);
}

// -----------------------------------------------------------------------------
void Desktop::hideProgress()
{
    this->m_progressbar->hide();
    this->m_label->hide();
}

// -----------------------------------------------------------------------------
void Desktop::draw(Graphics *g)
{
    g->setFillColor(COLOR_BLACK);
    g->fillRect(this->getClientRect());
    g->drawBitmap(90, 60, 300, 150, const_cast<uint16_t *>(LOGO+2));
}

// ================================================================================
//  PlaybackView
// ================================================================================
PlaybackView::PlaybackView(UIWidget *parent, HX8357 *display, MusicPlayer *player)
    : UIWidget(PlaybackView::ID, parent, display, 0, 0, Graphics::SCREEN_WIDTH, Graphics::SCREEN_HEIGHT-60),
    m_player(player)
{
    this->hide();
    Serial.println("PlaybackView");
    player->attachEvent(PlaybackView::handlePlayerEvent, this);

    this->m_statusPaintBox = new PaintBox(PlaybackView::ID+1, this, display, 4, 4, 48, 48);
    this->m_statusPaintBox->setPaintProc([](PaintBox *box, Graphics *g, void *context){
        PlaybackView *self = (PlaybackView *)context;
        self->drawStatus(g);
    }, this);

    this->m_coverImagePaintBox = new PaintBox(PlaybackView::ID+2, this, display, 4, 100, 152, 152);
    this->m_coverImagePaintBox->setPaintProc([](PaintBox *box, Graphics *g, void *context){
        PlaybackView *self = (PlaybackView *)context;
        self->drawCoverImage(g);
    }, this);

    this->m_spectrumView = new SpectrumView(PlaybackView::ID+3, this, display, 192, 200);

    this->m_songTitleLabel = new Label(PlaybackView::ID+4, this, display, 4, 72, 472, 20, PlayList::MAX_TITLE_BYTES+1);
    this->m_songTitleLabel->setFont(Graphics::LARGE_FONT);
    this->m_songTitleLabel->setTextColor(COLOR_SILVER);

    this->m_albumTitleLabel = new Label(PlaybackView::ID+5, this, display, 162, 110, 314, 16, Album::MAX_TITLE_BYTES+1);
    this->m_albumTitleLabel->setFont(Graphics::SMALL_FONT);
    this->m_albumTitleLabel->setTextColor(COLOR_SILVER);

    this->m_artistNameLabel = new Label(PlaybackView::ID+6, this, display, 162, 130, 314, 16, Artist::MAX_NAME_BYTES+1);
    this->m_artistNameLabel->setFont(Graphics::SMALL_FONT);
    this->m_artistNameLabel->setTextColor(COLOR_SILVER);

    this->m_albumInfoLabel = new Label(PlaybackView::ID+7, this, display, 162, 160, 314, 16, 32);
    this->m_albumInfoLabel->setFont(Graphics::SMALL_FONT);
    this->m_albumInfoLabel->setTextColor(COLOR_SILVER);

    this->m_trackLengthLabel = new Label(PlaybackView::ID+8, this, display, 300, 40, 176, 16, 32);
    this->m_trackLengthLabel->setFont(Graphics::SMALL_FONT);
    this->m_trackLengthLabel->setTextColor(COLOR_SILVER);
    this->m_trackLengthLabel->setTextAlign(Graphics::ALIGN_RIGHT);
    this->m_trackLengthLabel->setText("00:00");

    this->m_codecInfoLabel = new Label(PlaybackView::ID+8, this, display, 300, 23, 176, 16, 32);
    this->m_codecInfoLabel->setFont(Graphics::SMALL_FONT);
    this->m_codecInfoLabel->setTextAlign(Graphics::ALIGN_RIGHT);
    this->m_codecInfoLabel->setTextColor(COLOR_SILVER);

    this->m_trackLabel = new SevenSegLabel(PlaybackView::ID+9, this, display, 70, 22, 2);

    this->m_timeLabel = new SevenSegLabel(PlaybackView::ID+10, this, display, 150, 22, 5);
    this->m_timeLabel->setFormat([](uint16_t value, char *buffer){
        sprintf(buffer, "%02d:%02d", (int)(value/60), (int)(value%60));
    });

    this->m_statusIcon[PlaybackView::STOP ] = new Icon(ICON_STOP_48);
    this->m_statusIcon[PlaybackView::PLAY ] = new Icon(ICON_PLAY_48);
    this->m_statusIcon[PlaybackView::PAUSE] = new Icon(ICON_PAUSE_48);
}

// -----------------------------------------------------------------------------
void PlaybackView::handlePlayerEvent(MusicPlayer *player, uint16_t eventId, void *context)
{
    PlaybackView *self = (PlaybackView *)context;

    uint16_t elapsed;
    char buffer[30];
    Album *album;
    switch( eventId )
    {
        case MusicPlayer::EVT_ALBUM_CHANGED:
            Serial.println("album changed");
            album = player->getPlayList()->getAlbum();
            self->m_albumTitleLabel->setText(album->getTitle());
            self->m_albumTitleLabel->refresh();
            self->m_artistNameLabel->setText(album->getArtist()->getName());
            self->m_artistNameLabel->refresh();
            sprintf(buffer, "%04d年 / %02d:%02d", (int)(album->getYear()), (int)(album->getTotalTime()/60), (int)(album->getTotalTime()%60));
            self->m_albumInfoLabel->setText(buffer);
            self->m_albumInfoLabel->refresh();
            self->m_coverImagePaintBox->refresh();
            // not break        
        case MusicPlayer::EVT_STATUS_CHANGED:
            Serial.println("status changed");
            self->m_statusPaintBox->refresh();
            // not break
        case MusicPlayer::EVT_TRACK_CHANGED:
            Serial.println("track changed");
            self->m_trackLabel->setValue(player->getCurrentTrackNumber());
            self->m_songTitleLabel->setText(player->getCurrentSongTitle());
            self->m_songTitleLabel->refresh();
            if( player->isPlaying() )
            {
                elapsed = player->getCurrentSongLength();
                sprintf(buffer, "%02d:%02d", (int)elapsed/60, (int)elapsed%60);
                self->m_trackLengthLabel->setText(buffer);
                sprintf(buffer, "%s %dHz %dkbps", 
                    (player->getPlayList()->getCodec() == PlayList::CODEC_AAC)? "AAC" : "MP3",
                    100*(int)player->getCurrentSampleRate(),
                    (int)player->getCurrentBitRate()
                );
                self->m_codecInfoLabel->setText(buffer);
            }
            else
            {
                self->m_trackLengthLabel->setText("");
                self->m_codecInfoLabel->setText("");
            }
            self->m_trackLengthLabel->refresh();
            self->m_codecInfoLabel->refresh();
            // not break
        case MusicPlayer::EVT_TIME_CHANGED:
            elapsed = player->getElapsedTime();
            self->m_timeLabel->setValue(elapsed);
            break;
    }
}

// -----------------------------------------------------------------------------
void PlaybackView::drawStatus(Graphics *g)
{
    uint16_t fgcol = COLOR_SILVER;
    uint16_t bkcol = COLOR_BLACK;
    g->setFillColor(bkcol);
    g->fillRect(0, 0, 48, 48);
    if( this->m_player->isPlaying() )
    {
        if( this->m_player->isPaused() )
        {
            g->drawIcon(0, 0, this->m_statusIcon[PlaybackView::PAUSE], fgcol, bkcol);
        }
        else
        {
            g->drawIcon(0, 0, this->m_statusIcon[PlaybackView::PLAY], fgcol, bkcol);
        }
    }
    else
    {
        g->drawIcon(0, 0, this->m_statusIcon[PlaybackView::STOP], fgcol, bkcol);
    }
}

// -----------------------------------------------------------------------------
void PlaybackView::drawCoverImage(Graphics *g)
{
    g->setStrokeColor(COLOR_SILVER);
    g->drawRect(0, 0, 152, 152);
    g->drawBitmap(1, 1, this->m_player->getPlayList()->getImage());
}

// -----------------------------------------------------------------------------
void PlaybackView::draw(Graphics *g)
{
    g->setFillColor(COLOR_BLACK);
    g->fillRect(this->getClientRect());

    g->setFontColor(COLOR_LIGHTSLATEGRAY);
    g->setFont(Graphics::SMALL_FONT);
    g->drawText(70, 2, "track");
    g->drawText(150, 2, "time");
}

// -----------------------------------------------------------------------------
void PlaybackView::updateFFT(AudioAnalyzeFFT1024 *fft)
{
    if( this->m_player->isPlaying() && !this->m_player->isPaused() )
    {
        this->m_spectrumView->update(fft);
    }
    else
    {
        this->m_spectrumView->clear();
    }
}



// =============================================================================
//  ToolBar
// =============================================================================
ToolBar::ToolBar(UIWidget *parent, HX8357 *display)
    : UIWidget(ToolBar::ID_TOOLBAR, parent, display, 0, Graphics::SCREEN_HEIGHT-60, Graphics::SCREEN_WIDTH, 60)
{
    this->hide();
    this->m_buttons[ 0] = new Button(ToolBar::ID_PLAY,   this, display,   0, 0, 80, 56, new Icon(ICON_PLAY_32));
    this->m_buttons[ 1] = new Button(ToolBar::ID_STOP,   this, display,   0, 0, 80, 56, new Icon(ICON_STOP_32));
    this->m_buttons[ 2] = new Button(ToolBar::ID_PAUSE,  this, display,  80, 0, 80, 56, new Icon(ICON_PAUSE_32));
    this->m_buttons[ 3] = new Button(ToolBar::ID_PREV,   this, display, 160, 0, 80, 56, new Icon(ICON_PREV_32));
    this->m_buttons[ 4] = new Button(ToolBar::ID_UP,     this, display, 160, 0, 80, 56, new Icon(ICON_UP_32));
    this->m_buttons[ 5] = new Button(ToolBar::ID_NEXT,   this, display, 240, 0, 80, 56, new Icon(ICON_NEXT_32));
    this->m_buttons[ 6] = new Button(ToolBar::ID_DOWN,   this, display, 240, 0, 80, 56, new Icon(ICON_DOWN_32));
    this->m_buttons[ 7] = new Button(ToolBar::ID_CLOSE,  this, display, 320, 0, 80, 56, new Icon(ICON_CLOSE_32));
    this->m_buttons[ 8] = new Button(ToolBar::ID_SONG,   this, display, 400, 0, 80, 56, new Icon(ICON_SONG_32));
    this->m_buttons[ 9] = new Button(ToolBar::ID_ALBUM,  this, display, 400, 0, 80, 56, new Icon(ICON_ALBUM_32));
    this->m_buttons[10] = new Button(ToolBar::ID_ARTIST, this, display, 400, 0, 80, 56, new Icon(ICON_ARTIST_32));
}

// -----------------------------------------------------------------------------
void ToolBar::draw(Graphics *g)
{
    Serial.println("toolbar draw");
    g->setFillColor(COLOR_BLACK);
    g->fillRect(this->getClientRect());
}

// -----------------------------------------------------------------------------
Button *ToolBar::getToolButton(uint16_t id)
{
    for( int i = 0 ; i < ToolBar::NUM_BUTTONS ; i++ )
    {
        if( this->m_buttons[i]->getID() == id )
        {
            return this->m_buttons[i];
        }
    }
    return nullptr;
}


// ============================================================================= 
//  SelectSongView
// ============================================================================= 
SelectSongView::SelectSongView(UIWidget *parent, HX8357 *display, MusicPlayer *player, ToolBar *toolbar)
    : UIWidget(SelectSongView::ID, parent, display, 0, 0, Graphics::SCREEN_WIDTH, Graphics::SCREEN_HEIGHT-60),
    m_player(player), m_toolbar(toolbar), m_selectProc(nullptr), m_procParam(nullptr)
{
    this->m_player->attachEvent([](MusicPlayer *sender, uint16_t eventId, void *context){
        SelectSongView *self = (SelectSongView *)context;
        self->onPlayerEvent(eventId);
    }, this);

    this->m_listbox = new ListBox(SelectSongView::ID+1, this, display, 0, 20, Graphics::SCREEN_WIDTH, false); //240, 4);
    this->m_listbox->setDrawItemProc([](ListBox *sender, DRAWITEMSTRUCT *dis){
        SelectSongView *self = (SelectSongView *)(dis->param);
        self->onDrawListItem(dis);
    }, this);
    this->m_listbox->attachEvent([](ListBox *sender, SELECTITEMSTRUCT *sis){
        SelectSongView *self = (SelectSongView *)(sis->param);
        self->onSelectItem(sis->index);
    }, this);
}

// -----------------------------------------------------------------------------
void SelectSongView::show()
{
    int count = (int)(this->m_player->getPlayList()->getAlbum()->getNumTracks());
    uint16_t currentTrack = this->m_player->getCurrentTrackNumber();
    int select = (currentTrack > 0)? (int)(currentTrack - 1) : -1;  
    this->m_listbox->setItems(count, select);

    this->m_toolbar->getToolButton(ToolBar::ID_PREV)->hide();
    this->m_toolbar->getToolButton(ToolBar::ID_NEXT)->hide();
    this->m_toolbar->getToolButton(ToolBar::ID_SONG)->hide();
    this->m_toolbar->getToolButton(ToolBar::ID_ALBUM)->show();
    this->m_toolbar->getToolButton(ToolBar::ID_ARTIST)->hide();
    this->m_toolbar->getToolButton(ToolBar::ID_CLOSE)->show();
    this->updateToolBar();

    this->m_toolbar->getToolButton(ToolBar::ID_UP)->attachEvent([](Button *sender, void *param){
        ((SelectSongView *)param)->onPageUp();
    }, this);
    this->m_toolbar->getToolButton(ToolBar::ID_DOWN)->attachEvent([](Button *sender, void *param){
        ((SelectSongView *)param)->onPageDown();
    }, this);

    UIWidget::show();
}

// -----------------------------------------------------------------------------
void SelectSongView::draw(Graphics *g)
{
    char buffer[160];
    Album *album = this->m_player->getPlayList()->getAlbum();
    sprintf(buffer, "%s / %s", album->getTitle(), album->getArtist()->getName());
    g->setFillColor(COLOR_BLACK);
    g->setFontColor(COLOR_SILVER);
    g->setFont(Graphics::SMALL_FONT);
    g->fillRect(this->getClientRect());
    g->drawText(4, 0, buffer);
}

// -----------------------------------------------------------------------------
void SelectSongView::onDrawListItem(DRAWITEMSTRUCT *dis)
{
    uint16_t bkcol = Graphics::RGBToColor(0x0A, 0x03, 0x25);
    if( dis->touched )
    {
        bkcol = Graphics::RGBToColor(0x32, 0x25, 0x68);
    }
    else if( (dis->index % 2) == 0 )
    {
        bkcol = Graphics::RGBToColor(0x14, 0x09, 0x3F);
    }
    uint16_t fgcol = COLOR_DARKGRAY;
    if( dis->selected )
    {
        fgcol = COLOR_WHITE;
    }
    dis->graphics->setFillColor(bkcol);     
    dis->graphics->setFontColor(fgcol);
    dis->graphics->fillRect(dis->rect);

    char str[128];
    PlayList *playlist = this->m_player->getPlayList();
    dis->graphics->setFont(Graphics::LARGE_FONT);
    sprintf(str, "%02d. %s", 1+dis->index, playlist->getTitle(dis->index));
    dis->graphics->drawText(dis->rect.left+4, dis->rect.top+10, str);
    dis->graphics->setFont(Graphics::SMALL_FONT);
    uint16_t duration = playlist->getDuration(dis->index);
    sprintf(str, "%02d:%02d (%s %dHz %dkbps)", 
        (int)(duration / 60), (int)(duration % 60),
        (playlist->getCodec() == PlayList::CODEC_AAC)? "AAC" : "MP3",
        100*(int)(playlist->getSampleRate(dis->index)),
        (int)(playlist->getBitRate(dis->index))
    );
    Point pt = dis->rect.bottomRight().offset(-4, -4);
    dis->graphics->drawText(pt.x, pt.y, str, Graphics::ALIGN_RIGHT|Graphics::ALIGN_BOTTOM);
}

// -------------------------------------------------------------------
void SelectSongView::onPlayerEvent(uint16_t eventId)
{
    if( !this->isVisible() )
    {
        return;
    }
    if( eventId == MusicPlayer::EVT_STATUS_CHANGED || eventId == MusicPlayer::EVT_TRACK_CHANGED )
    {
        uint16_t currentTrack = this->m_player->getCurrentTrackNumber();
        int select = (currentTrack > 0)? (int)(currentTrack - 1) : -1;  
        this->m_listbox->setSelection(select);
        this->updateToolBar();
    }
}

// -----------------------------------------------------------------------------
void SelectSongView::updateToolBar()
{
    Button *upButton = this->m_toolbar->getToolButton(ToolBar::ID_UP);
    Button *downButton = this->m_toolbar->getToolButton(ToolBar::ID_DOWN);
    if( this->m_listbox->canMovePrevPage() )
    {
        upButton->show();
    }
    else
    {
        upButton->hide();
    }
    if( this->m_listbox->canMoveNextPage() )
    {
        downButton->show();
    }
    else
    {
        downButton->hide();
    }
    this->m_toolbar->refresh();
}

// -----------------------------------------------------------------------------
void SelectSongView::onPageUp()
{
    Serial.println("up");
    this->m_listbox->prevPage();
    this->updateToolBar();
}

// -----------------------------------------------------------------------------
void SelectSongView::onPageDown()
{
    Serial.println("down");
    this->m_listbox->nextPage();
    this->updateToolBar();
}

// -----------------------------------------------------------------------------
void SelectSongView::onSelectItem(int index)
{
    if( this->m_selectProc )
    {
        this->m_selectProc(this, index, this->m_procParam);
    }
}


// =============================================================================
//  SelectAlbumView
// =============================================================================
SelectAlbumView::SelectAlbumView(UIWidget *parent, HX8357 *display, MusicPlayer *player, ToolBar *toolbar)
    : UIWidget(SelectAlbumView::ID, parent, display, 0, 0, Graphics::SCREEN_WIDTH, Graphics::SCREEN_HEIGHT-60),
    m_player(player), m_toolbar(toolbar)
{
    this->m_artist = player->getPlayList()->getAlbum()->getArtist();
    this->m_listbox = new ListBox(SelectAlbumView::ID+1, this, display, 0, 20, Graphics::SCREEN_WIDTH, true); //240, 4);
    this->m_listbox->setDrawItemProc([](ListBox *sender, DRAWITEMSTRUCT *dis){
        SelectAlbumView *self = (SelectAlbumView *)(dis->param);
        self->onDrawListItem(dis);
    }, this);
    this->m_listbox->getImageProc([](ListBox *sender, GETIMAGESTRUCT *gis){
        SelectAlbumView *self = (SelectAlbumView *)(gis->param);
        self->onGetImage(gis);
    }, this);
    this->m_listbox->attachEvent([](ListBox *sender, SELECTITEMSTRUCT *sis){
        SelectAlbumView *self = (SelectAlbumView *)(sis->param);
        self->onSelectItem(sis->index);
    }, this);
}

// -----------------------------------------------------------------------------
void SelectAlbumView::setArtist(Artist *artist)
{
    this->m_artist = artist;
    int count = (int)(this->m_artist->getNumAlbums());
    int select = -1;
    if( this->m_player->getPlayList()->getAlbum()->getArtist() == this->m_artist )
    {
        // 現在のプレイリストに係るアーティスト（SelectSongView から入ってきた場合）
        select = this->m_artist->getIndexOfAlbum(this->m_player->getPlayList()->getAlbum());
    }
    this->m_listbox->setItems(count, select);
}

// -----------------------------------------------------------------------------
void SelectAlbumView::show()
{
    this->m_toolbar->getToolButton(ToolBar::ID_PREV)->hide();
    this->m_toolbar->getToolButton(ToolBar::ID_NEXT)->hide();
    this->m_toolbar->getToolButton(ToolBar::ID_SONG)->hide();
    this->m_toolbar->getToolButton(ToolBar::ID_ALBUM)->hide();
    this->m_toolbar->getToolButton(ToolBar::ID_ARTIST)->show();
    this->m_toolbar->getToolButton(ToolBar::ID_CLOSE)->show();
    this->updateToolBar();

    this->m_toolbar->getToolButton(ToolBar::ID_UP)->attachEvent([](Button *sender, void *param){
        ((SelectAlbumView *)param)->onPageUp();
    }, this);
    this->m_toolbar->getToolButton(ToolBar::ID_DOWN)->attachEvent([](Button *sender, void *param){
        ((SelectAlbumView *)param)->onPageDown();
    }, this);

    UIWidget::show();
}

// -----------------------------------------------------------------------------
void SelectAlbumView::updateToolBar()
{
    Button *upButton = this->m_toolbar->getToolButton(ToolBar::ID_UP);
    Button *downButton = this->m_toolbar->getToolButton(ToolBar::ID_DOWN);
    if( this->m_listbox->canMovePrevPage() )
    {
        upButton->show();
    }
    else
    {
        upButton->hide();
    }
    if( this->m_listbox->canMoveNextPage() )
    {
        downButton->show();
    }
    else
    {
        downButton->hide();
    }
    this->m_toolbar->refresh();
}

// -----------------------------------------------------------------------------
void SelectAlbumView::draw(Graphics *g)
{
    char buffer[90];
    sprintf(buffer, "%s のアルバム", this->m_artist->getName());
    g->setFillColor(COLOR_BLACK);
    g->setFontColor(COLOR_SILVER);
    g->setFont(Graphics::SMALL_FONT);
    g->fillRect(this->getClientRect());
    g->drawText(4, 0, buffer);
}

// -----------------------------------------------------------------------------
void SelectAlbumView::onGetImage(GETIMAGESTRUCT *gis)
{
    Album *album = this->m_artist->getAlbum(gis->index);
    uint16_t albumId = album->getID();
    uint8_t *p = (uint8_t *)(gis->buffer);
    const int CHUNK_SIZE = 240;
    for( int s = 0 ; s < 2 ; s++ )  // ２セクタを読み込む
    {
        uint32_t addr = (((uint32_t)albumId) * 2 + s) * 4096;   // このセクタの先頭アドレス
        for( int i = 0 ; i < 15 ; i++ ) // １セクタあたり 15 * 240 = 3600byte を読み込む
        {
            W25Q64_read(addr, p, CHUNK_SIZE);
            p    += CHUNK_SIZE;
            addr += CHUNK_SIZE;
        }
    }
}

// -----------------------------------------------------------------------------
void SelectAlbumView::onDrawListItem(DRAWITEMSTRUCT *dis)
{
    uint16_t bkcol = Graphics::RGBToColor(0x0A, 0x03, 0x25);
    if( dis->touched )
    {
        bkcol = Graphics::RGBToColor(0x32, 0x25, 0x68);
    }
    else if( (dis->index % 2) == 0 )
    {
        bkcol = Graphics::RGBToColor(0x14, 0x09, 0x3F);
    }
    uint16_t fgcol = COLOR_DARKGRAY;
    if( dis->selected )
    {
        fgcol = COLOR_WHITE;
    }
    dis->graphics->setFillColor(bkcol);     
    dis->graphics->setFontColor(fgcol);
    dis->graphics->fillRect(dis->rect);

    char str[128];
    Album *album = this->m_artist->getAlbum(dis->index);
    dis->graphics->setFont(Graphics::LARGE_FONT);
    dis->graphics->drawText(dis->rect.left+10, dis->rect.top+10, album->getTitle());
    dis->graphics->setFont(Graphics::SMALL_FONT);
    int totalTime = (int)album->getTotalTime();
    int numTracks = (int)album->getNumTracks();
    int year      = (int)album->getYear();
    sprintf(str, "%02d:%02d (%d Tracks / %d)", totalTime / 60, totalTime % 60, numTracks, year);
    Point pt = dis->rect.bottomRight().offset(-4, -4);
    dis->graphics->drawText(pt.x, pt.y, str, Graphics::ALIGN_RIGHT|Graphics::ALIGN_BOTTOM);
}

// -----------------------------------------------------------------------------
void SelectAlbumView::onPageUp()
{
    this->m_listbox->prevPage();
    this->updateToolBar();
}

// -----------------------------------------------------------------------------
void SelectAlbumView::onPageDown()
{
    this->m_listbox->nextPage();
    this->updateToolBar();
}

// -----------------------------------------------------------------------------
void SelectAlbumView::onSelectItem(int index)
{
    if( this->m_selectProc )
    {
        Album *album = this->m_artist->getAlbum(index);
        this->m_selectProc(this, album, this->m_procParam);
    }
}


// =============================================================================
//  SelectArtistView
// =============================================================================
SelectArtistView::SelectArtistView(UIWidget *parent, HX8357 *display, ArtistList *artistlist, ToolBar *toolbar)
    : UIWidget(SelectArtistView::ID, parent, display, 0, 0, Graphics::SCREEN_WIDTH, Graphics::SCREEN_HEIGHT-60),
    m_artistList(artistlist), m_toolbar(toolbar)
{
    this->m_listbox = new ListBox(SelectArtistView::ID+1, this, display, 0, 20, Graphics::SCREEN_WIDTH, true); //240, 4);
    this->m_listbox->setDrawItemProc([](ListBox *sender, DRAWITEMSTRUCT *dis){
        SelectArtistView *self = (SelectArtistView *)(dis->param);
        self->onDrawListItem(dis);
    }, this);
    this->m_listbox->getImageProc([](ListBox *sender, GETIMAGESTRUCT *gis){
        SelectArtistView *self = (SelectArtistView *)(gis->param);
        self->onGetImage(gis);
    }, this);
    this->m_listbox->attachEvent([](ListBox *sender, SELECTITEMSTRUCT *sis){
        SelectArtistView *self = (SelectArtistView *)(sis->param);
        self->onSelectItem(sis->index);
    }, this);

    this->m_listbox->setItems(this->m_artistList->getNumArtists(), -1);
} 

// -----------------------------------------------------------------------------
void SelectArtistView::setArtist(Artist *artist)
{
    int index = this->m_artistList->getIndexOfArtist(artist);
    this->m_listbox->setItems(this->m_artistList->getNumArtists(), index);
}

// -----------------------------------------------------------------------------
void SelectArtistView::show()
{
    this->m_toolbar->getToolButton(ToolBar::ID_PREV)->hide();
    this->m_toolbar->getToolButton(ToolBar::ID_NEXT)->hide();
    this->m_toolbar->getToolButton(ToolBar::ID_SONG)->hide();
    this->m_toolbar->getToolButton(ToolBar::ID_ALBUM)->hide();
    this->m_toolbar->getToolButton(ToolBar::ID_ARTIST)->hide();
    this->m_toolbar->getToolButton(ToolBar::ID_CLOSE)->show();
    this->updateToolBar();

    this->m_toolbar->getToolButton(ToolBar::ID_UP)->attachEvent([](Button *sender, void *param){
        ((SelectArtistView *)param)->onPageUp();
    }, this);
    this->m_toolbar->getToolButton(ToolBar::ID_DOWN)->attachEvent([](Button *sender, void *param){
        ((SelectArtistView *)param)->onPageDown();
    }, this);

    UIWidget::show();
}

// -----------------------------------------------------------------------------
void SelectArtistView::updateToolBar()
{
    Button *upButton = this->m_toolbar->getToolButton(ToolBar::ID_UP);
    Button *downButton = this->m_toolbar->getToolButton(ToolBar::ID_DOWN);
    if( this->m_listbox->canMovePrevPage() )
    {
        upButton->show();
    }
    else
    {
        upButton->hide();
    }
    if( this->m_listbox->canMoveNextPage() )
    {
        downButton->show();
    }
    else
    {
        downButton->hide();
    }
    this->m_toolbar->refresh();
}

// -----------------------------------------------------------------------------
void SelectArtistView::draw(Graphics *g)
{
    char buffer[30];
    sprintf(buffer, "アーティスト(%d)", this->m_artistList->getNumArtists());
    g->setFillColor(COLOR_BLACK);
    g->setFontColor(COLOR_SILVER);
    g->setFont(Graphics::SMALL_FONT);
    g->fillRect(this->getClientRect());
    g->drawText(4, 0, buffer);
}

// -----------------------------------------------------------------------------
void SelectArtistView::onGetImage(GETIMAGESTRUCT *gis)
{
    Artist *artist = this->m_artistList->getArtist(gis->index);
    uint16_t artistId = artist->getID();
    uint8_t *p = (uint8_t *)(gis->buffer);
    const int CHUNK_SIZE = 240;
    for( int s = 0 ; s < 2 ; s++ )  // ２セクタを読み込む
    {
        uint32_t addr = (((uint32_t)artistId) * 2 + s) * 4096;   // このセクタの先頭アドレス
        for( int i = 0 ; i < 15 ; i++ ) // １セクタあたり 15 * 240 = 3600byte を読み込む
        {
            W25Q64_read(addr, p, CHUNK_SIZE);
            p    += CHUNK_SIZE;
            addr += CHUNK_SIZE;
        }
    }
}

// -----------------------------------------------------------------------------
void SelectArtistView::onDrawListItem(DRAWITEMSTRUCT *dis)
{
    uint16_t bkcol = Graphics::RGBToColor(0x0A, 0x03, 0x25);
    if( dis->touched )
    {
        bkcol = Graphics::RGBToColor(0x32, 0x25, 0x68);
    }
    else if( (dis->index % 2) == 0 )
    {
        bkcol = Graphics::RGBToColor(0x14, 0x09, 0x3F);
    }
    uint16_t fgcol = COLOR_DARKGRAY;
    if( dis->selected )
    {
        fgcol = COLOR_WHITE;
    }
    dis->graphics->setFillColor(bkcol);     
    dis->graphics->setFontColor(fgcol);
    dis->graphics->fillRect(dis->rect);

    char str[128];
    Artist *artist = this->m_artistList->getArtist(dis->index);
    dis->graphics->setFont(Graphics::LARGE_FONT);
    dis->graphics->drawText(dis->rect.left+10, dis->rect.top+10, artist->getName());
    dis->graphics->setFont(Graphics::SMALL_FONT);
    sprintf(str, "%d album(s)", (int)(artist->getNumAlbums()));
    Point pt = dis->rect.bottomRight().offset(-4, -4);
    dis->graphics->drawText(pt.x, pt.y, str, Graphics::ALIGN_RIGHT|Graphics::ALIGN_BOTTOM);
}

// -----------------------------------------------------------------------------
void SelectArtistView::onPageUp()
{
    this->m_listbox->prevPage();
    this->updateToolBar();
}

// -----------------------------------------------------------------------------
void SelectArtistView::onPageDown()
{
    this->m_listbox->nextPage();
    this->updateToolBar();
}

// -----------------------------------------------------------------------------
void SelectArtistView::onSelectItem(int index)
{
    if( this->m_selectProc )
    {
        Artist *artist = this->m_artistList->getArtist(index);
        this->m_selectProc(this, artist, this->m_procParam);
    }
}
