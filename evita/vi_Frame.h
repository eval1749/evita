//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - Frame Window
// listener/winapp/vi_Frame.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_Frame.h#2 $
//
#if !defined(INCLUDE_visual_Frame_h)
#define INCLUDE_visual_Frame_h

#include <vector>

#include "evita/ctrl_StatusBar.h"
#include "evita/ctrl_TabBand.h"
#include "evita/ctrl_TitleBar.h"
#include "evita/li_util.h"
#include "evita/views/window.h"

namespace gfx {
class Graphics;
using common::win::Rect;
}

/// <summary>
///   Sevirity of message.
/// </summary>
enum MessageLevel
{
    MessageLevel_Min,

    MessageLevel_Idle = MessageLevel_Min,
    MessageLevel_Information,
    MessageLevel_Warning,
    MessageLevel_Error,

    MessageLevel_Limit,
}; // MessageLevel

class Pane;
class TextEditWindow;

namespace views {
class ContentWindow;
}

namespace dom {
class Buffer;
}

using Buffer = dom::Buffer;

/// <summary>
///   Represents a frame window aka toplevel window. This window communicates
///   with window manager.
/// </summary>
class Frame final : public views::Window,
                    public DoubleLinkedNode_<Frame> {
  private: typedef ui::Widget Widget;
  private: typedef ui::Widget Widget;
  DECLARE_CASTABLE_CLASS(Frame, views::Window);

  private: enum CtrlId {
    CtrlId_TabBand  = 1,
    CtrlId_StatusBar,
  };

  private: typedef ChildList_<Frame, Pane> Panes;
  private: common::OwnPtr<gfx::Graphics> gfx_;
  private: int m_cyTabBand;
  private: HWND m_hwndTabBand;
  private: Panes m_oPanes;
  private: ui::StatusBar m_oStatusBar;
  private: TitleBar m_oTitleBar;
  private: Pane* m_pActivePane;
  private: char16* m_rgpwszMessage[MessageLevel_Limit];
  private: mutable char16 m_wszToolTip[1024];

  public: explicit Frame(views::WindowId window_id);
  // TODO(yosi) We should remove no parameter Frame ctor.
  public: Frame();
  private: virtual ~Frame();

  // TODO: We should not use Frame to HWDN.
  public: operator HWND() const;

  public: gfx::Graphics& gfx() const { return *gfx_; }
  public: const Panes& panes() const { return m_oPanes; }
  public: Panes& panes() { return m_oPanes; }

  // [A]
  public: bool  Activate();
  private: void AddPane(Pane*);
  private: void AddTab(Pane*);
  public: TextEditWindow* AddWindow(Buffer* buffer);
  public: void AddWindow(views::ContentWindow*);

  // [C]
  private: virtual void CreateNativeWindow() const override;

  // [D]
  private: virtual void DidAddChildWidget(
      const Widget& widget) override;
  public: void DidActivatePane(Pane*);
  private: void DidChangeTabSelection(int selected_index);
  private: virtual void DidCreateNativeWindow() override;
  private: virtual void DidDestroyWidget() override;
  private: virtual void DidRemoveChildWidget(
      const Widget& widget) override;
  private: virtual void DidResize() override;
  private: virtual void DidSetFocus() override;
  public: void DidSetFocusOnChild(views::Window* window);

  // [G]
  public: Pane* GetActivePane();
  public: virtual const char* GetClass() const override {
    return class_name();
  }
  public: int GetCxStatusBar() const;

  public: Pane* GetFirstPane() const { return m_oPanes.GetFirst(); }
  public: Pane* GetLastPane() const { return m_oPanes.GetLast(); }
  private: Pane* getPaneFromTab(int) const;
  public: gfx::Rect GetPaneRect() const;
  private: int getTabFromPane(Pane*) const;
  private: const char16* getToolTip(NMTTDISPINFO*) const;

  // [F]
  public: static Frame* FindFrame(const Widget& widget);

  // [H]
  private: bool hasFocus() const;

  public: bool HasMultiplePanes() const {
    return GetFirstPane() != GetLastPane();
  }

  // [O]
  private: void onDropFiles(HDROP);
  public: virtual bool OnIdle(uint) override;
  private: virtual LRESULT OnMessage(uint, WPARAM, LPARAM) override;
  private: virtual void OnPaint(const gfx::Rect paint_rect) override;
  private: bool onTabDrag(TabBandDragAndDrop, HWND);

  // [P]
  private: void Paint();

  // [R]
  public: void Realize();
  public: void virtual RealizeWidget() override;
  public: void ResetMessages();

  // [S]
  public: void SetStatusBar(int part, const base::string16& text);
  public: void SetStatusBar(std::vector<base::string16> texts);
  public: bool ShowBuffer(Buffer*);
  public: void ShowMessage(MessageLevel, uint = 0, ...);

  // [U]
  private: void updateTitleBar();
  private: static void UpdateTitleBarTask(views::WindowId window_id);

  // [W]
  private: virtual void WillDestroyWidget() override;
  private: virtual void WillRemoveChildWidget(const Widget& widget) override;

  DISALLOW_COPY_AND_ASSIGN(Frame);
};

#endif //!defined(INCLUDE_visual_Frame_h)
