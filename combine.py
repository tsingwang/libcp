#!/usr/bin/env python3

from pathlib import Path

def combine(sourcefile):
    filepath = Path(sourcefile)

    with open(sourcefile, 'r') as f:
        code = f.read()

    h_lines = filter(lambda line: line.startswith('#include "'), code.split('\n'))
    for h_line in h_lines:
        header = h_line.split('"')[1]
        h_filename = filepath.parent / header
        if not h_filename.exists():
            h_filename = Path('/usr/local/include') / header
            if not h_filename.exists():
                print(f"{h_filename} not found")
                continue

        c_filename = h_filename.with_suffix('.c')
        if c_filename.exists():
            with open(c_filename, 'r') as f:
                c_code = f.read()
            code = code.replace(h_line, c_code)

        with open(h_filename, 'r') as f:
            h_code = f.read()
        code = code.replace(h_line, h_code)

    output_filename = sourcefile.rsplit('.', 1)[0] + '_combined.c'
    with open(output_filename, 'w') as f:
        f.write("/* The code used from https://github.com/tsingwang/libcp */\n");
        f.write(code)

if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('sourcefile', help='The filename before commit')
    args = parser.parse_args()
    combine(args.sourcefile)
