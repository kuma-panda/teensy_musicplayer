#ifndef VIEW_H
#define VIEW_H

#include <Arduino.h>
#include <Audio.h>
#include <TouchScreen.h>
#include "HX8357.h"
#include "display.h"
#include "algorithm.h"

//------------------------------------------------------------------------------
class TouchEvent
{
    public:
        bool touched;
        Point pos;
        TouchEvent() : touched(false){}
        TouchEvent(bool b, int16_t x = 0, int16_t y = 0) : touched(b), pos(x, y){}
};

// -----------------------------------------------------------------------------
#define TOUCH_XP    A10     // (24) ---> Y+ へ接続
#define TOUCH_YP    A13     // (27) ---> X- へ接続
#define TOUCH_XM    26      // (26) ---> Y- へ接続
#define TOUCH_YM    25      // (25) ---> X+ へ接続

class UIWidget;
class TouchManager
{
    private:
        TouchScreen      *m_touchScreen;
        bool              m_touched;
        uint32_t          m_waitUntil;
        enum{X_MIN=100, X_MAX=920};
        enum{Y_MIN=130, Y_MAX=900};
        bool convertPosition(TSPoint tp, Point *pt){
            if( tp.x < X_MIN || X_MAX < tp.x || tp.y < Y_MIN || Y_MAX < tp.y )
            {
                return false;
            }
            pt->y = (int16_t)((320 * (uint32_t)(tp.x - X_MIN))/(X_MAX - X_MIN));
            pt->x = (int16_t)((480 * (uint32_t)(tp.y - Y_MIN))/(Y_MAX - Y_MIN));
            return true;
        }

    public:
        TouchManager(TouchScreen *touch);
        void execute(UIWidget *listener);
};

// -----------------------------------------------------------------------------
class Graphics
{
    private:
        HX8357      *m_display;
        Rect         m_clipRect;
        int          m_fontIndex;
        uint16_t     m_fillColor;
        uint16_t     m_strokeColor;
        uint16_t     m_fontColor;
        static Font *m_font[2];

        Point toScreenCoord(Point pt){
            pt.x += this->m_clipRect.left;
            pt.y += this->m_clipRect.top;
            return pt;
        }

        Rect toScreenCoord(Rect rc){
            rc.offset(this->m_clipRect.left, this->m_clipRect.top);
            return rc;
        }

    public:
        enum{
            SCREEN_HEIGHT = 320,
            SCREEN_WIDTH = 480
        };
        enum {
            SMALL_FONT = 0,
            LARGE_FONT = 1
        };
        enum {
            ALIGN_LEFT   = 0x00,
            ALIGN_CENTER = 0x01,
            ALIGN_RIGHT  = 0x02,
            HZALIGN_MASK = 0x03
        };
        enum {
            ALIGN_TOP    = 0x00,
            ALIGN_MIDDLE = 0x10,
            ALIGN_BOTTOM = 0x20,
            VTALIGN_MASK = 0x30
        };
        Graphics(HX8357 *display, Rect rc);
        void beginPaint();
        void endPaint();
        void setFont(int index){ this->m_fontIndex = index; }
        void setFillColor(uint16_t color){ this->m_fillColor = color; }
        void setStrokeColor(uint16_t color){ this->m_strokeColor = color; }
        void setFontColor(uint16_t color){ this->m_fontColor = color; }
        void clear();
        void fillRect(int16_t left, int16_t top, int16_t width, int16_t height);
        void fillRect(Rect rc);
        void drawRect(int16_t left, int16_t top, int16_t width, int16_t height);
        void drawRect(Rect rc);
        void drawHzLine(int16_t x, int16_t y, int16_t length);
        void drawVtLine(int16_t x, int16_t y, int16_t length);
        // void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
        void drawText(int16_t x, int16_t y, const char *text, uint8_t alignment=ALIGN_LEFT|ALIGN_TOP);
        void drawText(Rect rc, const char *text, uint8_t alignment=ALIGN_CENTER|ALIGN_MIDDLE);
        void drawIcon(int16_t x, int16_t y, Icon *icon, uint16_t fgcol, uint16_t bkcol);
        void drawBitmap(int16_t x, int16_t y, Bitmap *bitmap);
        void drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *image);

        static uint16_t RGBToColor(uint8_t r, uint8_t g, uint8_t b){
		    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
	    }
};

