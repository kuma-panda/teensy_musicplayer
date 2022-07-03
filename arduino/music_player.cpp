#include <SPI.h>
#include <SD.h>
#include <W25Q64.h>
#include "music_player.h"


static uint8_t readByte(File& f)
{
    static uint8_t buf;
    f.read(&buf, 1);
    return buf;
}

static uint16_t readWord(File& f)
{
    static uint16_t buf;
    f.read((uint8_t *)&buf, 2);
    return buf;
}

static void readString(File& f, char *buf)
{
    uint8_t len = readByte(f);
    f.read(buf, len);
    buf[len] = '\0';
}

//==============================================================================
//   ArtistList
//==============================================================================
ArtistList::ArtistList() : m_numArtists(0)
{
}

//------------------------------------------------------------------------------
bool ArtistList::load(ArtistList::LOADING_CALLBACK callback, void *context)
{
    File f = SD.open("/playdata.bin");
    if( !f )
    {
        Serial.println("Cannot open file: playdata.bin");
        return false;
    }

    this->m_numArtists = (uint16_t)readByte(f);
    for( uint16_t n = 0 ; n < this->m_numArtists ; n++ )
    {
        this->m_artists[n] = new Artist();
        this->m_artists[n]->load(f);
    }
    f.close();

    return this->loadThumbnails(callback, context);
}

// -----------------------------------------------------------------------------
bool ArtistList::loadThumbnails(ArtistList::LOADING_CALLBACK callback, void *context)
{
    if( !callback )
    {
        return true;
    }

    int count = 0;
    char name[20];
    uint8_t buffer[2][4096];
    for( int i = 0 ; i < 1000 ; i++ )
    {
        sprintf(name, "/thumbs/%d.thb", i);
        if( SD.exists(name) )
        {
            count = i + 1;
        }
        else
        {
            break;
        }
    }
    if( count == 0 )
    {
        Serial.println("No thumbnail file exists in /thumbs");
        return false;
    }
    callback(0, count, context);
    for( int i = 0 ; i < count ; i++ )
    {
        memset(buffer[0], 0x00, 4096);
        memset(buffer[1], 0x00, 4096);
        sprintf(name, "/thumbs/%d.thb", i);
        File f = SD.open(name);
        f.read(buffer[0], 3600);
        f.read(buffer[1], 3600);
        f.close();

        // ページ単位でデータを書き込む
        // ページサイズは 256byte なので、１セクタあたり15ページを使用する
        for( uint16_t s = 0 ; s < 2 ; s++ )
        {
            uint16_t sect_no = i*2 + s;
            W25Q64_eraseSector(sect_no, true);
            for( int n = 0 ; n < 16 ; n++ )
            {
                uint16_t offset = n*256;
                W25Q64_pageWrite(sect_no, offset, buffer[s]+offset, 256);
            }
        }
        // delay(500);
        Serial.printf("write to flash done (%3d)\n", i);
        callback(i+1, count, context);
    }
    return true;
}

// -----------------------------------------------------------------------------
int ArtistList::getIndexOfArtist(Artist *artist)
{
    for( uint16_t n = 0 ; n < this->m_numArtists ; n++ )
    {
        if( artist == this->m_artists[n] )
        {
            return (int)n;
        }
    }
    return -1;
}



//==============================================================================
//   Artist
//==============================================================================
Artist::Artist()
{
}

//------------------------------------------------------------------------------
void Artist::load(File& f)
{
    this->m_id = readWord(f);
    readString(f, this->m_folderName);
    readString(f, this->m_name);
    this->m_numAlbums = (uint16_t)readByte(f);
    for( uint16_t n = 0 ; n < this->m_numAlbums ; n++ )
    {
        this->m_albums[n] = new Album(this);
        this->m_albums[n]->load(f);
    }
}

// -----------------------------------------------------------------------------
int Artist::getIndexOfAlbum(Album *album)
{
    for( int i = 0 ; i < (int)this->m_numAlbums ; i++ )
    {
        if( album == this->m_albums[i] )
        {
            return i;
        }
    }
    return -1;
}



//==============================================================================
//   Album
//==============================================================================
Album::Album(Artist *artist) : m_artist(artist)
{
}

//------------------------------------------------------------------------------
void Album::load(File& f)
{
    this->m_id = readWord(f);
    readString(f, this->m_folderName);
    readString(f, this->m_title);
    this->m_year = readWord(f);
    this->m_numTracks = readByte(f);
    this->m_totalTime = readWord(f);
}

// -----------------------------------------------------------------------------
void Album::getDirectory(char *buffer)
{
    buffer[0] = '/';
    strcpy(buffer+1, this->m_artist->getFolderName());
    strcat(buffer, "/");
    strcat(buffer, this->m_folderName);
}



