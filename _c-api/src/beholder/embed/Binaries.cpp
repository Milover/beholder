// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BH_EMBED_DIR
#error "BH_EMBED_DIR not defined"
#endif

#include <incbin/incbin.h>

namespace beholder {
namespace embed {

INCBIN(char, PylonArchive, BH_EMBED_DIR "/pylon.tar.gz");

}  // namespace embed
}  // namespace beholder
