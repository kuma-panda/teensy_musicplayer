#include <Arduino.h>
#include <EEPROM.h>
#include "application.h"

// -----------------------------------------------------------------------------
Application::Application(HX8357 *display, MusicPlayer *player)
    : m_display(display), m_player(player)
{
}

// -----------------------------------------------------------------------------
bool Application::begin(bool update)
{
    this->m_desktop = new Desktop(this->m_display);
    this->m_desktop->refresh();

    uint32_t t = millis() + 2000;

    if( !this->m_artistList.load(update? Application::onLoadThumbnail : nullptr, this) )
    {
        return false;
    }

    uint16_t artistIndex = EEPROM.read(0);
    if( artistIndex >= this->m_artistList.getNumArtists() )
    {
        artistIndex = 0;
    }
    uint8_t albumIndex  = EEPROM.read(1);
    if( albumIndex >= this->m_artistList.getArtist(artistIndex)->getNumAlbums() )
    {
        albumIndex = 0;
    }
    this->m_player->setAlbum(this->m_artistList.getArtist(artistIndex)->getAlbum(albumIndex));

    this->m_toolbar = new ToolBar(this->m_desktop, this->m_display);

    this->m_toolbar->getToolButton(ToolBar::ID_PREV)->attachEvent(Application::onToolbarCommand, this);
    this->m_toolbar->getToolButton(ToolBar::ID_STOP)->attachEvent(Application::onToolbarCommand, this);
    this->m_toolbar->getToolButton(ToolBar::ID_PLAY)->attachEvent(Application::onToolbarCommand, this);
    this->m_toolbar->getToolButton(ToolBar::ID_PAUSE)->attachEvent(Application::onToolbarCommand, this);
    this->m_toolbar->getToolButton(ToolBar::ID_NEXT)->attachEvent(Application::onToolbarCommand, this);
    this->m_toolbar->getToolButton(ToolBar::ID_SONG)->attachEvent(Application::onToolbarCommand, this);
    this->m_toolbar->getToolButton(ToolBar::ID_ALBUM)->attachEvent(Application::onToolbarCommand, this);
    this->m_toolbar->getToolButton(ToolBar::ID_ARTIST)->attachEvent(Application::onToolbarCommand, this);
    this->m_toolbar->getToolButton(ToolBar::ID_CLOSE)->attachEvent(Application::onToolbarCommand, this);

    this->m_views[PlaybackView::ID    ] = new PlaybackView(this->m_desktop, this->m_display, this->m_player);
    this->m_views[SelectSongView::ID  ] = new SelectSongView(this->m_desktop, this->m_display, this->m_player, this->m_toolbar);
    this->m_views[SelectAlbumView::ID ] = new SelectAlbumView(this->m_desktop, this->m_display, this->m_player, this->m_toolbar);
    this->m_views[SelectArtistView::ID] = new SelectArtistView(this->m_desktop, this->m_display, &(this->m_artistList), this->m_toolbar);
    
    ((SelectSongView   *)this->m_views[SelectSongView::ID  ])->attachEvent(Application::onSongSelected, this);
    ((SelectAlbumView  *)this->m_views[SelectAlbumView::ID ])->attachEvent(Application::onAlbumSelected, this);
    ((SelectArtistView *)this->m_views[SelectArtistView::ID])->attachEvent(Application::onArtistSelected, this);

    this->m_player->attachEvent(Application::onPlayerEvent, this);

    this->m_player->triggerEvent(MusicPlayer::EVT_ALBUM_CHANGED);

    while( millis() < t ){}

    this->m_toolbar->show();
    this->m_toolbar->refresh();
    this->showPlayback();

    // this->m_player->play(0);
    return true;
}

