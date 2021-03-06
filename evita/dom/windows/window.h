// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_WINDOWS_WINDOW_H_
#define EVITA_DOM_WINDOWS_WINDOW_H_

#include <vector>

#include "evita/base/tree/node.h"
#include "evita/dom/events/view_event_target.h"
#include "evita/dom/public/float_point.h"
#include "evita/dom/public/float_rect.h"
#include "evita/dom/public/float_size.h"
#include "evita/dom/public/window_id.h"
#include "evita/ginx/scriptable.h"

namespace dom {

class ScriptHost;
class WindowSet;

namespace bindings {
class WindowClass;
}

//////////////////////////////////////////////////////////////////////
//
// Window is correspond to |Widget| in DOM world.
//
class Window : public ginx::Scriptable<Window, ViewEventTarget>,
               public base::tree::Node<Window> {
  DECLARE_SCRIPTABLE_OBJECT(Window)

 public:
  using FloatPoint = gfx::FloatPoint;
  using FloatRect = gfx::FloatRect;
  using FloatSize = gfx::FloatSize;

  enum class State {
    Destroyed = -2,
    Destroying,
    NotRealized,
    Realizing,
    Realized,
  };
  static_assert(!static_cast<int>(State::NotRealized),
                "Window::State::kNotRealized should be zero.");

  ~Window() override;

  const gfx::FloatRect& bounds() const { return bounds_; }
  Window* first_child() const { return Node::first_child(); }
  Window* last_child() const { return Node::last_child(); }
  Window* next_sibling() const { return Node::next_sibling(); }
  Window* previous_sibling() const { return Node::previous_sibling(); }
  Window* parent_window() const { return parent_node(); }
  State state() const { return state_; }
  bool visible() const { return visible_; }
  domapi::WindowId window_id() const { return event_target_id(); }

  virtual void DidActivateWindow();
  virtual void DidChangeBounds(int left, int top, int right, int bottom);
  virtual void DidDestroyWindow();
  virtual void DidKillFocus();
  virtual void DidHideWindow();
  virtual void DidRealizeWindow();
  virtual void DidSetFocus();
  virtual void DidShowWindow();

  static Window* GetFocusWindow();
  static void ResetForTesting();

 protected:
  explicit Window(ScriptHost* script_host);

  ScriptHost* script_host() const { return script_host_; }

  virtual void DidChangeBounds();
  virtual void ForceUpdateWindow();

 private:
  friend class bindings::WindowClass;
  friend class WindowSet;

  using Node = base::tree::Node<Window>;

  // bindings
  std::vector<Window*> child_windows() const;
  float client_height() const { return bounds_.height(); }
  float client_left() const { return bounds_.x(); }
  float client_top() const { return bounds_.y(); }
  float client_width() const { return bounds_.width(); }

  void AddWindow(Window* window, ExceptionState* exception_state);
  void ChangeParentWindow(Window* new_parent_window,
                          ExceptionState* exception_state);
  void Destroy(ExceptionState* exception_state);
  void Focus(ExceptionState* exception_state);
  void Hide(ExceptionState* exception_state);
  bool IsDescendantOf(Window* other) const;
  void ReleaseCapture(ExceptionState* exception_state);
  void Realize(ExceptionState* exception_state);
  void RemoveWindow(Window* window, ExceptionState* exception_state);
  void SetCapture(ExceptionState* exception_state);
  void Show(ExceptionState* exception_state);
  void SplitHorizontally(Window* new_right_window,
                         ExceptionState* exception_state);
  void SplitVertically(Window* new_below_window,
                       ExceptionState* exception_state);
  void Update(ExceptionState* exception_state);

  gfx::FloatRect bounds_;
  ScriptHost* const script_host_;
  State state_;
  bool visible_ = false;

  DISALLOW_COPY_AND_ASSIGN(Window);
};

}  // namespace dom

namespace gin {
template <>
struct Converter<dom::Window::State> {
  static v8::Local<v8::Value> ToV8(v8::Isolate* isolate,
                                   dom::Window::State state);
};
}  // namespace gin

#endif  // EVITA_DOM_WINDOWS_WINDOW_H_
