rhasheq
=======
[![License](https://img.shields.io/badge/license-0BSD-blue.svg)](https://opensource.org/licenses/0BSD)
![Doc Coverage](https://img.shields.io/badge/doc__coverage-100.00%25-brightgreen)
![Test Coverage](https://img.shields.io/badge/test__coverage-100.00%25-brightgreen)
[![CI](https://github.com/offscale/rhasheq/actions/workflows/ci.yml/badge.svg)](https://github.com/offscale/rhasheq/actions/workflows/ci.yml)
[![C89](https://img.shields.io/badge/C-89-blue)](https://en.wikipedia.org/wiki/C89_(C_version))

Strict C89 `LibRHash` file checksum verification library with `NO_DISCARD` enum error percolation, Safe CRT support, and optional single-header STB-style amalgamation.

Originally the `librhash` implementation of `libacquire`'s checksum API.

## Features

- **Strict ISO C89 / ANSI C compatibility**: Compiles cleanly with `-std=c89 -pedantic -Wall -Wextra -Werror` on GCC/Clang and `/W4 /WX` on MSVC.
- **Explicit Error Percolation**: Every function returns a `NO_DISCARD` `rhasheq_error_t` status code (`RHASHEQ_SUCCESS`, `RHASHEQ_INVALID_ARGUMENT`, `RHASHEQ_OUT_OF_MEMORY`, `RHASHEQ_INIT_ERROR`, `RHASHEQ_HASH_ERROR`, `RHASHEQ_FORMAT_ERROR`) and writes verification results to an output `bool *`.
- **Flexible Build Modes**:
  - **Static library** (`-DRHASHEQ_BUILD_STATIC=ON`, default)
  - **Shared library** (`-DRHASHEQ_BUILD_SHARED=ON`)
  - **Header-only `INTERFACE` library** (`-DRHASHEQ_HEADER_ONLY=ON`)
  - **Single-header STB-style amalgamation** (`-DRHASHEQ_BUILD_AMALGAMATION=ON`, generates `rhasheq_amalgamation.h`; define `RHASHEQ_IMPLEMENTATION` before including in one translation unit)
- **31 Hash Algorithms Supported**: `crc32`, `crc32c`, `md4`, `md5`, `sha1`, `sha224`, `sha256`, `sha384`, `sha512`, `sha3_224`, `sha3_256`, `sha3_384`, `sha3_512`, `tiger`, `tth`, `btih`, `ed2k`, `aich`, `whirlpool`, `ripemd160`, `gost94`, `gost94_cryptopro`, `has160`, `gost12_256`, `gost12_512`, `edonr256`, `edonr512`, `snefru128`, `snefru256`, `blake2s`, `blake2b`.

## Dependencies

- [CMake](https://cmake.org) (3.14 or later, including CMake 4.x)
- C compiler (any C89-compatible compiler supported by CMake)
- [LibRHash](https://github.com/rhash/RHash) (resolved automatically via system packages/vcpkg or fetched via CMake `FetchContent`)
- [c89stringutils](https://github.com/offscale/c89stringutils) (resolved automatically via vcpkg/system packages or fetched via CMake `FetchContent`)

## Build & Test

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

### CMake Options

| Option | Default | Description |
| :--- | :--- | :--- |
| `RHASHEQ_BUILD_STATIC` | `ON` | Build `rhasheq` as a static library |
| `RHASHEQ_BUILD_SHARED` | `OFF` | Build `rhasheq` as a shared library |
| `RHASHEQ_HEADER_ONLY` | `OFF` | Use `rhasheq` as a header-only `INTERFACE` library |
| `RHASHEQ_BUILD_AMALGAMATION` | `ON` | Generate single-file `rhasheq_amalgamation.h` in the build directory |
| `RHASHEQ_BUILD_TESTING` / `BUILD_TESTING` | `ON` | Build unit and amalgamation tests |
| `RHASHEQ_LTO` | `OFF` | Enable Link-Time Optimization (IPO/LTO) |
| `RHASHEQ_CHARSET` | `Unicode` | Character set (`Unicode` or `ANSI`) |
| `RHASHEQ_THREADING` | `Multi-threaded` | Threading mode (`Multi-threaded` or `Single-threaded`) |
| `RHASHEQ_CRT_LINKAGE` | `Shared` | MSVC CRT linkage (`Shared` or `Static`) |

## Usage

### Linked Library

```c
#include <stdio.h>
#include <rhasheq.h>

int main(void) {
  bool is_match = false;
  rhasheq_error_t rc;

  rc = sha256("path/to/file.txt",
              "1292593d95c35eeccc89ffa1c91d6fe53b49f81cbf2c2b7758842b7f3186fcc2",
              &is_match);
  if (rc != RHASHEQ_SUCCESS) {
    return (int)rc;
  }

  printf("SHA-256 matches: %s\n", is_match ? "true" : "false");
  return 0;
}
```

### Header-Only / Amalgamation

Define `RHASHEQ_IMPLEMENTATION` in **one** `.c` file before including `rhasheq.h` (or the generated `rhasheq_amalgamation.h`):

```c
#define RHASHEQ_IMPLEMENTATION
#include "rhasheq_amalgamation.h"
```

---

## License

The code is distributed under [BSD Zero Clause License](COPYING).
