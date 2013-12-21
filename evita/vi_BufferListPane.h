//////////////////////////////////////////////////////////////////////////////
//
// Editor - Buffer List Pane
// listener/winapp/vi_Buffer.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_BufferListPane.h#1 $
//
#if !defined(INCLUDE_visual_BufferListPane_h)
#define INCLUDE_visual_BufferListPane_h

#include "./vi_Pane.h"

#include "./cm_CmdProc.h"

class Buffer;

class BufferListPane : public CommandWindow_<BufferListPane, Pane> {
  DECLARE_CASTABLE_CLASS(BufferListPane, Pane);

  private: enum Constant {
      ListViewId = 1234,
  };

  private: static HCURSOR sm_hDragCursor;
  private: static Command::KeyBinds* sm_pKeyBinds;

  private: Buffer* m_pDragItem;
  private: HWND m_hwndListView;

  // ctor/dtor
  public: BufferListPane();

  // [A]
  public: virtual void Activate() override;
  private: void ActivateBuffers(bool);

  // [C]
  private: static int CALLBACK compareItems(LPARAM, LPARAM, LPARAM);
  protected: virtual void CreateNaitiveWindow() const override;

  // [D]
  private: virtual void DidCreateNaitiveWindow() override;
  private: virtual void DidResize() override;
  private: void dragFinish(POINT);
  private: void dragMove(POINT);
  private: void dragStart(int);
  private: void dragStop();

  // [G]
  public: static const char* GetClass_() { return "BufferListPane"; }

  public: HWND GetListWindow() const { return m_hwndListView; }

  public: virtual int GetTitle(char16*, int) override;

  // [L]
  public: void Refresh();

  // [M]
  public: virtual Command::KeyBindEntry* MapKey(uint) override;

  // [O]
  private: void onKeyDown(uint);
  private: virtual LRESULT OnMessage(uint, WPARAM, LPARAM) override;
  private: virtual void OnPaint(const base::win::Rect rect) override;

  DISALLOW_COPY_AND_ASSIGN(BufferListPane);
};

#endif //!defined(INCLUDE_visual_BufferListPane_h)
