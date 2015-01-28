// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_BROWSER_PRESENTATION_SCREEN_AVAILABILITY_LISTENER_H_
#define CONTENT_PUBLIC_BROWSER_PRESENTATION_SCREEN_AVAILABILITY_LISTENER_H_

#include <string>

namespace content {

// A listener interface used for receiving updates on screen availability
// associated with a presentation URL from an embedder.
// See also PresentationServiceDelegate.
class PresentationScreenAvailabilityListener {
 public:
  // Returns the Presentation URL associated with this listener.
  virtual std::string GetPresentationUrl() = 0;

  // Called when screen availability for the associated Presentation URL has
  // changed to |available|.
  virtual void OnScreenAvailabilityChanged(bool available) = 0;

 protected:
  virtual ~ScreenAvailabilityListener() {}
};

}  // namespace content

#endif  // CONTENT_PUBLIC_BROWSER_PRESENTATION_SCREEN_AVAILABILITY_LISTENER_H_

