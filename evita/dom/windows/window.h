// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_windows_window_h)
#define INCLUDE_evita_dom_windows_window_h

#include <vector>

#include "common/tree/node.h"
#include "evita/dom/events/view_event_target.h"
#include "evita/dom/windows/window_id.h"
#include "evita/v8_glue/nullable.h"
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
  friend class bindings::WindowClass;
  friend class WindowSet;

  private: typedef common::tree::Node<Window> Node;

  public: enum class State {
    Destroyed = -2,
    Destroying,
    NotRealized,
    Realizing,
    Realized,
  };
  static_assert(!static_cast<int>(State::NotRealized),
                "Window::State::kNotRealized should be zero.");

  private: State state_;

  protected: Window();
  protected: virtual ~Window();

  private: std::vector<Window*> child_windows() const;
  public: v8_glue::Nullable<Window> first_child() const {
    return Node::first_child();
  }
  public: v8_glue::Nullable<Window> last_child() const {
    return Node::last_child();
  }
  public: v8_glue::Nullable<Window> next_sibling() const {
    return Node::next_sibling();
  }
  public: v8_glue::Nullable<Window> previous_sibling() const {
    return Node::previous_sibling();
  }
  public: v8_glue::Nullable<Window> parent_window() const {
    return parent_node();
  }
  public: State state() const { return state_; }
  public: WindowId window_id() const { return event_target_id(); }

  private: void AddWindow(Window* window);
  private: void ChangeParentWindow(Window* new_parent_window);
  private: void Destroy();
  public: virtual void DidDestroyWindow();
  public: virtual void DidRealizeWindow();
  public: void DidChangeBounds(int left, int top, int right, int bottom);
  public: void DidSetFocus();
  private: void Focus();
  private: void Hide();
  private: bool IsDescendantOf(Window* other) const;
  private: void ReleaseCapture();
  private: void Realize();
  private: void RemoveWindow(Window* window);
  public: static void ResetForTesting();
  private: void SetCapture();
  private: void Show();
  private: void SplitHorizontally(Window* new_right_window);
  private: void SplitVertically(Window* new_below_window);
  private: void Update();

  DISALLOW_COPY_AND_ASSIGN(Window);
};

}  // namespace dom

namespace gin {
template<>
struct Converter<dom::Window::State> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
                                    dom::Window::State state);
};
}  // namespace gin

#endif //!defined(INCLUDE_evita_dom_windows_window_h)
