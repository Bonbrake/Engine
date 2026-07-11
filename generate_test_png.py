import os
import zlib
import struct

def make_png(width, height, color):
    # PNG signature
    png = b'\x89PNG\r\n\x1a\n'
    
    # IHDR chunk
    ihdr_data = struct.pack('>IIBBBBB', width, height, 8, 2, 0, 0, 0) # 8-bit RGB
    png += struct.pack('>I', 13) + b'IHDR' + ihdr_data + struct.pack('>I', zlib.crc32(b'IHDR' + ihdr_data))
    
    # IDAT chunk
    raw_data = b''
    for y in range(height):
        raw_data += b'\x00' + bytes(color * width)
    idat_data = zlib.compress(raw_data)
    png += struct.pack('>I', len(idat_data)) + b'IDAT' + idat_data + struct.pack('>I', zlib.crc32(b'IDAT' + idat_data))
    
    # IEND chunk
    png += struct.pack('>I', 0) + b'IEND' + struct.pack('>I', zlib.crc32(b'IEND'))
    return png

os.makedirs('assets/textures', exist_ok=True)
with open('assets/textures/test.png', 'wb') as f:
    f.write(make_png(1, 1, [255, 0, 0]))
print("Generated assets/textures/test.png")
