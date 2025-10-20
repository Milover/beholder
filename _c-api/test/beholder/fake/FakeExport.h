// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

// Symbol visibility macros.

#ifndef FAKE_EXPORT_H
#define FAKE_EXPORT_H

// FAKE_API qualifies all symbols that must be exported.
#ifdef FAKE_STATIC_DEFINE
#define FAKE_API
#else
#ifndef FAKE_API
#define FAKE_API __attribute__((visibility("default")))
#endif
#endif	// FAKE_STATIC_DEFINE

#endif	// FAKE_EXPORT_H
