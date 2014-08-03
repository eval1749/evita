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

#include <unordered_set>
#include <vector>

#pragma warning(push)
#pragma warning(disable: 4625 4626)
#include "base/observer_list.h"
#pragma warning(pop)
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

  private: class DragController;
  friend class DragController;

  private: Pane* active_tab_content_;
  private: std::unique_ptr<DragController> drag_controller_;
  private: views::MessageView* message_view_;
  private: views::MetricsView* metrics_view_;
  private: ObserverList<views::FrameObserver> observers_;
  private: gfx::Rect pending_update_rect_;
  private: std::unordered_set<Pane*> tab_contents_;
  private: std::unique_ptr<ui::Layer> tab_content_layer_;
  private: views::TabStrip* tab_strip_;
  private: std::unique_ptr<views::TitleBar> title_bar_;

  public: explicit Frame(views::WindowId window_id);
  private: virtual ~Frame();

  // [A]
  public: bool Activate();
  public: void AddObserver(views::FrameObserver* observer);
  private: void AddTab(Pane* tab_content);
  private: void AddTabContent(views::ContentWindow* content_window);
  public: void AddOrActivateTabContent(views::ContentWindow*);

  // [D]
  public: void DidSetFocusOnChild(views::Window* window);
  private: void DrawForResize();

  // [G]
  private: Pane* GetActiveTabContent();
  private: gfx::Rect GetTabContentBounds() const;
  private: Pane* GetTabContentByTabIndex(int tab_index) const;
  private: int GetTabIndexOfTabContent(Pane* tab_content) const;

  // [O]
  private: void OnDropFiles(HDROP);

  // [S]
  public: void SetStatusBar(std::vector<base::string16> texts);
  public: void ShowMessage(MessageLevel, const base::string16& text) const;

  // [U]
  private: void UpdateTitleBar();

  // ui::Widget
  private: virtual void CreateNativeWindow() const override;
  private: virtual void DidAddChildWidget(const ui::Widget& widget) override;
  private: virtual void DidChangeChildVisibility(ui::Widget* child) override;
  private: virtual void DidRealize() override;
  private: virtual void DidRemoveChildWidget(const ui::Widget& widget) override;
  private: virtual void DidChangeBounds() override;
  private: virtual void DidSetFocus(ui::Widget* last_focused) override;
  private: virtual LRESULT OnMessage(uint, WPARAM, LPARAM) override;
  private: virtual void OnMouseMoved(const ui::MouseEvent& event) override;
  private: virtual void OnMousePressed(const ui::MouseEvent& event) override;
  private: virtual void OnMouseReleased(const ui::MouseEvent& event) override;
  private: virtual void OnPaint(const gfx::Rect paint_rect) override;
  private: virtual void WillDestroyWidget() override;
  private: virtual void WillRemoveChildWidget(
      const ui::Widget& widget) override;

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
