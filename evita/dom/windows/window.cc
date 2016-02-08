// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/windows/window.h"

#include "base/bind.h"
#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "common/tree/child_nodes.h"
#include "common/tree/descendants.h"
#include "evita/dom/bindings/exception_state.h"
#include "evita/dom/events/event.h"
#include "evita/dom/events/view_event_target_set.h"
#include "evita/dom/public/view_delegate.h"
#include "evita/dom/script_host.h"
#include "evita/dom/v8_strings.h"
#include "evita/dom/windows/window_ostream.h"
#include "evita/dom/windows/window_set.h"
#include "evita/gc/weak_ptr.h"
#include "evita/ginx/runner.h"

namespace dom {
namespace internal {
const char* WindowStateString(dom::Window::State state) {
  static const char* const state_strings[] = {
      "destroyed", "destroying", "notrealized", "realizing", "realized",
  };
  auto const index =
      static_cast<size_t>(static_cast<int>(state) -
                          static_cast<int>(dom::Window::State::Destroyed));
  DCHECK_LE(index, arraysize(state_strings) - 1);
  return state_strings[index];
}
}  // namespace internal
}  // namespace dom

namespace gin {

v8::Local<v8::Value> Converter<dom::Window::State>::ToV8(
    v8::Isolate* isolate,
    dom::Window::State state) {
  return gin::StringToSymbol(isolate, dom::internal::WindowStateString(state));
}

}  // namespace gin

