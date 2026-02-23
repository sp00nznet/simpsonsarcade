# The Simpsons Arcade - Binary Analysis Notes

## STFS Container

The game is distributed as a LIVE (STFS) container:
- **Magic:** LIVE (expected)
- **Content Type:** 0x000D0000 (Xbox Live Arcade)
- **Title ID:** 0x584111FA
- **Package Path:** `The Simpsons Arcade\584111FA\000D0000\5BA2C88EDCCD952114B3809DA53200C5C3B2236358`
- **Package Size:** ~18 MB
- **Extracted using:** `tools/extract_stfs.py`

## XEX File Information

- **Filename:** `default.xex` (to be extracted from STFS container)
- **Format:** XEX2 (Xbox 360 executable)
- **Architecture:** PowerPC (Xenon, big-endian)
- **Platform:** Xbox Live Arcade (XBLA)
- **Title ID:** 0x584111FA
- **Base Address:** TODO (run extract_pe.py)
- **Entry Point:** TODO (run extract_pe.py)
- **Image Size:** TODO (run extract_pe.py)

## ABI Addresses

TODO: Run `tools/find_abi_addrs.py` after extracting the PE image.

| Function | Address | Verified |
|----------|---------|----------|
| savegprlr_14 | TODO | NO |
| restgprlr_14 | TODO | NO |
| savefpr_14 | TODO | NO |
| restfpr_14 | TODO | NO |
| savevmx_14 | TODO | NO |
| restvmx_14 | TODO | NO |
| savevmx_64 | TODO | NO |
| restvmx_64 | TODO | NO |
| setjmp | TODO | NO |
| longjmp | TODO | NO |

## Jump Tables

TODO: Run XenonAnalyse to detect jump tables.

## Recompilation Statistics

TODO: Run XenonRecomp first pass.

## Game Information

- **Title:** The Simpsons Arcade
- **Developer:** Backbone Entertainment
- **Publisher:** Konami
- **Release:** February 2012 (XBLA)
- **Original Arcade:** 1991, Konami
- **Genre:** Beat 'em up (up to 4 players)
- **Players:** Homer, Marge, Bart, Lisa
