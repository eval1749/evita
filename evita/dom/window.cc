// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/window.h"

#include <algorithm>
#include <unordered_map>

#include "base/bind.h"
#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "common/memory/singleton.h"
#include "evita/dom/events/event.h"
#include "evita/dom/script_controller.h"
#include "evita/dom/view_delegate.h"
#include "evita/gc/weak_ptr.h"
#include "evita/v8_glue/wrapper_info.h"

namespace gin {

const char* state_strings[] = {
  "destroyed",
  "destroying",
  "notrealized",
  "realizing",
  "realized",
};

template<>
struct Converter<dom::Window::State> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
                                    dom::Window::State state) {
    auto const index = static_cast<size_t>(
        state - dom::Window::State::kDestroyed);
    DCHECK_LE(index, arraysize(state_strings) - 1);
    return gin::StringToSymbol(isolate, state_strings[index]);
  }
};

} // namespace gin

namespace dom {

namespace {
class WindowWrapperInfo :
    public v8_glue::DerivedWrapperInfo<Window, EventTarget> {

  public: explicit WindowWrapperInfo(const char* name)
      : BaseClass(name) {
  }
  public: ~WindowWrapperInfo() = default;

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetValue("firstChild", v8::Maybe<Window*>())
        .SetValue("lastChild", v8::Maybe<Window*>())
        .SetValue("nextSibling", v8::Maybe<Window*>())
        .SetValue("previousSibling", v8::Maybe<Window*>())
        .SetProperty("children", &Window::child_windows)
        .SetProperty("focusTick_", &Window::focus_tick)
        .SetProperty("id", &Window::id)
        .SetProperty("parent", &Window::parent_window)
        .SetProperty("state", &Window::state)
        .SetMethod("appendChild", &Window::AddWindow)
        .SetMethod("changeParent", &Window::ChangeParentWindow)
        .SetMethod("destroy", &Window::Destroy)
        .SetMethod("focus", &Window::Focus)
        .SetMethod("realize", &Window::Realize)
        .SetMethod("remove", &Window::RemoveWindow);
  }
};

std::vector<Window*> DescendantsOrSelf(Window* window) {
  class Collector {
    public: std::vector<Window*> windows_;
    public: Collector() = default;
    public: ~Collector() = default;
    public: void Collect(Window* window) {
      windows_.push_back(window);
      for (auto child : window->child_windows()) {
        Collect(child);
      }
    }
  };
  Collector collector;
  collector.Collect(window);
  return std::move(collector.windows_);
}

int global_focus_tick;
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Window::WindowIdMapper
//
// This class represents mapping from widget id to DOM Window object.
//
// WindowIdMapper resets Window::window_id() when corresponding widget is
// destroyed.
class Window::WindowIdMapper : public common::Singleton<WindowIdMapper> {
  friend class common::Singleton<WindowIdMapper>;

  private: typedef WindowId WindowId;

  private: std::unordered_map<WindowId, gc::WeakPtr<Window>> map_;

  private: WindowIdMapper() = default;
  public: ~WindowIdMapper() = default;

  public: void DidDestroyWidget(WindowId window_id) {
    DCHECK_NE(kInvalidWindowId, window_id);
    auto it = map_.find(window_id);
    if (it == map_.end()) {
      DVLOG(0) << "Why we don't have a widget for WindowId " << window_id <<
        " in WindowIdMap?";
      return;
    }
    auto const window = it->second.get();
    DCHECK_NE(kDestroyed, window->state_);
    window->state_ = kDestroyed;
    if (auto const parent = window->parent_window_) {
      parent->child_windows_.erase(window);
      window->parent_window_ = nullptr;
    }
  }

  public: Window* Find(WindowId window_id) {
    auto it = map_.find(window_id);
    return it == map_.end() ? nullptr : it->second.get();
  }

  public: void Register(Window* window) {
    map_[window->window_id()] = window;
  }

  public: void ResetForTesting() {
    map_.clear();
  }

