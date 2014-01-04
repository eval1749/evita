// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/window.h"

#include <algorithm>
#include <unordered_map>

#include "base/bind.h"
#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "common/memory/singleton.h"
#include "evita/dom/script_controller.h"
#include "evita/dom/view_delegate.h"
#include "evita/gc/weak_ptr.h"

namespace gin {

const char* state_strings[] = {
  "destroyed",
  "notrealized",
  "realizing",
  "realized",
};

template<>
struct Converter<dom::Window::State> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
                                    dom::Window::State state) {
    size_t index = state - dom::Window::State::kDestroyed;
    DCHECK_LE(index, arraysize(state_strings));
    return gin::StringToSymbol(isolate, state_strings[index]);
  }
};

} // namespace gin

namespace dom {

namespace {
class WindowWrapperInfo : public v8_glue::WrapperInfo {
  public: WindowWrapperInfo() : v8_glue::WrapperInfo("Window") {
  }
  public: ~WindowWrapperInfo() = default;

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetMethod("add", &Window::AddWindow)
        .SetProperty("children", &Window::child_windows)
        .SetProperty("id", &Window::id)
        .SetProperty("parent", &Window::parent_window)
        .SetProperty("state", &Window::state)
        .SetMethod("realize", &Window::Realize)
        .SetMethod("remove", &Window::RemoveWindow);
  }
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Window::WidgetIdMapper
//
// This class represents mapping from widget id to DOM Window object.
//
// WidgetIdMapper resets Window::widget_id_ when corresponding widget is
// destroyed.
class Window::WidgetIdMapper : public common::Singleton<WidgetIdMapper> {
  friend class common::Singleton<WidgetIdMapper>;

  private: typedef WidgetId WidgetId;

  private: std::unordered_map<WidgetId, gc::WeakPtr<Window>> map_;
  private: WidgetId next_widget_id_;

  private: WidgetIdMapper() : next_widget_id_(1) {
  }
  public: ~WidgetIdMapper() = default;

  public: void DidDestroyWidget(WidgetId widget_id) {
    DCHECK_NE(kInvalidWidgetId, widget_id);
    auto it = map_.find(widget_id);
    if (it == map_.end()) {
      DVLOG(0) << "Why we don't have a widget for WidgetId " << widget_id <<
        " in WidgetIdMap?";
      return;
    }
    auto const window = it->second.get();
    window->widget_id_ = kInvalidWidgetId;
    DCHECK_EQ(kRealized, window->state_);
    window->state_ = kDestroyed;
    if (auto const parent = window->parent_window_) {
      parent->child_windows_.erase(window);
      window->parent_window_ = nullptr;
    }
  }

  public: Window* Find(WidgetId widget_id) {
    auto it = map_.find(widget_id);
    return it == map_.end() ? nullptr : it->second.get();
  }

  public: WidgetId Register(Window* window) {
    auto widget_id = next_widget_id_;
    map_[widget_id] = window;
    ++next_widget_id_;
    return widget_id;
  }

  public: void ResetForTesting() {
    next_widget_id_ = 1;
    map_.clear();
  }

  public: void Unregister(WidgetId widget_id) {
    DCHECK_NE(kInvalidWidgetId, widget_id);
    map_.erase(widget_id);
  }
};

//////////////////////////////////////////////////////////////////////
//
// Window
//
Window::Window()
    : parent_window_(nullptr),
      state_(kNotRealized),
      ALLOW_THIS_IN_INITIALIZER_LIST(widget_id_(
          WidgetIdMapper::instance()->Register(this))) {
  DCHECK_NE(kInvalidWidgetId, widget_id_);
}

Window::~Window() {
  if (widget_id_ == kInvalidWidgetId)
    return;
  WidgetIdMapper::instance()->Unregister(widget_id_);
  ScriptController::instance()->view_delegate()->DestroyWindow(widget_id_);
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

v8_glue::WrapperInfo* Window::static_wrapper_info() {
  DEFINE_STATIC_LOCAL(WindowWrapperInfo, wrapper_info, ());
  return &wrapper_info;
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
      widget_id_, window->widget_id());
}

void Window::DidDestroyWidget(WidgetId widget_id) {
  DCHECK_NE(kInvalidWidgetId, widget_id);
  WidgetIdMapper::instance()->DidDestroyWidget(widget_id);
}

void Window::DidRealizeWidget(WidgetId widget_id) {
  auto const widget = FromWidgetId(widget_id);
  DCHECK_EQ(kRealizing, widget->state_);
  widget->state_ = kRealized;
  for (auto child : widget->child_windows_) {
    if (child->state_ == kNotRealized)
      child->state_ = kRealized;
  }
}

Window* Window::FromWidgetId(WidgetId widget_id) {
  DCHECK_NE(kInvalidWidgetId, widget_id);
  return WidgetIdMapper::instance()->Find(widget_id);
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
    DCHECK_EQ(kInvalidWidgetId, widget_id_);
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
  state_ = kRealizing;
  ScriptController::instance()->view_delegate()->RealizeWindow(widget_id_);
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
  WidgetIdMapper::instance()->ResetForTesting();
}

}  // namespace dom

std::ostream& operator<<(std::ostream& ostream, const dom::Window& window) {
  ostream << "(" << window.wrapper_info()->class_name() << " widget:" <<
      window.widget_id() << ")";
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream, const dom::Window* window) {
  return ostream << *window;
}
