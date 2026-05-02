#!/usr/bin/env python3

from pathlib import Path


def combine(sourcefile):
    filepath = Path(sourcefile).resolve()
    script_dir = Path(__file__).resolve().parent
    expanded_headers = set()
    expanded_sources = set()

    def resolve_header(header, base_dir):
        candidates = [
            base_dir / header,
            script_dir / "src" / header,
            script_dir / "src" / "legacy" / header,
            Path("/usr/local/include") / header,
        ]
        return next((candidate.resolve() for candidate in candidates if candidate.exists()), None)

    def expand_code(code, base_dir):
        expanded_lines = []
        for line in code.splitlines():
            if not line.startswith('#include "'):
                expanded_lines.append(line)
                continue

            header = line.split('"')[1]
            h_filename = resolve_header(header, base_dir)
            if h_filename is None:
                print(f"{header} not found")
                continue

            c_filename = h_filename.with_suffix(".c")
            if c_filename.exists() and c_filename not in expanded_sources:
                expanded_sources.add(c_filename)
                expanded_lines.append(expand_file(c_filename))

            if h_filename not in expanded_headers:
                expanded_headers.add(h_filename)
                expanded_lines.append(expand_file(h_filename))

        return "\n".join(expanded_lines)

    def expand_file(path):
        with open(path, "r") as f:
            return expand_code(f.read(), path.parent)

    code = expand_file(filepath)
    output_filename = sourcefile.rsplit(".", 1)[0] + "_combined.c"
    with open(output_filename, "w") as f:
        f.write(code)

if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('sourcefile', help='The filename before commit')
    args = parser.parse_args()
    combine(args.sourcefile)
