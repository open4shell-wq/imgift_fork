#!/bin/sh
set -eu

tmpdir=$(mktemp -d "${TMPDIR:-/tmp}/imgify-test-cli.XXXXXX")
trap 'rm -rf "$tmpdir"' EXIT HUP INT TERM

input="$tmpdir/input.bin"
image="$tmpdir/output.png"
roundtrip="$tmpdir/roundtrip.bin"

printf '\000\001\002hello imgify\377' > "$input"

./bin2png --help > "$tmpdir/bin2png-help.out" 2>&1
./png2bin --help > "$tmpdir/png2bin-help.out" 2>&1
./bin2png --version > "$tmpdir/bin2png-version.out"
./png2bin --version > "$tmpdir/png2bin-version.out"

if ./bin2png -i "$input" > "$tmpdir/missing-output.out" 2>&1; then
	echo "bin2png accepted missing output option" >&2
	exit 1
fi

./bin2png -i "$input" -o "$image" -p 0xab > "$tmpdir/bin2png.out"
test -s "$image"

./png2bin -i "$image" -o "$roundtrip" > "$tmpdir/png2bin.out"
cmp "$input" "$roundtrip"

nonpng_output="$tmpdir/nonpng-output.bin"
./png2bin -i "$input" -o "$nonpng_output" > "$tmpdir/not-png.out" 2>&1
if test -e "$nonpng_output"; then
	echo "png2bin wrote output for a non-PNG input file" >&2
	exit 1
fi

if ./bin2png -i "$input" -o "$image" -p 256 > "$tmpdir/invalid-range.out" 2>&1; then
	echo "bin2png accepted an out-of-range pad byte" >&2
	exit 1
fi
