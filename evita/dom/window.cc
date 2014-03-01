// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/window.h"

#include <algorithm>
#include <unordered_map>

#include "base/bind.h"
#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "common/memory/singleton.h"
#include "common/tree/child_nodes.h"
#include "common/tree/descendants.h"
#include "evita/dom/events/event.h"
#include "evita/dom/script_controller.h"
#include "evita/dom/view_delegate.h"
#include "evita/dom/window_ostream.h"
#include "evita/gc/weak_ptr.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/function_template_builder.h"
#include "evita/v8_glue/runner.h"
#include "evita/v8_glue/wrapper_info.h"
#include "v8_strings.h"

namespace dom {
namespace internal {
const char* WindowStateString(dom::Window::State state) {
  static const char* const state_strings[] = {
    "destroyed",
    "destroying",
    "notrealized",
    "realizing",
    "realized",
  };
  auto const index = static_cast<size_t>(static_cast<int>(state) -
      static_cast<int>(dom::Window::State::Destroyed));
  DCHECK_LE(index, arraysize(state_strings) - 1);
  return state_strings[index];
}
}  // namespace internal
}  // namespace dom

namespace gin {

template<>
struct Converter<dom::Window::State> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
                                    dom::Window::State state) {
    return gin::StringToSymbol(isolate,
                               dom::internal::WindowStateString(state));
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

  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    auto templ = v8_glue::CreateConstructorTemplate(isolate,
        &WindowWrapperInfo::NewWindow);
    return v8_glue::FunctionTemplateBuilder(isolate, templ)
        .SetValue("focus", v8::Maybe<Window*>(false, nullptr))
        .Build();
  }

  private: static Window* NewWindow() {
    ScriptController::instance()->ThrowError(
        "You can't create Window instance.");
    return nullptr;
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("children", &Window::child_windows)
        .SetProperty("firstChild", &Window::first_child)
        .SetProperty("id", &Window::id)
        .SetProperty("lastChild", &Window::last_child)
        .SetProperty("nextSibling", &Window::next_sibling)
        .SetProperty("parent", &Window::parent_window)
        .SetProperty("previousSibling", &Window::previous_sibling)
        .SetProperty("state", &Window::state)
        .SetMethod("appendChild", &Window::AddWindow)
        .SetMethod("changeParent", &Window::ChangeParentWindow)
        .SetMethod("destroy", &Window::Destroy)
        .SetMethod("focus", &Window::Focus)
        .SetMethod("releaseCapture", &Window::ReleaseCapture)
        .SetMethod("realize", &Window::Realize)
        .SetMethod("removeChild", &Window::RemoveWindow)
        .SetMethod("setCapture", &Window::SetCapture)
        .SetMethod("splitHorizontally", &Window::SplitHorizontally)
        .SetMethod("splitVertically", &Window::SplitVertically);
  }

  DISALLOW_COPY_AND_ASSIGN(WindowWrapperInfo);
};

const v8::PropertyAttribute kDefaultPropertyAttribute =
    static_cast<v8::PropertyAttribute>(v8::DontEnum | v8::DontDelete);

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
    DCHECK_NE(State::Destroyed, window->state_);
    window->state_ = State::Destroyed;
    if (auto const parent = window->parent_node()) {
      parent->RemoveChild(window);
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

  DISALLOW_COPY_AND_ASSIGN(WindowIdMapper);
};

//////////////////////////////////////////////////////////////////////
//
// Window
//
DEFINE_SCRIPTABLE_OBJECT(Window, WindowWrapperInfo)

Window::Window()
    : state_(State::NotRealized) {
  WindowIdMapper::instance()->Register(this);
}

Window::~Window() {
  if (window_id() == kInvalidWindowId)
    return;
  WindowIdMapper::instance()->Unregister(window_id());
  ScriptController::instance()->view_delegate()->DestroyWindow(window_id());
}

std::vector<Window*> Window::child_windows() const {
  std::vector<Window*> child_windows;
  for (auto child : child_nodes()) {
    child_windows.push_back(const_cast<Window*>(child));
  }
  return child_windows;
}

void Window::AddWindow(Window* window) {
  if (window == this) {
    ScriptController::instance()->ThrowError(
        base::StringPrintf("Can't add window(%d) to itself.", id()));
    return;
  }
  if (window->parent_node()) {
    ScriptController::instance()->ThrowError(
        base::StringPrintf("Window(%d) is already child of window(%d).",
            window->id(), window->parent_node()->id()));
    return;
  }
  if (IsDescendantOf(window)) {
    ScriptController::instance()->ThrowError(
        base::StringPrintf("Window(%d) is parent or ancestor of window(%d).",
            window->id(), id()));
    return;
  }
  AppendChild(window);
  ScriptController::instance()->view_delegate()->AddWindow(
      window_id(), window->window_id());
}

void Window::ChangeParentWindow(Window* new_parent_window) {
  if (parent_node() == new_parent_window)
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
  if (parent_node())
    parent_node()->RemoveChild(this);
  new_parent_window->AppendChild(this);
  ScriptController::instance()->view_delegate()->ChangeParentWindow(
      window_id(), new_parent_window->window_id());
}

void Window::Destroy() {
  if (state_ == State::NotRealized)
    return;
  if (state_ != State::Realized && state_ != State::Realizing) {
    ScriptController::instance()->ThrowError(
        "You can't destroy unrealized window.");
    return;
  }
  for (auto descendant : common::tree::descendants_or_self(this)) {
    descendant->state_= State::Destroying;
  }
  ScriptController::instance()->view_delegate()->DestroyWindow(window_id());
}

