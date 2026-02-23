"""
STFS extractor for Xbox 360 LIVE/PIRS/CON packages.
Ported from extract360.py by Rene Ladan to Python 3.
Simplified to just extract files without interactive prompts.

Original: Copyright (c) 2007, 2008, Rene Ladan, 2-clause BSD license.
Block reading algorithm from wxPirs.
"""
import hashlib
import os
import struct
import sys
import time


def get_cluster(startclust, offset):
    """Get the real starting cluster offset (from wxPirs algorithm)."""
    rst = 0
    while startclust >= 170:
        startclust //= 170
        rst += (startclust + 1) * offset
    return rst


def mstime(intime):
    """Convert Microsoft FAT time format to time tuple."""
    num_d = (intime & 0xFFFF0000) >> 16
    num_t = intime & 0x0000FFFF
    return ((num_d >> 9) + 1980, (num_d >> 5) & 0x0F, num_d & 0x1F,
            (num_t & 0xFFFF) >> 11, (num_t >> 5) & 0x3F, (num_t & 0x1F) * 2,
            0, 0, -1)


def extract_live_pirs(input_path, output_dir):
    """Extract files from a LIVE/PIRS STFS package."""
    sys.stdout.reconfigure(encoding='utf-8')

    with open(input_path, 'rb') as infile:
        fsize = os.path.getsize(input_path)
        magic = infile.read(4)
        print(f"Magic: {magic}")
        assert magic in (b'LIVE', b'PIRS'), f"Not a LIVE/PIRS file: {magic}"

        if fsize < 0xD000:
            print(f"File too small: {fsize} bytes (need at least 0xD000)")
            return

        infile.seek(0xC032)
        pathind = struct.unpack(">H", infile.read(2))[0]
        if pathind == 0xFFFF:
            start = 0xC000
        else:
            start = 0xD000

        if start == 0xC000:
            offset = 0x1000
        else:
            offset = 0x2000

        print(f"Data start: 0x{start:X}")
        print(f"Hash table offset: 0x{offset:X}")

        # Determine file table size: scan entries until name_len == 0.
        # Read generously (up to 16 blocks) and trim to actual entries.
        infile.seek(start)
        ft_data = infile.read(0x1000 * 16)
        # Count valid entries to find actual table size
        _num = len(ft_data) // 64
        _actual = 0
        for _j in range(_num):
            if (ft_data[_j * 64 + 40] & 0x3F) == 0:
                break
            _actual = _j + 1
        firstclust = (_actual * 64 + 0xFFF) // 0x1000
        if firstclust < 1:
            firstclust = 1
        ft_data = ft_data[:0x1000 * firstclust]
        print(f"File table entries: {_actual}, blocks: {firstclust}")

        paths = {0xFFFF: ""}

        os.makedirs(output_dir, exist_ok=True)
        original_dir = os.getcwd()
        os.chdir(output_dir)

        files_extracted = []

        num_entries = len(ft_data) // 64
        for i in range(num_entries):
            cur = ft_data[i * 64:(i + 1) * 64]
            namelen_flags = cur[40]

            name_len = namelen_flags & 0x3F
            is_dir = bool(namelen_flags & 0x80)
            is_contiguous = bool(namelen_flags & 0x40)

            if name_len == 0:
                break

            outname = cur[0:name_len].decode('ascii', errors='replace')

            clustsize1 = struct.unpack("<H", cur[41:43])[0] + (cur[43] << 16)
            clustsize2 = struct.unpack("<H", cur[44:46])[0] + (cur[46] << 16)
            startclust = struct.unpack("<H", cur[47:49])[0] + (cur[49] << 16)
            pathind = struct.unpack(">H", cur[50:52])[0]
            filelen = struct.unpack(">I", cur[52:56])[0]
            dati1 = struct.unpack(">I", cur[56:60])[0]
            dati2 = struct.unpack(">I", cur[60:64])[0]

            type_str = "DIR " if is_dir else "FILE"
            contig = " [contiguous]" if is_contiguous else ""
            print(f"  [{type_str}] {outname:<40s} {filelen:>12d} bytes  start_block={startclust}  blocks={clustsize1}{contig}  path={pathind}")

            if name_len < 1 or name_len > 40:
                print(f"    WARNING: Name length {name_len} out of range, skipping")
                continue

            if clustsize1 != clustsize2:
                print(f"    WARNING: Cluster sizes don't match ({clustsize1} != {clustsize2})")

            if is_dir:
                paths[i] = paths.get(pathind, "") + outname + "/"
                full_dir = os.path.join(output_dir, paths[i])
                os.makedirs(full_dir, exist_ok=True)
                print(f"    -> Created directory: {paths[i]}")
            else:
                if startclust < 1:
                    print(f"    WARNING: Starting cluster must be >= 1, skipping")
                    continue

                parent = paths.get(pathind, "")
                out_path = os.path.join(output_dir, parent, outname)
                os.makedirs(os.path.dirname(out_path) or '.', exist_ok=True)

                adstart = startclust * 0x1000 + start
                remaining = filelen
                file_data = bytearray()
                cur_clust = startclust

                while remaining > 0:
                    realstart = adstart + get_cluster(cur_clust, offset)
                    infile.seek(realstart)
                    chunk = infile.read(min(0x1000, remaining))
                    file_data.extend(chunk)
                    cur_clust += 1
                    adstart += 0x1000
                    remaining -= 0x1000

                with open(out_path, 'wb') as outfile:
                    outfile.write(bytes(file_data[:filelen]))

                file_magic = bytes(file_data[:4]) if len(file_data) >= 4 else b''
                magic_str = ""
                if file_magic == b'XEX2':
                    magic_str = " >> XEX2 executable!"
                elif file_magic == b'XEX1':
                    magic_str = " >> XEX1 executable!"
                elif file_magic == b'\x89PNG':
                    magic_str = " >> PNG image"

                print(f"    -> Extracted: {out_path} ({filelen} bytes){magic_str}")
                files_extracted.append((outname, out_path, filelen, file_magic))

        os.chdir(original_dir)
        print(f"\nDone! Extracted {len(files_extracted)} file(s) to {output_dir}")

        return files_extracted


if __name__ == '__main__':
    if len(sys.argv) < 2:
        input_path = r'E:\simpsons\The Simpsons Arcade\584111FA\000D0000\5BA2C88EDCCD952114B3809DA53200C5C3B2236358'
    else:
        input_path = sys.argv[1]

    if len(sys.argv) < 3:
        output_dir = os.path.join(os.path.dirname(input_path) or '.', 'extracted')
    else:
        output_dir = sys.argv[2]

    print(f"Input:  {input_path}")
    print(f"Output: {output_dir}")
    print()

    extract_live_pirs(input_path, output_dir)
