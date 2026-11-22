# Patches Applied to Festival 3.1

Upstream source: Festival 3.1 by Yaron Shoham (MIT License, obtained via joriswit/Festival).
The following minimal portability patches were applied so Festival builds and runs cleanly as a non-interactive CLI subprocess on Windows (MSVC), Linux (GCC), and macOS (Clang):

1. `global.cpp`:
   - Always initialize `global_dir` to `"."` (instead of `"c:\\sokoban"` under `VISUAL_STUDIO`) so command-line level file arguments work when built with MSVC.
2. `sol.cpp`:
   - Guard `#include <malloc.h>` with `#if !defined(__APPLE__)` because macOS provides `malloc` in `<stdlib.h>` and does not ship `<malloc.h>`.
3. `util.cpp` & `io.cpp`:
   - Under `FESTIVAL_NONINTERACTIVE`, make `my_getch()` a no-op and exit with status `1` on fatal errors so headless subprocess execution never blocks waiting for console key input.
4. `sokoban_solver.cpp`:
   - When `THREADS` is not defined, clamp `cores_num` to `1` so single-threaded MSVC builds do not silently skip solving if `-cores > 1` is passed.
   - Ensure `packing_search_control` receives at least 1 second (`pack_time >= 1`) so short time limits (e.g. `-time 10`) do not leave `h->imagined_hf_board` uninitialized.