void Window::DidDestroyWindow() {
  WindowIdMapper::instance()->DidDestroyWidget(window_id());
}

// Possible state transitions:
//  State::Realizing -> State::Realized 
//    |realize()| call.
//  State::NotRealized -> State::Realized
//    Adding |State::NotRealized| window to |State::Realized| window.
//  State::Destroying -> State::Realized
//    The window was |State::Realizing| then |destroy()|.
void Window::DidRealizeWindow() {
  DCHECK(state_ == State::Realizing ||
         state_ == State::Destroying ||
         state_ == State::NotRealized);
  state_ = State::Realized;
  for (auto child : child_nodes()) {
    if (child->state_ == State::NotRealized)
      child->state_ = State::Realized;
  }
}

void Window::DidRequestFocus() {
  ++global_focus_tick;

  // Update |Window.focus| and |Window.prototype.focusTick_|
  auto const runner = ScriptController::instance()->runner();
  auto const isolate = runner->isolate();
  v8_glue::Runner::Scope runner_scope(runner);
  auto const instance = GetWrapper(isolate);
  auto const klass = runner->global()->Get(v8Strings::Window.Get(isolate));
  instance->Set(v8Strings::focusTick_.Get(isolate),
                v8::Integer::New(isolate, global_focus_tick),
                kDefaultPropertyAttribute);
  klass->ToObject()->Set(v8Strings::focus.Get(isolate), instance,
                         kDefaultPropertyAttribute);
}

void Window::DidResize(int left, int top, int right, int bottom) {
  auto const runner = ScriptController::instance()->runner();
  auto const isolate = runner->isolate();
  v8_glue::Runner::Scope runner_scope(runner);
  auto const instance = GetWrapper(isolate);
  #define SET_PROP(name) \
    instance->Set(v8Strings::name.Get(isolate), \
                  v8::Integer::New(isolate, name), \
                  kDefaultPropertyAttribute)
  SET_PROP(left);
  SET_PROP(top);
  SET_PROP(right);
  SET_PROP(bottom);
}

void Window::Focus() {
  if (state_ != State::Realized && state_ != State::Realizing) {
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
  for (auto descendant : common::tree::descendants(other)) {
    if (descendant == this)
      return true;
  }
  return false;
}

void Window::Realize() {
  if (state_ == State::Destroyed) {
    DCHECK_EQ(kInvalidWindowId, window_id());
    ScriptController::instance()->ThrowError(
        "Can't realize deatched window.");
    return;
  }
  if (state_ == State::Realized) {
    ScriptController::instance()->ThrowError(
        "This window is already realized.");
    return;
  }
  if (state_ == State::Realizing) {
    ScriptController::instance()->ThrowError(
        "This window is being realized.");
    return;
  }
  if (parent_node() && parent_node()->state_ == State::NotRealized) {
    ScriptController::instance()->ThrowError(
        "Parent window isn't realized.");
    return;
  }
  for (auto descendant : common::tree::descendants_or_self(this)) {
    descendant->state_= State::Realizing;
  }
  ScriptController::instance()->view_delegate()->RealizeWindow(window_id());
}

void Window::RemoveWindow(Window* window) {
  if (window->parent_node() != window) {
    ScriptController::instance()->ThrowError(base::StringPrintf(
        "Can't remove window(%d) which isn't child of window(%d).",
        window->id(), id()));
    return;
  }
  parent_node()->RemoveChild(this);
}

void Window::ResetForTesting() {
  EventTarget::ResetForTesting();
  global_focus_tick = 0;
  WindowIdMapper::instance()->ResetForTesting();
}

static bool CheckSplitParameter(Window* ref_window, Window* new_window) {
  if (!ref_window->parent_node()) {
    ScriptController::instance()->ThrowError(
        "Can't split top-level window.");
    return false;
  }

  if (ref_window == new_window) {
    ScriptController::instance()->ThrowError(
        "Can't split window with itself.");
    return false;
  }

  if (ref_window->state() != Window::State::Realized) {
    ScriptController::instance()->ThrowError(
        "Can't split unrealized window.");
    return false;
  }

  if (new_window->parent_node()) {
    ScriptController::instance()->ThrowError(
        "Can't split with child window.");
    return false;
  }

  if (new_window->state() != Window::State::NotRealized) {
    ScriptController::instance()->ThrowError(
        "Can't split with realized window.");
    return false;
  }

  return true;
}

void Window::ReleaseCapture() {
  if (window_id() == kInvalidWindowId) {
    ScriptController::instance()->ThrowError(
        "Can't release capture to unralized window.");
    return;
  }
  ScriptController::instance()->view_delegate()->ReleaseCapture(window_id());
}

void Window::SetCapture() {
  if (state_ != State::Realized) {
    ScriptController::instance()->ThrowError(
        "Can't set capture to unralized window.");
    return;
  }
  ScriptController::instance()->view_delegate()->SetCapture(window_id());
}

void Window::SplitHorizontally(Window* new_right_window) {
  if (!CheckSplitParameter(this, new_right_window))
    return;
  parent_node()->InsertAfter(new_right_window, this);
  ScriptController::instance()->view_delegate()->SplitHorizontally(
    id(), new_right_window->id());
}

void Window::SplitVertically(Window* new_below_window) {
  if (!CheckSplitParameter(this, new_below_window))
    return;
  parent_node()->InsertAfter(new_below_window, this);
  ScriptController::instance()->view_delegate()->SplitVertically(
    id(), new_below_window->id());
}

}  // namespace dom