//------------------------------------------------------------------------------
class UIWidget
{
    protected:
        Graphics   *m_graphics;
        uint16_t    m_id;
        UIWidget   *m_parent;
        List        m_children;
        Point       m_position;
        int16_t     m_width;
        int16_t     m_height;
        bool        m_visible;
        bool        m_captured;

        virtual void onTouched(int16_t x, int16_t y);
        virtual void onReleased();
        virtual void draw(Graphics *g);

    public:
        UIWidget(uint16_t id, UIWidget *parent, HX8357 *display, int16_t left, int16_t top, uint16_t width, uint16_t height);
        uint16_t getID(){ return m_id; }

        Rect  getClientRect();
        Point clientToScreen(Point pt);
        Rect  clientToScreen(Rect rc);
        Point screenToClient(Point pt);

        virtual bool contains(int16_t x, int16_t y);
        virtual bool handleTouchEvent(TouchEvent e);
        virtual void show();
        virtual void hide();
        virtual void refresh();

        bool isVisible();
};



// -----------------------------------------------------------------------------
class Label : public UIWidget
{
    private:
        char   *m_text;
        int16_t m_padding;
        uint8_t m_alignment;
    protected:
        void draw(Graphics *g);
    public:
        Label(uint16_t id, UIWidget *parent, HX8357 *display, int16_t left, int16_t top, uint16_t width, uint16_t height, int size);
        Label(uint16_t id, UIWidget *parent, HX8357 *display, int16_t left, int16_t top, uint16_t width, uint16_t height, const char *text);
        void setPadding(int16_t value);
        void setTextAlign(uint8_t align);
        void setTextColor(uint16_t color);
        void setBackColor(uint16_t color);
        void setFont(int index);
        void setText(const char *text);          
        const char *getText(){ return this->m_text; }
};

// -----------------------------------------------------------------------------
class Button : public UIWidget
{
    typedef void (*CALLBACK_PROC)(Button *, void *);
    private:
        Icon         *m_icon;
        CALLBACK_PROC m_callback;
        void         *m_param;
    protected:
        void onTouched(int16_t x, int16_t y);
        void onReleased();
        void draw(Graphics *g);
    public:
        Button(uint16_t id, UIWidget *parent, HX8357 *display, int16_t left, int16_t top, uint16_t width, uint16_t height, Icon *icon);
        void attachEvent(CALLBACK_PROC proc, void *param){ 
            this->m_callback = proc; 
            this->m_param    = param;
        }
};

// -----------------------------------------------------------------------------
struct DRAWITEMSTRUCT
{
    Graphics *graphics;
    int       index;
    Rect      rect;
    bool      selected;
    bool      touched;
    void     *param;
};

struct SELECTITEMSTRUCT
{
    int index;
    void *param;
};

struct GETIMAGESTRUCT
{
    int index;
    void *param;
    uint16_t *buffer;
};

class ListBox : public UIWidget
{
    typedef void (*DRAWITEM_PROC)(ListBox *, DRAWITEMSTRUCT *);
    typedef void (*SELECTITEM_PROC)(ListBox *, SELECTITEMSTRUCT *);
    typedef void (*GETIMAGE_PROC)(ListBox *, GETIMAGESTRUCT *);
    public:
        enum{ITEM_HEIGHT = 60};
        enum{IMAGE_WIDTH = 60};
        enum{PAGE_SIZE = 4};
        static uint16_t m_images[PAGE_SIZE][ITEM_HEIGHT*ITEM_HEIGHT];
    private:
        static int      m_imageIndices[PAGE_SIZE];
        int16_t         m_itemHeight;
        int             m_itemCount;
        int             m_selectedIndex;
        int             m_touchedIndex;
        int             m_pageIndex;
        int             m_pageSize;
        DRAWITEM_PROC   m_drawItemProc;
        void           *m_drawItemParam;
        SELECTITEM_PROC m_selectItemProc;
        void           *m_selectItemParam;
        bool            m_hasImage;
        GETIMAGE_PROC   m_getImageProc;
        void           *m_getImageParam;
        Rect getItemRect(int index);
        bool isItemVisible(int index);
        int  getLastPageIndex();
        void drawItem(Graphics *g, int index, bool selected, bool touched);

    protected:
        void draw(Graphics *g);
        void onTouched(int16_t x, int16_t y);
        void onReleased();

