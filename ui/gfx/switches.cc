// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/gfx/switches.h"

namespace switches {

// The ImageSkia looks up the resource pack with the closest available scale
// factor instead of the actual device scale factor and then rescale on
// ImageSkia side. This switch disables this feature.
const char kDisableArbitraryScaleFactorInImageSkia[] =
    "disable-arbitrary-scale-factor-in-image-skia";

// Disables the HarfBuzz port of RenderText on all platforms.
const char kDisableHarfBuzzRenderText[] = "disable-harfbuzz-rendertext";

// Enables the HarfBuzz port of RenderText on all platforms.
const char kEnableHarfBuzzRenderText[] = "enable-harfbuzz-rendertext";

// Enable text glyphs to have X-positions that aren't snapped to the pixel grid
// in webkit renderers.
const char kEnableWebkitTextSubpixelPositioning[] =
    "enable-webkit-text-subpixel-positioning";

// Overrides the device scale factor for the browser UI and the contents.
const char kForceDeviceScaleFactor[] = "force-device-scale-factor";

// Enables/Disables High DPI support (windows)
const char kHighDPISupport[] = "high-dpi-support";

}  // namespace switches
