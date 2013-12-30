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
#include "evita/dom/script_thread.h"
#include "evita/editor/application.h"
#include "evita/gc/weak_ptr.h"

namespace dom {

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

  private: typedef widgets::WidgetId WidgetId;

  private: std::unordered_map<WidgetId, gc::WeakPtr<Window>> map_;
  private: WidgetId next_widget_id_;

  private: WidgetIdMapper() : next_widget_id_(1) {
    ASSERT_CALLED_ON_SCRIPT_THREAD();
  }
  public: ~WidgetIdMapper() = default;

  public: void DidDestroyWidget(WidgetId widget_id) {
    ASSERT_CALLED_ON_SCRIPT_THREAD();
    DCHECK_NE(widgets::kInvalidWidgetId, widget_id);
    auto it = map_.find(widget_id);
    if (it == map_.end()) {
      DVLOG(0) << "Why we don't have a widget for WidgetId " << widget_id <<
        " in WidgetIdMap?";
      return;
    }
    it->second->widget_id_ = widgets::kInvalidWidgetId;
  }

  public: Window* Find(WidgetId widget_id) {
    ASSERT_CALLED_ON_SCRIPT_THREAD();
    auto it = map_.find(widget_id);
    return it == map_.end() ? nullptr : it->second.get();
  }

  public: WidgetId Register(Window* window) {
    ASSERT_CALLED_ON_SCRIPT_THREAD();
    auto widget_id = next_widget_id_;
    map_[widget_id] = window;
    ++next_widget_id_;
    return widget_id;
  }

  public: void Unregister(WidgetId widget_id) {
    ASSERT_CALLED_ON_SCRIPT_THREAD();
    DCHECK_NE(widgets::kInvalidWidgetId, widget_id);
    map_.erase(widget_id);
  }
};

//////////////////////////////////////////////////////////////////////
//
// Window
//
Window::Window()
    : parent_window_(nullptr),
      ALLOW_THIS_IN_INITIALIZER_LIST(widget_id_(
          WidgetIdMapper::instance()->Register(this))) {
  ASSERT_CALLED_ON_SCRIPT_THREAD();
  DCHECK_NE(widgets::kInvalidWidgetId, widget_id_);
}

Window::~Window() {
  ASSERT_CALLED_ON_SCRIPT_THREAD();
  if (widget_id_ == widgets::kInvalidWidgetId)
    return;
  WidgetIdMapper::instance()->Unregister(widget_id_);
  widgets::Widget::DidDestroyDomWindow(widget_id_);
}

v8_glue::WrapperInfo* Window::static_wrapper_info() {
  DEFINE_STATIC_LOCAL(v8_glue::WrapperInfo, wrapper_info, ("Window"));
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
  child_windows_.push_back(window);
}

void Window::DidDestroyWidget(WidgetId widget_id) {
  ASSERT_CALLED_ON_UI_THREAD();
  DCHECK_NE(widgets::kInvalidWidgetId, widget_id);
  ScriptThread::instance()->PostTask(FROM_HERE,
    base::Bind(&WidgetIdMapper::DidDestroyWidget,
        base::Unretained(WidgetIdMapper::instance()), widget_id));
}

Window* Window::FromWidgetId(WidgetId widget_id) {
  return WidgetIdMapper::instance()->Find(widget_id);
}

gin::ObjectTemplateBuilder Window::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  return GetObjectTemplateBuilderFromBase(isolate)
      .SetMethod("add", &Window::AddWindow)
      // Note: we expose widget id for debugging.
      .SetProperty("id", &Window::id)
      .SetProperty("parent", &Window::parent_window)
      .SetMethod("remove", &Window::RemoveWindow);
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

}  // namespace dom

std::ostream& operator<<(std::ostream& ostream, const dom::Window& window) {
  ostream << "(" << window.wrapper_info()->class_name() << " widget:" <<
      window.widget_id() << ")";
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream, const dom::Window* window) {
  return ostream << *window;
}
