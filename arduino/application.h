#ifndef APPLICATION_H
#define APPLICATION_H

#include <Arduino.h>
#include <Audio.h>
#include "HX8357.h"
#include "music_player.h"
#include "view.h"

class Application
{
    private:
        enum{NUM_VIEWS = 4};
        UIWidget     *m_views[NUM_VIEWS];
        Desktop      *m_desktop;
        ToolBar      *m_toolbar;
        HX8357       *m_display;
        MusicPlayer  *m_player;
        ArtistList    m_artistList;
        uint16_t      m_activeViewID;

        static void onLoadThumbnail(int value, int count, void *param);
        static void onToolbarCommand(Button *sender, void *param);
        static void onPlayerEvent(MusicPlayer *sender, uint16_t eventId, void *context);
        static void onSongSelected(SelectSongView *sender, int index, void *param);
        static void onAlbumSelected(SelectAlbumView *sender, Album *album, void *param);
        static void onArtistSelected(SelectArtistView *sender, Artist *artist, void *param);

        // void loadThumbnails();
        void showPlayback();
        void switchView(uint16_t id);
        void selectSong();
        void selectAlbum(Artist *artist=nullptr);
        void selectArtist();

    public:
        Application(HX8357 *display, MusicPlayer *player);
        bool begin(bool update);
        void loop(AudioAnalyzeFFT1024 *fft, TouchManager *touch);
};

#endif
