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
#ifndef EVITA_VI_FRAME_H_
#define EVITA_VI_FRAME_H_

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

  typedef views::FrameObserver FrameObserver;
  typedef views::MessageView MessageView;
  typedef views::TabContent TabContent;
  typedef views::TabStrip TabStrip;

 public:
  explicit Frame(views::WindowId window_id);

  void AddObserver(views::FrameObserver* observer);
  void AddOrActivateTabContent(views::ContentWindow*);
  void SetStatusBar(std::vector<base::string16> texts);
  void ShowMessage(MessageLevel, const base::string16& text) const;

 private:
  ~Frame() final;

  void AddTabContent(views::ContentWindow* content_window);
  void DrawForResize();
  TabContent* GetRecentTabContent();
  gfx::Rect GetTabContentBounds() const;
  TabContent* GetTabContentByTabIndex(int tab_index) const;
  void OnDropFiles(HDROP);

  int GetTabIndexOfTabContent(TabContent* tab_content) const;

  void UpdateTitleBar();

  // ui::AnimationFrameHandler
  void DidBeginAnimationFrame(base::Time time) final;

  // ui::Widget
  void CreateNativeWindow() const final;
  void DidAddChildWidget(ui::Widget* new_child) final;
  void DidRealize() final;
  void DidRemoveChildWidget(ui::Widget* old_child) final;
  void DidRequestDestroy() final;
  void DidChangeBounds() final;
  void DidSetFocus(ui::Widget* last_focused) final;
  LRESULT OnMessage(uint32_t, WPARAM, LPARAM) final;
  void OnPaint(const gfx::Rect paint_rect) final;
  void WillDestroyWidget() final;
  void WillRemoveChildWidget(ui::Widget* old_child) final;

  // views::TabContentObserver
  void DidActivateTabContent(TabContent* tab_content);

  // views::TabDataSet::Observer
  void DidSetTabData(dom::WindowId window_id,
                     const domapi::TabData& tab_data) final;

  // views::TabStripDelegate
  void DidDropTab(TabContent* tab_content,
                  const gfx::Point& screen_point) final;
  void DidSelectTab(int new_selected_index) final;
  void RequestCloseTab(int tab_index) final;
  void RequestSelectTab(int new_selected_index) final;

  MessageView* message_view_;
  base::ObserverList<FrameObserver> observers_;
  std::unordered_set<TabContent*> tab_contents_;
  std::unique_ptr<ui::Layer> tab_content_layer_;
  TabStrip* tab_strip_;
  const std::unique_ptr<views::TabStripAnimator> tab_strip_animator_;
  const std::unique_ptr<views::TitleBar> title_bar_;

  DISALLOW_COPY_AND_ASSIGN(Frame);
};

#endif  // EVITA_VI_FRAME_H_
