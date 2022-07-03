import os
import re
import sys
import subprocess
import struct

from pathlib import Path
from PIL import Image


class Glyph:
    def __init__(self, code):
        self.__code = code
        self.__patterns = []

    @property
    def code(self):
        return self.__code
    @property
    def patterns(self):
        return self.__patterns

    def load(self, img, x, y, w, h):
        if self.__code == 0:
            self.__patterns = [0 for n in range(w)]
            return

        for i in range(w):
            data = 0
            for j in range(h):
                r, g, b = img.getpixel((x+i, y+j))
                if r == 0:
                    data |= (1 << j)
            self.__patterns.append(data)

        # print('[U+{0:04X}] {1}'.format(self.__code, chr(self.__code)))
        print('[U+{0:04X}]'.format(self.__code))
        for p in self.__patterns:
            print('  {:0>16b}'.format(p).replace('1', '*').replace('0', ' '))
        print('')

    def save(self, fp):
        if self.__code == 0:
            return
        fp.write(struct.pack('<H', self.__code))
        fp.write(struct.pack('<H', len(self.__patterns)))
        for data in self.__patterns:
            fp.write(struct.pack('<I', data))

class Font:
    def __init__(self, bdf_path):
        self.__glyphs = []
        bdf_path = str(Path(bdf_path).absolute())
        with open(bdf_path, mode='r') as fp:
            lines = fp.read().split('\n')
        s = [s for s in lines if s.find('FONTBOUNDINGBOX') == 0][0].replace('FONTBOUNDINGBOX ', '')
        m = re.match('^(\d+)\s(\d+)\s([0-9-]+)\s([0-9-]+)', s)
        self.__bbox = {'width': int(m[1]), 'height': int(m[2])}
        print(self.__bbox)
        bmp_name = os.path.splitext(os.path.basename(bdf_path))[0]
        bmp_path = str(Path('.\\{}.bmp'.format(bmp_name)).absolute())
        subprocess.run(['bdf2bmp', '-s1', '-c16', '-w', bdf_path, bmp_path])

        entries = []
        for line in [s for s in lines if s.find('STARTCHAR') == 0]:
            m = re.match('^STARTCHAR\s+([0-9a-f]{2})([0-9a-f]{2})$', line)  # 「JISコード」を16進で表している
            if m[1] == '00':
                code = int(m[2], 16)
                if 0x20 <= code < 0x7F:
                    entries.append(code)
                else:
                    entries.append(0)
            else:
                b = bytes([0x1b, 0x24, 0x42, int(m[1], 16), int(m[2], 16), 0x1b, 0x28, 0x42])   # エスケープIN/OUT を含めたJIS漢字エンコーディング形式
                try:
                    c = b.decode('iso-2022-jp')
                    # print('{}{} -> {} (U+{:04X})'.format(m[1], m[2], c, ord(c)))
                    entries.append(ord(c))
                except:
                    entries.append(0)

        img = Image.open(bmp_path)
        img = img.convert('RGB')
        num_rows = (img.size[1]-1)//17
        index = 0
        for row in range(num_rows):
            y = 1+(self.__bbox['height'] + 1)*row
            for col in range(16):
                if entries[index] > 0:
                    x = 1+(self.__bbox['width'] + 1)*col
                    glyph = Glyph(entries[index])
                    glyph.load(img, x, y, self.__bbox['width'], self.__bbox['height'])
                    if (len(glyph.patterns) <= 8) or [p for p in glyph.patterns if p != 0]: 
                        self.__glyphs.append(glyph)
                index += 1
                if index >= len(entries):
                    break
            if index >= len(entries):
                break
        self.__glyphs.sort(key=lambda g: g.code)

    @property
    def bbox(self):
        return self.__bbox

    @property
    def glyphs(self):
        return self.__glyphs


if __name__ == '__main__':
    kanji_font = Font('.\\k16.bdf')
    ascii_font = Font('.\\a16.bdf')
    width = max([kanji_font.bbox['width'], ascii_font.bbox['width']])
    height = max([kanji_font.bbox['height'], ascii_font.bbox['height']])
    glyphs = kanji_font.glyphs + ascii_font.glyphs
    glyphs.sort(key=lambda g: g.code)
    save_path = str(Path('.\\font16.dat').absolute())
    with open(save_path, mode='wb') as fp:
        fp.write(struct.pack('<H', len(glyphs)))
        fp.write(struct.pack('BB', width, height))
        for g in glyphs:
            g.save(fp)        
