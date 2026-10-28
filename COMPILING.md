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

## Fedora package (RPM)

The default build bakes build-tree paths for `resources/` and `ui/` into
the binaries and is meant for development. A package build bakes the
installed FHS paths instead. No source changes are needed, only a separate
build directory with `PILOT_USE_INSTALL_PATHS=ON`:

```bash
cmake -S . -B build-rpm \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr \
      -DPILOT_USE_INSTALL_PATHS=ON
cmake --build build-rpm -j "$(nproc)"
```

Layout of the installed package:

- `/usr/bin/pilot`: wrapper that runs the real binary with `--no-sandbox`
  (so `chrome-sandbox` needs no SUID permissions).
- `/usr/libexec/pilot/`: `pilot` binary plus the files CEF must find next
  to the executable (`libcef.so`, `*.pak`, `icudtl.dat`, `locales/`,
  SwiftShader/Vulkan libs, `chrome-sandbox`) and the bundled shared
  libraries (`libgeneric.so`, `libcglm.so`, `libSDL3.so`) found via
  `$ORIGIN` RPATH.
- `/usr/share/pilot/resources/`: shaders and textures. New files dropped
  under `resources/` in the source tree are shipped automatically.
- `/usr/share/pilot/ui/`: the CEF web page. New files under `ui/` are
  shipped automatically.

Build the RPM with CPack (needs `rpm-build`):

```bash
cpack -G RPM -B packages --config build-rpm/CPackConfig.cmake
```

or run `cpack -G RPM` inside `build-rpm`. Install with
`sudo dnf install packages/pilot-*.rpm`.

> [!NOTE]
> `CPACK_PACKAGE_VERSION` (`cmake/packaging.cmake`) and
> `CPACK_RPM_PACKAGE_LICENSE` are placeholders. Set the real version and
> license before publishing. The vendored `cglm` build also installs its
> public headers and CMake/pkgconfig files to `/usr`; harmless, but split
> into a `-devel` subpackage if the package is submitted to Fedora.
