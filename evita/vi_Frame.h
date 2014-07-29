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

#pragma warning(push)
#pragma warning(disable: 4625 4626)
#include "base/observer_list.h"
#pragma warning(pop)
#include "evita/li_util.h"
#include "evita/views/tab_strip_delegate.h"
#include "evita/views/window.h"

namespace gfx {
class Canvas;
class DxDevice;
using common::win::Rect;
}

namespace ui {
class Layer;
}

/// <summary>
///   Severity of message.
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
class FrameObserver;
class MessageView;
class MetricsView;
class TitleBar;
class TabStrip;
}

/// <summary>
///   Represents a frame window aka top-level window. This window communicates
///   with window manager.
/// </summary>
class Frame final : public views::Window,
                    public views::TabStripDelegate {
  DECLARE_CASTABLE_CLASS(Frame, views::Window);

  private: typedef ui::Widget Widget;
  private: typedef ChildList_<Frame, Pane> Panes;

  private: std::unique_ptr<gfx::Canvas> canvas_;
  private: std::unique_ptr<ui::Layer> message_view_layer_;
  private: std::unique_ptr<ui::Layer> tab_strip_layer_;
  private: Panes m_oPanes;
  private: std::unique_ptr<views::MessageView> message_view_;
  private: views::MetricsView* metrics_view_;
  private: ObserverList<views::FrameObserver> observers_;
  private: gfx::Rect pending_update_rect_;
  private: views::TabStrip* tab_strip_;
  private: std::unique_ptr<views::TitleBar> title_bar_;
  private: Pane* m_pActivePane;

  public: explicit Frame(views::WindowId window_id);
  private: virtual ~Frame();

  // TODO: We should not use Frame to HWDN.
  public: operator HWND() const;

  public: gfx::Canvas* canvas() const { return canvas_.get(); }

  // [A]
  public: bool Activate();
  public: void AddObserver(views::FrameObserver* observer);
  private: void AddPane(views::ContentWindow* content_window);
  private: void AddTab(Pane*);
  public: void AddWindow(views::ContentWindow*);

  // [D]
  public: void DidActivatePane(Pane*);
  public: void DidSetFocusOnChild(views::Window* window);
  private: void DrawForResize();

  // [G]
  public: Pane* GetActivePane();
  private: Pane* getPaneFromTab(int) const;
  public: gfx::Rect GetPaneRect() const;
  private: int getTabFromPane(Pane*) const;

  // [F]
  public: static Frame* FindFrame(const Widget& widget);

  // [O]
  private: void onDropFiles(HDROP);

  // [S]
  public: void SetStatusBar(std::vector<base::string16> texts);
  public: void ShowMessage(MessageLevel, const base::string16& text) const;

  // [U]
  private: void updateTitleBar();

  // ui::Widget
  private: virtual void CreateNativeWindow() const override;
  private: virtual void DidAddChildWidget(const Widget& widget) override;
  private: virtual void DidCreateNativeWindow() override;
  private: virtual void DidRemoveChildWidget(const Widget& widget) override;
  private: virtual void DidChangeBounds() override;
  private: virtual void DidSetFocus(ui::Widget* last_focused) override;
  private: virtual LRESULT OnMessage(uint, WPARAM, LPARAM) override;
  private: virtual void OnPaint(const gfx::Rect paint_rect) override;
  private: virtual void WillDestroyWidget() override;
  private: virtual void WillRemoveChildWidget(const Widget& widget) override;

  // views::TabStripDelegate
  private: virtual void DidClickTabCloseButton(int tab_index) override;
  private: virtual void DidChangeTabSelection(int new_selected_index) override;
  private: virtual void DidThrowTab(LPARAM lParam);
  private: virtual base::string16 GetTooltipTextForTab(int tab_index) override;
  private: virtual void OnDropTab(LPARAM lParam);

  // views::Window
  public: virtual bool OnIdle(int hint) override;

  DISALLOW_COPY_AND_ASSIGN(Frame);
};

#endif //!defined(INCLUDE_visual_Frame_h)
