// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

// Symbol visibility macros.

#ifndef BEHOLDER_CAMERA_EXPORT_H
#define BEHOLDER_CAMERA_EXPORT_H

// BH_CAM_API qualifies all symbols that must be exported.
#ifdef BEHOLDER_CAMERA_STATIC_DEFINE
#define BH_CAM_API
#else
#ifndef BH_CAM_API
#define BH_CAM_API __attribute__((visibility("default")))
#endif
#endif	// BEHOLDER_CAMERA_STATIC_DEFINE

#endif	// BEHOLDER_CAMERA_EXPORT_H