  public: void Unregister(WindowId window_id) {
    DCHECK_NE(kInvalidWindowId, window_id);
    map_.erase(window_id);
  }
};

//////////////////////////////////////////////////////////////////////
//
// Window
//
DEFINE_SCRIPTABLE_OBJECT(Window, WindowWrapperInfo)

Window::Window()
    : focus_tick_(0),
      parent_window_(nullptr),
      state_(kNotRealized) {
  WindowIdMapper::instance()->Register(this);
}

Window::~Window() {
  if (window_id() == kInvalidWindowId)
    return;
  WindowIdMapper::instance()->Unregister(window_id());
  ScriptController::instance()->view_delegate()->DestroyWindow(window_id());
}

std::vector<Window*> Window::child_windows() const {
  std::vector<Window*> child_windows(child_windows_.size());
  auto dest = child_windows.begin();
  for (auto child : child_windows_) {
    *dest = child;
    ++dest;
  }
  return child_windows;
}

void Window::AddWindow(Window* window) {
  if (window == this) {
    ScriptController::instance()->ThrowError(
        base::StringPrintf("Can't add window(%d) to itself.", id()));
    return;
  }
  if (window->parent_window_) {
    ScriptController::instance()->ThrowError(
        base::StringPrintf("Window(%d) is already child of window(%d).",
            window->id(), window->parent_window_->id()));
    return;
  }
  if (IsDescendantOf(window)) {
    ScriptController::instance()->ThrowError(
        base::StringPrintf("Window(%d) is parent or ancestor of window(%d).",
            window->id(), id()));
    return;
  }
  window->parent_window_ = this;
  child_windows_.insert(window);
  ScriptController::instance()->view_delegate()->AddWindow(
      window_id(), window->window_id());
}

void Window::ChangeParentWindow(Window* new_parent_window) {
  if (parent_window_ == new_parent_window)
    return;
  if (this == new_parent_window) {
    ScriptController::instance()->ThrowError(
      "Can't change parent to itself.");
    return;
  }
  if (new_parent_window->IsDescendantOf(this)) {
    ScriptController::instance()->ThrowError(base::StringPrintf(
        "Can't change parent of window(%d) to window(%d), becase window(%d)"
        " is descendant of window(%d).",
        window_id(), new_parent_window->window_id(),
        new_parent_window->window_id(), window_id()));
    return;
  }
  parent_window_->child_windows_.erase(this);
  parent_window_ = new_parent_window;
  parent_window_->child_windows_.insert(this);
  ScriptController::instance()->view_delegate()->ChangeParentWindow(
      window_id(), new_parent_window->window_id());
}

void Window::Destroy() {
  if (state_ != kRealized && state_ != kRealizing) {
    ScriptController::instance()->ThrowError(
        "You can't destroy unrealized window.");
    return;
  }
  for (auto descendant : DescendantsOrSelf(this)) {
    descendant->state_= kDestroying;
  }
  ScriptController::instance()->view_delegate()->DestroyWindow(window_id());
}

void Window::DidKillFocus(WindowId) {
  // TODO(yosi) Dispatch blur event.
}

void Window::DidDestroyWidget(WindowId window_id) {
  DCHECK_NE(kInvalidWindowId, window_id);
  WindowIdMapper::instance()->DidDestroyWidget(window_id);
}

// Possible state transitions:
//  kRealizing -> kRealized 
//    |realize()| call.
//  kNotRealized -> kRealized
//    Adding |kNotRealized| window to |kRealized| window.
//  kDestroying -> kRealized
//    The window was |kRealizing| then |destroy()|.
void Window::DidRealizeWidget(WindowId window_id) {
  auto const widget = FromWindowId(window_id);
  DCHECK(kRealizing == widget->state_ || kDestroying == widget->state_ ||
         kNotRealized == widget->state_);
  widget->state_ = kRealized;
  for (auto child : widget->child_windows_) {
    if (child->state_ == kNotRealized)
      child->state_ = kRealized;
  }
}

void Window::DidSetFocus(dom::WindowId window_id) {
  auto const window = FromWindowId(window_id);
  if (!window)
    return;
  ++global_focus_tick;
  window->focus_tick_ = global_focus_tick;
  // TODO(yosi) Dispatch focus event.
}

void Window::Focus() {
  if (state_ != kRealized && state_ != kRealizing) {
    ScriptController::instance()->ThrowError(
        "You can't focus unrealized window.");
    return;
  }
  ScriptController::instance()->view_delegate()->FocusWindow(window_id());
}

Window* Window::FromWindowId(WindowId window_id) {
  DCHECK_NE(kInvalidWindowId, window_id);
  return WindowIdMapper::instance()->Find(window_id);
}

bool Window::IsDescendantOf(Window* other) const {
  if (parent_window_ == other)
    return true;
  for (auto child : child_windows_) {
    if (child->IsDescendantOf(other))
      return true;
  }
  return false;
}

void Window::Realize() {
  if (state_ == kDestroyed) {
    DCHECK_EQ(kInvalidWindowId, window_id());
    ScriptController::instance()->ThrowError(
        "Can't realize deatched window.");
    return;
  }
  if (state_ == kRealized) {
    ScriptController::instance()->ThrowError(
        "This window is already realized.");
    return;
  }
  if (state_ == kRealizing) {
    ScriptController::instance()->ThrowError(
        "This window is being realized.");
    return;
  }
  if (parent_window_ && parent_window_->state_ == kNotRealized) {
    ScriptController::instance()->ThrowError(
        "Parent window isn't realized.");
    return;
  }
  for (auto descendant : DescendantsOrSelf(this)) {
    descendant->state_= kRealizing;
  }
  ScriptController::instance()->view_delegate()->RealizeWindow(window_id());
}

void Window::RemoveWindow(Window* window) {
  if (window->parent_window_ != window) {
    ScriptController::instance()->ThrowError(base::StringPrintf(
        "Can't remove window(%d) which isn't child of window(%d).",
        window->id(), id()));
    return;
  }
  auto present = std::find(child_windows_.begin(), child_windows_.end(),
                           window);
  if (present == child_windows_.end()) {
    ScriptController::instance()->ThrowError(base::StringPrintf(
        "INTERNAL ERROR: window(%d) isn't in child list of window(%d).",
        window->id(), id()));
    return;
  }
  window->parent_window_ = nullptr;
  child_windows_.erase(present);
}

void Window::ResetForTesting() {
  EventTarget::ResetForTesting();
  global_focus_tick = 0;
  WindowIdMapper::instance()->ResetForTesting();
}

}  // namespace dom

std::ostream& operator<<(std::ostream& ostream, const dom::Window& window) {
  ostream << "(" << window.wrapper_info()->class_name() << " widget:" <<
      window.window_id() << ")";
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream, const dom::Window* window) {
  return ostream << *window;
}
