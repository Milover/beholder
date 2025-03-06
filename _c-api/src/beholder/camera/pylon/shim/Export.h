// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

// Symbol visibility macros.

#ifndef BEHOLDER_PYLON_SHIM_EXPORT_H
#define BEHOLDER_PYLON_SHIM_EXPORT_H

// BH_PYLON_SHIM_API qualifies all symbols that must be exported.
#ifndef BH_PYLON_SHIM_API
#define BH_PYLON_SHIM_API __attribute__((visibility("default")))
#endif

#endif	// BEHOLDER_PYLON_SHIM_EXPORT_H
