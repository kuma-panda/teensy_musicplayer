import struct
import sys
from PIL import Image

def write_word(fp, value):
    fp.write(struct.pack('<H', value))

def color_565(r, g, b):
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

def png_to_bin(src_path, dst_path):
    img = Image.open(src_path)
    img = img.convert('RGB')
    w, h = img.size
    print('const uint16_t LOGO[] PROGMEM = {')
    print('    0x{0:04X},0x{1:04X},    // width = {0}, height = {1}'.format(w, h))
    with open(dst_path, mode='wb') as fp:
        write_word(fp, w)
        write_word(fp, h)
        for y in range(h):
            data = []
            for x in range(w):
                r, g, b = img.getpixel((x, y))
                v = color_565(r, g, b)
                write_word(fp, v)
                data.append(v)
            print('    ' + ','.join(['0x{:04X}'.format(v) for v in data]) + ',')                     
        print('};')

if __name__ == '__main__':
    png_to_bin(sys.argv[1], sys.argv[2])
