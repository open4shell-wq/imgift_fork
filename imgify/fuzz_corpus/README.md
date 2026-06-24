# Fuzz corpus

Seed inputs for AFL++ runs.

`bin2png/` contains small, empty, textual, binary, repeated, and option-like
payloads. The program accepts arbitrary input bytes, so these files are meant
to give AFL++ a diverse starting queue without making the repository large.

`png2bin/` contains valid PNG files, truncated PNGs, bad-signature PNGs, and
bit-flipped PNGs. The valid PNGs exercise normal libpng decoding; the malformed
PNG files exercise parser and error-handling paths.

`../fuzz_dicts/` contains AFL dictionaries for text-like input and PNG chunk
tokens. Jenkins should pass these dictionaries with `-x`.

The actual AFL++ invocation must use the imgify CLI options:

```sh
./bin2png -i @@ -o /dev/null
./png2bin -i @@ -o /dev/null
```

Fuzzer feedback is intentionally not committed to the repository. Jenkins must
archive AFL++ runtime output from `out/default/queue`, `out/default/crashes`,
`out/default/hangs`, `fuzzer_stats`, and `plot_data`. The queue is the coverage
feedback mechanism: replay it after the fuzzing phase to build the fuzz coverage
report and keep it as a build artifact for analysis.
