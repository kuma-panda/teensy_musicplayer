import os
from PIL import Image

img = Image.open('digits.png')
img = img.convert('RGBA')
r, g, b, a = img.split()

print('const uint8_t DIGITS[] PROGMEM = {')

for n in range(11):
    print('    20, 30')
    x = n*20
    for y in range(30):
        values = []
        for i in range(20):
            values.append('{: >3}'.format(a.getpixel((x+i, y))))
        print('    {},'.format(','.join(values)))
    print('')
print('};')