//==============================================================================
//   PlayList
//==============================================================================
PlayList::PlayList() : m_album(nullptr)
{
    this->m_image = new Bitmap(PlayList::COVER_IMAGE_SIZE, PlayList::COVER_IMAGE_SIZE);
}

// -----------------------------------------------------------------------------
void PlayList::load(Album *album)
{
    char path[32]; // '/xxxxxxxx/xxxxxxxx/album.bin' 1+8+1+8+1+9+1
    this->m_album = album;
    this->m_album->getDirectory(path);
    strcat(path, "/album.bin");
    File f = SD.open(path);
    if( !f )
    {
        Serial.print("cannot open ");
        Serial.println(path);
        while(1){}
    }
    this->m_codec = readByte(f);
    this->m_numSongs = (uint16_t)readByte(f);
    for( uint16_t n = 0 ; n < this->m_numSongs ; n++ )
    {
        readString(f, this->m_fileNames[n]);
        this->m_durations[n] = readWord(f);
        this->m_bitRate[n] = readWord(f);
        this->m_sampleRate[n] = readWord(f);
        readString(f, this->m_titles[n]);
    }
    this->m_image->load(f);
    f.close();
}

// -----------------------------------------------------------------------------
void PlayList::getFilePath(int index, char *buffer)
{
    this->m_album->getDirectory(buffer);
    strcat(buffer, "/");
    strcat(buffer, this->m_fileNames[index]);
}



//==============================================================================
//   MusicPlayer
//==============================================================================
MusicPlayer::MusicPlayer(AudioPlaySdMp3 *mp3, AudioPlaySdAac *aac, AudioMixer4 *left, AudioMixer4 *right)
    : m_MP3(mp3), m_AAC(aac), m_left(left), m_right(right), m_playing(false), m_paused(false)
{
    this->m_playList = new PlayList();
    this->m_currentCodec = this->m_MP3;
    // this->stop();
}

// -----------------------------------------------------------------------------
//   プレイリストの再生処理を行う
// -----------------------------------------------------------------------------
void MusicPlayer::control()
{
    static uint16_t t0;
    uint16_t t1 = this->getElapsedTime();
    if( t1 != t0 )
    {
        this->triggerEvent(MusicPlayer::EVT_TIME_CHANGED);
        t0 = t1;
    }

    if( !this->m_playing )
    {
        return;
    }

    if( !this->m_currentCodec->isPlaying() )
    {
        Serial.println("PLAYING TRACK ENDED");
        ++this->m_currentSongIndex;
        if( this->m_currentSongIndex < this->m_playList->getNumSongs() )
        {
            this->play(this->m_currentSongIndex);
            this->triggerEvent(MusicPlayer::EVT_TRACK_CHANGED);
        }
        else
        {
            this->stop();
        }
    }
}

// -----------------------------------------------------------------------------
//   演奏対象のアルバムを設定する
// -----------------------------------------------------------------------------
void MusicPlayer::setAlbum(Album *album)
{
    this->m_MP3->stop();
    this->m_AAC->stop();
    this->m_playing = false;
    this->m_paused = false;
    this->m_playList->load(album);
    if( this->m_playList->getCodec() == PlayList::CODEC_AAC )
    {
        this->m_currentCodec = this->m_AAC;
    }
    else
    {
        this->m_currentCodec = this->m_MP3;
    }
    this->triggerEvent(MusicPlayer::EVT_ALBUM_CHANGED);
}

// -----------------------------------------------------------------------------
//   指定した曲を再生（indexは先頭の曲なら0）
// -----------------------------------------------------------------------------
void MusicPlayer::play(uint16_t index)
{
    // this->m_MP3->stop();
    // this->m_AAC->stop();
    this->m_left->gain(0, 0);
    this->m_left->gain(1, 0);
    this->m_right->gain(0, 0);
    this->m_right->gain(1, 0);
    delay(100);
    this->m_paused = false;
    if( index >= this->m_playList->getNumSongs() )
    {
        index = this->m_playList->getNumSongs() - 1;
    }
    this->m_currentSongIndex = index;
    char path[48];
    this->m_playList->getFilePath(index, path);
    if( this->m_playList->getCodec() == PlayList::CODEC_AAC )
    {
        this->m_AAC->play(path);
        delay(100);
        this->m_left->gain(1, 1);
        this->m_right->gain(1, 1);
    }
    else
    {
        this->m_MP3->play(path);
        delay(100);
        this->m_left->gain(0, 1);
        this->m_right->gain(0, 1);
    }
    this->m_playing = true;
    this->triggerEvent(MusicPlayer::EVT_STATUS_CHANGED);
    this->m_timer.reset();
    this->m_timer.start();
    Serial.println("PLAY");
}

