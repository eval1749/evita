// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/window.h"

#include <unordered_map>

#include "base/bind.h"
#include "base/logging.h"
#include "common/memory/singleton.h"
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
      DVLOG(0) << "Why we don't have widget_id(" << widget_id <<
        ") in WidgetIdMap?";
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
Window::Window(WidgetId widget_id)
    : ALLOW_THIS_IN_INITIALIZER_LIST(
        widget_id_(WidgetIdMapper::instance()->Register(this))) {
  ASSERT_CALLED_ON_SCRIPT_THREAD();
  DCHECK_NE(widgets::kInvalidWidgetId, widget_id);
}

Window::~Window() {
  ASSERT_CALLED_ON_SCRIPT_THREAD();
  WidgetIdMapper::instance()->Unregister(widget_id_);
}

v8_glue::WrapperInfo* Window::static_wrapper_info() {
  DEFINE_STATIC_LOCAL(v8_glue::WrapperInfo, wrapper_info, ("Window"));
  return &wrapper_info;
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
  return GetEmptyObjectTemplateBuilder(isolate)
    .SetProperty("id", &Window::id);
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
