# Testing

BitWeave keeps test **definitions** in the repository and leaves captured build logs and binaries to CI or release artifacts.

## Local

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure

tests/test.sh build/bitweave build/bitpack
tests/negative.sh build/bitweave
python3 tests/all_bytes.py build/bitweave build/bitpack
tests/authoring_matrix.sh build/bitweave
```

All generated output stays under the local build directory or temporary test directories.

## GitHub

`.github/workflows/ci.yml` builds with GCC and Clang and runs the functional, negative, all-byte, authoring-matrix, ASan and UBSan checks.

The repository intentionally does **not** commit local verification logs, compiler build directories, or generated native example binaries.
