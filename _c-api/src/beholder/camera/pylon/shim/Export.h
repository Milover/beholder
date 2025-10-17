// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

// Symbol visibility macros.

#ifndef BEHOLDER_PYLONSHIM_EXPORT_H
#define BEHOLDER_PYLONSHIM_EXPORT_H

// BH_PYLONSHIM_API qualifies all symbols that must be exported.
#ifndef BH_PYLONSHIM_API
#define BH_PYLONSHIM_API __attribute__((visibility("default")))
#endif

#endif	// BEHOLDER_PYLONSHIM_EXPORT_H
