// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_WINDOWS_WINDOW_H_
#define EVITA_DOM_WINDOWS_WINDOW_H_

#include <vector>

#include "common/tree/node.h"
#include "evita/dom/events/view_event_target.h"
#include "evita/dom/public/window_id.h"
#include "evita/v8_glue/scriptable.h"

namespace dom {

class WindowSet;

namespace bindings {
class WindowClass;
}

// |Window| is correspond to |Widget| in DOM world.
class Window : public v8_glue::Scriptable<Window, ViewEventTarget>,
               public common::tree::Node<Window> {
  DECLARE_SCRIPTABLE_OBJECT(Window)

 public:
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

  Window* first_child() const { return Node::first_child(); }
  Window* last_child() const { return Node::last_child(); }
  Window* next_sibling() const { return Node::next_sibling(); }
  Window* previous_sibling() const { return Node::previous_sibling(); }
  Window* parent_window() const { return parent_node(); }
  State state() const { return state_; }
  domapi::WindowId window_id() const { return event_target_id(); }

  virtual void DidActivateWindow();
  virtual void DidChangeBounds(int left, int top, int right, int bottom);
  virtual void DidDestroyWindow();
  virtual void DidHideWindow();
  virtual void DidRealizeWindow();
  virtual void DidShowWindow();
  void DidSetFocus();
  static void ResetForTesting();

 protected:
  Window();

 private:
  friend class bindings::WindowClass;
  friend class WindowSet;

  using Node = common::tree::Node<Window>;

  std::vector<Window*> child_windows() const;

  void AddWindow(Window* window);
  void ChangeParentWindow(Window* new_parent_window);
  void Destroy();
  void Focus();
  void Hide();
  bool IsDescendantOf(Window* other) const;
  void ReleaseCapture();
  void Realize();
  void RemoveWindow(Window* window);
  void SetCapture();
  void Show();
  void SplitHorizontally(Window* new_right_window);
  void SplitVertically(Window* new_below_window);
  void Update();

  State state_;

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
