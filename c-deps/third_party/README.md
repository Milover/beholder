# beholder third-party dependencies

### Notes

Some notes in no apparent order:
- `release` and `debug` CMake use the system-default C/C++ compiler
- the Docker image should/will get built/pushed by the CI (GitHub action)
- when building the Docker image, additional CMake arguments can be supplied
  as Docker build-args through `bh_cmake_args`, however they should all be
  supplied as a single string
- the `test` directory contains unit tests, which verify the build and get
  run automatically during the (CMake) build step
- for local development, this project should be built separately from the main
  project, using one of the predefined CMake workflows; this will package the
  dependencies into an archive, which can then be used through `FetchContent`,
  in conjunction with `find_package` from the main project;

### TODO

- [ ] clean up/improve README
- [ ] add CI stuff (GitHub actions)
- [ ] re-enable CUDA/cuDNN support
