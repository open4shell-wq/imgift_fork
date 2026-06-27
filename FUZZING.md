# Fuzzing

This project has two file-oriented targets:

- `png2bin`: consumes PNG files through `png_load`.
- `bin2png`: consumes arbitrary binary input and writes a PNG.

Seed corpora are stored in `fuzz_corpus/`.

## AFL++ build with sanitizers

Build from a clean tree with AFL++ compiler wrappers. `AFL_USE_ASAN=1` and
`AFL_USE_UBSAN=1` add AddressSanitizer and UndefinedBehaviorSanitizer checks to
the AFL-instrumented build.

```sh
make clean
AFL_USE_ASAN=1 AFL_USE_UBSAN=1 make \
  CC=afl-clang-fast \
  CFLAGS="-g -O1 -fno-omit-frame-pointer -D_XOPEN_SOURCE=600 -std=c99" \
  LDFLAGS="-lpng -lm"
```

The compile log must show `afl-clang-fast`, not plain `clang`.

Check that AFL++ sees coverage feedback:

```sh
afl-showmap -o /tmp/imgify.map -- \
  ./png2bin -i fuzz_corpus/png2bin/pngsuite/basn6a08.png -o /tmp/imgify.bin
```

If the binary is not instrumented, `afl-showmap` will report that no
instrumentation was detected.

## Run fuzzing

Fuzz PNG decoding:

```sh
afl-fuzz -i fuzz_corpus/png2bin/pngsuite -o fuzz_out/png2bin -- \
  ./png2bin -i @@ -o /tmp/imgify-fuzz.bin
```

Fuzz arbitrary binary input:

```sh
afl-fuzz -i fuzz_corpus/bin2png -o fuzz_out/bin2png -- \
  ./bin2png -i @@ -o /tmp/imgify-fuzz.png
```

You can also run the AFL++ PNG seeds separately:

```sh
afl-fuzz -i fuzz_corpus/png2bin/aflplusplus -o fuzz_out/png2bin-aflplusplus -- \
  ./png2bin -i @@ -o /tmp/imgify-fuzz.bin
```

## Human-readable coverage report

AFL++ uses its own edge coverage instrumentation for feedback. For a readable
line/function coverage report, make a separate LLVM coverage build and replay
the seed corpus or AFL queue.

```sh
make clean
make CC=clang \
  CFLAGS="-g -O0 -fprofile-instr-generate -fcoverage-mapping -D_XOPEN_SOURCE=600 -std=c99" \
  LDFLAGS="-fprofile-instr-generate -lpng -lm"

mkdir -p coverage
for f in fuzz_corpus/png2bin/pngsuite/*.png fuzz_corpus/png2bin/aflplusplus/*.png; do
  LLVM_PROFILE_FILE="coverage/png2bin-%p.profraw" ./png2bin -i "$f" -o /tmp/imgify-cov.bin >/dev/null 2>&1 || true
done

for f in fuzz_corpus/bin2png/*; do
  LLVM_PROFILE_FILE="coverage/bin2png-%p.profraw" ./bin2png -i "$f" -o /tmp/imgify-cov.png >/dev/null 2>&1 || true
done

llvm-profdata merge -sparse coverage/*.profraw -o coverage/imgify.profdata
llvm-cov report ./png2bin -instr-profile=coverage/imgify.profdata
llvm-cov report ./bin2png -instr-profile=coverage/imgify.profdata
```

For coverage after fuzzing, replay files from `fuzz_out/*/default/queue/` with
the same coverage build.
