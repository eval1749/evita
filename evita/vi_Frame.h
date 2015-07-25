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

#include <windows.h>
#include <shellapi.h>

#include <unordered_set>
#include <vector>

#include "base/observer_list.h"
#include "evita/views/tabs/tab_content_observer.h"
#include "evita/views/tabs/tab_data_set.h"
#include "evita/views/tabs/tab_strip_delegate.h"
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

namespace views {
class ContentWindow;
class FrameObserver;
class MessageView;
class TabContent;
class TitleBar;
class TabStrip;
class TabStripAnimator;
}

/// <summary>
///   Represents a frame window aka top-level window. This window communicates
///   with window manager.
/// </summary>
class Frame final : public views::Window,
                    private views::TabContentObserver,
                    private views::TabDataSet::Observer,
                    public views::TabStripDelegate {
  DECLARE_CASTABLE_CLASS(Frame, views::Window);

  private: typedef views::FrameObserver FrameObserver;
  private: typedef views::MessageView MessageView;
  private: typedef views::TabContent TabContent;
  private: typedef views::TabStrip TabStrip;

  private: MessageView* message_view_;
  private: base::ObserverList<FrameObserver> observers_;
  private: std::unordered_set<TabContent*> tab_contents_;
  private: std::unique_ptr<ui::Layer> tab_content_layer_;
  private: TabStrip* tab_strip_;
  private: const std::unique_ptr<views::TabStripAnimator> tab_strip_animator_;
  private: const std::unique_ptr<views::TitleBar> title_bar_;

  public: explicit Frame(views::WindowId window_id);
  private: ~Frame() final;

  // [A]
  public: bool Activate();
  public: void AddObserver(views::FrameObserver* observer);
  private: void AddTabContent(views::ContentWindow* content_window);
  public: void AddOrActivateTabContent(views::ContentWindow*);

  // [D]
  private: void DrawForResize();

  // [G]
  private: TabContent* GetRecentTabContent();
  private: gfx::Rect GetTabContentBounds() const;
  private: TabContent* GetTabContentByTabIndex(int tab_index) const;
  private: int GetTabIndexOfTabContent(TabContent* tab_content) const;

  // [O]
  private: void OnDropFiles(HDROP);

  // [S]
  public: void SetStatusBar(std::vector<base::string16> texts);
  public: void ShowMessage(MessageLevel, const base::string16& text) const;

  // [U]
  private: void UpdateTitleBar();

  // ui::AnimationFrameHandler
  private: void DidBeginAnimationFrame(base::Time time) final;

  // ui::Widget
  private: void CreateNativeWindow() const final;
  private: void DidAddChildWidget(ui::Widget* new_child) final;
  private: void DidRealize() final;
  private: void DidRemoveChildWidget(ui::Widget* old_child) final;
  private: void DidRequestDestroy() final;
  private: void DidChangeBounds() final;
  private: void DidSetFocus(ui::Widget* last_focused) final;
  private: LRESULT OnMessage(uint32_t, WPARAM, LPARAM) final;
  private: void OnPaint(const gfx::Rect paint_rect) final;
  private: void WillDestroyWidget() final;
  private: void WillRemoveChildWidget(ui::Widget* old_child) final;

  // views::TabContentObserver
  private: void DidActivateTabContent(TabContent* tab_content);

  // views::TabDataSet::Observer
  private: void DidSetTabData(dom::WindowId window_id,
                                      const domapi::TabData& tab_data) final;

  // views::TabStripDelegate
  private: void DidDropTab(TabContent* tab_content,
                                   const gfx::Point& screen_point) final;
  private: void DidSelectTab(int new_selected_index) final;
  private: void RequestCloseTab(int tab_index) final;
  private: void RequestSelectTab(
      int new_selected_index) final;

  DISALLOW_COPY_AND_ASSIGN(Frame);
};

#endif //!defined(INCLUDE_visual_Frame_h)