    public:
        ListBox(uint16_t id, UIWidget *parent, HX8357 *display, int16_t left, int16_t top, int16_t width, bool hasImage);
        // ListBox(uint16_t id, UIWidget *parent, HX8357 *display, int16_t left, int16_t top, int16_t width, int16_t height, int pagesize);
        void setItems(int count, int sel);
        void setDrawItemProc(DRAWITEM_PROC proc, void *param){ 
            this->m_drawItemProc  = proc;
            this->m_drawItemParam = param; 
        }
        void getImageProc(GETIMAGE_PROC proc, void *param){
            this->m_getImageProc  = proc;
            this->m_getImageParam = param;
        }
        void attachEvent(SELECTITEM_PROC proc, void *param){ 
            this->m_selectItemProc = proc; 
            this->m_selectItemParam = param;
        }
        void setSelection(int index);
        int  getSelection(){ return this->m_selectedIndex; }
        int  getItemCount(){ return this->m_itemCount; }
        int  getPageCount(){ return (this->m_itemCount > 0)? (this->getLastPageIndex() + 1) : 0; }
        void nextPage();
        void prevPage();
        bool canMoveNextPage();
        bool canMovePrevPage();
};

// -----------------------------------------------------------------------------
class SevenSegLabel : public UIWidget
{
    typedef void (*FORMAT_PROC)(uint16_t, char *);
    private:
        enum {
            DIGIT_SPACE = 4,
            DIGIT_WIDTH = 20,
            DIGIT_HEIGHT = 30
        };
        static Icon *m_icon[11];
        int16_t      m_length;
        char        *m_value;
        char        *m_buffer;
        bool        *m_dirty;
        FORMAT_PROC  m_formatProc;
    
    protected:
        void draw(Graphics *g);

    public:
        SevenSegLabel(uint16_t id, UIWidget *parent, HX8357 *display, int16_t left, int16_t top, int16_t length);
        void setFormat(FORMAT_PROC proc){ this->m_formatProc = proc; }
        void setValue(uint16_t value);
        void refresh();
};

// -----------------------------------------------------------------------------
class PaintBox : public UIWidget
{
    typedef void (*PAINT_PROC)(PaintBox *, Graphics *, void *);
    private:
        PAINT_PROC m_paintProc;
        void      *m_context;
    protected:
        void draw(Graphics *g);
    public:
        PaintBox(uint16_t id, UIWidget *parent, HX8357 *display, int16_t left, int16_t top, int16_t width, int16_t height);
        void setPaintProc(PAINT_PROC proc, void *context){
            this->m_paintProc = proc;
            this->m_context = context;
        }
};

// -----------------------------------------------------------------------------
class SpectrumView : public UIWidget
{
    private:
        static const float SCALE;
        enum{NUM_BANDS = 16};
        int16_t m_spectrum[NUM_BANDS];
        void internalDraw(Graphics *g);
    protected:
        void draw(Graphics *g);
    public:
        SpectrumView(uint16_t id, UIWidget *parent, HX8357 *display, int16_t left, int16_t top);
        void update(AudioAnalyzeFFT1024 *fft);
        void clear();
};

//------------------------------------------------------------------------------
class ProgressBar : public UIWidget
{
    private:
        int m_maximum;
        int m_value;
        int getBarSizeOfValue(int v){
            float w = this->m_width - 2;
            return (int)((w * v) / this->m_maximum);
        }

    protected:
        void draw(Graphics *g);
    public:
        ProgressBar(uint16_t id, UIWidget *parent, HX8357 *display, int16_t left, int16_t top, int16_t width, int16_t height);
        void setMaximum(int value);
        void setValue(int value);
        int getValue(){ return this->m_value; }
};

// -----------------------------------------------------------------------------
class Desktop : public UIWidget
{
    private:
        ProgressBar *m_progressbar;
        Label       *m_label;
    protected:
        void draw(Graphics *g);
    public:
        Desktop(HX8357 *display);
        void showProgress(int maximum, const char *message);
        void updateProgress(int value);
        void hideProgress();
};

//------------------------------------------------------------------------------
class MusicPlayer;
class PlaybackView : public UIWidget
{
    private:
        enum {
            STOP = 0,
            PLAY = 1,
            PAUSE = 2,
            NUM_STATUS = 3
        };
        PaintBox      *m_statusPaintBox;
        PaintBox      *m_coverImagePaintBox;
        SpectrumView  *m_spectrumView;
        Label         *m_songTitleLabel;
        Label         *m_albumTitleLabel;
        Label         *m_artistNameLabel;
        Label         *m_albumInfoLabel;
        Label         *m_trackLengthLabel;
        Label         *m_codecInfoLabel;
        SevenSegLabel *m_trackLabel;
        SevenSegLabel *m_timeLabel;
        MusicPlayer   *m_player;
        Icon          *m_statusIcon[NUM_STATUS];
        static void handlePlayerEvent(MusicPlayer *player, uint16_t eventId, void *context);
        void drawStatus(Graphics *g);
        void drawCoverImage(Graphics *g);

