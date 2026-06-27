# fuzz_corpus

Seed corpora for fuzzing imgify command-line tools.

## png2bin

- `png2bin/pngsuite`: PNG samples from PngSuite mirror: https://github.com/lunapaint/pngsuite
- `png2bin/aflplusplus`: PNG samples from AFL++: https://github.com/AFLplusplus/AFLplusplus/tree/stable/testcases/images/png

Use these with `png2bin`, where `@@` is replaced by AFL++ with a mutated PNG path.
Some PngSuite `x*.png` files are intentionally malformed and are useful for
exercising PNG error paths.

## bin2png

- `bin2png/aflplusplus_hello_world.txt`: generic AFL++ text seed: https://github.com/AFLplusplus/AFLplusplus/tree/stable/testcases/others/text
- `bin2png/size-*.bin`: local size-boundary seeds around `n*n*4` image-capacity changes.
- `bin2png/pattern-*.bin`: local byte-pattern seeds with low entropy, high entropy, alternating bytes, and PNG-like headers.

Use these with `bin2png`, where `@@` is replaced by AFL++ with a mutated arbitrary input path.
