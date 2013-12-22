// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_content_buffer_list_window_h)
#define INCLUDE_evita_content_buffer_list_window_h

#include "evita/content/content_window.h"

#include "evita/cm_CmdProc.h"

class Buffer;

namespace content {

class BufferListWindow
    : public CommandWindow_<BufferListWindow, ContentWindow> {
  DECLARE_CASTABLE_CLASS(BufferListWindow, ContentWindow);

  private: typedef CommandWindow_ ParentClass;

  private: enum Constant {
      ListViewId = 1234,
  };

  private: static HCURSOR sm_hDragCursor;
  private: static Command::KeyBinds* sm_pKeyBinds;

  private: Buffer* m_pDragItem;
  private: HWND m_hwndListView;

  // ctor/dtor
  public: BufferListWindow();

  // [A]
  private: void ActivateBuffers(bool);

  // [C]
  private: static int CALLBACK compareItems(LPARAM, LPARAM, LPARAM);
  protected: virtual void CreateNativeWindow() const override;

  // [D]
  private: virtual void DidChangeHierarchy() override;
  private: virtual void DidCreateNativeWindow() override;
  private: virtual void DidResize() override;
  private: virtual void DidSetFocus() override;
  private: void dragFinish(POINT);
  private: void dragMove(POINT);
  private: void dragStart(int);
  private: void dragStop();

  // [G]
  public: static const char* GetClass_() { return "BufferListWindow"; }

  public: HWND GetListWindow() const { return m_hwndListView; }

  private: virtual base::string16 GetTitle(size_t max_length) const override;

  // [L]
  public: virtual void Redraw() override;

  // [M]
  private: virtual void MakeSelectionVisible() override;
  public: virtual Command::KeyBindEntry* MapKey(uint) override;

  // [O]
  private: void onKeyDown(uint);
  private: virtual LRESULT OnMessage(uint, WPARAM, LPARAM) override;
  private: virtual void OnPaint(const base::win::Rect rect) override;

  // [U]
  private: virtual void UpdateStatusBar() const override;

  DISALLOW_COPY_AND_ASSIGN(BufferListWindow);
};

}  // namespace content

#endif //!defined(INCLUDE_evita_content_buffer_list_window_h)
