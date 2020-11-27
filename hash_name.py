import sys

def hash_str(s):
    h = 1
    for b in s.encode('ascii'):
        h = (h - b) & 0xffffffff
        h = (h * 0x314159) & 0xffffffff
        h ^= h >> 3

    return (h - len(s) - 1) & 0xffffffff

print('0x%.8x: %s' % (hash_str(sys.argv[1]), sys.argv[1]))