// -----------------------------------------------------------------------------
//   停止
// -----------------------------------------------------------------------------
void MusicPlayer::stop()
{
    Serial.println("STOP");
    this->m_timer.reset();
    this->m_MP3->stop();
    this->m_AAC->stop();
    this->m_left->gain(0, 0);
    this->m_left->gain(1, 0);
    this->m_right->gain(0, 0);
    this->m_right->gain(1, 0);
    this->m_currentSongIndex = 0;
    this->m_paused = false;
    this->m_playing = false;
    this->triggerEvent(MusicPlayer::EVT_STATUS_CHANGED);
}

// -----------------------------------------------------------------------------
//   一時停止。一時停止中なら演奏再開
// -----------------------------------------------------------------------------
void MusicPlayer::pause()
{
    if( this->m_paused )
    {
        this->m_paused = this->m_currentCodec->pause(false);
        this->m_timer.start();
    }
    else if( this->m_playing )
    {
        this->m_paused = this->m_currentCodec->pause(true);
        this->m_timer.stop();
    }
    this->triggerEvent(MusicPlayer::EVT_STATUS_CHANGED);
}

// -----------------------------------------------------------------------------
//   前の曲を再生。先頭の曲なら頭に戻って再生。
// -----------------------------------------------------------------------------
void MusicPlayer::prev()
{
    if( !this->m_playing )
    {
        return;
    }
    uint16_t prevIndex = this->m_currentSongIndex;
    if( this->m_currentSongIndex > 0 )
    {
        prevIndex--;
    }
    this->play(prevIndex);
    this->triggerEvent(MusicPlayer::EVT_TRACK_CHANGED);
}

// -----------------------------------------------------------------------------
//   次の曲を再生
// -----------------------------------------------------------------------------
void MusicPlayer::next()
{
    if( !this->m_playing )
    {
        return;
    }
    this->play(this->m_currentSongIndex + 1);
    this->triggerEvent(MusicPlayer::EVT_TRACK_CHANGED);
}

// -----------------------------------------------------------------------------
//   現在演奏中の曲のトラック番号(1～)を返す。停止中なら0を返す。
// -----------------------------------------------------------------------------
uint16_t MusicPlayer::getCurrentTrackNumber()
{
    if( this->m_playing )
    {
        return this->m_currentSongIndex + 1;
    }
    return 0;
}

// -----------------------------------------------------------------------------
//   現在演奏中の曲のタイトルを返す。停止中ならnullptrを返す。
// -----------------------------------------------------------------------------
const char *MusicPlayer::getCurrentSongTitle()
{
    if( this->m_playing )
    {
        return this->m_playList->getTitle(this->m_currentSongIndex);
    }
    return nullptr;
}

uint16_t MusicPlayer::getCurrentSongLength()
{
    if( this->m_playing )
    {
        return this->m_playList->getDuration(this->m_currentSongIndex);
    }
    return 0;
}

uint16_t MusicPlayer::getCurrentBitRate()
{
    if( this->m_playing )
    {
        return this->m_playList->getBitRate(this->m_currentSongIndex);
    }
    return 0;
}

uint16_t MusicPlayer::getCurrentSampleRate()
{
    if( this->m_playing )
    {
        return this->m_playList->getSampleRate(this->m_currentSongIndex);
    }
    return 0;
}

// -----------------------------------------------------------------------------
//   現在演奏中の曲の再生経過時間を秒単位で返す。停止中なら0を返す。
// -----------------------------------------------------------------------------
uint16_t MusicPlayer::getElapsedTime()
{
    if( this->m_currentCodec->isPlaying() )
    {
        return (uint16_t)this->m_timer.getValue();
        // return (uint16_t)(this->m_currentCodec->positionMillis() / 1814000);    // 320000 / 4 = 
    }
    return 0;
}

// -----------------------------------------------------------------------------
void MusicPlayer::attachEvent(PlayerProc proc, void *context)
{
    PlayerEvent *event = new PlayerEvent();
    event->sender  = this;
    event->proc    = proc;
    event->context = context;
    this->m_eventHandler.add(event);
}

// -----------------------------------------------------------------------------
void MusicPlayer::triggerEvent(uint16_t eventid)
{
    this->m_eventHandler.forEach([](int n, void *value, void *param){
        PlayerEvent *event = (PlayerEvent *)value;
        uint16_t id = *((uint16_t *)param);
        event->emit(id);
        return true;
    }, &eventid);
}