namespace dom {

namespace {

const v8::PropertyAttribute kDefaultPropertyAttribute =
    static_cast<v8::PropertyAttribute>(v8::DontEnum | v8::DontDelete);

int global_focus_tick;
Window* global_focus_window;

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Window
//
Window::Window(ScriptHost* script_host)
    : script_host_(script_host), state_(State::NotRealized) {
  WindowSet::instance()->Register(this);
}

Window::~Window() {
  if (window_id() == domapi::kInvalidWindowId)
    return;
  WindowSet::instance()->Unregister(window_id());
  script_host_->view_delegate()->DestroyWindow(window_id());
}

std::vector<Window*> Window::child_windows() const {
  std::vector<Window*> child_windows;
  for (auto child : child_nodes()) {
    child_windows.push_back(const_cast<Window*>(child));
  }
  return child_windows;
}

void Window::AddWindow(Window* window, ExceptionState* exception_state) {
  if (window == this) {
    exception_state->ThrowError(
        base::StringPrintf("Can't add window(%d) to itself.", window_id()));
    return;
  }
  if (window->parent_node()) {
    exception_state->ThrowError(base::StringPrintf(
        "Window(%d) is already child of window(%d).", window->window_id(),
        window->parent_node()->window_id()));
    return;
  }
  if (IsDescendantOf(window)) {
    exception_state->ThrowError(
        base::StringPrintf("Window(%d) is parent or ancestor of window(%d).",
                           window->window_id(), window_id()));
    return;
  }
  AppendChild(window);
  script_host_->view_delegate()->AddWindow(window_id(), window->window_id());
}

void Window::ChangeParentWindow(Window* new_parent_window,
                                ExceptionState* exception_state) {
  if (parent_node() == new_parent_window)
    return;
  if (this == new_parent_window) {
    exception_state->ThrowError("Can't change parent to itself.");
    return;
  }
  if (new_parent_window->IsDescendantOf(this)) {
    exception_state->ThrowError(base::StringPrintf(
        "Can't change parent of window(%d) to window(%d), because window(%d)"
        " is descendant of window(%d).",
        window_id(), new_parent_window->window_id(),
        new_parent_window->window_id(), window_id()));
    return;
  }
  if (parent_node())
    parent_node()->RemoveChild(this);
  new_parent_window->AppendChild(this);
  script_host_->view_delegate()->ChangeParentWindow(
      window_id(), new_parent_window->window_id());
}

void Window::Destroy(ExceptionState* exception_state) {
  if (state_ == State::NotRealized)
    return;
  if (state_ != State::Realized && state_ != State::Realizing) {
    exception_state->ThrowError("You can't destroy unrealized window.");
    return;
  }
  for (auto descendant : common::tree::descendants_or_self(this)) {
    descendant->state_ = State::Destroying;
  }
  script_host_->view_delegate()->DestroyWindow(window_id());
}

void Window::DidActivateWindow() {
  // Nothing to do.
}

void Window::DidDestroyWindow() {
  if (global_focus_window == this)
    global_focus_window = nullptr;
  WindowSet::instance()->DidDestroyWindow(window_id());
}

// Possible state transitions:
//  State::Realizing -> State::Realized
//    |realize()| call.
//  State::NotRealized -> State::Realized
//    Adding |State::NotRealized| window to |State::Realized| window.
//  State::Destroying -> State::Realized
//    The window was |State::Realizing| then |destroy()|.
void Window::DidRealizeWindow() {
  DCHECK(state_ == State::Realizing || state_ == State::Destroying ||
         state_ == State::NotRealized);
  state_ = State::Realized;
  for (auto child : child_nodes()) {
    if (child->state_ == State::NotRealized)
      child->state_ = State::Realized;
  }
}

void Window::DidChangeBounds(int clientLeft,
                             int clientTop,
                             int clientRight,
                             int clientBottom) {
  const auto& new_bounds = FloatRect(FloatPoint(clientLeft, clientTop),
                                     FloatPoint(clientRight, clientBottom));
  if (bounds_ == new_bounds)
    return;
  bounds_ = new_bounds;
  DidChangeBounds();
}

void Window::DidChangeBounds() {}

void Window::DidHideWindow() {
  visible_ = false;
}

void Window::DidKillFocus() {
  if (global_focus_window == this)
    global_focus_window = nullptr;
}

void Window::DidSetFocus() {
  ++global_focus_tick;
  global_focus_window = this;

  // Update |Window.focus| and |Window.prototype.focusTick_|
  auto const runner = script_host_->runner();
  auto const isolate = runner->isolate();
  ginx::Runner::Scope runner_scope(runner);
  auto const instance = GetWrapper(isolate);
  auto const klass = runner->global()->Get(v8Strings::Window.Get(isolate));
  instance->ForceSet(v8Strings::focusTick_.Get(isolate),
                     v8::Integer::New(isolate, global_focus_tick),
                     kDefaultPropertyAttribute);
  klass->ToObject()->ForceSet(v8Strings::focus.Get(isolate), instance,
                              kDefaultPropertyAttribute);
}

void Window::DidShowWindow() {
  visible_ = true;
}

void Window::ForceUpdateWindow() {}

void Window::Focus(ExceptionState* exception_state) {
  if (state_ != State::Realized && state_ != State::Realizing) {
    exception_state->ThrowError("You can't focus unrealized window.");
    return;
  }
  script_host_->view_delegate()->FocusWindow(window_id());
}

// static
Window* Window::GetFocusWindow() {
  return global_focus_window;
}

void Window::Hide(ExceptionState* exception_state) {
  if (state_ != State::Realized && state_ != State::Realizing) {
    exception_state->ThrowError("You can't hide unrealized window.");
    return;
  }
  script_host_->view_delegate()->HideWindow(window_id());
}

bool Window::IsDescendantOf(Window* other) const {
  for (auto descendant : common::tree::descendants(other)) {
    if (descendant == this)
      return true;
  }
  return false;
}

void Window::Realize(ExceptionState* exception_state) {
  if (state_ == State::Destroyed) {
    DCHECK_EQ(domapi::kInvalidWindowId, window_id());
    exception_state->ThrowError("Can't realize deatched window.");
    return;
  }
  if (state_ == State::Realized) {
    exception_state->ThrowError("This window is already realized.");
    return;
  }
  if (state_ == State::Realizing) {
    exception_state->ThrowError("This window is being realized.");
    return;
  }
  if (parent_node() && parent_node()->state_ == State::NotRealized) {
    exception_state->ThrowError("Parent window isn't realized.");
    return;
  }
  for (auto descendant : common::tree::descendants_or_self(this)) {
    descendant->state_ = State::Realizing;
  }
  script_host_->view_delegate()->RealizeWindow(window_id());
}

void Window::RemoveWindow(Window* window, ExceptionState* exception_state) {
  if (window->parent_node() != window) {
    exception_state->ThrowError(base::StringPrintf(
        "Can't remove window(%d) which isn't child of window(%d).",
        window->window_id(), window_id()));
    return;
  }
  parent_node()->RemoveChild(this);
}

void Window::ResetForTesting() {
  ViewEventTargetSet::instance()->ResetForTesting();
  global_focus_tick = 0;
  global_focus_window = nullptr;
  WindowSet::instance()->ResetForTesting();
}

static bool CheckSplitParameter(Window* ref_window,
                                Window* new_window,
                                ExceptionState* exception_state) {
  if (!ref_window->parent_node()) {
    exception_state->ThrowError("Can't split top-level window.");
    return false;
  }

  if (ref_window == new_window) {
    exception_state->ThrowError("Can't split window with itself.");
    return false;
  }

  if (ref_window->state() != Window::State::Realized) {
    exception_state->ThrowError("Can't split unrealized window.");
    return false;
  }

  if (new_window->parent_node()) {
    exception_state->ThrowError("Can't split with child window.");
    return false;
  }

  if (new_window->state() != Window::State::NotRealized) {
    exception_state->ThrowError("Can't split with realized window.");
    return false;
  }

  return true;
}

void Window::ReleaseCapture(ExceptionState* exception_state) {
  if (window_id() == domapi::kInvalidWindowId) {
    exception_state->ThrowError("Can't release capture to unralized window.");
    return;
  }
  ViewEventTarget::ReleaseCapture();
}

void Window::SetCapture(ExceptionState* exception_state) {
  if (state_ != State::Realized) {
    exception_state->ThrowError("Can't set capture to unralized window.");
    return;
  }
  ViewEventTarget::SetCapture();
}

void Window::Show(ExceptionState* exception_state) {
  if (state_ != State::Realized && state_ != State::Realizing) {
    exception_state->ThrowError("You can't show unrealized window.");
    return;
  }
  script_host_->view_delegate()->ShowWindow(window_id());
}

void Window::SplitHorizontally(Window* new_right_window,
                               ExceptionState* exception_state) {
  if (!CheckSplitParameter(this, new_right_window, exception_state))
    return;
  parent_node()->InsertAfter(new_right_window, this);
  script_host_->view_delegate()->SplitHorizontally(
      window_id(), new_right_window->window_id());
}

void Window::SplitVertically(Window* new_below_window,
                             ExceptionState* exception_state) {
  if (!CheckSplitParameter(this, new_below_window, exception_state))
    return;
  parent_node()->InsertAfter(new_below_window, this);
  script_host_->view_delegate()->SplitVertically(window_id(),
                                                 new_below_window->window_id());
}

void Window::Update(ExceptionState* exception_state) {
  if (state_ != State::Realized && state_ != State::Realizing) {
    exception_state->ThrowError("You can't update unrealized window.");
    return;
  }
  ForceUpdateWindow();
}

}  // namespace dom
