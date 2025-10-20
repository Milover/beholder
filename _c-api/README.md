`third_party` stuff needs to be built, and installed to `build/staging` to be
able to build locally (as in with proper staging).

To build locally:
```sh
$ cmake [--fresh] --install-prefix "$PWD/build/staging" --preset release [-DBH_NO_LINT=1]
$ cmake --workflow --preset release
```

Just rebuild and test:
```sh
$ cmake --build [--clean-first] --preset release
$ ctest [--verbose] --preset release
```

Builds with system compiler by default, we removed clang presets at some point.
So to build with clang, configure it through `-DCMAKE_C/CXX_COMPILER` options
when configuring.
