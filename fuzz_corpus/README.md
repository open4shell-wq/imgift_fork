# Fuzz corpus

Seed inputs for AFL++ runs.

`bin2png/` contains small portable files for fuzzing `./bin2png @@ /dev/null`.
The program accepts arbitrary input bytes, so these seeds are intentionally
simple and varied; AFL++ mutates them into binary cases during the run.
