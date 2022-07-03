import sys

def to_unicode(ku, ten):
    euc0 = ( int( ku) + 0x20 ) | 0x80
    euc1 = ( int(ten) + 0x20 ) | 0x80
    euc = bytearray([euc0,euc1]).decode('euc_jisx0213','replace')
    return ord(euc.encode('utf-8', 'backslashreplace').decode('utf-8'))

if __name__ == '__main__':
    ku = int(sys.argv[1])
    ten = int(sys.argv[2])
    print('U+{:04X}'.format(to_unicode(ku, ten)))
