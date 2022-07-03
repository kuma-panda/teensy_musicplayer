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
            print('  {:0>20b}'.format(p).replace('1', '*').replace('0', ' '))
        print('')

    def save(self, fp):
        if self.__code == 0:
            return
        fp.write(struct.pack('<H', self.__code))
        fp.write(struct.pack('<H', len(self.__patterns)))
        for data in self.__patterns:
            fp.write(struct.pack('<I', data))

    # def jis_to_sjis(self, jiscode):
    #     if jiscode <= 255:
    #         return jiscode

    #     if jiscode == 9332:


    #     print('jis: {}'.format(jiscode))
    #     knj1 = jiscode // 256   # １バイト目
    #     knj2 = jiscode % 256    # ２バイト目

    #     # 2byte目の変換処理
    #     # 1. 1byte目の変換前の値が 0x01(1) とのANDの結果、0x01(1)か判定
    #     # 2. 1.の結果が真の場合で、さらに、2バイト目が0x5F(95)以下の場合、
    #     #    2バイト目に0x1F(31)を加算。それ以外の場合は、0x20(32)を加算。
    #     # 3. 1.の結果が偽の場合は、2バイト目に0x7E(126)を加算。
    #     if knj1 & 0x01:
    #         if knj2 <= 0x5F:
    #             knj2 += 0x1F
    #         else:
    #             knj2 += 0x20            
    #     else:
    #         knj2 += 0x7E

    #     # 1byte目の変換処理
    #     # 1. 1byte目から 0x21(33) 減算後に、/2 を実行 (端数は切り捨て)
    #     # 2. 1byte目の元の値が0x5E(94)以下の場合、1.の結果に0x81(129)を加算。
    #     #    それ以外の場合、1.の結果に0xC1(193)を加算。
    #     if knj1 <= 0x5E:
    #         knj1 = ((knj1 - 0x21) // 2) + 0x81
    #     else:
    #         knj1 = ((knj1 - 0x21) // 2) + 0xC1         

    #     # if knj1 & 0x01:
    #     #     knj1 >>= 1
    #     #     if knj1 < 0x2F:
    #     #         knj1 += 0x71
    #     #     else: 
    #     #         knj1 -= 0x4F
    #     #     if knj2 > 0x5F:
    #     #         knj2 += 0x20
    #     #     else:
    #     #         knj2 += 0x1F
    #     # else:
    #     #     knj1 >>= 1
    #     #     if knj1 < 0x2F:
    #     #         knj1 += 0x70
    #     #     else:
    #     #         knj1 -= 0x50
    #     #     knj2 += 0x7E

    #     fmt = ''.join([
    #         'B' if knj1 >= 0 else 'b',
    #         'B' if knj2 >= 0 else 'b'
    #     ])
    #     sjis_code = struct.unpack('>H', struct.pack(fmt, knj1, knj2))
    #     print('(shift-jis: {:04X})'.format(sjis_code[0]))
    #     # return sjis_code[0]
    #     sjis_char = struct.pack(fmt, knj1, knj2).decode('cp932')
    #     return ord(sjis_char)

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

        for line in [s for s in lines if s.find('STARTCHAR') == 0]:
            m = re.match('STARTCHAR\s(\d+)-(\d+)', line)
            if not m:
                break
            ku = int(m[1])
            ten = int(m[2])
            code = self.jis_kuten_to_unicode(ku, ten)
            self.__glyphs.append(Glyph(code))
        if not self.__glyphs:
            for line in [s for s in lines if s.find('ENCODING') == 0]:
                m = re.match('ENCODING\s(\d+)', line)
                code = int(m[1])
                if code >= 0xA0:
                    code = 0xFF60 + (code - 0xA0)
                self.__glyphs.append(Glyph(code))

        img = Image.open(bmp_path)
        img = img.convert('RGB')
        num_rows = (img.size[1]-1)//21
        index = 0
        for row in range(num_rows):
            y = 1+(self.__bbox['height'] + 1)*row
            for col in range(16):
                x = 1+(self.__bbox['width'] + 1)*col
                self.__glyphs[index].load(img, x, y, self.__bbox['width'], self.__bbox['height'])
                index += 1
                if index >= len(self.__glyphs):
                    break
            if index >= len(self.__glyphs):
                break
        self.__glyphs = [g for g in self.__glyphs if g.code != 0]
        self.__glyphs.sort(key=lambda g: g.code)

    @property
    def bbox(self):
        return self.__bbox

    @property
    def glyphs(self):
        return self.__glyphs

    def jis_kuten_to_unicode(self, ku, ten):
        # JIS X 0213 の１面のみが変換対象
        try:
            euc0 = ( int( ku) + 0x20 ) | 0x80
            euc1 = ( int(ten) + 0x20 ) | 0x80
            euc = bytearray([euc0,euc1]).decode('euc_jisx0213','replace')
            s = euc.encode('utf-8').decode('utf-8')
            return ord(s)
        except:
            return 0

if __name__ == '__main__':
    size = sys.argv[1]
    kanji_font = Font('.\\k{}.bdf'.format(size))
    ascii_font = Font('.\\a{}.bdf'.format(size))
    width = max([kanji_font.bbox['width'], ascii_font.bbox['width']])
    height = max([kanji_font.bbox['height'], ascii_font.bbox['height']])
    glyphs = kanji_font.glyphs + ascii_font.glyphs
    glyphs.sort(key=lambda g: g.code)
    save_path = str(Path('.\\font{}.dat'.format(size)).absolute())
    with open(save_path, mode='wb') as fp:
        fp.write(struct.pack('<H', len(glyphs)))
        fp.write(struct.pack('BB', width, height))
        for g in glyphs:
            g.save(fp)        
