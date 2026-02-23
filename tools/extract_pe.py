#!/usr/bin/env python3
"""
Extract and decrypt the PE image from an Xbox 360 XEX2 file.

Based on XenonRecomp's XEX loading implementation (XenonUtils/xex.cpp).

Usage: python extract_pe.py <input.xex> <output.bin>
"""

import struct
import sys

try:
    from Crypto.Cipher import AES
except ImportError:
    try:
        from Cryptodome.Cipher import AES
    except ImportError:
        print("ERROR: pycryptodome is required. Install with: pip install pycryptodome")
        sys.exit(1)


XEX2_RETAIL_KEY = bytes([
    0x20, 0xB1, 0x85, 0xA5, 0x9D, 0x28, 0xFD, 0xC3,
    0x40, 0x58, 0x3F, 0xBB, 0x08, 0x96, 0xBF, 0x91
])
AES_BLANK_IV = b'\x00' * 16

SEC_AES_KEY_OFFSET = 0x150


def read_be32(data, off):
    return struct.unpack('>I', data[off:off+4])[0]

def read_be16(data, off):
    return struct.unpack('>H', data[off:off+2])[0]


def decrypt_xex2(xex_path, output_path):
    with open(xex_path, 'rb') as f:
        data = bytearray(f.read())

    if data[0:4] != b'XEX2':
        print(f"ERROR: Not a XEX2 file (magic: {bytes(data[0:4])})")
        return False

    pe_data_offset = read_be32(data, 8)
    sec_info_offset = read_be32(data, 16)
    opt_header_count = read_be32(data, 20)

    print(f"XEX2 file: {len(data)} bytes")
    print(f"  PE data offset: 0x{pe_data_offset:X}")
    print(f"  Security info: 0x{sec_info_offset:X}")
    print(f"  Optional headers: {opt_header_count}")

    ffi_offset = 0
    entry_point = 0
    image_base = 0
    pos = 24
    for i in range(opt_header_count):
        hdr_id = read_be32(data, pos)
        hdr_val = read_be32(data, pos + 4)
        key_id = (hdr_id >> 8) & 0xFFFFFF

        if key_id == 0x000003:
            ffi_offset = hdr_val
        elif key_id == 0x000101:
            entry_point = hdr_val
        elif key_id == 0x000102:
            image_base = hdr_val
        pos += 8

    print(f"  Entry point: 0x{entry_point:08X}")
    print(f"  Image base: 0x{image_base:08X}")

    if not ffi_offset:
        print("ERROR: File format info header not found")
        return False

    ffi_size = read_be32(data, ffi_offset)
    enc_type = read_be16(data, ffi_offset + 4)
    comp_type = read_be16(data, ffi_offset + 6)

    enc_names = {0: 'None', 1: 'Normal'}
    comp_names = {0: 'None', 1: 'Basic', 2: 'Normal (LZX)', 3: 'Delta'}
    print(f"  Encryption: {enc_names.get(enc_type, f'Unknown({enc_type})')}")
    print(f"  Compression: {comp_names.get(comp_type, f'Unknown({comp_type})')}")

    image_size = read_be32(data, sec_info_offset + 4)
    load_address = read_be32(data, sec_info_offset + 0x110)
    print(f"  Image size: 0x{image_size:X} ({image_size} bytes)")
    print(f"  Load address: 0x{load_address:08X}")

    enc_key = bytes(data[sec_info_offset + SEC_AES_KEY_OFFSET:
                         sec_info_offset + SEC_AES_KEY_OFFSET + 16])
    print(f"\n  Encrypted file key: {enc_key.hex()}")

    if enc_type == 1:
        cipher = AES.new(XEX2_RETAIL_KEY, AES.MODE_CBC, AES_BLANK_IV)
        file_key = cipher.decrypt(enc_key)
        print(f"  Decrypted file key: {file_key.hex()}")
    else:
        file_key = None
        print("  No encryption")

    raw_pe_data = bytes(data[pe_data_offset:])
    if file_key:
        pad_len = (16 - (len(raw_pe_data) % 16)) % 16
        if pad_len:
            raw_pe_data += b'\x00' * pad_len
        cipher = AES.new(file_key, AES.MODE_CBC, AES_BLANK_IV)
        decrypted_data = cipher.decrypt(raw_pe_data)
        print(f"  Decrypted {len(decrypted_data)} bytes")
    else:
        decrypted_data = raw_pe_data

    print(f"  First 32 bytes: {' '.join(f'{b:02X}' for b in decrypted_data[:32])}")

    if comp_type == 1:
        print("\nDecompressing basic blocks...")
        blocks = []
        block_pos = ffi_offset + 8
        while block_pos + 8 <= ffi_offset + ffi_size:
            data_size = read_be32(data, block_pos)
            zero_size = read_be32(data, block_pos + 4)
            if data_size == 0 and zero_size == 0:
                break
            blocks.append((data_size, zero_size))
            block_pos += 8

        print(f"  Found {len(blocks)} compression blocks")
        total_size = sum(ds + zs for ds, zs in blocks)
        pe_image = bytearray(max(image_size, total_size))
        src_pos = 0
        dst_pos = 0

        for i, (ds, zs) in enumerate(blocks):
            if src_pos + ds > len(decrypted_data):
                avail = len(decrypted_data) - src_pos
                pe_image[dst_pos:dst_pos + avail] = decrypted_data[src_pos:src_pos + avail]
                dst_pos += avail + (ds - avail) + zs
                break
            pe_image[dst_pos:dst_pos + ds] = decrypted_data[src_pos:src_pos + ds]
            src_pos += ds
            dst_pos += ds
            dst_pos += zs

        print(f"  Decompressed to {dst_pos} bytes")
        final_image = bytes(pe_image[:image_size])
    elif comp_type == 0:
        final_image = decrypted_data[:image_size]
    else:
        print(f"  WARNING: Compression type {comp_type} not supported")
        final_image = decrypted_data[:image_size]

    print(f"\nValidating PE image ({len(final_image)} bytes)...")

    has_mz = final_image[0:2] == b'MZ'
    if has_mz:
        e_lfanew = struct.unpack('<I', final_image[0x3C:0x40])[0]
        pe_off = e_lfanew
    else:
        if final_image[0:4] == b'PE\x00\x00':
            pe_off = 0
        else:
            pe_off = None
            for try_off in [0, 0x80, 0x100, 0x1000]:
                if try_off + 4 <= len(final_image) and final_image[try_off:try_off+4] == b'PE\x00\x00':
                    pe_off = try_off
                    break
            if pe_off is None:
                with open(output_path, 'wb') as f:
                    f.write(final_image)
                print(f"\nWrote {len(final_image)} bytes to {output_path} (unvalidated)")
                return True

    with open(output_path, 'wb') as f:
        f.write(final_image)
    print(f"\nWrote {len(final_image)} bytes to {output_path}")
    return True


if __name__ == '__main__':
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} <input.xex> <output.bin>")
        sys.exit(1)
    if not decrypt_xex2(sys.argv[1], sys.argv[2]):
        sys.exit(1)
