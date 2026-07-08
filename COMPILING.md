# COMPILING

Compiling is straight forward using CMake.

## Release build

```bash
mkdir release
cd release
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      ..
make
```
## Debug build

```bash
mkdir debug
cd debug
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      ..
make
```
