# syntax=docker/dockerfile:1

# CMake preset used for the build.
ARG bh_cmake_preset
# CMake arguments, in addition to the ones from the preset, defined on the CLI.
# Usually unnecessary, but here just in case.
ARG bh_cmake_args=
# Version of Clang we're working with.
ARG bh_clang_version=16

# local vars which don't need to be supplied
ARG bh_srcdir=/usr/src/bh-thirdparty
ARG bh_installdir=/opt/bh-thirdparty
ARG DEBIAN_FRONTEND=noninteractive

# A base image containing shared packages.
# For example CUDA/cuDNN stuff should get pulled here (once set up) because
# it's required by both the build, and the final image.
FROM debian:bookworm-slim AS base

ARG DEBIAN_FRONTEND

RUN <<EOF
set -ex
apt-get update
EOF

# An intermediate image which builds, installs and tests the dependencies.
FROM base AS builder

ARG bh_cmake_preset
ARG bh_cmake_args
ARG bh_clang_version
ARG bh_srcdir
ARG bh_installdir
ARG DEBIAN_FRONTEND

WORKDIR ${bh_srcdir}
COPY ./third_party ./

RUN <<EOF
set -ex
apt-get install -y --no-install-recommends \
	g++ clang-${bh_clang_version} llvm-${bh_clang_version} cmake ninja-build \
	git ca-certificates
update-alternatives \
	--install /usr/bin/clang   clang   /usr/bin/clang-${bh_clang_version} 50 \
	--slave   /usr/bin/clang++ clang++ /usr/bin/clang++-${bh_clang_version}
EOF

# Tests are run automatically during the build step because they are
# an ExternalProject, hence we don't have to run CTest, since the main
# project doesn't actually have any tests.
RUN <<EOF
set -ex
cmake --preset=${bh_cmake_preset} "${bh_cmake_args}"
cmake --build --preset=${bh_cmake_preset}
cmake --install build/${bh_cmake_preset} --prefix=${bh_installdir}
EOF

# The final image which gets pushed to the registry.
FROM base AS runtime

ARG bh_installdir

COPY --from=builder ${bh_installdir} /usr/local/
