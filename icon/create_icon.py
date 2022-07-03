import sys
import os

from PIL import Image

def dump_image(path, name):
    img = Image.open(path)
    img = img.convert('RGBA')
    r, g, b, a = img.split()
    # name = os.path.splitext(os.path.basename(path).upper())[0]
    print('const uint8_t {}[] PROGMEM = {{'.format(name))
    print('    {0: >3}, // width ({0}px)'.format(a.size[0]))
    print('    {0: >3}, // height({0}px)'.format(a.size[1]))
    for y in range(a.size[1]):
        values = []
        for x in range(a.size[0]):
            values.append('{: >3}'.format(a.getpixel((x, y))))
        print('    {},'.format(','.join(values)))
    print('};')
    print('')

if __name__ == '__main__':
    images = [
        ('48\\stop.png',   'ICON_STOP_48'),
        ('48\\play.png',   'ICON_PLAY_48'),
        ('48\\pause.png',  'ICON_PAUSE_48'),
        ('32\\stop.png',   'ICON_STOP_32'),
        ('32\\play.png',   'ICON_PLAY_32'),
        ('32\\pause.png',  'ICON_PAUSE_32'),
        ('32\\prev.png',   'ICON_PREV_32'),
        ('32\\next.png',   'ICON_NEXT_32'),
        ('32\\song.png',   'ICON_SONG_32'),
        ('32\\album.png',  'ICON_ALBUM_32'),
        ('32\\artist.png', 'ICON_ARTIST_32'),
        ('32\\up.png',     'ICON_UP_32'),
        ('32\\down.png',   'ICON_DOWN_32'),
        ('32\\close.png',  'ICON_CLOSE_32')
    ]

    for img in images:
        dump_image(*img)
