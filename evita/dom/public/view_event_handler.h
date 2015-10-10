// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_PUBLIC_VIEW_EVENT_HANDLER_H_
#define EVITA_DOM_PUBLIC_VIEW_EVENT_HANDLER_H_

#include <string>
#include <vector>

#include "base/callback_forward.h"
#include "base/strings/string16.h"
#include "evita/dom/public/event_target_id.h"
#include "evita/dom/windows/window_id.h"

namespace base {
class Time;
}

namespace text {
class Buffer;
}

namespace domapi {

struct FocusEvent;
struct FormEvent;
struct KeyboardEvent;
struct MouseEvent;
struct TextCompositionEvent;
enum class Visibility;
struct WheelEvent;
using dom::WindowId;

class ViewEventHandler {
 public:
  virtual ~ViewEventHandler();

  virtual void DidBeginFrame(const base::Time& deadline) = 0;
  virtual void DidChangeWindowBounds(WindowId window_id,
                                     int left,
                                     int top,
                                     int right,
                                     int bottom) = 0;
  virtual void DidChangeWindowVisibility(WindowId window_id,
                                         Visibility visibility) = 0;
  virtual void DidDestroyWidget(WindowId window_id) = 0;
  virtual void DidDropWidget(WindowId source_id, WindowId target_id) = 0;
  virtual void DidRealizeWidget(WindowId window_id) = 0;
  virtual void DidStartViewHost() = 0;
  virtual void DispatchFocusEvent(const FocusEvent& event) = 0;
  virtual void DispatchKeyboardEvent(const KeyboardEvent& event) = 0;
  virtual void DispatchMouseEvent(const MouseEvent& event) = 0;
  virtual void DispatchTextCompositionEvent(
      const TextCompositionEvent& event) = 0;
  virtual void DispatchWheelEvent(const WheelEvent& event) = 0;
  virtual void OpenFile(WindowId window_id,
                        const base::string16& file_name) = 0;
  virtual void ProcessCommandLine(base::string16 working_directory,
                                  const std::vector<base::string16>& args) = 0;
  virtual void QueryClose(WindowId window_id) = 0;
  virtual void RunCallback(base::Closure) = 0;
  virtual void WillDestroyHost() = 0;

 protected:
  ViewEventHandler();

 private:
  DISALLOW_COPY_AND_ASSIGN(ViewEventHandler);
};

}  // namespace domapi

#endif  // EVITA_DOM_PUBLIC_VIEW_EVENT_HANDLER_H_
