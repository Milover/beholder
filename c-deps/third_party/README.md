# beholder third-party dependencies


### Notes

Some notes in no apparent order:
- `release` and `debug` CMake presets will use the system-default C/C++ compiler
- the Docker image should/will get built/pushed by the CI (GitHub action)
- the `docker-compose.yaml` is intended for development use, that is,
  to test/debug the Docker image build; we opted for a compose-file instead of
  random shell scripts, CMake/Makefiles or trying to remember the build command
- when building the Docker image, additional CMake arguments can be supplied
  as Docker build-args through `bh_cmake_args`, however they should all be
  supplied as a single string
- the `test` directory contains unit tests, which verify the build and get
  run automatically during the (CMake) build step
- for local development, this project should be built separately from the main
  project, using one of the predefined CMake workflows; this will package the
  dependencies into an archive, which can then be used through `FetchContent`,
  in conjunction with `find_package` from the main project;
- the main project Docker image (the main project's CI pipeline, when building
  locally, the previous bullet applies, since the packaged dependencies will
  likely be unavailable by themselves) should be built using this project's
  image as the base, i.e. use `FROM beholder-third_party:latest as base`
  instead of `COPY some/packaged/archives ./` in the main project's Dockerfile


### TODO

- [ ] clean up/improve README
- [ ] add CI stuff (GitHub actions)
- [ ] add CUDA/cuDNN support
