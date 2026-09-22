#!/usr/bin/env python3
import pathlib, subprocess, sys, tempfile
if len(sys.argv) != 3:
    raise SystemExit('usage: all_bytes.py BITWEAVE BITPACK')
bw,bp=sys.argv[1:]
with tempfile.TemporaryDirectory() as td:
    d=pathlib.Path(td)
    expected=bytes(range(256))
    bits=d/'all.bits'; a=d/'a.bin'; b=d/'b.bin'
    bits.write_text('\n'.join(' '.join(f'{x:08b}' for x in range(i,min(i+16,256))) for i in range(0,256,16))+'\n')
    subprocess.run([bw,'pack',str(bits),str(a)],check=True)
    subprocess.run([bp,str(bits),str(b)],check=True)
    if a.read_bytes()!=expected or b.read_bytes()!=expected:
        raise SystemExit('byte-domain pack mismatch')
print('PASS all 256 byte values pack exactly through bitweave and bitpack')
