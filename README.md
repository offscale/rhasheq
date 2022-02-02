rhasheq
=======
[![License](https://img.shields.io/badge/license-0BSD-blue.svg)](https://opensource.org/licenses/0BSD)
[![CI for Linux, Windows, macOS, sunOS](https://github.com/offscale/rhasheq/actions/workflows/linux-Windows-macOS.yml/badge.svg)](https://github.com/offscale/rhasheq/actions/workflows/github-actions.yml)
[![C89](https://img.shields.io/badge/C-89-blue)](https://en.wikipedia.org/wiki/C89_(C_version))

Simple header-only librhash wrapper for verifying hashes from a file to a given `const char *`.

Header only (to simplify including). Just `#define RHASHEQ_IMPLEMENTATION` once-only in your program (before including the header).

### Dependencies

- [CMake](https://cmake.org) (3.19 or later)
- C compiler (any that work with CMake, and were released within the last 30 years)

### Build

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

---

## License

The code is distributed under [BSD Zero Clause License](COPYING).
