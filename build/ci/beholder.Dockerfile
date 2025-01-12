# syntax=docker/dockerfile:1

# CMake preset used for the build.
ARG bh_cmake_preset
# CMake arguments, in addition to the ones from the preset, defined on the CLI.
# Usually unnecessary, but here just in case.
ARG bh_cmake_args=
# Version of Clang we're working with.
ARG bh_clang_version=16

# local vars which don't need to be supplied
ARG bh_src=/usr/local/src/beholder
ARG bh_staging=/opt/beholder
ARG DEBIAN_FRONTEND=noninteractive

# -----------------------------------------------------------------------------
# A base image containing shared packages.
# For example CUDA/cuDNN stuff should get pulled here (once set up) because
# it's required by both the build, and the final image.
FROM debian:bookworm-slim AS base

ARG DEBIAN_FRONTEND

RUN <<EOF
set -ex
apt-get update
EOF

# -----------------------------------------------------------------------------
# A base image containing build tools.
FROM base AS builder-base

ARG bh_clang_version
ARG DEBIAN_FRONTEND

RUN <<EOF
set -ex
apt-get install -y --no-install-recommends \
	g++ clang-${bh_clang_version} llvm-${bh_clang_version} cmake ninja-build \
	git ca-certificates
update-alternatives \
	--install /usr/bin/clang   clang   /usr/bin/clang-${bh_clang_version} 50 \
	--slave   /usr/bin/clang++ clang++ /usr/bin/clang++-${bh_clang_version}
EOF

# -----------------------------------------------------------------------------
# Build, test and install libbeholder third-party libraries.
FROM builder-base AS builder-bh-third-party

ARG bh_cmake_preset
ARG bh_cmake_args
ARG bh_src
ARG bh_staging

WORKDIR ${bh_src}
COPY ./c-deps/third_party ./

# Tests are run automatically during the build step because they are
# an ExternalProject, hence we don't have to run CTest, since the main
# project doesn't actually have any tests.
RUN <<EOF
set -ex
cmake --preset=${bh_cmake_preset} "${bh_cmake_args}"
cmake --build --preset=${bh_cmake_preset}
cmake --install build/${bh_cmake_preset} --prefix=${bh_staging}
EOF

# -----------------------------------------------------------------------------
# Build, test and install the libbeholder library.
FROM builder-base AS builder-bh

ARG bh_cmake_preset
ARG bh_cmake_args
ARG bh_src
ARG bh_staging

WORKDIR ${bh_src}
COPY --exclude=./c-deps/third_party ./c-deps ./
COPY --from=builder-bh-third-party ${bh_staging} /usr/local/

RUN <<EOF
set -ex
cmake --preset=${bh_cmake_preset} "${bh_cmake_args}"
cmake --build --preset=${bh_cmake_preset}
# NOTE: CI should probably drive the tests, but they should definitely run
# even when this is not the final target
ctest --preset=${bh_cmake_preset}
cmake --install build/${bh_cmake_preset} --prefix=${bh_staging}
EOF

# -----------------------------------------------------------------------------
# Build, test and install the beholder binary.
FROM builder-base AS builder

ARG bh_cmake_preset
ARG bh_cmake_args
ARG bh_src
ARG bh_staging

WORKDIR ${bh_src}
COPY --exclude=./c-deps ./ ./
COPY --from=builder-bh ${bh_staging} /usr/local/

RUN <<EOF
set -ex
make test-integration
make build
EOF

# -----------------------------------------------------------------------------
# The final image which gets pushed to the registry.
FROM base AS runtime

ARG bh_src
ARG bh_staging

# FIXME: globbing the shared libs is pretty janky
COPY --from=builder ${bh_staging}/lib/*.so* /usr/local/lib/
COPY --from=builder ${bh_src}/bin /usr/local/bin/