    protected:
        void draw(Graphics *g);
    public:
        enum{ID = 0};
        PlaybackView(UIWidget *parent, HX8357 *display, MusicPlayer *player);
        void updateFFT(AudioAnalyzeFFT1024 *fft);
};

// -----------------------------------------------------------------------------
class ToolBar : public UIWidget
{
    public:
        enum {
            ID_TOOLBAR = 200,
            ID_STOP    = 201,
            ID_PLAY    = 202,
            ID_PAUSE   = 203,
            ID_PREV    = 204,
            ID_NEXT    = 205,
            ID_UP      = 206,
            ID_DOWN    = 207,
            ID_SONG    = 208,
            ID_ALBUM   = 209,
            ID_ARTIST  = 210,
            ID_CLOSE   = 211
        };
        enum{NUM_BUTTONS = 11};
    private:
        Button *m_buttons[NUM_BUTTONS];
    protected:
        void draw(Graphics *g);
    public:
        ToolBar(UIWidget *parent, HX8357 *display);
        Button *getToolButton(uint16_t id);
};

// -----------------------------------------------------------------------------
class SelectSongView : public UIWidget
{
    typedef void (*SELECTSONGPROC)(SelectSongView *, int, void *);
    private:
        ListBox       *m_listbox;
        MusicPlayer   *m_player;
        ToolBar       *m_toolbar;
        SELECTSONGPROC m_selectProc;
        void          *m_procParam;
        void onDrawListItem(DRAWITEMSTRUCT *dis);
        void onPlayerEvent(uint16_t eventId);
        void onPageUp();
        void onPageDown();
        void updateToolBar();
        void onSelectItem(int index);
    protected:
        void draw(Graphics *g);
    public:
        enum{ID = 1};
        SelectSongView(UIWidget *parent, HX8357 *display, MusicPlayer *player, ToolBar *toolbar);
        void show();
        void attachEvent(SELECTSONGPROC proc, void *param){
            this->m_selectProc = proc;
            this->m_procParam = param;
        }
};

// -----------------------------------------------------------------------------
class Artist;
class Album;
class SelectAlbumView : public UIWidget
{
    typedef void (*SELECTALBUMPROC)(SelectAlbumView *, Album *, void *);
    private:
        ListBox        *m_listbox;
        MusicPlayer    *m_player;
        Artist         *m_artist;
        ToolBar        *m_toolbar;
        SELECTALBUMPROC m_selectProc;
        void           *m_procParam;
        void onGetImage(GETIMAGESTRUCT *gis);
        void onDrawListItem(DRAWITEMSTRUCT *dis);
        void onPageUp();
        void onPageDown();
        void updateToolBar();
        void onSelectItem(int index);
    protected:
        void draw(Graphics *g);
    public:
        enum{ID = 2};
        SelectAlbumView(UIWidget *parent, HX8357 *display, MusicPlayer *player, ToolBar *toolbar);
        void setArtist(Artist *artist);
        void show();
        void attachEvent(SELECTALBUMPROC proc, void *param){
            this->m_selectProc = proc;
            this->m_procParam = param;
        }
        Artist *getArtist(){ return this->m_artist; }
};

// -----------------------------------------------------------------------------
class ArtistList;
class SelectArtistView : public UIWidget
{
    typedef void (*SELECTARTISTPROC)(SelectArtistView *, Artist *, void *);
    private:
        ListBox         *m_listbox;
        ArtistList      *m_artistList;
        ToolBar         *m_toolbar;
        SELECTARTISTPROC m_selectProc;
        void            *m_procParam;
        void onGetImage(GETIMAGESTRUCT *gis);
        void onDrawListItem(DRAWITEMSTRUCT *dis);
        void onPageUp();
        void onPageDown();
        void updateToolBar();
        void onSelectItem(int index);
    protected:
        void draw(Graphics *g);
    public:
        enum{ID = 3};
        SelectArtistView(UIWidget *parent, HX8357 *display, ArtistList *artistlist, ToolBar *toolbar);
        void setArtist(Artist *artist);
        void show();
        void attachEvent(SELECTARTISTPROC proc, void *param){
            this->m_selectProc = proc;
            this->m_procParam = param;
        }
};

#endif
