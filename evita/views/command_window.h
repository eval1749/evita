// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#if !defined(INCLUDE_evita_views_command_window_h)
#define INCLUDE_evita_views_command_window_h

#include "evita/cm_CmdProc.h"
#include "evita/views/window.h"

//////////////////////////////////////////////////////////////////////
//
// CommandWindow
//
class CommandWindow : public views::Window {
  protected: explicit CommandWindow(
      std::unique_ptr<widgets::NativeWindow>&& native_window);
  protected: explicit CommandWindow(views::WindowId window_id);
  protected: virtual ~CommandWindow();

  public: static void BindKey(int key_code,
      const common::scoped_refptr<Command::KeyBindEntry>& entry);
  public: static void BindKey(uint32 key_code,
      Command::Command::CommandFn function);

  public: template<class T> T* DynamicCast() {
    return Is<T>() ? static_cast<T*>(this) : nullptr;
  }

  public: virtual bool IsPane() const { return false; }

  public: template<class T> bool Is() const {
    return T::Is_(this);
  }

  public: virtual Command::KeyBindEntry* MapKey(uint key_code);

  // widgets::Widget
  protected: void virtual DidSetFocus() override;

  DISALLOW_COPY_AND_ASSIGN(CommandWindow);
};

template<class T, class Parent_ = CommandWindow>
class CommandWindow_ : public Parent_  {
  protected: CommandWindow_(
      std::unique_ptr<widgets::NativeWindow>&& native_window)
    : Parent_(std::move(native_window)) {
  }

  protected: explicit CommandWindow_(views::WindowId window_id)
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

#endif //!defined(INCLUDE_evita_views_command_window_h)
