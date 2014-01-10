// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#if !defined(INCLUDE_visual_CommandWindow_h)
#define INCLUDE_visual_CommandWindow_h

#include "widgets/widget.h"

namespace Command {
class KeyBindEntry;
}

//////////////////////////////////////////////////////////////////////
//
// CommandWindow
//
class CommandWindow : public widgets::Widget {
  protected: explicit CommandWindow(
      std::unique_ptr<widgets::NativeWindow>&& native_window)
      : widgets::Widget(std::move(native_window)) {
  }

  protected: explicit CommandWindow(view::WindowId window_id)
      : widgets::Widget(window_id) {
  }

  protected: CommandWindow() {
  }

  public: template<class T> T* DynamicCast() {
    return Is<T>() ? static_cast<T*>(this) : nullptr;
  }

  public: virtual bool IsPane() const { return false; }

  public: template<class T> bool Is() const {
    return T::Is_(this);
  }

  public: virtual Command::KeyBindEntry* MapKey(uint) = 0;

  DISALLOW_COPY_AND_ASSIGN(CommandWindow);
};

template<class T, class Parent_ = CommandWindow>
class CommandWindow_ : public Parent_  {
  protected: CommandWindow_(
      std::unique_ptr<widgets::NativeWindow>&& native_window)
    : Parent_(std::move(native_window)) {
  }

  protected: explicit CommandWindow_(view::WindowId window_id)
      : Parent_(window_id) {
  }

  protected: CommandWindow_() {
  }

  public: static bool Is_(const CommandWindow* p) {
    return T::GetClass_() == p->GetClass();
  }

  public: static T* FromHwnd(HWND const hwnd) {
    auto* const p = reinterpret_cast<CommandWindow*>(MapHwndToWindow(hwnd));
    return p ? p->DynamicCast<T>() : nullptr;
  }

  public: virtual const char* GetClass() const override {
    return T::GetClass_();
  }

  DISALLOW_COPY_AND_ASSIGN(CommandWindow_);
};

#endif //!defined(INCLUDE_visual_CommandWindow_h)
