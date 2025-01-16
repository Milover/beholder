# syntax=docker/dockerfile:1

# CMake preset used for the build.
ARG bh_cmake_preset=release
# CMake arguments, in addition to the ones from the preset, defined on the CLI.
# Usually unnecessary, but here just in case.
ARG bh_cmake_args=
# Version of Clang we're working with.
ARG bh_clang_version=16
# Version of Go we're working with
ARG bh_go_version=1.23.2

# local vars which don't need to be supplied
ARG bh_src=/usr/local/src
ARG bh_staging=/usr/local
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
ARG bh_go_version
ARG DEBIAN_FRONTEND

ENV PATH="$PATH:/usr/local/go/bin:/root/go/bin"

RUN <<EOF
set -ex
apt-get install -y --no-install-recommends \
	g++ clang-${bh_clang_version} llvm-${bh_clang_version} cmake ninja-build \
	git ca-certificates curl pkg-config make protobuf-compiler
update-alternatives \
	--install /usr/bin/clang   clang   /usr/bin/clang-${bh_clang_version} 50 \
	--slave   /usr/bin/clang++ clang++ /usr/bin/clang++-${bh_clang_version}

# FIXME: bad!
curl -L -O https://go.dev/dl/go${bh_go_version}.linux-amd64.tar.gz
rm -rf /usr/local/go
tar -C /usr/local -xzf go${bh_go_version}.linux-amd64.tar.gz
rm go${bh_go_version}.linux-amd64.tar.gz

go install google.golang.org/protobuf/cmd/protoc-gen-go@latest
EOF

# -----------------------------------------------------------------------------
# Build, test and install libbeholder third-party libraries.
FROM builder-base AS builder-bh-third-party

ARG bh_cmake_preset
ARG bh_cmake_args
ARG bh_src
ARG bh_staging

WORKDIR ${bh_src}/bh-third-party
COPY ./c-deps/third_party ./

# Tests are run automatically during the build step because they are
# an ExternalProject, hence we don't have to run CTest, since the main
# project doesn't actually have any tests.
#
# We always build third-party stuff in Release config.
RUN <<EOF
set -ex
cmake --preset=release -DCMAKE_INSTALL_PREFIX=${bh_staging} "${bh_cmake_args}"
cmake --build --preset=release
cmake --install build/release
EOF

# -----------------------------------------------------------------------------
# Build, test and install the libbeholder library.
FROM builder-bh-third-party AS builder-bh

ARG bh_cmake_preset
ARG bh_cmake_args
ARG bh_src
ARG bh_staging

WORKDIR ${bh_src}/bh
COPY ./c-deps ./

RUN <<EOF
set -ex
cmake --preset=${bh_cmake_preset} -DCMAKE_INSTALL_PREFIX=${bh_staging} "${bh_cmake_args}"
cmake --build --preset=${bh_cmake_preset}
# NOTE: CI should probably drive the tests, but they should definitely run
# even when this is not the final target
#ctest --preset=${bh_cmake_preset}
cmake --install build/${bh_cmake_preset}
EOF

# -----------------------------------------------------------------------------
# Build, test and install the beholder binary.
FROM builder-bh AS builder

ARG bh_src
ARG bh_staging

WORKDIR ${bh_src}/beholder
COPY ./ ./

RUN <<EOF
set -ex
ldconfig
make generate
make testv
make build
EOF

# -----------------------------------------------------------------------------
# The final image which gets pushed to the registry.
FROM base AS runtime

ARG bh_src
ARG bh_staging

# FIXME: globbing the shared libs is pretty janky
COPY --from=builder ${bh_staging}/lib/*.so* /usr/local/lib/
COPY --from=builder ${bh_src}/beholder/bin /usr/local/bin/

RUN <<EOF
set -ex
ldconfig
EOF
