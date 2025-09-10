#pragma once

// Keyball profile (kbpf) persistence helpers.
//
// These small routines isolate EEPROM access for kbpf so that
// future profile fields can be added without touching other modules.
// Each function comes with detailed comments to aid understanding.

// Reset kbpf structure to build-time defaults.
void kbpf_defaults(void);

// Apply migration and range checks after loading old data.
// Call this if you need to upgrade profile layout across versions.
void kbpf_after_load_fixup(void);

// Load kbpf from EEPROM, creating defaults when data is missing or corrupt.
void kbpf_read(void);

// Save current kbpf to EEPROM.
void kbpf_write(void);

