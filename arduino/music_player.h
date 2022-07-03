#ifndef MusicPlayerH
#define MusicPlayerH

#include <Arduino.h>
#include <Audio.h>
#include <SPI.h>
#include <SD.h>

#include <play_sd_mp3.h> //mp3 decoder
#include <play_sd_aac.h> // AAC decoder

#include "display.h"

//------------------------------------------------------------------------------
class Artist;
class Album
{
    public:
        enum{MAX_TRACK_COUNT = 20};
        enum{MAX_TITLE_BYTES = 64};
    private:
        uint16_t m_id;
        Artist  *m_artist;                 // このアルバムを所有するアーティスト
        char     m_title[MAX_TITLE_BYTES]; // アルバムタイトル
        uint8_t  m_numTracks;              // このアルバムに収められている曲の数
        uint16_t m_totalTime;              // 総演奏時間（各曲の演奏時間の総和）
        uint16_t m_year;                   // アルバムの発売年（西暦）
        char     m_folderName[10];         // フォルダ名
    public:
        Album(Artist *artist);
        void load(File& f);
        uint16_t getID(){ return this->m_id; }
        Artist *getArtist(){ return this->m_artist; }
        const char *getTitle(){ return this->m_title; }
        const char *getFolderName(){ return this->m_folderName;}
        void getDirectory(char *buffer);
        uint16_t getNumTracks(){ return this->m_numTracks; }
        uint16_t getTotalTime(){ return this->m_totalTime; }
        uint16_t getYear(){ return this->m_year; }
};

//------------------------------------------------------------------------------
class Artist
{
    public:
        enum{MAX_ALBUM_COUNT = 10};
        enum{MAX_NAME_BYTES = 64};
    private:
        uint16_t m_id;
        char     m_folderName[10];              // フォルダ名
        uint16_t m_numAlbums;
        Album   *m_albums[MAX_ALBUM_COUNT];     // アルバムのリスト
        char     m_name[MAX_NAME_BYTES];        // アーティスト名
    public:
        Artist();
        void load(File& f);
        uint16_t getID(){ return this->m_id; }
        const char *getName(){ return this->m_name; }
        const char *getFolderName(){ return this->m_folderName; }
        uint16_t getNumAlbums(){ return this->m_numAlbums; }
        Album *getAlbum(int index){ return this->m_albums[index]; }
        int getIndexOfAlbum(Album *album);
};

//------------------------------------------------------------------------------
class ArtistList
{
    typedef void (*LOADING_CALLBACK)(int, int, void *);
    private:
        enum{MAX_ARTIST_COUNT = 100};
        uint16_t m_numArtists;
        Artist *m_artists[MAX_ARTIST_COUNT];
        bool loadThumbnails(LOADING_CALLBACK callback, void *context);
    public:
        ArtistList();
        bool load(LOADING_CALLBACK callback, void *context);
        uint16_t getNumArtists(){ return m_numArtists; }
        Artist *getArtist(int index){ return this->m_artists[index]; }
        int getIndexOfArtist(Artist *artist);
};

// -----------------------------------------------------------------------------
class PlayList
{
    public:
        enum{
            CODEC_MP3 = 0,
            CODEC_AAC = 1
        };
        enum{MAX_SONG_COUNT = 20};
        enum{MAX_TITLE_BYTES = 64};
        enum{COVER_IMAGE_SIZE = 150};
    private:
        Album   *m_album;                                       // 選択されたアルバム
        uint16_t m_numSongs;                                    // 曲数
        char     m_titles[MAX_SONG_COUNT][MAX_TITLE_BYTES];     // 各曲のタイトル
        uint16_t m_durations[MAX_SONG_COUNT];                   // 各曲の演奏時間(秒)
        char     m_fileNames[MAX_SONG_COUNT][10];               // 各曲ファイル名
        uint16_t m_bitRate[MAX_SONG_COUNT];                     // 各曲のビットレート(kbps単位)(320など)
        uint16_t m_sampleRate[MAX_SONG_COUNT];                  // 各曲のサンプルレート(100Hz単位)(441など)
        uint8_t  m_codec;                                       // コーデック種別(MP3/AAC)
        Bitmap  *m_image;                                       // アルバム画像
    public:
        PlayList();
        void load(Album *album);
        Album *getAlbum(){ return this->m_album; }
        uint16_t getNumSongs(){ return this->m_numSongs; }
        const char *getTitle(int index){ return this->m_titles[index]; }
        uint16_t getDuration(int index){ return this->m_durations[index]; }
        uint16_t getBitRate(int index){ return this->m_bitRate[index]; }
        uint16_t getSampleRate(int index){ return this->m_sampleRate[index]; }
        void getFilePath(int index, char *buffer);
        uint8_t getCodec(){ return this->m_codec; }
        Bitmap *getImage(){ return this->m_image; }
};

// -----------------------------------------------------------------------------
class PlayerTimer
{
    private:
        uint32_t m_startTime;
        uint32_t m_stopTime;
        bool     m_active;
    public:
        PlayerTimer() : m_startTime(0), m_stopTime(0), m_active(false){}
        uint32_t getValue(){
            if( !m_active )
            {
                return (m_stopTime - m_startTime)/1000;
            }
            return (millis() - m_startTime)/1000;
        }
        void start(){
            m_startTime = millis() - (m_stopTime - m_startTime);
            m_active = true;
        }
        void stop(){
            m_stopTime = millis();
            m_active = false;
        }
        void reset(){
            m_startTime = 0;
            m_stopTime = 0;
            m_active = false;
        }
};

//------------------------------------------------------------------------------
class MusicPlayer;
typedef void (*PlayerProc)(MusicPlayer *, uint16_t, void *);
struct PlayerEvent
{
    MusicPlayer *sender;
    PlayerProc   proc;
    void        *context;
    void emit(uint16_t eventId){
        this->proc(this->sender, eventId, this->context);
    };
};

class MusicPlayer
{
    public:
        // 発火するイベント
        enum {
            EVT_ALBUM_CHANGED,      // プレイリスト（アルバム）が変わった
            EVT_TRACK_CHANGED,      // 曲が変わった
            EVT_STATUS_CHANGED,     // ステータス（再生・停止・一時停止）が変わった
            EVT_TIME_CHANGED        // 演奏時間（秒単位）が変わった
        };
    private:
        AudioPlaySdMp3 *m_MP3;
        AudioPlaySdAac *m_AAC;
        AudioMixer4    *m_left;
        AudioMixer4    *m_right;
        AudioCodec     *m_currentCodec;
        PlayList       *m_playList;
        uint16_t        m_currentSongIndex;
        bool            m_playing; 
        bool            m_paused;
        List            m_eventHandler;
        PlayerTimer     m_timer;

    public:
        MusicPlayer(AudioPlaySdMp3 *mp3, AudioPlaySdAac *aac, AudioMixer4 *left, AudioMixer4 *right);
        void control();
        void setAlbum(Album *album);
        void play(uint16_t index);
        void stop();
        void pause();
        void prev();
        void next();

        void attachEvent(PlayerProc proc, void *context);
        void triggerEvent(uint16_t eventid);

        PlayList *getPlayList(){ return this->m_playList; }
        uint16_t  getCurrentTrackNumber();
        const char *getCurrentSongTitle();
        uint16_t  getCurrentSongLength();
        uint16_t  getCurrentBitRate();
        uint16_t  getCurrentSampleRate();
        uint16_t  getElapsedTime();
        bool      isPlaying(){ return this->m_playing; }
        bool      isPaused(){ return this->m_paused; }
};

//------------------------------------------------------------------------------
#endif