// -----------------------------------------------------------------------------
void Application::onLoadThumbnail(int value, int count, void *param)
{
    Application *app = (Application *)param;

    if( value == 0 )
    {
        EEPROM.write(0, 0);
        EEPROM.write(1, 0);
        app->m_desktop->showProgress(count, "データを更新中です");
        app->m_desktop->refresh();
    }
    else
    {
        app->m_desktop->updateProgress(value);
        if( value == count )
        {
            app->m_desktop->hideProgress();
            app->m_desktop->refresh();
        }
    }
}

// -----------------------------------------------------------------------------
void Application::onToolbarCommand(Button *sender, void *param)
{
    Application *app = (Application *)param;

    Serial.println(sender->getID(), DEC);

    switch( sender->getID() )
    {
        case ToolBar::ID_PREV:
            app->m_player->prev();
            break;
        case ToolBar::ID_NEXT:
            app->m_player->next();
            break;
        case ToolBar::ID_PLAY:
            app->m_player->play(0);
            break;
        case ToolBar::ID_STOP:
            app->m_player->stop();
            break;
        case ToolBar::ID_PAUSE:
            app->m_player->pause();
            break;
        case ToolBar::ID_SONG:
            app->selectSong();
            break;
        case ToolBar::ID_ALBUM:
            app->selectAlbum();
            break;
        case ToolBar::ID_ARTIST:
            app->selectArtist();
            break;
        case ToolBar::ID_CLOSE:
            app->showPlayback();
            break;
    }
}

// -----------------------------------------------------------------------------
void Application::onPlayerEvent(MusicPlayer *player, uint16_t eventId, void *context)
{
    Application *app = (Application *)context;

    if( eventId == MusicPlayer::EVT_STATUS_CHANGED || eventId == MusicPlayer::EVT_ALBUM_CHANGED )
    {
        if( player->isPlaying() )
        {
            app->m_toolbar->getToolButton(ToolBar::ID_STOP)->show();
            app->m_toolbar->getToolButton(ToolBar::ID_PLAY)->hide();
        }
        else
        {
            app->m_toolbar->getToolButton(ToolBar::ID_STOP)->hide();
            app->m_toolbar->getToolButton(ToolBar::ID_PLAY)->show();
        }
        app->m_toolbar->refresh();
    }
}

// -----------------------------------------------------------------------------
// void Application::loadThumbnails()
// {
//     int count = 0;
//     char name[20];
//     uint8_t buffer[2][4096];

//     for( int i = 0 ; i < 1000 ; i++ )
//     {
//         sprintf(name, "/thumbs/%d.thb", i);
//         if( SD.exists(name) )
//         {
//             count = i + 1;
//         }
//         else
//         {
//             break;
//         }
//     }
//     if( !count )
//     {
//         Serial.println("No thumbnail data was found in /thumbs");
//         while( true );
//     }

//     this->m_desktop->showProgress(count, "画像データを読み込んでいます");
//     for( int i = 0 ; i < count ; i++ )
//     {
//         this->m_desktop->updateProgress((int)(i+1));
//         memset(buffer[0], 0x00, 4096);
//         memset(buffer[1], 0x00, 4096);
//         sprintf(name, "/thumbs/%d.thb", i);
//         File f = SD.open(name);
//         if( !f )
//         {
//             Serial.printf("Unable to open %s\n", name);
//             break;
//         }
//         f.read(buffer[0], 3600);
//         f.read(buffer[1], 3600);
//         f.close();

//         // ページ単位でデータを書き込む
//         // ページサイズは 256byte なので、１セクタあたり15ページを使用する
//         for( uint16_t s = 0 ; s < 2 ; s++ )
//         {
//             uint16_t sect_no = i*2 + s;
//             W25Q64_eraseSector(sect_no, true);
//             for( int n = 0 ; n < 16 ; n++ )
//             {
//                 uint16_t offset = n*256;
//                 W25Q64_pageWrite(sect_no, offset, buffer[s]+offset, 256);
//             }
//         }
//         Serial.printf("write to flash done (%3d)\n", i);
//     }
// }

