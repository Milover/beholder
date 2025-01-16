// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

// Symbol visibility macros.

#ifndef BEHOLDER_EXPORT_H
#define BEHOLDER_EXPORT_H

// BH_API qualifies all symbols that must be exported.
#ifdef BEHOLDER_STATIC_DEFINE
#define BH_API
#else
#ifndef BH_API
#define BH_API __attribute__((visibility("default")))
#endif
#endif	// BEHOLDER_STATIC_DEFINE

#endif	// BEHOLDER_EXPORT_H
