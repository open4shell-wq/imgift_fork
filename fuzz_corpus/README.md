# Fuzz corpus

Seed inputs for AFL++ runs.

`bin2png/` contains small portable files for fuzzing `./bin2png @@ /dev/null`.
The program accepts arbitrary input bytes, so these seeds are intentionally
simple and varied; AFL++ mutates them into binary cases during the run.

`png2bin/` contains valid PNG files for fuzzing `png2bin`.
Most of them are generated from the `bin2png/` seeds, plus one larger sample
PNG from the upstream repository. This target exercises libpng parsing and is
expected to produce more meaningful AFL++ coverage than `bin2png`.

The actual AFL++ invocation must use the imgify CLI options:

```sh
./bin2png -i @@ -o /dev/null
./png2bin -i @@ -o /dev/null
```