// -----------------------------------------------------------------------------
void Application::switchView(uint16_t id)
{
    for( uint16_t i = 0 ; i < Application::NUM_VIEWS ; i++ )
    {
        if( i != id )
        {
            this->m_views[i]->hide();
        }
    }
    this->m_activeViewID = id;
    this->m_views[id]->show();
    this->m_views[id]->refresh();
}

// -----------------------------------------------------------------------------
void Application::loop(AudioAnalyzeFFT1024 *fft, TouchManager *touch)
{
    this->m_player->control();
    UIWidget *active = this->m_views[this->m_activeViewID];
    if( active->getID() == PlaybackView::ID )
    {
        ((PlaybackView *)active)->updateFFT(fft);
    }
    touch->execute(this->m_desktop);   
}

// ----------------------------------------------------------------------------
void Application::showPlayback()
{
    switchView(PlaybackView::ID);
    this->m_toolbar->getToolButton(ToolBar::ID_ARTIST)->hide();
    this->m_toolbar->getToolButton(ToolBar::ID_ALBUM)->hide();
    this->m_toolbar->getToolButton(ToolBar::ID_UP)->hide();
    this->m_toolbar->getToolButton(ToolBar::ID_DOWN)->hide();
    this->m_toolbar->getToolButton(ToolBar::ID_CLOSE)->hide();
    this->m_toolbar->getToolButton(ToolBar::ID_SONG)->show();
    this->m_toolbar->getToolButton(ToolBar::ID_PREV)->show();
    this->m_toolbar->getToolButton(ToolBar::ID_NEXT)->show();
    this->m_toolbar->getToolButton(ToolBar::ID_PAUSE)->show();
    if( this->m_player->isPlaying() )
    {
        this->m_toolbar->getToolButton(ToolBar::ID_STOP)->show();
        this->m_toolbar->getToolButton(ToolBar::ID_PLAY)->hide();
    }
    else
    {
        this->m_toolbar->getToolButton(ToolBar::ID_STOP)->hide();
        this->m_toolbar->getToolButton(ToolBar::ID_PLAY)->show();
    }
    this->m_toolbar->refresh();
}

// -----------------------------------------------------------------------------
void Application::selectSong()
{
    if( this->m_activeViewID == PlaybackView::ID )
    {
        this->switchView(SelectSongView::ID);
    }
}

// -----------------------------------------------------------------------------
void Application::onSongSelected(SelectSongView *sender, int index, void *param)
{
    Application *app = (Application *)param;
    app->m_player->play(index);
    app->showPlayback();
}

// -----------------------------------------------------------------------------
void Application::selectAlbum(Artist *artist)
{
    if( !artist )
    {
        artist = this->m_player->getPlayList()->getAlbum()->getArtist();
    }
    ((SelectAlbumView *)this->m_views[SelectAlbumView::ID])->setArtist(artist);
    this->switchView(SelectAlbumView::ID);
}

// -----------------------------------------------------------------------------
void Application::onAlbumSelected(SelectAlbumView *sender, Album *album, void *param)
{
    Application *app = (Application *)param;

    Artist *artist = album->getArtist();
    uint8_t artistIndex = (uint8_t)(app->m_artistList.getIndexOfArtist(artist));
    uint8_t albumIndex  = (uint8_t)(artist->getIndexOfAlbum(album));
    EEPROM.write(0, artistIndex);
    EEPROM.write(1, albumIndex);
    app->m_player->setAlbum(album);
    app->showPlayback();
}

// -----------------------------------------------------------------------------
void Application::selectArtist()
{
    Artist *artist = ((SelectAlbumView *)this->m_views[SelectAlbumView::ID])->getArtist();
    ((SelectArtistView *)this->m_views[SelectArtistView::ID])->setArtist(artist);
    this->switchView(SelectArtistView::ID);
}

// -----------------------------------------------------------------------------
void Application::onArtistSelected(SelectArtistView *sender, Artist *artist, void *param)
{
    Application *app = (Application *)param;
    app->selectAlbum(artist);
}
