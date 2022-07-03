#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <play_sd_mp3.h>
#include <play_sd_aac.h>
#include <TouchScreen.h>
#include <W25Q64.h>
#include "HX8357.h"
#include "application.h"
#include "music_player.h"
#include "view.h"

// GUItool: begin automatically generated code
AudioPlaySdMp3           playSdMp3;     //xy=154,334
AudioPlaySdAac           playSdAac;     //xy=156,409
AudioMixer4              mixLeft;       //xy=413,306
AudioMixer4              mixRight;      //xy=411,456
AudioMixer4              mixFFT;        //xy=415,572
AudioAnalyzeFFT1024      fft1024;       //xy=627,569
AudioOutputI2S           i2s1;          //xy=628,387

// Lチャンネル出力用ミキサー
AudioConnection          patchCord1(playSdMp3, 0, mixLeft, 0);
AudioConnection          patchCord2(playSdAac, 0, mixLeft, 1);

// Rチャンネル出力用ミキサー
AudioConnection          patchCord3(playSdMp3, 1, mixRight, 0);
AudioConnection          patchCord4(playSdAac, 1, mixRight, 1);

// FFT用ミキサー
AudioConnection          patchCord5(playSdMp3, 0, mixFFT, 0);
AudioConnection          patchCord6(playSdAac, 0, mixFFT, 1);
AudioConnection          patchCord7(playSdMp3, 1, mixFFT, 2);
AudioConnection          patchCord8(playSdAac, 1, mixFFT, 3);

// ミキサー > I2S出力
AudioConnection          patchCord9(mixLeft, 0, i2s1, 0);
AudioConnection          patchCordA(mixRight, 0, i2s1, 1);

// ミキサー > FFT
AudioConnection          patchCordB(mixFFT, fft1024);
// GUItool: end automatically generated code

// For the Adafruit shield, these are the default.
// #define TFT_DC  3
// #define TFT_CS 10
// #define TFT_RST 2
#define PIN_DIPSW_1     2
#define PIN_DIPSW_2     3
#define PIN_LED_B       8
#define PIN_LED_R       9
#define PIN_BACKLIGHT   41

HX8357 tft;
                            //   Y+   X-   Y-  X+
// TouchScreen ts = TouchScreen(A10, A13, 26, 25, 340);
TouchScreen ts = TouchScreen(25, 26, 27, 24, 340);

MusicPlayer player(&playSdMp3, &playSdAac, &mixLeft, &mixRight);
TouchManager touch(&ts);
Application app(&tft, &player);

void setup()
{
    pinMode(PIN_BACKLIGHT, OUTPUT);
    digitalWrite(PIN_BACKLIGHT, LOW);
    pinMode(PIN_DIPSW_1, INPUT);
    pinMode(PIN_DIPSW_2, INPUT);
    pinMode(PIN_LED_B, OUTPUT);
    digitalWrite(PIN_LED_B, LOW);
    pinMode(PIN_LED_R, OUTPUT);
    digitalWrite(PIN_LED_R, LOW);
    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(COLOR_BLACK);
    Serial.begin(9600);
    if( digitalRead(PIN_DIPSW_1) == 0 )
    {
        while(!Serial)
        {
            digitalWrite(PIN_LED_R, HIGH);
            delay(250);
            digitalWrite(PIN_LED_R, LOW);
            delay(250);
        }
        digitalWrite(PIN_LED_R, LOW);
    }

    AudioMemory(20);

    if( !SD.begin(BUILTIN_SDCARD) ) 
    {
        digitalWrite(PIN_LED_R, HIGH);
        while( true ) 
        {
            Serial.println("Unable to access the SD card :-(");
            delay(500);
        }
    }

    W25Q64_begin(10);

    Serial.println("READY");

    // FFTには MP3、AACのL,R各信号を同じ比率で混合させて与える
    mixFFT.gain(0, 0.25);
    mixFFT.gain(1, 0.25);
    mixFFT.gain(2, 0.25);
    mixFFT.gain(3, 0.25);

    bool update = (digitalRead(PIN_DIPSW_2) == LOW)? true : false; 
    digitalWrite(PIN_BACKLIGHT, HIGH);
    if( app.begin(update) )
    {
        digitalWrite(PIN_LED_B, HIGH);
    }
    else
    {
        digitalWrite(PIN_LED_R, HIGH);
        while( true );
    }
}

void loop()
{
    app.loop(&fft1024, &touch);
}
