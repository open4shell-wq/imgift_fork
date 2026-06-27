#!/bin/sh
set -eu

ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$ROOT_DIR"

WORK_DIR=$(mktemp -d "${TMPDIR:-/tmp}/imgify-tests.XXXXXX")

cleanup() {
	rm -rf "$WORK_DIR"
}
trap cleanup EXIT HUP INT TERM

fail() {
	printf '%s\n' "$*" >&2
	exit 1
}

expect_failure() {
	if "$@" >"$WORK_DIR/failure.stdout" 2>"$WORK_DIR/failure.stderr"; then
		fail "expected command to fail: $*"
	fi
}

roundtrip() {
	name=$1
	input=$2
	png="$WORK_DIR/$name.png"
	output="$WORK_DIR/$name.out"

	if [ "$#" -eq 3 ]; then
		./bin2png -i "$input" -o "$png" -p "$3" >"$WORK_DIR/$name.encode.log"
	else
		./bin2png -i "$input" -o "$png" >"$WORK_DIR/$name.encode.log"
	fi

	./png2bin -i "$png" -o "$output" >"$WORK_DIR/$name.decode.log"
	cmp "$input" "$output" > /dev/null || fail "roundtrip mismatch: $name"
}

exact_input="$WORK_DIR/exact.bin"
padded_input="$WORK_DIR/padded.bin"
custom_pad_input="$WORK_DIR/custom-pad.bin"

printf '0123456789abcdef' > "$exact_input"
printf '\000\001\002\003\004\005\377' > "$padded_input"
printf 'imgify roundtrip with a non-default pad byte and multiple rows' > "$custom_pad_input"

./bin2png --version > /dev/null
./png2bin --help > /dev/null

expect_failure ./bin2png
expect_failure ./png2bin -i "$exact_input"
expect_failure ./bin2png -i "$exact_input" -o "$WORK_DIR/invalid-pad.png" -p 256

roundtrip exact-fill "$exact_input"
roundtrip padded-binary "$padded_input"
roundtrip custom-pad "$custom_pad_input" 0x2a

printf 'all tests passed\n'
