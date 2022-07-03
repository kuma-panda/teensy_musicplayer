from PIL import Image, ImageFont, ImageDraw
import sys

def collect_chars():
    ucs2_codes = [c for c in range(0x20, 0x7F)]
    with open('k20gm.bdf', mode='r') as fp:
        lines = fp.read().split('\n')
    jis_codes = [int(s[9:]) for s in lines if s.find('ENCODING') == 0]
    for jiscode in jis_codes:
        hi = jiscode // 256
        lo = jiscode % 256
        try:
            ucs2_codes.append(ord(bytes([0x1b, ord('$'), ord('B'), hi, lo, 0x1b, ord('('), ord('B')]).decode('ISO-2022-JP')))
        except:
            pass
    ucs2_codes.sort()
    return ucs2_codes

def create_glyph(code, source_size, target_size):
    font = ImageFont.truetype('rounded-mgenplus-1cp-medium.ttf', source_size*8)
    text = chr(code)

    image = Image.new('RGBA', (1, 1))
    draw = ImageDraw.Draw(image)
    ascending  =  draw.textbbox((0, 0), text+' |g', font, anchor='la')[1]
    height     = -draw.textbbox((0, 0), text+' |g', font, anchor='lb')[1]
    descending = -(height + draw.textbbox((0, 0), text+' |g', font, anchor='ld')[1])

    # print('ascending : {}'.format(ascending))
    # print('height    : {}'.format(height))
    # print('descending: {}'.format(descending))

    width, _ = draw.textsize(text, font)
    del draw
    image = Image.new('RGBA', (width, height))
    draw = ImageDraw.Draw(image)
    draw.text((0, -ascending), text+' |g', font=font, fill=(255,255,255))
    del draw
    image = image.resize((width // 8, height // 8), Image.LANCZOS)
    width, height = image.size
    if height > target_size:
        print('ERROR: height > {2} (U+{0:04X}, height={1})'.format(code, height, target_size), file=sys.stderr)
        return 0
    print('    {0}, // U+{1:04X}'.format(width, code))
    r, g, b, a = image.split()
    pixels = []
    max_pixel_value = 0x00
    for y in range(height):
        values = []
        for x in range(width):
            p = a.getpixel((x, y))
            values.append(p)
        max_pixel_value = max(values+[max_pixel_value])
        pixels.append(values)
    r = 1
    if 0x00 < max_pixel_value < 0xFF:
        r = 0xFF / max_pixel_value
        print('WARNING: maximum pixel value = {0:02X} (U+{1:04X})'.format(max_pixel_value, code), file=sys.stderr)
    for y in range(len(pixels)):
        for x in range(len(pixels[y])):
            pixels[y][x] = int(pixels[y][x] * r + 0.5)
            if pixels[y][x] > 0xFF:
                pixels[y][x] = 0xFF
        values = ','.join(['0x{:02X}'.format(p) for p in pixels[y]])
        print('    {},'.format(values))

    # for y in range(height):
    #     values = []
    #     for x in range(width):
    #         p = a.getpixel((x, y))
    #         values.append(p)
    #     max_pixel_value = max(values+[max_pixel_value])
    #     values = ','.join(['0x{:02X}'.format(p) for p in values])
    #     print('    {},'.format(values))

    if height < target_size:
        while height < target_size:
            values = ','.join(['0x00' for p in range(width)])
            print('    {},  // <= brank line added'.format(values))
            height += 1
    
    print('')
    return 1+width*height

if __name__ == '__main__':
    source_size = int(sys.argv[1])
    target_size = int(sys.argv[2])
    print('source size : {}'.format(source_size), file=sys.stderr)
    print('target size : {}'.format(target_size), file=sys.stderr)

    chars = collect_chars()
    print('{} characters'.format(len(chars)), file=sys.stderr)
    maps = [0xFFFFFFFF for i in range(65536)]
    offset = 0
    print('const uint8_t FONT_{}AA[] PROGMEM = {{'.format(target_size))
    for c in chars:
        size = create_glyph(c, source_size, target_size)
        if size > 0:
            maps[c] = offset
            offset += size
    print('};')
    print('')
    print('const uint32_t FONTMAP_{}AA[] PROGMEM = {{'.format(target_size))
    for code, addr in enumerate(maps):
        if addr != 0xFFFFFFFF:
            name = 'U+{:04X}'.format(code)
        else:
            name = 'U+{:04X} (not used)'.format(code)
        print('    0x{0:08X}, // {1}'.format(addr, name))
    print('};')

    print('total: {} bytes'.format(offset+65536*4), file=sys.stderr)
