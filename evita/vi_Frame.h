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

#include "evita/li_util.h"
#include "evita/views/tab_strip_delegate.h"
#include "evita/views/window.h"

namespace gfx {
class Graphics;
using common::win::Rect;
}

/// <summary>
///   Sevirity of message.
/// </summary>
enum MessageLevel {
  MessageLevel_Information,
  MessageLevel_Warning,
  MessageLevel_Error,
};

class Pane;
class TextEditWindow;

namespace views {
class ContentWindow;
class MessageView;
class TitleBar;
class TabStrip;
}

/// <summary>
///   Represents a frame window aka toplevel window. This window communicates
///   with window manager.
/// </summary>
class Frame final : public views::Window,
                    public views::TabStripDelegate,
                    public DoubleLinkedNode_<Frame> {
  DECLARE_CASTABLE_CLASS(Frame, views::Window);

  private: typedef ui::Widget Widget;
  private: typedef ChildList_<Frame, Pane> Panes;

  private: std::unique_ptr<gfx::Graphics> gfx_;
  private: int m_cyTabBand;
  private: HWND m_hwndTabBand;
  private: Panes m_oPanes;
  private: std::unique_ptr<views::MessageView> message_view_;
  private: views::TabStrip* tab_strip_;
  private: std::unique_ptr<views::TitleBar> title_bar_;
  private: base::string16 tooltip_;
  private: Pane* m_pActivePane;

  public: explicit Frame(views::WindowId window_id);
  private: virtual ~Frame();

  // TODO: We should not use Frame to HWDN.
  public: operator HWND() const;

  public: gfx::Graphics& gfx() const { return *gfx_; }
  public: const Panes& panes() const { return m_oPanes; }
  public: Panes& panes() { return m_oPanes; }

  // [A]
  public: bool Activate();
  private: void AddPane(Pane*);
  private: void AddTab(Pane*);
  public: void AddWindow(views::ContentWindow*);

  // [C]
  private: virtual void CreateNativeWindow() const override;

  // [D]
  private: virtual void DidAddChildWidget(
      const Widget& widget) override;
  public: void DidActivatePane(Pane*);
  private: virtual void DidCreateNativeWindow() override;
  private: virtual void DidDestroyWidget() override;
  private: virtual void DidRemoveChildWidget(
      const Widget& widget) override;
  private: virtual void DidResize() override;
  private: virtual void DidSetFocus() override;
  public: void DidSetFocusOnChild(views::Window* window);

  // [G]
  public: Pane* GetActivePane();
  public: int GetCxStatusBar() const;

  public: Pane* GetFirstPane() const { return m_oPanes.GetFirst(); }
  public: Pane* GetLastPane() const { return m_oPanes.GetLast(); }
  private: Pane* getPaneFromTab(int) const;
  public: gfx::Rect GetPaneRect() const;
  private: int getTabFromPane(Pane*) const;

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
  public: void SetStatusBar(std::vector<base::string16> texts);
  public: void ShowMessage(MessageLevel, const base::string16& text) const;
  public: void ShowMessage(MessageLevel, uint32_t string_id, ...) const;

  // [U]
  private: void updateTitleBar();
  private: static void UpdateTitleBarTask(views::WindowId window_id);
  private: void UpdateTooltip(NMTTDISPINFO*);

  // [W]
  private: virtual void WillDestroyWidget() override;
  private: virtual void WillRemoveChildWidget(const Widget& widget) override;

  // views::TabStripDelegate
  private: virtual void DidChangeTabSelection(int new_selected_index) override;

  DISALLOW_COPY_AND_ASSIGN(Frame);
};

#endif //!defined(INCLUDE_visual_Frame_h)
