# syntax=docker/dockerfile:1

# CMake preset used for the build.
ARG cmake_preset=release
# Version of Go we're working with
ARG go_version=1.23.5
# Version of Clang we're working with.
ARG gcc_version=12
# Version of Clang we're working with.
ARG clang_version=16

# local vars which don't need to be supplied
ARG cmake_prefix=/usr/local
ARG bh_src=/usr/local/src
ARG DEBIAN_FRONTEND=noninteractive

# -----------------------------------------------------------------------------
# A base image containing shared packages.
# For example CUDA/cuDNN stuff should get pulled here (once set up) because
# it's required by both the build, and the final image.
FROM debian:bookworm-slim AS base

ARG DEBIAN_FRONTEND

RUN <<EOF
set -exu

apt-get update
EOF

# -----------------------------------------------------------------------------
# A base image containing build tools.
FROM base AS builder-base

ARG go_version
ARG gcc_version
ARG clang_version
ARG DEBIAN_FRONTEND

ENV PATH="$PATH:/usr/local/go/bin:/root/go/bin"

RUN <<EOF
set -exu

apt-get install -y --no-install-recommends \
	ca-certificates \
	clang-${clang_version} \
	clang-tidy-${clang_version} \
	cmake \
	curl \
	findutils \
	g++-${gcc_version} \
	git \
	make \
	ninja-build \
	pkg-config \
	protobuf-compiler
rm -rf /var/lib/apt/lists/*

update-alternatives \
	--install /usr/bin/gcc gcc /usr/bin/gcc-${gcc_version} 50 \
	--slave   /usr/bin/g++ g++ /usr/bin/g++-${gcc_version}
update-alternatives \
	--install /usr/bin/clang   clang   /usr/bin/clang-${clang_version} 50 \
	--slave   /usr/bin/clang++ clang++ /usr/bin/clang++-${clang_version}

# FIXME: bad!
curl -L -O https://go.dev/dl/go${go_version}.linux-amd64.tar.gz
rm -rf /usr/local/go
tar -C /usr/local -xzf go${go_version}.linux-amd64.tar.gz
rm go${go_version}.linux-amd64.tar.gz

go install google.golang.org/protobuf/cmd/protoc-gen-go@latest

# golangci recommends not using 'go install'
curl -sSfL https://raw.githubusercontent.com/golangci/golangci-lint/master/install.sh | sh -s -- -b $(go env GOPATH)/bin v1.63.4
EOF

# -----------------------------------------------------------------------------
# Build, test and install libbeholder third-party libraries.
FROM builder-base AS builder-third-party

ARG cmake_preset
ARG cmake_prefix
ARG bh_src

COPY ./ ${bh_src}/

# Tests are run automatically during the build step because they are
# an ExternalProject, hence we don't have to run CTest, since the main
# project doesn't actually have any tests.
#
# We always build third-party stuff in Release config.
RUN <<EOF
set -exu

cd ${bh_src}

make third-party

# purge source tree here to reduce image size
# when running as part of CI, we can dump a package artifact here for
# inspection and caching
#rm -rf ${bh_src}/* ${bh_src}/.* 2> /dev/null || true
EOF

# -----------------------------------------------------------------------------
# Build, test and install the libbeholder library.
FROM builder-third-party AS builder-capi

ARG cmake_preset
ARG cmake_prefix
ARG bh_src

# if we're not cleaning builder-third-party, then we don't have to re-copy
#COPY ./ ${bh_src}/

RUN <<EOF
set -exu

cd ${bh_src}

make c-api

# purge source tree here to reduce image size
# when running as part of CI, we can dump a package artifact here for
# inspection and caching
#rm -rf ${bh_src}/* ${bh_src}/.* 2> /dev/null || true
EOF

# -----------------------------------------------------------------------------
# Build, test and install the beholder binary.
FROM builder-capi AS builder

ARG cmake_prefix
ARG bh_src

# if we're not cleaning builder-capi, then we don't have to re-copy
#COPY ./ ${bh_src}/

RUN <<EOF
set -exu

cd ${bh_src}

ldconfig
make lint
make build
# FIXME: this should be a make call
mv bin/* ${cmake_prefix}/bin/

# purge source tree here to reduce image size
# when running as part of CI, we can dump a package artifact here for
# inspection and caching
#rm -rf ${bh_src}/* ${bh_src}/.* 2> /dev/null || true
EOF

# -----------------------------------------------------------------------------
# The final image which gets pushed to the registry.
FROM base AS runtime

ARG cmake_prefix
ARG bh_src

# FIXME: globbing the shared libs is pretty janky
COPY --from=builder ${cmake_prefix}/lib/*.so* /usr/local/lib/
COPY --from=builder ${cmake_prefix}/bin /usr/local/bin/

RUN <<EOF
set -exu

ldconfig
EOF
